#include "sterta.h"
#include <pthread.h>
#include "custom_unistd.c"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

struct heap_control pages = {0, NULL, NULL, 0,0,0};

//Funkcja heap_setup inicjuje (organizuje) stertę w obszarze przeznaczonej do tego pamięci.
int heap_setup(void)
{
    pthread_mutex_lock(&mutex);
    //Wywołanie funckji powoduje rezerwacje miejsca i wskaże na początek tego miejsca PAGE_SIZE = 4096
    intptr_t start = (intptr_t)custom_sbrk(PAGE_SIZE);

    //Drugie wywołanie funckji powoduje zwrócenie wskaźnika na początku nowego bloku co za tym idzie zaraz za wczesniej zarezerwowanym
    intptr_t end = (intptr_t)custom_sbrk(0);

    //Sprawdzenie czy sterta została poprawnie stworzona, custom_sbrk może zwrócić -1
    if((void*)start == (void*)-1)
    {
        pthread_mutex_unlock(&mutex);
        return -1;
    }

    //Ustawienie płotków kontrolnych z obu stron
    pages.left_control_fence = 0;
    pages.right_control_fence = 0;

    //Ustawienie początku i końca sterty start wskazuje na początek starty,
    //natomiast tail wskazuje na koniec pomniejszony o rozmiar struktury bloku pamięci
    //(wskazuje na koniec wolnego miejsca w pamięci)
    pages.head = (struct heap_block*)start;
    pages.tail = (struct heap_block*)((void*)end-sizeof(struct heap_block));

    //Ustawienie wszystkich danych w strukturze bloku pamięci dla głowy sterty(płotki wskaźniki na prev, rozmiar, linia programu oraz nazwa programu)
    pages.head->left_control_fence = 0;
    pages.head->prev = NULL;
    pages.head->block_size = 0;
    pages.head->code_line = 0;
    pages.head->program_name = NULL;
    pages.head->right_control_fence = 0;

    //Ustawienie wszystkich danych w strukturze bloku pamięci dla ogona sterty(płotki wskaźniki na next, rozmiar, linia programu oraz nazwa programu)
    pages.tail->left_control_fence = 0;
    pages.tail->next = NULL;
    pages.tail->block_size = 0;
    pages.tail->code_line = 0;
    pages.tail->program_name = NULL;
    pages.tail->right_control_fence = 0;

    //Utworzenie pierwsze bloku pamięci na całą wielkość stery który następnie będziemy dzielić w zależności od rozmiaru pamięci jak będziemy chcieli przydzielić
    //Zwróci wskaźnik przesunięty od początku sterty o rozmiar struktury bloku pamięci oraz ustawi dla bloku stery head->next oraz tail->prev
    struct heap_block *full = (struct heap_block*)((void*)start+sizeof(struct heap_block));
    pages.head->next = full;
    pages.tail->prev = full;

    //Ustawienie wszystkich danych w nowym bloku pamięci(płotki, prev oraz next, rozmiar, linie programu oraz nazwe programu)
    full->left_control_fence = 0;
    full->prev = pages.head;
    full->next = pages.tail;
    full->code_line = 0;
    full->program_name = NULL;
    full->right_control_fence = 0;
    //Rozmiar zostanie ustawiony na ujemny ponieważ oznacza wolną przestrzeń pamięci którą można zarezerwować
    //Zmniejszamy i 3 rozmiary sktruktury (head + tail + nowy blok pamięci)
    full->block_size = -(int)(end-start-3*sizeof(struct heap_block));

    pages.init = 1;

    //Obliczenie nowej sumy kotrolnej która ma wskazywać na zmiany zachodzące na stercie lub w bloku pamięci,
    //jakakolwiek operacja na nich zawsze ma być odnotowana w postaci sumy kontrolnej
    pages.head->check_sum = check_sum_function(pages.head);
    pages.tail->check_sum = check_sum_function(pages.tail);
    full->check_sum = check_sum_function(full);
    pages.check_sum = check_sum_for_control();

    pthread_mutex_unlock(&mutex);
    return 0;
}

void * heap_malloc(size_t count)
{
    pthread_mutex_lock(&mutex);
    //Sprawdzenie czy wartość nie jest zerem oraz czy sterta jest stworzona, zwraca NULL jeśli coś jest nie tak
    if(count <= 0 || pages.init == 0)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    //Funkcja zaokrągla do słowa maszynowego dla ułatwienia pracy maszynie
    size_t new_count = add_to_word(count);

    //Sprawdzenie czy funkcja nie zaalokuje więcej miejsca niż jest to możliwe
    //Wartość są spisane w pliku custom_unistd.c
    if(new_count > PAGE_SIZE * PAGES_AVAILABLE)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    if(heap_validate() != 0)
    {
        return NULL;
    }

    //Sprawdzenie czy na stercie jest miejsce na nowy blok pamięci o podanej wielkości
    while(check_heap_size(new_count) != 1)
    {
        //jeśli miejsca nie dodawana jest nowa strona o rozmiarze PAGE_SIZE(4096),
        //ponownie sprawdzanie jest czy tym razem miejsca starczy
        if(add_new_page() == -1)
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
    }

    struct heap_block *check = pages.head;

    //Przejście po stercie w poszukiwaniu tego miejsca gdzie można zaalokowac pamięć
    while(check != NULL)
    {
        if(check->block_size < 0)
        {
            if(-(check->block_size) >= (new_count + sizeof(void*) + sizeof(struct heap_block)) || -(check->block_size) == new_count)
            {
                break;
            }
        }
        check = check->next;
    }


    //Jeśli przeszliśmy całe a coś nie zadziałało to zwróci NULL
    if(check == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    //Jeśli rozmiar bloku pamięci jest inny niż podany rozmiara idź dalej
    if(-check->block_size != new_count)
    {
        //Stworzenie nowej struktury do wcześniej znalezionego miejsca dodajemy rozmiar bloku i
        //podany rozmiar da nam to wskaźnik na koniec bloku i jednocześniej początek miejsca gdzie można wstawić kolejny
        struct heap_block *create_memory = (struct heap_block*)((void*)check+sizeof(struct heap_block)+new_count);

        //Przypisanie rozmiaru strukturze zmniejszając wolne miesce o rozmiar struktury i podana wartość (rozmiar jest ujemny bo jest wolny)
        create_memory->block_size = (int)(check->block_size+sizeof(struct heap_block)+new_count);
        check->block_size = new_count;

        //Jeśli next(nastepny blok) bloku który został wyznaczony nie jest ogonem starty to idź dalej
        if(check->next != pages.tail)
        {
            //Przepisanie odpowiednio wskaźników do next i prev bloków na około nowego(jak w liście dwukierunkowej)
            //Obliczenie na nowo sumy kontrolnej dla check->next
            struct heap_block *temp = check->next;
            temp->prev = create_memory;
            create_memory->next = check->next;
            create_memory->prev = check;
            check->next = create_memory;
            temp->check_sum = check_sum_function(temp);
        }
            //Jeśli następny blok to ogon sterty idź dalej
        else
        {
            //Przepisanie odpowiedno wskaźników do next i prev bloków jakby blok był przed ogonem
            pages.tail->prev = create_memory;
            check->next = create_memory;
            create_memory->prev = check;
            create_memory->next = pages.tail;
        }

        //Dopisanie wartości płotków, linii programu oraz nazwy programu
        create_memory->left_control_fence = 0;
        create_memory->code_line = 0;
        create_memory->program_name = NULL;
        create_memory->right_control_fence = 0;

        check->left_control_fence = 0;
        check->code_line = 0;
        check->program_name = NULL;
        check->right_control_fence = 0;

        //Obliczenie nowych sum kontrolnych dla wszystkich zmodyfikowanych elementów
        check->check_sum = check_sum_function(check);
        create_memory->check_sum = check_sum_function(create_memory);
        pages.tail->check_sum = check_sum_function(pages.tail);
        pages.check_sum = check_sum_for_control();

        //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
        pthread_mutex_unlock(&mutex);
        return (void*)check+sizeof(struct heap_block);
    }
        //Jeśli wskaźnik był juz wcześniej i miejsca jest równa ilość idż dalej
    else
    {
        //Przypisuje nowy rozmiar, oblicza sume kontrolną i zwraca wskaźnik na pierwsze miejsce do zapisu danych
        check->block_size = new_count;
        check->check_sum = check_sum_function(check);
        pthread_mutex_unlock(&mutex);
        return (void*)check + sizeof(struct heap_block);
    }
}


void * heap_calloc(size_t number, size_t size)
{
    pthread_mutex_lock(&mutex);
    //Sprawdzenie czy podane liczby są większe od zera
    if (number <= 0 || size <= 0)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    //Za pomocą funkcji malloc tworzy nowy blok(lub używa starego) i zwraca wskaźnik na początek miejsca gdzie można zapisać dane
    pthread_mutex_unlock(&mutex);
    void *new_memory = heap_malloc(size * number);
    pthread_mutex_lock(&mutex);
    //Jeśli udało się zaalokowac miejsce to zerowane są wszystkie pola
    if (new_memory != NULL)
    {
        //Jeśli memset wyzerował wszystkie pola od wskaźnika new_memory do rozmiaru size*number(czyli nie zwrócił NULL)
        //to zwraca wskaźnika na początek wyzerowanych pól
        if(memset(new_memory, 0, size * number) != NULL)
        {
            pthread_mutex_unlock(&mutex);
            return new_memory;
        }
            //Jeśli coś poszło nie tak zwraca NULL
        else
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
    }
        //Zwraca NULL jeśli malloc sie nie udał
    else
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
}


void heap_free(void * memblock)
{
    pthread_mutex_lock(&mutex);
    //Sprawdzenie czy wskaźnik jest w ogóle poprawny i jeśli tak przechodzi dalej
    if(get_pointer_type(memblock) == pointer_valid)
    {
        //Ustawienie wskaźnika na początek bloku - rozmiar struktury
        struct heap_block *temp = (struct heap_block*)(void*)(heap_get_data_block_start(memblock)-sizeof(struct heap_block));

        //Zamiana rozmiaru z dodatniego na ujemny który oznacza wolne miejsce
        temp->block_size = -temp->block_size;

        //Podwójne wykonanie łączenia wolnych bloków zapewnia bezpieczeństwo i
        //eliminuje przypadek gdzie po zwolnieniu będziemy mieli obok siebie 3 wolne bloki
        connect_memory();
        connect_memory();

        //Sprawdzenie ile bloków jest zaalokowanych jeśli 0 idź dalej
        if(heap_get_used_blocks_count() == 0)
        {

            while(1)
            {
                //Obliczanie rozmiaru aktualnej starty
                size_t sum = 0;
                struct heap_block *add = pages.head;

                //Przechodzenie po wszystkich blokach i dodawanie obszarów pamięci
                while(add != NULL)
                {
                    sum += abs(add->block_size) + sizeof(struct heap_block);
                    add = add->next;
                }

                //Jeśli rozmiar jest równy PAGE_SIZE(4096) czyli rozmiarowi jednej strony przerywa pętle,
                //ilość stron nie może być mniejszy
                if(sum == PAGE_SIZE)
                {
                    break;
                }

                //Jeśli stron jest więcej i nie ma na nich nic to zostają one usunięte
                //Stera zostaje zmniejszona do możliwie jak najmniejszych rozmiarów
                if (delete_page() != 0)
                {
                    pthread_mutex_unlock(&mutex);
                    return;
                }
            }
        }

        //Dla pewności pętla przechodzi przez wszystkie bloki pamięci i na nowo oblicza sumy kontrolne
        temp = pages.head;
        while(temp != NULL)
        {
            temp->check_sum = check_sum_function(temp);
            temp = temp->next;
        }
    }
    pthread_mutex_unlock(&mutex);
}


void * heap_realloc(void * memblock, size_t size)
{
    pthread_mutex_lock(&mutex);
    //Sprawdzenie czy wskazany blok istnieje, jeśli nie to wykonywana jest funckja malloc
    if(memblock == NULL)
    {   
        pthread_mutex_unlock(&mutex);
        return heap_malloc(size);
    }

    //Sprawdzenie czy wskaźnik podany w argumencie jest poprawny, jeśli nie zwraca NULL
    if(get_pointer_type(memblock) != pointer_valid)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    //Funkcja zaokrągla do słowa maszynowego dla ułatwienia pracy maszynie
    size_t new_count = add_to_word(size);

    //Sprawdzenie czy funkcja nie zaalokuje więcej miejsca niż jest to możliwe
    //Wartość są spisane w pliku custom_unistd.c
    if(new_count > PAGE_SIZE * PAGES_AVAILABLE || size <= 0)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    //Ustawia wskaźnik na początek bloku pamięci(początek struktury)
    struct heap_block *temp = (struct heap_block *)(void*)(memblock - sizeof(struct heap_block));

    int memory_size_correct = 0;

    //Sprawdzenie czy można dokonać operacji rozszerzenie bloku,
    //jeśli podany rozmiar jest większy to idź dalej
    if(new_count > temp->block_size)
    {
        //Jeśli następny blok ma ujemny rozmiar(jest wolny) i w połaczeniu z rozmiarem poprzedniego
        //pomieści nową alokacje to ustaw zmienna na 1
        if(temp->next->block_size < 0 && -temp->next->block_size >= new_count - temp->block_size)
        {
            memory_size_correct = 1;
        }

        if(temp->next->block_size < 0 && -temp->next->block_size >= temp->block_size - new_count)
        {
            memory_size_correct = 1;
        }
    }

    //Jeśli zmienna jest równa 0 czyli nie uda się rozszerzyć bloku,
    //ponieważ następny blok jest zajęty albo w połączeniu nadal rozmiar jest za mały
    //wykonywana jest funckja malloc
    if(memory_size_correct == 0)
    {
        //Utworzenie nowego bloku pamięci na podany rozmiar
        pthread_mutex_unlock(&mutex);
        void *new_memory = heap_malloc(size);
        pthread_mutex_lock(&mutex);


        //Jeśli nowy rozmiar jest większy to idź dalej
        if(new_count > temp->block_size)
        {
            //Sprawdza czy malloc został wykonany poprawnie,
            //Za pomocą memcpy kopiuje cała zawartość z poprzedniego bloku do nowego
            //Rozmiara kopiowania to rozmiar starego bloku
            if(new_memory == NULL || memcpy(new_memory, memblock, temp->block_size) == NULL)
            {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
            pthread_mutex_unlock(&mutex);
            return new_memory;
        }
            //Jeśli nowy rozmiar jest mniejszy lub równy staremu to idź dalej
        else
        {
            //Sprawdza czy malloc został wykonany poprawnie,
            //Za pomocą memcpy kopiuje cała zawartość z poprzedniego bloku do nowego
            //Rozmiara kopiowania to podany rozmiar w argumencie
            if(new_memory == NULL || memcpy(new_memory, memblock, size) == NULL)
            {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
            pthread_mutex_unlock(&mutex);
            return new_memory;
        }
    }

        //Jeśli rozmiar wskazanego bloku w połączeniu ze starym da możliwość
        //relokowania bloku to idź dalej
    else
    {
        //Jeśli nowy rozmiar jest różny od starego idź dalej
        if(new_count != -temp->block_size)
        {
            //Obliczenie część rozmiaru który zostanie pobrany z następnego bloku
            int needed_memory = (int)new_count - temp->block_size;
            temp->block_size = new_count;

            //Obliczenie rozmiary który pozostanie po połaczeniu bloków w następnym bloku
            int rest_size = temp->next->block_size + needed_memory;

            //Pobranie następnego bloku celem jego modyfikacji
            struct heap_block *temp2 = (void*)memblock + new_count;

            //Przepisanie odpowiednio wskaźników do next i prev bloków na około nowego(jak w liście dwukierunkowej)
            temp2->next = temp->next->next;
            temp2->prev = temp;
            temp->next = temp2;
            temp2->next->prev = temp2;
            temp2->block_size = rest_size;
            temp2->left_control_fence = 0;
            temp2->code_line = 0;
            temp2->program_name = NULL;
            temp2->right_control_fence = 0;

            //Obliczenie na nowo sumy kontrolnej
            temp2->next->check_sum = check_sum_function(temp2->next);
            temp2->check_sum = check_sum_function(temp2);
            temp->check_sum = check_sum_function(temp);

            //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
            pthread_mutex_unlock(&mutex);
            return memblock;
        }
            //Jeśli nowy rozmiar jest równy staremu to idź dalej
        else
        {
            //Wystarczy ustawić nowy rozmiar i obliczyć sume kontrolną
            temp->block_size = new_count;
            temp->check_sum = check_sum_function(temp);

            //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
            pthread_mutex_unlock(&mutex);
            return memblock;
        }
    }
}

void * heap_malloc_debug(size_t count, int fileline, const char * filename)
{
    //Sprawdzenie czy podany rozmiar linia kodu oraz nazwa programu sa poprawne
    if(count <= 0 || fileline <= 0 || filename == NULL)
    {
        return NULL;
    }

    //Użycie funkcji malloc do stworzenie nowe bloku pamięci
    void* new_memory = heap_malloc(count);

    //Jeśli malloc się udał idź dalej
    if(new_memory != NULL)
    {
        //Ustawienie nowych danych dla rozszerzenia debu takich jak linia programu i nazwa programu
        struct heap_block *temp = (struct heap_block*)(void*)(new_memory - sizeof(struct heap_block));

        temp->left_control_fence = 0;
        temp->code_line = fileline;
        temp->program_name = filename;
        temp->right_control_fence = 0;

        //Obliczenie sumy kontrolnej dla modyfikowanych elementów
        temp->check_sum = check_sum_function(temp);
        pages.check_sum = check_sum_for_control();

        //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
        return new_memory;
    }
        //Jeśli malloc sie nie powiódł to zwraca NULL
    else
    {
        return NULL;
    }
}


void * heap_calloc_debug(size_t number, size_t size, int fileline, const char * filename)
{
    //Sprawdzenie czy podany rozmiar linia kodu oraz nazwa programu sa poprawne
    if(number <= 0 || size <= 0 || fileline <= 0 || filename == NULL)
    {
        return NULL;
    }

    //Użycie funkcji calloc do stworzenie nowe bloku pamięci
    void* new_memory = heap_calloc(number, size);

    //Jeśli calloc się udał idź dalej
    if(new_memory != NULL)
    {
        //Ustawienie nowych danych dla rozszerzenia debu takich jak linia programu i nazwa programu
        struct heap_block *temp = (struct heap_block*)(void*)(new_memory - sizeof(struct heap_block));

        temp->left_control_fence = 0;
        temp->code_line = fileline;
        temp->program_name = filename;
        temp->right_control_fence = 0;

        //Obliczenie sumy kontrolnej dla modyfikowanych elementów
        temp->check_sum = check_sum_function(temp);
        pages.check_sum = check_sum_for_control();

        //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
        return new_memory;
    }
        //Jeśli malloc sie nie powiódł to zwraca NULL
    else
    {
        return NULL;
    }
}


void * heap_realloc_debug(void * memblock, size_t size, int fileline, const char * filename)
{
    //Sprawdzenie czy podany rozmiar linia kodu oraz nazwa programu sa poprawne
    if(size <= 0 || fileline <= 0 || filename == NULL)
    {
        return NULL;
    }

    //Użycie funkcji realloc do stworzenie nowe bloku pamięci
    void* new_memory = heap_realloc(memblock, size);

    //Jeśli realloc się udał idź dalej
    if(new_memory != NULL)
    {
        //Ustawienie nowych danych dla rozszerzenia debu takich jak linia programu i nazwa programu
        struct heap_block *temp = (struct heap_block *)(void*)(new_memory - sizeof(struct heap_block));

        temp->left_control_fence = 0;
        temp->code_line = fileline;
        temp->program_name = filename;
        temp->right_control_fence = 0;

        //Obliczenie sumy kontrolnej dla modyfikowanych elementów
        temp->check_sum = check_sum_function(temp);
        pages.check_sum = check_sum_for_control();

        //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
        return new_memory;
    }
        //Jeśli malloc sie nie powiódł to zwraca NULL
    else
    {
        return NULL;
    }
}

void * heap_malloc_aligned(size_t count)
{
    pthread_mutex_lock(&mutex);
    //Sprawdzenie czy sterta istnieje i czy liczba jest poprawna
    if(count <= 0 || pages.init == 0)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    //Funkcja zaokrągla do słowa maszynowego dla ułatwienia pracy maszynie
    size_t new_count = add_to_word(count);

    //Sprawdzenie czy funkcja nie zaalokuje więcej miejsca niż jest to możliwe
    //Wartość są spisane w pliku custom_unistd.c
    if(new_count > PAGE_SIZE * PAGES_AVAILABLE)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    void* new_memory = (void*)pages.head;
    int condition = 0;

    //Sprawdzenie i przygotowania dodania nowej strony aby mozna było zaalokować pamięć na początku strony
    while (1)
    {
        new_memory = (void*)pages.head;
        //Rozpoczyna przechodzenie od poczatku bloków pamięci
        while(1)
        {
            //Funckja sprawdza czy gdzieś w stercie znajduje się miejsce na początku strony i ma odpowiednio dużo miejsca
            if(aligned_help_function((intptr_t)new_memory, new_count) != 1)
            {
                //Jeśli funckja zwróci 0 to sprawdzamy czy kolejna strona istnieje i można przejść na jej początek
                if((intptr_t)new_memory + new_count <= (intptr_t)pages.tail)
                {
                    //Przesuwa wskaźnik na początek strony
                    new_memory = new_memory + PAGE_SIZE;
                }
                    //Jeśli będzie to nie możliwe zmienna condition będzie równa 1
                else
                {
                    condition = 1;
                    break;
                }
            }
                //W innym przypadku przerywamy pętle ponieważ można zaalokować miejsce na początku strony
            else
            {
                break;
            }
        }

        //Jeśli zmienna jest równa 0 przerywa pętle while ponieważ można zaalokować miejsce na początku strony
        if(condition == 0)
        {
            break;
        }
            //Jeśli nie ustawia zmienną na 0 i dodaje nową stronę do sterty +PAGE_SIZE(4096)
        else
        {
            //Ponownie ustawiamy zmienną na zero aby sprawdzić czy tym razem można stworzyć blok pamięci na początku strony
            //Nadal miejsca moze być za mało
            condition = 0;
            //Stworzenie nowej strony, jeśli zwróciła coś innego niż 0 wystapił błąd
            if(add_new_page() != 0)
            {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
        }
    }

    //Ustawia wskaźnik na początek bloku pamięci(początek struktury)
    struct heap_block *temp = pages.head;
    while(temp != NULL)
    {
        //Szukamy bloku pamięci który znajduje się przed wskaźnikiem
        int x = (int)((void *)new_memory -(void *)temp);
        //Jeśli wartość jest ujemna to znaczy że juz przeszliśmy ten wskaźnik i poprzedni blok był tym przed wskaźnikiem
        if(x < 0)
        {
            //Wracamy do poprzedniego bloku
            temp = temp->prev;
            break;
        }
        //Przechodzimy dalej do nastepnego bloku
        temp = temp->next;
    }

    //Sprawdzenie czy wskaźnik czy po cofnięciu znajduje się juz blok pamięci z wolną przestrzenią
    //Sprawdza czy blok ma ujemny rozmiar czyli ma wolna przestrzeń
    if(get_pointer_type(new_memory - sizeof(struct heap_block)) == pointer_control_block && -temp->block_size == new_count)
    {
        //Zostaje ustawiony nowy rozmiar dla istniejącego bloku i suma kontrolna zostaje na nowo obliczona
        temp->block_size = new_count;
        temp->check_sum = check_sum_function(temp);

        //Zwraca wskaźnik na nowy blok(już istniejący blok) pamięci
        pthread_mutex_unlock(&mutex);
        return new_memory;
    }

    //Jeśli koniec bloku pamięci wypadnie na końcu strony czyli na tailu a rozmiar jest idealnie wyliczony
    if(new_memory + new_count == (void*)pages.tail)
    {
        //Przemieszczami wskaźnik na początek strony licząc od bloku pamięci przed początkiem strony
        struct heap_block *temp2 = (struct heap_block *)(void*)(temp-sizeof(struct heap_block));

        //Ustawia dla niego okreslony rozmiar
        temp2->block_size = new_count;

        //Rozmiar poprzedniego zmniejszamy o rozmiar nowego bloku utworzonego na początku strony i rozmiar struktury
        temp->block_size = temp->block_size + new_count + sizeof(struct heap_block);

        //Przed ogonem tworzy się nowy blok pamięci
        pages.tail->prev = temp2;

        //Następny blok to jest blok na początku strony
        temp->next = temp2;
        temp2->prev = temp;
        temp2->next = pages.tail;

        //Ustawiamy dane bloku
        temp2->left_control_fence = 0;
        temp2->code_line = 0;
        temp2->program_name = NULL;
        temp2->right_control_fence = 0;

        //Obliczamy nową sume kontrolną wszystkich modyfikowanych elementów
        temp->check_sum = check_sum_function(temp);
        temp2->check_sum = check_sum_function(temp2);
        pages.tail->check_sum = check_sum_function(pages.tail);
        pages.check_sum = check_sum_for_control();
    }
    //Jeśli w miejscu wskaźnika nie ma bloku pamięci wcześniej
    else if(get_pointer_type(new_memory - sizeof(struct heap_block)) != pointer_control_block)
    {
        //Tworzy blok, ustawiamy rozmiar, zmieniamy rozmiar pozostałych
        //Jest to blok który ma być zaraz za blokiem o rozmiarze new_count
        struct heap_block* temp2 = (struct heap_block*)((void*)new_memory + new_count);
        int x = (int)((new_memory - (void*)temp) - sizeof(struct heap_block));
        temp2->block_size = temp->block_size + x + new_count;
        temp->block_size = -(int)(x - sizeof(struct heap_block) + new_count);

        //Jeśli nastepny blok nie jest tail to ustawiamy odpowiednio wskaźniki(jak w liście dwukierunkowej)
        if(temp->next != pages.tail)
        {
            //Blok który jest następny po temp2
            struct heap_block *temp3 = (struct heap_block*)temp->next;
            temp3->prev = temp2;
            temp2->next = temp->next;
            temp2->prev = temp;
            temp->next = temp2;

            temp3->check_sum = check_sum_function(temp3);
        }
        //Inaczej, ustawiamy wskaźniki tak jakby po bloku miał byc tail
        else
        {
            pages.tail->prev = temp2;
            temp->next = temp2;
            temp2->prev = temp;
            temp2->next = pages.tail;
        }

        //Ustawiamy pozostałe dane(płoti, linie kodu, nazwe kodu)
        temp2->left_control_fence = 0;
        temp2->code_line = 0;
        temp2->program_name = NULL;
        temp2->right_control_fence = 0;

        //Obliczamy nowe sumy kontrolne dla modyfikowanych elementów
        temp->check_sum = check_sum_function(temp);
        temp2->check_sum = check_sum_function(temp2);
        pages.tail->check_sum = check_sum_function(pages.tail);
        pages.check_sum = check_sum_for_control();

        //Jest to blok głowny, który tworzymy i zwracamy jest on po temp a przed temp2 i temp3
        struct heap_block *temp4 = (struct heap_block*)(void*)(new_memory - sizeof(struct heap_block));

        //Ustawiamy odpowiednio wskaźniki na nastepne i porzednie bloki
        temp->block_size = temp->block_size + new_count + sizeof(struct heap_block);
        temp4->block_size = new_count;
        temp2->prev = temp4;
        temp4->next = temp->next;
        temp4->prev = temp;
        temp->next = temp4;

        //Ustawiamy pozostałe dane(płotki, linie programu, nazwe programu)
        temp4->left_control_fence = 0;
        temp4->code_line = 0;
        temp4->program_name = NULL;
        temp4->right_control_fence = 0;

        //Obliczamy nowe sumy kontrolne zmodyfikowanych elementów
        temp4->check_sum = check_sum_function(temp4);
        temp->check_sum = check_sum_function(temp);
        temp2->check_sum = check_sum_function(temp2);
        pages.tail->check_sum = check_sum_function(pages.tail);
        pages.check_sum = check_sum_for_control();
    }
    //Inaczej uzupełniamy stary blok danych, nowymi danymi
    else
    {
        //Odpowiednio ustawiamy wskaźnik na next i prev oraz rozmiar
        struct heap_block *temp2 = (struct heap_block *)(void*)(temp+new_count);
        temp2->prev = temp;
        temp2->next = temp->next;
        temp->next = temp2;
        temp2->block_size = new_count + sizeof(struct heap_block) + temp->block_size;
        temp->block_size = new_count;

        //Ustawiamy pozostałe dane(płotki, numer linii, nazwe programu)
        temp2->left_control_fence = 0;
        temp2->code_line = 0;
        temp2->program_name = NULL;
        temp2->right_control_fence = 0;

        //Obliczamy na nowo sumy kontrolne modyfikowanych elementów
        temp->check_sum = check_sum_function(temp);
        temp2->check_sum = check_sum_function(temp2);
        pages.tail->check_sum = check_sum_function(pages.tail);
        pages.check_sum = check_sum_for_control();
    }
    pthread_mutex_unlock(&mutex);
    return new_memory;
}


void * heap_calloc_aligned(size_t number, size_t size)
{
    pthread_mutex_lock(&mutex);
    //Sprawdzenie czy podane liczby są większe od zera
    if (number <= 0 || size <= 0)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    //Za pomocą funkcji malloc_aligned tworzy nowy blok(lub używa starego) i zwraca wskaźnik na początek miejsca gdzie można zapisać dane
    pthread_mutex_unlock(&mutex);
    void *new_memory = heap_malloc_aligned(size * number);
    pthread_mutex_lock(&mutex);

    //Jeśli udało się zaalokowac miejsce to zerowane są wszystkie pola
    if (new_memory != NULL)
    {
        //Jeśli memset wyzerował wszystkie pola od wskaźnika new_memory do rozmiaru size*number(czyli nie zwrócił NULL)
        //to zwraca wskaźnika na początek wyzerowanych pól
        if(memset(new_memory, 0, size * number) != NULL)
        {
            pthread_mutex_unlock(&mutex);
            return new_memory;
        }
            //Jeśli coś poszło nie tak zwraca NULL
        else
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
    }
        //Jeśli malloc_aligned nie udał się zwraca NULL
    else
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
}

void* heap_realloc_aligned(void * memblock, size_t size)
{
    pthread_mutex_lock(&mutex);
    //Sprawdza czy blok pamięci jest poprawny
    if(memblock == NULL)
    {
        pthread_mutex_unlock(&mutex);
        return heap_malloc_aligned(size);
    }

    //Sprawdza czy wskaźnik jest poprawny
    if(get_pointer_type(memblock) != pointer_valid)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    //Funkcja zaokrągla do słowa maszynowego dla ułatwienia pracy maszynie
    size_t new_count = add_to_word(size);

    //Sprawdzenie czy funkcja nie zaalokuje więcej miejsca niż jest to możliwe
    //Wartość są spisane w pliku custom_unistd.c
    if(new_count > PAGE_SIZE * PAGES_AVAILABLE || size <= 0)
    {
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    void* new_memory = (void*)pages.head;
    int condition = 0;
    int check = 0;

    //Sprawdzenie i przygotowania dodania nowej strony aby mozna było zaalokować pamięć na początku strony
    while (1)
    {
        new_memory = (void*)pages.head;
        //Rozpoczyna przechodzenie od poczatku bloków pamięci
        while(1)
        {
            //Funckja sprawdza czy gdzieś w stercie znajduje się miejsce na początku strony i ma odpowiednio dużo miejsca
            if(aligned_help_function((intptr_t)new_memory, new_count) != 1)
            {
                //Jeśli funckja zwróci 0 to sprawdzamy czy kolejna strona istnieje i można przejść na jej początek
                if((intptr_t)new_memory + new_count <= (intptr_t)pages.tail)
                {
                    //Przesuwa wskaźnik na początek strony
                    new_memory = new_memory + PAGE_SIZE;
                    check = 1;
                }
                    //Jeśli będzie to nie możliwe zmienna condition będzie równa 1
                else
                {
                    condition = 1;
                    break;
                }
            }
                //W innym przypadku przerywamy pętle ponieważ można zaalokować miejsce na początku strony
            else
            {
                break;
            }
        }

        //Jeśli zmienna jest równa 0 przerywa pętle while ponieważ można zaalokować miejsce na początku strony
        if(condition == 0)
        {
            break;
        }
            //Jeśli nie ustawia zmienną na 0 i dodaje nową stronę do sterty +PAGE_SIZE(4096)
        else
        {
            //Ponownie ustawiamy zmienną na zero aby sprawdzić czy tym razem można stworzyć blok pamięci na początku strony
            //Nadal miejsca moze być za mało
            condition = 0;
            check = 1;
            //Stworzenie nowej strony, jeśli zwróciła coś innego niż 0 wystapił błąd
            if(add_new_page() != 0)
            {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
        }
    }

    if(check == 1)
    {
        //Utworzenie nowego bloku pamięci na podany rozmiar
        pthread_mutex_unlock(&mutex);
        new_memory = heap_malloc_aligned(size);
        pthread_mutex_lock(&mutex);

        //Sprawdza czy malloc_aligned został wykonany poprawnie,
        //Za pomocą memcpy kopiuje cała zawartość z poprzedniego bloku do nowego
        //Rozmiara kopiowania to podany rozmiar w argumencie
        if(new_memory == NULL || memcpy(new_memory, memblock, size) == NULL)
        {
            pthread_mutex_unlock(&mutex);
            return NULL;
        }
        //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
        pthread_mutex_unlock(&mutex);
        return new_memory;
    }

    //Ustawia wskaźnik na początek bloku pamięci(początek struktury)
    struct heap_block *temp = (struct heap_block *)(void*)(memblock - sizeof(struct heap_block));

    int memory_size_correct = 0;

    //Sprawdzenie czy można dokonać operacji rozszerzenie bloku,
    //jeśli podany rozmiar jest większy to idź dalej
    if(new_count > temp->block_size)
    {
        //Jeśli następny blok ma ujemny rozmiar(jest wolny) i w połaczeniu z rozmiarem poprzedniego
        //pomieści nową alokacje to ustaw zmienna na 1
        if(temp->next->block_size < 0 && -temp->next->block_size >= new_count - temp->block_size)
        {
            memory_size_correct = 1;
        }
        if(temp->next->block_size < 0 && -temp->next->block_size >= temp->block_size - new_count)
        {
            memory_size_correct = 1;
        }
    }

    //Jeśli zmienna jest równa 0 czyli nie uda się rozszerzyć bloku,
    //ponieważ następny blok jest zajęty albo w połączeniu nadal rozmiar jest za mały
    //wykonywana jest funckja malloc_aligned
    if(memory_size_correct == 0)
    {
        //Utworzenie nowego bloku pamięci na podany rozmiar
        pthread_mutex_unlock(&mutex);
        void *new_memory = heap_malloc_aligned(size);
        pthread_mutex_lock(&mutex);

        //Jeśli nowy rozmiar jest większy to idź dalej
        if(new_count > temp->block_size)
        {
            //Sprawdza czy malloc został wykonany poprawnie,
            //Za pomocą memcpy kopiuje cała zawartość z poprzedniego bloku do nowego
            //Rozmiara kopiowania to rozmiar starego bloku
            if(new_memory == NULL || memcpy(new_memory, memblock, temp->block_size) == NULL)
            {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }

            //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
            pthread_mutex_unlock(&mutex);
            return new_memory;
        }
            //Jeśli nowy rozmiar jest mniejszy lub równy staremu to idź dalej
        else
        {
            //Sprawdza czy malloc_aligned został wykonany poprawnie,
            //Za pomocą memcpy kopiuje cała zawartość z poprzedniego bloku do nowego
            //Rozmiara kopiowania to podany rozmiar w argumencie
            if(new_memory == NULL || memcpy(new_memory, memblock, size) == NULL)
            {
                pthread_mutex_unlock(&mutex);
                return NULL;
            }
            //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
            pthread_mutex_unlock(&mutex);
            return new_memory;
        }
    }
        //Jeśli rozmiar wskazanego bloku w połączeniu ze starym da możliwość
        //relokowania bloku to idź dalej
    else
    {
        //Jeśli nowy rozmiar jest różny od starego idź dalej
        if(new_count != -temp->block_size)
        {
            //Obliczenie część rozmiaru który zostanie pobrany z następnego bloku
            int needed_memory = (int)new_count - temp->block_size;
            temp->block_size = new_count;

            //Obliczenie rozmiary który pozostanie po połaczeniu bloków w następnym bloku
            int rest_size = temp->next->block_size + needed_memory;

            //Pobranie następnego bloku celem jego modyfikacji
            struct heap_block *temp2 = (void*)memblock + new_count;

            //Przepisanie odpowiednio wskaźników do next i prev bloków na około nowego(jak w liście dwukierunkowej)
            temp2->next = temp->next->next;
            temp2->prev = temp;
            temp->next = temp2;
            temp2->next->prev = temp2;
            temp2->block_size = rest_size;
            temp2->left_control_fence = 0;
            temp2->code_line = 0;
            temp2->program_name = NULL;
            temp2->right_control_fence = 0;

            //Obliczenie na nowo sumy kontrolnej
            temp2->next->check_sum = check_sum_function(temp2->next);
            temp2->check_sum = check_sum_function(temp2);
            temp->check_sum = check_sum_function(temp);

            //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
            pthread_mutex_unlock(&mutex);
            return memblock;
        }
            //Jeśli nowy rozmiar jest równy staremu to idź dalej
        else
        {
            //Wystarczy ustawić nowy rozmiar i obliczyć sume kontrolną
            temp->block_size = new_count;
            temp->check_sum = check_sum_function(temp);

            //Zwraca wksaźnik na miejsce gdzie mozna zacząc wpisywać dane
            pthread_mutex_unlock(&mutex);
            return memblock;
        }
    }
}


void * heap_malloc_aligned_debug(size_t count, int fileline, const char * filename)
{
    //Sprawdza czy dane przekazane w argumentach są poprawne
    if(count <= 0 || fileline <= 0 || filename == NULL)
    {
        return NULL;
    }

    //Stworzony jest nowy blok pamięci za pomoca malloc_aligned
    void* new_memory = heap_malloc_aligned(count);

    //Jeśli blok zostal stworzony poprawnie idź dalej
    if(new_memory != NULL)
    {
        //Przesuwamy w tył wskaźnik, aby móc modyfikować strukture bloku
        struct heap_block *temp = (struct heap_block*)(void*)(new_memory - sizeof(struct heap_block));

        //Ustawiamy odpowiednie dane zgodnie z przeznaczeniem funkcji DEBUG
        temp->left_control_fence = 0;
        temp->code_line = fileline;
        temp->program_name = filename;
        temp->right_control_fence = 0;

        //Obliczamy nowe sumy kontrolne dla mofikowanych elementów
        temp->check_sum = check_sum_function(temp);
        pages.check_sum = check_sum_for_control();

        //Zwracamy wskaźnik na początek miejsca gdzie można zapisać dane
        return new_memory;
    }
    //Jeśli alokacja się nie powiodła to zwracamy NULL
    else
    {
        return NULL;
    }
}

void * heap_calloc_aligned_debug(size_t number, size_t size, int fileline, const char * filename)
{
    //Sprawdza czy dane przekazane w argumentach są poprawne
    if(number <= 0 || size <= 0 || fileline <= 0 || filename == NULL)
    {
        return NULL;
    }

    //Stworzony jest nowy blok pamięci za pomoca calloc_aligned
    void* new_memory = heap_calloc_aligned(number, size);

    //Jeśli blok zostal stworzony poprawnie idź dalej
    if(new_memory != NULL)
    {
        //Przesuwamy w tył wskaźnik, aby móc modyfikować strukture bloku
        struct heap_block *temp = (struct heap_block*)(void*)(new_memory - sizeof(struct heap_block));

        //Ustawiamy odpowiednie dane zgodnie z przeznaczeniem funkcji DEBUG
        temp->left_control_fence = 0;
        temp->code_line = fileline;
        temp->program_name = filename;
        temp->right_control_fence = 0;

        //Obliczamy nowe sumy kontrolne dla mofikowanych elementów
        temp->check_sum = check_sum_function(temp);
        pages.check_sum = check_sum_for_control();

        //Zwracamy wskaźnik na początek miejsca gdzie można zapisać dane
        return new_memory;
    }
        //Jeśli alokacja się nie powiodła to zwracamy NULL
    else
    {
        return NULL;
    }
}

void * heap_realloc_aligned_debug(void * memblock, size_t size, int fileline, const char * filename)
{
    //Sprawdza czy dane przekazane w argumentach są poprawne
    if(size <= 0 || fileline <= 0 || filename == NULL)
    {
        return NULL;
    }

    //Stworzony jest nowy blok pamięci za pomoca realloc_aligned
    void* new_memory = heap_realloc_aligned(memblock, size);

    //Jeśli blok zostal stworzony poprawnie idź dalej
    if(new_memory != NULL)
    {
        //Przesuwamy w tył wskaźnik, aby móc modyfikować strukture bloku
        struct heap_block *temp = (struct heap_block*)(void*)(new_memory - sizeof(struct heap_block));

        //Ustawiamy odpowiednie dane zgodnie z przeznaczeniem funkcji DEBUG
        temp->left_control_fence = 0;
        temp->code_line = fileline;
        temp->program_name = filename;
        temp->right_control_fence = 0;

        //Obliczamy nowe sumy kontrolne dla mofikowanych elementów
        temp->check_sum = check_sum_function(temp);
        pages.check_sum = check_sum_for_control();

        //Zwracamy wskaźnik na początek miejsca gdzie można zapisać dane
        return new_memory;
    }
        //Jeśli alokacja się nie powiodła to zwracamy NULL
    else
    {
        return NULL;
    }
}

//Funkcja zwraca liczbę wykorzystanych bajtów sterty.
size_t heap_get_used_space(void)
{
    if(pages.init == 0)
    {
        return 0;
    }
    else
    {
        size_t sum = 0;
        struct heap_block *temp = pages.head;

        while(temp != NULL)
        {
            sum = sum + sizeof(struct heap_block);
            if(temp->block_size > 0)
            {
                sum = sum + temp->block_size;
            }
            temp=temp->next;
        }

        return sum;
    }
}

//Funkcja zwraca długość największego zaalokowanego bloku, lub 0 gdy niczego na niej nie zaalokowano
size_t heap_get_largest_used_block_size(void)
{
    if(pages.init == 0)
    {
        return 0;
    }
    else
    {
        size_t largest = 0;
        struct heap_block *temp = pages.head;

        while(temp != NULL)
        {
            if(temp->block_size > 0)
            {
                if(temp->block_size > largest)
                {
                    largest = temp->block_size;
                }
            }

            temp=temp->next;
        }

        return largest;
    }
}

//Funkcja zwraca liczbę zaalokowanych bloków.
uint64_t heap_get_used_blocks_count(void)
{
    if(pages.init == 0)
    {
        return 0;
    }
    else
    {
        uint64_t count = 0;
        struct heap_block *temp = pages.head;

        while(temp != NULL)
        {
            if(temp->block_size > 0)
            {
                count++;
            }

            temp=temp->next;
        }

        return count;
    }
}

//Funkcja zwraca liczbę dostępnych bajtów sterty
size_t heap_get_free_space(void)
{
    if(pages.init == 0)
    {
        return 0;
    }
    else
    {
        size_t sum = 0;
        struct heap_block *temp = pages.head;

        while(temp != NULL)
        {
            if(temp->block_size < 0)
            {
                sum += -temp->block_size;
            }

            temp=temp->next;
        }

        return sum;
    }
}

//Funkcja zwraca długość największego wolnego obszaru dostępnego na stercie
size_t heap_get_largest_free_area(void)
{
    if(pages.init == 0)
    {
        return 0;
    }
    else
    {
        size_t largest = 0;
        struct heap_block *temp = pages.head;

        while(temp != NULL)
        {
            if(temp->block_size < 0)
            {
                if(-temp->block_size > largest)
                {
                    largest = -temp->block_size;
                }
            }

            temp=temp->next;
        }

        return largest;
    }
}

//Funkcja zwraca liczbę wolnych obszarów, w których można zaalokować blok o długości nie mniejszej niż długość słowa danych CPU.
uint64_t heap_get_free_gaps_count(void)
{
    if(pages.init == 0)
    {
        return 0;
    }
    else
    {
        uint64_t count = 0;
        struct heap_block *temp = pages.head;

        while(temp != NULL)
        {
            if(temp->block_size < 0)
            {
                count++;
            }

            temp=temp->next;
        }

        return count;
    }
}
/**
 * Funkcja get_pointer_type zwraca informację o przynależności wskaźnika pointer do sterty lub pamięci spoza sterty.
 * Funkcja ta, na podstawie informacji zawartych w strukturze sterty klasyfikuje wskaźnik
 * pointer i zwraca jedną z wartości typu pointer_type_t.
 *
 * pointer_type_t informuje co znajduje sie pod podanym wskaźnikiem
 * pointer_null podany wskaźnik jest pusty
 * pointer_out_of_heap podany wskaźnik poza stertą
 * pointer_control_block podany wskaźnik jest w gdzies w stercie ale jest w strukturze wewnętrznej
 * pointer_inside_data_block podany wskaźnik jest w zaalokowanym miejscu
 * pointer_unallocated podany wskaźnik jest w miejscu w którym nie dokonala się alokacja
 * pointer_valid podany wskaźnik jest poprawny
 */
enum pointer_type_t get_pointer_type(const const void * pointer)
{
    if(pointer == NULL || pages.init == 0)
    {
        return pointer_null;
    }

    struct heap_block *temp = pages.head;

    if((int)(pointer - (void*)temp)< 0)
    {
        return pointer_out_of_heap;
    }
    else if((int)(pointer - (void*)temp) == 0)
    {
        return pointer_control_block;
    }

    while(temp != NULL)
    {
        int x = (int)(pointer - (void*)temp);

        if((void*)temp+sizeof(struct heap_block) == pointer)
        {
            if(temp->block_size < 0)
            {
                return pointer_unallocated;
            }
            else if(temp->block_size > 0)
            {
                return pointer_valid;
            }
        }
        if(x < sizeof(struct heap_block))
        {
            if(x >= 0)
            {
                return pointer_control_block;
            }
        }
        else if(x < 0)
        {
            if(temp->prev->block_size > 0)
            {
                return pointer_inside_data_block;
            }
            else if(temp->prev->block_size < 0)
            {
                return pointer_unallocated;
            }
        }

        temp = temp->next;
    }

    return pointer_out_of_heap;
}

//Funkcja zwraca wskaźnik na początek bloku, na którego dowolny bajt wskazuje pointer.
void * heap_get_data_block_start(const void * pointer)
{
    enum pointer_type_t type = get_pointer_type(pointer);

    if(type == pointer_valid)
    {
        return (void*)pointer;
    }
    else if(type == pointer_control_block || type == pointer_unallocated || type == pointer_out_of_heap || type == pointer_null)
    {
        return NULL;
    }
    else
    {
        struct heap_block *temp = pages.head;
        while(temp != NULL)
        {
            int value = (int)(pointer - (void*)temp);
            if(value < 0)
            {
                break;
            }
            else
            {
                temp = temp->next;
            }
        }

        temp = temp->prev;
        return (void*)(temp + sizeof(struct heap_block));
    }
}

//Funkcja zwraca długość bloku danego wskaźnikiem memblock.
//Jeżeli memblock nie wskazuje na zaalokowany blok (inny niż pointer_valid), to funkcja zwraca 0
size_t heap_get_block_size(const const void * memblock)
{
    if(get_pointer_type(memblock) == pointer_valid)
    {
        struct heap_block *temp = (struct heap_block*)(void*)(memblock - sizeof(struct heap_block));
        return temp->block_size;
    }
    else
    {
        return 0;
    }
}

/*
Funkcja wykonuje sprawdzenie spójności sterty
Sterta poprawna - 0
Sterta nie zainicjalizowana - -1
Błąd prawego płotka na stercie - -2
Błąd lewego płotka na stercie -  -3
Błąd sumy kontrolneh na stercie - -4
Błąd głowy sterty - -5
Błąd ogona sterty -6
Błąd elementu przed głową(!= NULL) -7
Błąd elementu za ogonem(!= NULL) -8
Element prev dla bloku kontrolnego to NULL -9
Element next dla bloku kontrolnego to NULL-10
Błąd lewego płotka w bloku -11
Błąd prawego płotka w bloku -12
Błąd sumy kontrolnej w bloku - -13
Rozmiar równy 0 dla bloku - -14
Dwa wolne bloki pamięci obok siebie - -15
Zły rozmiar sterty - -16
*/
int heap_validate(void)
{
    if(pages.init == 0)
    {
        return -1;
    }
    else if(pages.right_control_fence != 0)
    {
        return -2;
    }
    else if(pages.left_control_fence != 0)
    {
        return -3;
    }
    else if(pages.check_sum != check_sum_for_control())
    {
        return -4;
    }
    else if(pages.head == NULL)
    {
        return -5;
    }
    else if(pages.tail == NULL)
    {
        return -6;
    }
    else if(pages.head->prev != NULL)
    {
        return -7;
    }
    else if(pages.tail->next != NULL)
    {
        return -8;
    }

    struct heap_block *temp = pages.head;
    while(temp != NULL)
    {
        if(temp->prev == NULL && temp != pages.head)
        {
            return -9;
        }
        else if(temp->next == NULL && temp != pages.tail)
        {
            return -10;
        }
        else if(temp->left_control_fence != 0)
        {
            return -11;
        }
        else if(temp->right_control_fence != 0)
        {
            return -12;
        }
        else if(temp->check_sum != check_sum_function(temp))
        {
            return -13;
        }
        else if(temp->block_size == 0 && temp != pages.head && temp != pages.tail)
        {
            return -14;
        }
        else if(temp->block_size < 0 && temp->next != NULL && temp->next->block_size < 0)
        {
            return -15;
        }
        temp = temp->next;
    }

    struct heap_block *temp2 = pages.head;
    size_t check = 0;

    while(temp2 != NULL)
    {
        check = check + abs(temp2->block_size) + sizeof(struct heap_block);
        temp2=temp2->next;
    }

    if(check % PAGE_SIZE != 0)
    {
        return -16;
    }

    return 0;
}

/*
Funkcja wyświetla informacje o blokach zaalokowanych na stercie, wraz z podsumowaniem. Informacje
te powinny zawierać:
- Listę wszystkich zaalokowanych bloków ze wskazaniem:
        - Adresu bloku,
        - długości bloku w bajtach,
        - nazwy pliku źródłowego, w którym nastąpiła alokacja (jeżeli użyto funkcji _debug),
        - numeru linii pliku źródłowego, w której nastąpiła alokacja (j/w).
- wielkość sterty w bajtach,
- liczbę bajtów zajętych,
- liczbę bajtów do zaalokowania,
- wielkość największego wolnego bloku.
*/
void heap_dump_debug_information(void)
{
    size_t heap_size = 0;
    struct heap_block *temp = pages.head;

    //Przejście po blokach pamięci w celu ustalenia rozmiaru sterty
    while(temp != NULL)
    {
        heap_size += abs(temp->block_size) + sizeof(struct heap_block);
        temp=temp->next;
    }

    //Wypisanie danych do które są obliczone albo mają do tego funckje
    printf("Heap size - %lu\n", heap_size);
    printf("Used blocks count - %lu\n", heap_get_used_blocks_count());
    printf("Used space - %lu\n", heap_get_used_space());
    printf("Free gaps count - %lu\n", heap_get_free_gaps_count());
    printf("Free space - %lu\n", (long unsigned int)heap_get_free_space());
    printf("Largest used block size - %lu\n", heap_get_largest_used_block_size());
    printf("Largest free block size - %lu\n", heap_get_largest_free_area());
    printf("Heap head - %p\n", pages.head);
    printf("Heap tail - %p\n", pages.tail);

    struct heap_block *temp2 = pages.head;
    int used_block_count = 1;

    //Przejście ponowne po blokach pamięci w celu wyświetlenia informacji o nich
    //takich jak adres, rozmiar czy ewentualnie linia programu oraz nazwa programu w których została stworzona
    while(temp2 != NULL)
    {
        if(temp2->block_size > 0)
        {
            printf("\n|--------------------------------------------------|\n");
            printf("Block number - %d\n" , used_block_count++);
            printf("Adress: - %p\n", (void*)(temp2) + sizeof(struct heap_block));
            printf("Block size - %d\n", temp2->block_size);
            if(temp2->program_name != NULL)
            {
                printf("Program name - %s\n", temp2->program_name);
            }

            if(temp2->code_line != 0)
            {
                printf("Code line - %d\n", temp2->code_line);
            }
        }
        temp2 = temp2->next;
    }
}

//Funkcja oblicza nową sume kontrolną dla bloku pamięci
size_t check_sum_function(struct heap_block * x)
{
    //Funkcja sprawdza czy sterta i blok istnieją
    if(x == NULL || pages.init == 0)
    {
        return 0;
    }
    else
    {
        //Jeśli stera istnieje to w pętli zostają dodane kolejne wartości
        size_t sum = 0;

        unsigned char * first_position = (unsigned char *)(&x->check_sum);
        unsigned char * last_position = (unsigned char *)(&x->check_sum+1);

        for (unsigned char * check = (unsigned char *)x; check < (unsigned  char *)(x+1); check++)
        {
            if(check == first_position)
            {
                check = last_position;
            }
            else
            {
                sum = sum + *check;
            }
        }

        //Zwraca nową wartość
        return sum;
    }
}

//Funkcja oblicza nową sume kontrolną dla sterty
size_t check_sum_for_control()
{
    //Funkcja sprawdza czy sterta istnieje
    if(pages.init == 0)
    {
        return 0;
    }
    else
    {
        //Jeśli stera istnieje to w pętli zostają dodane kolejne wartości
        size_t sum = 0;

        unsigned char * first_position = (unsigned char *)(&pages.check_sum);
        unsigned char * last_position = (unsigned char *)(&pages.check_sum+1);

        for (unsigned char * check = (unsigned char *)&pages; check < (unsigned  char *)(&pages+1); check++)
        {
            if(check == first_position)
            {
                check = last_position;
            }
            else
            {
                sum = sum + *check;
            }
        }

        //Zwraca nową wartość
        return sum;
    }
}

//Funkcja dodająca nową stronę do sterty
int add_new_page()
{
    //Sprawdzenie czy sterta istnieje
    if(pages.init == 0)
    {
        return -1;
    }

    //Wywołanie funckji powoduje rezerwacje miejsca i wskaże na początek tego miejsca PAGE_SIZE = 4096
    intptr_t start = (intptr_t)custom_sbrk(PAGE_SIZE);

    //Drugie wywołanie funckji powoduje zwrócenie wskaźnika na początku nowego bloku co za tym idzie zaraz za wczesniej zarezerwowanym ale bez rozszerzenie strony
    intptr_t end = (intptr_t)custom_sbrk(0);

    //Sprawdzenie czy funkcja zadziałała dobrze
    if((void*)start == (void*)-1 || start == end)
    {
        return -1;
    }

    //Ustawienie nowych wskaźników oraz wartości
    struct heap_block *full = (struct heap_block *)(end-sizeof(struct heap_block));
    full->next = NULL;
    full->left_control_fence = 0;
    full->code_line = 0;
    full->program_name = NULL;
    full->block_size = 0;
    full->right_control_fence = 0;

    //Jeśli ostani blok pamięci przy starym rozmiarze był wolny to idź dalej
    if(pages.tail->prev->block_size < 0)
    {
        //Rozszerza ostatni blok pamięci o nowe dostępne miejsce
        //Modyfikuje jego dane
        full->prev = pages.tail;
        pages.tail->next = full;
        pages.tail->block_size = -(int)(end-start-sizeof(struct heap_block));
        pages.tail->code_line = 0;
        pages.tail->program_name = NULL;
        pages.tail->check_sum = check_sum_function(pages.tail);
        pages.tail = full;
    }
        //Jeśli ostatni blok był zajęty to idź dalej
    else
    {
        //Ustawia nowe wskaźniki dla nowego bloku jak i dla ostatniego
        full->prev = pages.tail->prev;
        pages.tail->prev->next = full;
        pages.tail = full;
        pages.tail->prev->block_size = pages.tail->prev->block_size - (int)(end-start);
        pages.tail->prev->code_line = 0;
        pages.tail->prev->program_name = NULL;
        pages.tail->prev->check_sum = check_sum_function(pages.tail->prev);
    }

    //Łączy wolne obszary powstałe w wyniku dodania strony
    connect_memory();
    pages.check_sum = check_sum_for_control();

    return 0;
}

//Funkcja usuwająca stronę kiedy nie jest ona potrzebna
int delete_page()
{
    //Sprawdzenie czy sterta istnieje
    if(pages.init == 0)
    {
        return -1;
    }

    //Ustalenie danych pomocniczych(cofanie strony)
    intptr_t start = (intptr_t)custom_sbrk(-PAGE_SIZE);
    intptr_t end = (intptr_t)custom_sbrk(0);

    //Sprawdzenie czy funkcja zadziałała dobrze
    if((void*)start == (void*)-1)
    {
        return -1;
    }

    //Ustawienie nowych wskaźników dla next i prev
    struct heap_block *temp = pages.tail->prev;

    //Przemieszczenie końca sterty w tyl i ustawienie nowych danych
    pages.tail = (struct heap_block*)((void*)end - sizeof(struct heap_block));
    pages.tail->next = NULL;
    pages.tail->left_control_fence = 0;
    pages.tail->block_size = 0;
    pages.tail->code_line = 0;
    pages.tail->program_name = NULL;
    pages.tail->right_control_fence = 0;

    //Ustawienie nowego rozmiaru i danych ostatniego bloku pamięci
    temp->block_size = temp->block_size + (int)(start-end);
    temp->next = pages.tail;
    pages.tail->prev = temp;

    //Obliczenie sum kontrolych dla zmodyfikowanych elementów
    temp->check_sum = check_sum_function(temp);
    pages.tail->check_sum = check_sum_function(pages.tail);
    pages.check_sum = check_sum_for_control();

    return 0;
}

//Funkcja łącząca puste bloki obok siebie pamięci w jeden
void connect_memory()
{
    //Przechodzenie po kolejnych blokach szukając dwóch wolnych
    for (struct heap_block *temp = pages.head; temp != NULL; temp=temp->next)
    {
        //Jeśli blok jest wolny idź dalej
        if(temp->block_size < 0)
        {
            struct heap_block * check = temp->next;

            //Jeśli następny też jest wolny to połącz je
            if(check->block_size < 0)
            {
                temp->block_size = temp->block_size + check->block_size - sizeof(struct heap_block);
                temp->next = check->next;
                temp->next->prev = temp;
            }
        }
    }

    //Obliczenie sumy kontrolnej dla wszytkich bloków
    for (struct heap_block *temp = pages.head; temp != NULL; temp=temp->next)
    {
        temp->check_sum = check_sum_function(temp);
    }
}

//Funkcja sprawdzająca możliwości rozszerzenia stery
int aligned_help_function(intptr_t check, size_t size)
{
    //Sprawdzenie czy wskaźnik jest poprawny
    if(get_pointer_type((void *)check) != pointer_unallocated)
    {
        return 0;
    }

    //Sprawdzenie czy rozmiar nie przekroczył maksymalnego
    if(size > PAGE_SIZE * PAGES_AVAILABLE)
    {
        return 0;
    }

    //Przechodzimy od glowy po każdym bloku pamięci pętle się kończy jeśli struktura będzie NULL
    struct heap_block *temp = pages.head;
    while(temp != NULL)
    {
        //Szukamy bloku pamięci który znajduje się przed wskaźnikiem
        int x = (int)((void *)check -(void *)temp);
        //Jeśli wartość jest ujemna to znaczy że juz przeszliśmy ten wskaźnik i poprzedni blok był tym przed wskaźnikiem
        if(x < 0)
        {
            temp = temp->prev;
            break;
        }
        //Przechodzimy do następnego wskaźnika
        temp = temp->next;
    }

    //Jesli temp jest NULL to zwraca 0
    if(temp == NULL)
    {
        return 0;
    }

    //Warunek który musi zostać spełniony aby wskaźnik był połaczony na początku strony
    if((check & (intptr_t)(PAGE_SIZE - 1)) == 0)
    {
        //Sprawdzenie czy wskaźnik jest w miejscu nie zaalokwanym
        if(get_pointer_type((void *)check) == pointer_unallocated)
        {
            //Sprawdzenie czy jest odpowiednio dużo miejsca na alokacje
            //Czy wskaźnik nastepny odjąć wskaźnik check jest równy rządanemu rozmiarowi i czy jest większy od podanego rozmiaru + rozmiar struktury bloku pamięci
            if((intptr_t)temp->next - check == size || (intptr_t)temp->next - check > size + sizeof(struct heap_block))
            {
                //Zwraca 1 czyli nie trzeba rozszerzać sterty
                return 1;
            }
        }
            //Sprawdzenie czy wskaźnik po odjęciu rozmiaru bloku pamięci będzie się znajdował na początku bloku pamięci
        else if(get_pointer_type((void *)check - sizeof(struct heap_block)) == pointer_control_block)
        {
            //Sprawdzenie czy jest odpowiednio dużo miejsca na alokacje
            //Czy wskaźnik nastepny odjąć wskaźnik check jest równy rządanemu rozmiarowi i czy jest większy od podanego rozmiaru + rozmiar struktury bloku pamięci
            if((intptr_t)temp->next - check == size || (intptr_t)temp->next - check > size + sizeof(struct heap_block))
            {
                //Zwraca 1 czyli nie trzeba rozszerzać sterty
                return 1;
            }
        }
    }
        //Inaczej zwracamy 0 ifnormacja o tym że należy rozszerzyć sterte
    else
    {
        return 0;
    }
}

//Sprawdzenie czy w stercie znajduje się miejsce gdzie można umieścić nowy blok pamięci
int check_heap_size(size_t number)
{
    //Sprawdzenie czy sterta istnieje i czy nie podano liczby mniejszej lub równej 0
    if(pages.init == 0 || number <= 0)
    {
        return 0;
    }
    struct heap_block *temp = pages.head;

    //Przechodzenie po kolenych blokach pamięci od head az do NULL(tail->prev == NULL)
    while(temp != NULL)
    {
        //Jeśli w bloku pamięci jest rozmiar mniejszy od 0 (czyli wolny) idź dalej
        if(temp->block_size < 0)
        {
            //Jeśli w bloku pamięci jest rozmiar który zdoła pomieścić podaną liczbę + rozmiar struktury zwróć 1
            //Sam block możę już istnieć ale zostal wcześniej zwolniony czyli rozmiar bloku jest juz uwzględniony
            if(-(temp->block_size) >= (number + sizeof(struct heap_block)) || -(temp->block_size) == number)
            {
                return 1;
            }
        }
        //Sprzejście do kolejnego bloku pamięci
        temp = temp->next;
    }
    return 0;
}

int add_to_word(size_t number)
{
    size_t size;
    for(size = number;size % sizeof(void *) != 0;size++); //Zaokrąglenie do słowa maszynowego
    return size;
}
