#include <stdio.h>
#include <unistd.h>
#include "sterta.c"
#include "sterta.h"


void *thread1(void *arg)
{
    usleep(2000);
    void* help1 = heap_malloc(sizeof(int)*10);
    printf("Udalo sie zaalokowac w pierwszym watku\n");

    usleep(30000);
    heap_free(help1);
    printf("Udalo sie zwolnic pamiec w pierwszym watku\n");
    return NULL;
}

void *thread2(void *arg)
{
    usleep(4000);
    void* help2 = heap_realloc(NULL, sizeof(char)*50);
    printf("Udalo sie zaalokowac w drugim watku\n");
    heap_dump_debug_information();
    printf("\n");

    usleep(10000);
    heap_free(help2);
    printf("Udalo sie zwolnic pamiec w drugim watku\n");
    return NULL;
}

void *thread3(void *arg)
{
    usleep(1000);
    void* help3 = heap_calloc(sizeof(long), 32);
    printf("Udalo sie zaalokowac w trzecim watku\n");

    usleep(40000);
    heap_free(help3);
    printf("Udalo sie zwolnic pamiec w trzecim watku\n");
    return NULL;
}


void *thread4(void *arg)
{
    usleep(3000);
    void* help1 = heap_malloc_aligned(sizeof(int)*10);
    printf("Udalo sie zaalokowac w czwartym watku\n");

    usleep(30000);
    heap_free(help1);
    printf("Udalo sie zwolnic pamiec w czwartym watku\n");
    return NULL;
}

void *thread5(void *arg)
{
    usleep(1000);
    void* help2 = heap_realloc_aligned(NULL, sizeof(char)*50);
    printf("Udalo sie zaalokowac w piatym watku\n");

    usleep(4000);
    heap_free(help2);
    printf("Udalo sie zwolnic pamiec w piatym watku\n");
    return NULL;
}

void *thread6(void *arg)
{
    usleep(2000);
    void* help3 = heap_calloc_aligned(sizeof(long), 32);
    printf("Udalo sie zaalokowac w szostym watku\n");
    heap_dump_debug_information();
    printf("\n");
    usleep(60000);
    heap_free(help3);
    printf("Udalo sie zwolnic pamiec w szostym watku\n");
    return NULL;
}


/*
int main(int argc, char **argv) {
    int status = heap_setup();
    assert(status == 0);

    // parametry pustej sterty
    size_t free_bytes = heap_get_free_space();
    size_t used_bytes = heap_get_used_space();

    void* p1 = heap_malloc(8 * 1024 * 1024); // 8MB
    void* p2 = heap_malloc(8 * 1024 * 1024); // 8MB
    void* p3 = heap_malloc(8 * 1024 * 1024); // 8MB
    void* p4 = heap_malloc(45 * 1024 * 1024); // 45MB
    assert(p1 != NULL); // malloc musi się udać
    assert(p2 != NULL); // malloc musi się udać
    assert(p3 != NULL); // malloc musi się udać
    assert(p4 == NULL); // nie ma prawa zadziałać

    // Ostatnia alokacja, na 45MB nie może się powieść,
    // ponieważ sterta nie może być aż tak
    // wielka (brak pamięci w systemie operacyjnym).

    status = heap_validate();
    assert(status == 0); // sterta nie może być uszkodzona

    // zaalokowano 3 bloki
    assert(heap_get_used_blocks_count() == 3);

    // zajęto 24MB sterty; te 2000 bajtów powinno
    // wystarczyć na wewnętrzne struktury sterty
    assert(
            heap_get_used_space() >= 24 * 1024 * 1024 &&
            heap_get_used_space() <= 24 * 1024 * 1024 + 2000
    );

    // zwolnij pamięć
    heap_free(p1);
    heap_free(p2);
    heap_free(p3);

    // wszystko powinno wrócić do normy
    assert(heap_get_free_space() == free_bytes);
    assert(heap_get_used_space() == used_bytes);

    // już nie ma bloków
    assert(heap_get_used_blocks_count() == 0);

    return 0;
}
*/

int main()
{
    int test = 1;

    printf("TEST %d - Inicjalizacja stery\n", test);
    assert(heap_setup() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    pages.init = 0;
    void *wsk = heap_malloc(sizeof(int));

    printf("TEST %d - Niezainicjaloizowana sterta\n", test);
    assert(get_pointer_type(wsk) == pointer_null);
    assert(check_sum_function((struct heap_block *)(void *)(wsk - sizeof(struct heap_block))) == 0);
    assert(check_sum_for_control() == 0);
    assert(add_new_page(PAGE_SIZE) == -1);
    assert(heap_malloc(sizeof(int)) == NULL);
    assert(heap_calloc(sizeof(int), 4) == NULL);
    assert(heap_realloc(wsk, sizeof(int) * 4) == NULL);
    assert(heap_malloc_debug(sizeof(int) * 4, __LINE__, __FILE__) == NULL);
    assert(heap_calloc_debug(sizeof(int), 4, __LINE__, __FILE__) == NULL);
    assert(heap_realloc_debug(wsk, sizeof(int) * 4, __LINE__, __FILE__) == NULL);
    assert(heap_malloc_aligned(sizeof(int) * 4) == NULL);
    assert(heap_calloc_aligned(sizeof(int), 4) == NULL);
    assert(heap_realloc_aligned(wsk, sizeof(int) * 4) == NULL);
    assert(heap_malloc_aligned_debug(sizeof(int) * 4, __LINE__, __FILE__) == NULL);
    assert(heap_calloc_aligned_debug(sizeof(int), 4, __LINE__, __FILE__) == NULL);
    assert(heap_realloc_aligned_debug(wsk, sizeof(int) * 4, __LINE__, __FILE__) == NULL);
    heap_free(wsk);
    assert(heap_get_used_space() == 0);
    assert(heap_get_used_blocks_count() == 0);
    assert(heap_get_block_size(wsk) == 0);
    assert(heap_get_free_space() == 0);
    assert(heap_get_free_gaps_count() == 0);
    assert(heap_get_largest_free_area() == 0);
    assert(heap_get_largest_used_block_size() == 0);
    assert(heap_get_data_block_start(wsk) == NULL);
    assert(heap_get_block_size(wsk) == 0);
    printf("Funkcja 'heap_validate' dla niezainicjalizowanej sterty - (%d)\n", heap_validate());
    assert(heap_validate() == -1);
    printf("TEST WYKONANY\n\n");
    test++;

    pages.init = 1;
    heap_free(wsk);

    printf("TEST %d - Alokacja za pomoca malloc(0)\n", test);
    assert(heap_malloc(0) == NULL);
    printf("Funkcja 'heap_validate' po malloc(0) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc(sizeof(int));
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(int))\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(int)) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc(sizeof(int) * 4);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(int) * 4)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(int) * 4) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc(sizeof(int) * 8);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(int) * 8)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(int) * 8) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    for(int i=0;i<8;i++)
    {
        *((int*)wsk+i) = i;
    }

    for(int i=0;i<8;i++)
    {
        printf("%d ", *((int*)wsk+i));
    }
    printf("\n");
    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc(sizeof(char));
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(char))\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(char)) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc(sizeof(char) * 3);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(char) * 3)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(char) * 3) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    for(int i=0;i<3;i++)
    {
        *((char*)wsk+i) = i + 'A';
    }

    for(int i=0;i<3;i++)
    {
        printf("%c ", *((char*)wsk+i));
    }
    printf("\n");
    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc(sizeof(char) * 4);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(char) * 4)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(char) * 4) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc(sizeof(char) * 6);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(char) * 6)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(char) * 6) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    for(int i=0;i<6;i++)
    {
        *((char*)wsk+i) = i + 'A';
    }

    for(int i=0;i<6;i++)
    {
        printf("%c ", *((char*)wsk+i));
    }
    printf("\n");
    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc(sizeof(char) * 8);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(char) * 8)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(char) * 8) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc(20*1024);
    printf("TEST %d - Alokacja za pomoca malloc(20*1024)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(20*1024) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    heap_dump_debug_information();
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;


    printf("TEST %d - Alokacja za pomoca calloc(0, 0)\n", test);
    assert(heap_calloc(0, 0) == NULL);
    printf("Funkcja 'heap_validate' po calloc(0, 0) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    printf("TEST %d - Alokacja za pomoca calloc(sizeof(int), 0)\n", test);
    assert(heap_calloc(sizeof(int), 0) == NULL);
    printf("Funkcja 'heap_validate' po calloc(sizeof(int), 0) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    printf("TEST %d - Alokacja za pomoca calloc(0, 20)\n", test);
    assert(heap_calloc(0, 20) == NULL);
    printf("Funkcja 'heap_validate' po calloc(0, 20) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc(sizeof(int), 1);
    printf("TEST %d - Alokacja za pomoca calloc(sizeof(int), 1)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(sizeof(int), 1) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc(sizeof(int), 4);
    printf("TEST %d - Alokacja za pomoca calloc(sizeof(int), 4)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(sizeof(int), 4) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    for(int i=0;i<4;i++)
    {
        printf("%d ", *((int*)wsk+i));
    }
    printf("\n");
    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc(sizeof(int), 8);
    printf("TEST %d - Alokacja za pomoca calloc(sizeof(int), 8)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(sizeof(int), 8) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc(sizeof(char), 1);
    printf("TEST %d - Alokacja za pomoca calloc(sizeof(char), 1)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(sizeof(char), 1) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc(sizeof(char), 3);
    printf("TEST %d - Alokacja za pomoca calloc(sizeof(char), 3)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(sizeof(char), 3) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc(sizeof(char), 10);
    printf("TEST %d - Alokacja za pomoca calloc(sizeof(char), 10)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(sizeof(char), 10) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    for(int i=0;i<10;i++)
    {
        *((char*)wsk+i) = 'Z' - i;
    }

    for(int i=0;i<10;i++)
    {
        printf("%c ", *((char*)wsk+i));
    }
    printf("\n");
    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc(67, 1024);
    printf("TEST %d - Alokacja za pomoca calloc(67, 1024)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(67, 1024) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    heap_dump_debug_information();
    printf("\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    printf("TEST %d - Alokacja za pomoca realloc(NULL, 0)\n", test);
    assert(heap_realloc(NULL, 0) == NULL);
    printf("Funkcja 'heap_validate' po realloc(NULL, 0)- (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_realloc(NULL, sizeof(int));
    printf("TEST %d - Alokacja za pomoca realloc(NULL, sizeof(int))\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po realloc(NULL, sizeof(int)) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_realloc(NULL, sizeof(int) * 16);
    printf("TEST %d - Alokacja za pomoca realloc(NULL, sizeof(int) * 16)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po realloc(NULL, sizeof(int) * 16) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    for(int i=0;i<16;i++)
    {
        *((int*)wsk+i) = i + 102;
    }

    for(int i=0;i<16;i++)
    {
        printf("%d ", *((int*)wsk+i));
    }
    printf("\n");
    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_realloc(NULL, sizeof(int) * 64);
    printf("TEST %d - Alokacja za pomoca realloc(NULL, sizeof(int) * 64)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po realloc(NULL, sizeof(int) * 64) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_realloc(NULL, sizeof(char) * 33);
    printf("TEST %d - Alokacja za pomoca realloc(NULL, sizeof(char) * 33)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po realloc(NULL, sizeof(char) * 33) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    char name[33] = "Jest to napis testowy";
    for(int i=0;i<33;i++)
    {
        if(name[i] == '\0') break;
        *((char*)wsk+i) = name[i];
    }

    for(int i=0;i<33;i++)
    {
        if(name[i] == '\0') break;
        printf("%c ", *((char*)wsk+i));
    }
    printf("\n");
    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_realloc(NULL, sizeof(char) * 341);
    printf("TEST %d - Alokacja za pomoca realloc(NULL, sizeof(char) * 341)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po realloc(NULL, sizeof(char) * 341) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    char name2[341] = "  Stoi na stacji lokomotywa,\nCiężka, ogromna i pot z niej spływa,\nTłusta oliwa.\nStoi i sapie, dyszy i dmucha,\nŻar z rozgrzanego jej brzucha bucha\n";
    for(int i=0;i<341;i++)
    {
        if(name2[i] == '\0') break;
        *((char*)wsk+i) = name[i];
    }

    for(int i=0;i<341;i++)
    {
        if(name2[i] == '\0') break;
        printf("%c", *((char*)wsk+i));
    }
    printf("\n");
    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc(sizeof(int), 50);
    printf("TEST %d - Alokacja za pomoca calloc(sizeof(int), 50)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(sizeof(int), 50) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    void* temp = heap_realloc(wsk, sizeof(int) * 64);
    printf("TEST %d - Relokacja za pomoca realloc(wsk, sizeof(int) * 64)\n", test);
    assert(temp != NULL);
    printf("Funkcja 'heap_validate' po realloc(wsk, sizeof(int) * 64) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_realloc(NULL, sizeof(char) * 512);
    printf("TEST %d - Alokacja za pomoca realloc(NULL, sizeof(char) * 512)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po realloc(NULL, sizeof(char) * 512) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp = heap_realloc(wsk, sizeof(char) * 1024);
    printf("TEST %d - Relokacja za pomoca realloc(wsk, sizeof(char) * 1024)\n", test);
    assert(temp != NULL);
    printf("Funkcja 'heap_validate' po realloc(wsk, sizeof(char) * 1024) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(temp);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;


    wsk = heap_malloc_debug(sizeof(int) * 2, __LINE__, NULL);
    printf("TEST %d - Alokacja za pomoca malloc_debug(sizeof(int) * 2, __LINE__, NULL)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po malloc_debug(sizeof(int) * 2, __LINE__, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc_debug(sizeof(int) * 10, 0, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_debug(sizeof(int) * 10, 0, __FILE__)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po malloc_debug(sizeof(int) * 10, 0, __FILE__) - (%d)\n", heap_validate());
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc_debug(sizeof(int) * 22, 0, NULL);
    printf("TEST %d - Alokacja za pomoca malloc_debug(sizeof(int) * 22, 0, NULL)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po malloc_debug(sizeof(int) * 22, 0, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc_debug(0, 0, NULL);
    printf("TEST %d - Alokacja za pomoca malloc_debug(0, 0, NULL)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po malloc_debug(0, 0, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc_debug(0, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_debug(0, __LINE__, __FILE__)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po malloc_debug(0, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc_debug(sizeof(int) * 43, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_debug(sizeof(int) * 43, __LINE__, __FILE__)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po malloc_debug(sizeof(int) * 43, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc_debug(sizeof(int) * 54, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_debug(sizeof(int) * 54, __LINE__, __FILE__)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po malloc_debug(sizeof(int) * 54, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;


    wsk = heap_calloc_debug(sizeof(int), 2, __LINE__, NULL);
    printf("TEST %d - Alokacja za pomoca calloc_debug(sizeof(int), 2, __LINE__, NULL)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_debug(sizeof(int), 2, __LINE__, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_debug(sizeof(int), 10, 0, NULL);
    printf("TEST %d - Alokacja za pomoca calloc_debug(sizeof(int), 10, 0, NULL)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_debug(sizeof(int), 10, 0, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_debug(sizeof(int), 0, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_debug(sizeof(int), 0, __LINE__, __FILE__)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_debug(sizeof(int), 0, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_debug(0, 0, 0, NULL);
    printf("TEST %d - Alokacja za pomoca calloc_debug(0, 0, 0, NULL)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_debug(0, 0, 0, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_debug(0, 10, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_debug(0, 10, __LINE__, __FILE__)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_debug(0, 10, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_debug(sizeof(int), 43, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_debug(sizeof(int), 43, __LINE__, __FILE__)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po calloc_debug(sizeof(int), 43, __LINE__, __FILE__) - (%d)\n", heap_validate());
    heap_dump_debug_information();
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_debug(sizeof(char), 54, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_debug(sizeof(char), 54, __LINE__, __FILE__)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po calloc_debug(sizeof(char), 54, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc(sizeof(char), 67);
    printf("TEST %d - Alokacja za pomoca calloc(sizeof(char), 67)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(sizeof(char), 67) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp = heap_realloc_debug(wsk, sizeof(char) * 100, 0, __FILE__);
    printf("TEST %d - Reokacja za pomoca realloc_debug(wsk, sizeof(char) * 100, 0, __FILE__)\n", test);
    assert(temp == NULL);
    printf("Funkcja 'heap_validate' po realloc_debug(wsk, sizeof(char) * 100, 0, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    temp = heap_realloc_debug(wsk, sizeof(char) * 100, __LINE__, NULL);
    printf("TEST %d - Reokacja za pomoca realloc_debug(wsk, sizeof(char) * 100, __LINE__, NULL)\n", test);
    assert(temp == NULL);
    printf("Funkcja 'heap_validate' po realloc_debug(wsk, sizeof(char) * 100, __LINE__, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    temp = heap_realloc_debug(wsk, 0, __LINE__, __FILE__);
    printf("TEST %d - Reokacja za pomoca realloc_debug(wsk, 0, __LINE__, __FILE__)\n", test);
    assert(temp == NULL);
    printf("Funkcja 'heap_validate' po realloc_debug(wsk, 0, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    temp = heap_realloc_debug(wsk, sizeof(char) * 100, __LINE__, __FILE__);
    printf("TEST %d - Reokacja za pomoca realloc_debug(wsk, sizeof(char) * 100, __LINE__, __FILE__)\n", test);
    assert(temp != NULL);
    printf("Funkcja 'heap_validate' po realloc_debug(wsk, sizeof(char) * 100, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(temp);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");


















    wsk = heap_malloc_aligned(0);
    printf("TEST %d - Alokacja za pomoca malloc_aligned(0)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po malloc_aligned(0) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;


    wsk = heap_malloc_aligned(sizeof(int) * 20);
    printf("TEST %d - Alokacja za pomoca malloc_aligned(sizeof(int) * 20)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po malloc_aligned(sizeof(int) * 20) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_malloc_aligned(sizeof(char) * 43);
    printf("TEST %d - Alokacja za pomoca malloc_aligned(sizeof(char) * 43)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po malloc_aligned(sizeof(char) * 43) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    char name3[33] = "Jest to napis testowy";
    for(int i=0;i<33;i++)
    {
        if(name3[i] == '\0') break;
        *((char*)wsk+i) = name[i];
    }

    for(int i=0;i<33;i++)
    {
        if(name3[i] == '\0') break;
        printf("%c", *((char*)wsk+i));
    }
    printf("\n");
    printf("\n");
    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_malloc_aligned(sizeof(char) * 2000);
    printf("TEST %d - Alokacja za pomoca malloc_aligned(sizeof(char) * 2000)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po malloc_aligned(sizeof(char) * 2000) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("%d\n\n", get_pointer_type(wsk));
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_calloc_aligned(0, 0);
    printf("TEST %d - Alokacja za pomoca calloc_aligned(0, 0)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned(0, 0) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_aligned(sizeof(short), 0);
    printf("TEST %d - Alokacja za pomoca calloc_aligned(sizeof(short), 0)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned(sizeof(short), 0) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_aligned(0, 43);
    printf("TEST %d - Alokacja za pomoca calloc_aligned(0, 43)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned(0, 43) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_aligned(sizeof(int), 24);
    printf("TEST %d - Alokacja za pomoca calloc_aligned(sizeof(int), 24)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned(sizeof(int), 24) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    for(int i=0;i<24;i++)
    {
        printf("%d ", *((int*)wsk+i));
    }

    printf("\n");
    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_calloc_aligned(sizeof(char), 12);
    printf("TEST %d - Alokacja za pomoca calloc_aligned(sizeof(char), 12)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned(sizeof(char), 12) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_realloc_aligned(NULL, 0);
    printf("TEST %d - Alokacja za pomoca relloc_aligned(0, 0)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po relloc_aligned(0, 0) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    wsk = heap_realloc_aligned(NULL, sizeof(int) * 25);
    printf("TEST %d - Alokacja za pomoca realloc_aligned(NULL, sizeof(int) * 25)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po realloc_aligned(NULL, sizeof(int) * 25) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    void * wsk1 = heap_realloc_aligned(NULL, 165);
    printf("TEST %d - Alokacja za pomoca realloc_aligned(NULL, 165)\n", test);
    assert(wsk1 != NULL);
    printf("Funkcja 'heap_validate' po realloc_aligned(NULL, 165) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk1 & (intptr_t)(PAGE_SIZE -1)) == 0);

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_realloc_aligned(NULL, sizeof(char) * 12);
    printf("TEST %d - Alokacja za pomoca realloc_aligned(NULL, sizeof(char) * 12)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po realloc_aligned(NULL, sizeof(char) * 12) - (%d)\n", heap_validate());
    heap_dump_debug_information();
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    heap_free(wsk1);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk1) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_realloc_aligned(NULL, sizeof(long) * 12);
    printf("TEST %d - Alokacja za pomoca realloc_aligned(NULL, sizeof(long) * 12)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po realloc_aligned(NULL, sizeof(long) * 12) - (%d)\n", heap_validate());
    heap_dump_debug_information();
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    wsk1 = heap_realloc_aligned(wsk, sizeof(long) * 40);
    printf("TEST %d - Alokacja za pomoca realloc_aligned(wsk, sizeof(long) * 40)\n", test);
    assert(wsk1 != NULL);
    printf("Funkcja 'heap_validate' po realloc_aligned(wsk, sizeof(long) * 40) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk1 & (intptr_t)(PAGE_SIZE -1)) == 0);

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk1);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk1) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");



    wsk = heap_malloc_aligned_debug(0, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_aligned_debug(0, __LINE__, __FILE__);\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po malloc_aligned_debug(0, __LINE__, __FILE__); - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc_aligned_debug(sizeof(int)*98, 0, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_aligned_debug(sizeof(int)*98, 0, __FILE__)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po malloc_aligned_debug(sizeof(int)*98, 0, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc_aligned_debug(sizeof(int)*98, __LINE__, NULL);
    printf("TEST %d - Alokacja za pomoca malloc_aligned_debug(sizeof(int)*98, __LINE__, NULL)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po malloc_aligned_debug(sizeof(int)*98, __LINE__, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc_aligned_debug(0, 0, NULL);
    printf("TEST %d - Alokacja za pomoca malloc_aligned_debug(0, 0, NULL)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po malloc_aligned_debug(0, 0, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc_aligned_debug(sizeof(int) * 30, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_aligned_debug(sizeof(int) * 30, __LINE__, __FILE__)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po malloc_aligned_debug(sizeof(int) * 30, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_malloc_aligned_debug(sizeof(char) * 3004, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_aligned(sizeof(char) * 3004, __LINE__, __FILE__)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po malloc_aligned(sizeof(char) * 3004, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");


    wsk = heap_calloc_aligned_debug(0, 0, 0, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_aligned_debug(0, 0, 0, __FILE__)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned_debug(0, 0, 0, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_aligned_debug(0, 34, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_aligned_debug(0, 34, __LINE__, __FILE__)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned_debug(0, 34, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_aligned_debug(0, 0, 0, NULL);
    printf("TEST %d - Alokacja za pomoca calloc_aligned_debug(0, 0, 0, NULL)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned_debug(0, 0, 0, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_aligned_debug(sizeof(int), 0, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_aligned_debug(sizeof(int), 0, __LINE__, __FILE__)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned_debug(sizeof(int), 0, __LINE__, __FILE__)- (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_aligned_debug(sizeof(int), 34, __LINE__, NULL);
    printf("TEST %d - Alokacja za pomoca calloc_aligned_debug(sizeof(int), 34, __LINE__, NULL)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned_debug(sizeof(int), 34, __LINE__, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_aligned_debug(sizeof(char), 123, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_aligned_debug(sizeof(char), 123, __LINE__, __FILE__)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned_debug(sizeof(char), 123, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_calloc_aligned_debug(sizeof(char), 12, __LINE__, NULL);
    printf("TEST %d - Alokacja za pomoca calloc_aligned_debug(sizeof(char), 12, __LINE__, NULL)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned_debug(sizeof(char), 12, __LINE__, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_calloc_aligned_debug(sizeof(int), 22, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_aligned_debug(sizeof(char), 22, __LINE__, __FILE__)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po calloc_aligned_debug(sizeof(char), 22, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_realloc_aligned_debug(NULL, 0, 0, __FILE__);
    printf("TEST %d - Alokacja za pomoca realloc_aligned_debug(NULL, 0, 0, __FILE__)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po realloc_aligned_debug(NULL, 0, 0, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_realloc_aligned_debug(NULL, sizeof(int) * 2, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca realloc_aligned_debug(NULL, sizeof(int) * 2, __LINE__, __FILE__)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po realloc_aligned_debug(NULL, sizeof(int) * 2, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;


    wsk = heap_realloc_aligned_debug(NULL, 1024, __LINE__, NULL);
    printf("TEST %d - Alokacja za pomoca realloc_aligned_debug(NULL, 1024, __LINE__, NULL)\n", test);
    assert(wsk == NULL);
    printf("Funkcja 'heap_validate' po realloc_aligned_debug(NULL, 1024, __LINE__, NULL) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_realloc_aligned_debug(NULL, sizeof(char) * 65, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca realloc_aligned_debug(NULL, sizeof(char) * 65, __LINE__, __FILE__)\n", test);
    assert(wsk != NULL);
    printf("Funkcja 'heap_validate' po realloc_aligned_debug(NULL, sizeof(char) * 65, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    assert(((intptr_t)wsk & (intptr_t)(PAGE_SIZE -1)) == 0);

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;






    wsk = heap_malloc(sizeof(int) * 16);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(int) * 16)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(int) * 16) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    void* temp1 = heap_calloc(sizeof(char), 32);
    printf("TEST %d - Alokacja za pomoca calloc(sizeof(char) * 32)\n", test);
    assert(get_pointer_type(temp1) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(sizeof(char) * 32) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    void* temp2 = heap_realloc(NULL, sizeof(int) * 20);
    printf("TEST %d - Alokacja za pomoca realloc(sizeof(int) * 20)\n", test);
    assert(get_pointer_type(temp2) == pointer_valid);
    printf("Funkcja 'heap_validate' po realloc(sizeof(int) * 20) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp1);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp1) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp2);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp2) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");











    wsk = heap_malloc_debug(sizeof(int) * 16, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_debug(sizeof(int) * 16, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc_debug(sizeof(int) * 16, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp1 = heap_calloc_debug(sizeof(char), 32, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_debug(sizeof(char), 32, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(temp1) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc_debug(sizeof(char), 32, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp2 = heap_realloc_debug(wsk, sizeof(int) * 20, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca realloc_debug(wsk, sizeof(int) * 20, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(temp2) == pointer_valid);
    printf("Funkcja 'heap_validate' po realloc_debug(wsk, sizeof(int) * 20, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp1);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp1) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp2);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp2) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

















    wsk = heap_malloc(sizeof(int) * 30);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(int) * 30)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(int) * 30) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp1 = heap_calloc_debug(sizeof(char), 11, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_debug(sizeof(char), 11, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(temp1) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc_debug(sizeof(char), 11, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp2 = heap_malloc_aligned(sizeof(int) * 40);
    printf("TEST %d - Alokacja za pomoca malloc_aligned(sizeof(int) * 40)\n", test);
    assert(get_pointer_type(temp2) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc_aligned(sizeof(int) * 40) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_malloc(sizeof(char) * 30);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(char) * 30)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(char) * 30) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    void* temp3 = heap_calloc_aligned_debug(sizeof(char), 102, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_aligned_debug(sizeof(char), 102, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(temp3) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc_aligned_debug(sizeof(char), 102, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(temp2);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp2) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp3);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp3) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp1);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp1) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

















    temp1 = heap_calloc(sizeof(int), 150);
    printf("TEST %d - Alokacja za pomoca calloc(sizeof(int), 150)\n", test);
    assert(get_pointer_type(temp1) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(sizeof(int), 150) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_malloc_aligned_debug(sizeof(char) * 1022, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_aligned_debug(sizeof(char) * 1022, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc_aligned_debug(sizeof(char) * 1022, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp2 = heap_realloc_debug(NULL, sizeof(short)*400, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca realloc_debug(sizeof(short), 400, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(temp2) == pointer_valid);
    printf("Funkcja 'heap_validate' po realloc_debug(sizeof(short), 400, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(temp1);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp1) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp3 = heap_realloc(wsk, sizeof(char)*300);
    printf("TEST %d - Alokacja za pomoca realloc(wsk, sizeof(char)*300)\n", test);
    assert(get_pointer_type(temp3) == pointer_valid);
    printf("Funkcja 'heap_validate' po realloc(wsk, sizeof(char)*300) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp3);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp3) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp2);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp2) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");
















    wsk = heap_malloc_aligned_debug(sizeof(int) * 1050, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_aligned_debug(sizeof(int) * 1050, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc_aligned_debug(sizeof(int) * 1050, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp3 = heap_realloc_debug(NULL, sizeof(long)*10, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca realloc_debug(NULL, sizeof(long)*10, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(temp3) == pointer_valid);
    printf("Funkcja 'heap_validate' po realloc_debug(NULL, sizeof(long)*10, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp2 = heap_calloc_aligned(sizeof(char), 200);
    printf("TEST %d - Alokacja za pomoca calloc_aligned(sizeof(char) * 200)\n", test);
    assert(get_pointer_type(temp2) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc_aligned(sizeof(char) * 200) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_malloc(sizeof(int)*1);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(int)*1)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(int)*1) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp1 = heap_calloc_debug(sizeof(int), 12, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_debug(sizeof(int), 12, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(temp1) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc_debug(sizeof(int), 12, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(temp2);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp2) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp3);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp3) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp1);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp1) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");



















    temp3 = heap_malloc_debug(sizeof(int)*654, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_debug(sizeof(int)*654, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(temp3) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc_debug(sizeof(int)*654, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");


    temp1 = heap_calloc_aligned_debug(sizeof(int), 43, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_aligned_debug(sizeof(int), 43, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(temp1) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc_aligned_debug(sizeof(int), 43, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_realloc(NULL, sizeof(char)*20);
    printf("TEST %d - Alokacja za pomoca realloc(NULL, sizeof(char)*20)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po realloc(NULL, sizeof(char)*20) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp2 = heap_malloc(sizeof(short)*291);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(short)*291)\n", test);
    assert(get_pointer_type(temp2) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(short)*291)- (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp1);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp1) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_realloc_debug(temp2, sizeof(short)*293, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca realloc_debug(temp2, sizeof(short)*293, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po realloc_debug(temp2, sizeof(short)*293, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(temp2);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp2) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp3);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp3) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");




















    temp2 = heap_malloc_aligned(sizeof(long)*120);
    printf("TEST %d - Alokacja za pomoca malloc_aligned(sizeof(long)*120)\n", test);
    assert(get_pointer_type(temp2) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc_aligned(sizeof(long)*120)- (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_calloc_aligned_debug(sizeof(char), 10, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_aligned_debug(sizeof(char), 10, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc_aligned_debug(sizeof(char), 10, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp3 = heap_realloc_debug(wsk, sizeof(char)*100, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca realloc_debug(wsk, sizeof(char)*100, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(temp3) == pointer_valid);
    printf("Funkcja 'heap_validate' po realloc_debug(wsk, sizeof(char)*100, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp1 = heap_realloc_aligned_debug(NULL, sizeof(short)*2, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca realloc_aligned_debug(NULL, sizeof(short)*2, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(temp1) == pointer_valid);
    printf("Funkcja 'heap_validate' po realloc_aligned_debug(NULL, sizeof(short)*2, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp3);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp3) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp2);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp2) == pointer_unallocated);
     printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp1);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp1) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");













    temp3 = heap_calloc(sizeof(long), 3);
    printf("TEST %d - Alokacja za pomoca calloc(sizeof(long), 3)\n", test);
    assert(get_pointer_type(temp3) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(sizeof(long), 3) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_calloc_aligned_debug(40, 40, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca calloc_aligned_debug(40, 40, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc_aligned_debug(40, 40, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp1 = heap_realloc_aligned(temp3, sizeof(long)*15);
    printf("TEST %d - Alokacja za pomoca realloc_aligned(temp3, sizeof(long)*15)\n", test);
    assert(get_pointer_type(temp1) == pointer_valid);
    printf("Funkcja 'heap_validate' po realloc_aligned(temp3, sizeof(long)*15) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    wsk = heap_malloc_debug(sizeof(char)*100, __LINE__, __FILE__);
    printf("TEST %d - Alokacja za pomoca malloc_debug(sizeof(char)*100, __LINE__, __FILE__)\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc_debug(sizeof(char)*100, __LINE__, __FILE__) - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(temp3);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp3) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp1);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp1) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    temp2 = heap_malloc(sizeof(long)*700);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(long)*700)\n", test);
    assert(get_pointer_type(temp2) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(long)*700)- (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp2);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp2) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");













    pages.right_control_fence = 1;
    printf("TEST %d - Zly prawy plotek sterty\n", test);
    assert(heap_validate() == -2);
    printf("TEST WYKONANY\n\n");
    pages.right_control_fence = 0;
    test++;

    pages.left_control_fence = 1;
    printf("TEST %d - Zly lewy plotek sterty\n", test);
    assert(heap_validate() == -3);
    printf("TEST WYKONANY\n\n");
    pages.left_control_fence = 0;
    test++;

    pages.check_sum = 0;
    printf("TEST %d - Zly suma kontrolna sterty\n", test);
    assert(heap_validate() == -4);
    printf("TEST WYKONANY\n\n");
    pages.check_sum = check_sum_for_control();
    test++;

    struct heap_block *temp_s = pages.head;
    pages.head = NULL;
    pages.check_sum = check_sum_for_control();
    printf("TEST %d - Zla glowa sterty\n", test);
    assert(heap_validate() == -5);
    printf("TEST WYKONANY\n\n");
    pages.head = temp_s;
    pages.check_sum = check_sum_for_control();
    test++;

    temp_s = pages.tail;
    pages.tail = NULL;
    pages.check_sum = check_sum_for_control();
    printf("TEST %d - Zly ogon sterty\n", test);
    assert(heap_validate() == -6);
    printf("TEST WYKONANY\n\n");
    pages.tail = temp_s;
    pages.check_sum = check_sum_for_control();
    test++;

    temp_s = pages.tail;
    pages.head->prev = temp_s;
    pages.check_sum = check_sum_for_control();
    printf("TEST %d - Wskaznik head->prev == NULL\n", test);
    assert(heap_validate() == -7);
    printf("TEST WYKONANY\n\n");
    pages.head->prev = NULL;
    pages.check_sum = check_sum_for_control();
    test++;

    temp_s = pages.head;
    pages.tail->next = temp_s;
    pages.check_sum = check_sum_for_control();
    printf("TEST %d - Wskaznik tail->next == NULL\n", test);
    assert(heap_validate() == -8);
    printf("TEST WYKONANY\n\n");
    pages.tail->next = NULL;
    pages.check_sum = check_sum_for_control();
    test++;

    wsk = heap_malloc(sizeof(int)*8);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(int)*8);\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(int)*8); - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    temp_s = pages.head->next;
    temp_s->prev = NULL;
    temp_s->check_sum = check_sum_function(temp_s);
    pages.check_sum = check_sum_for_control();
    printf("TEST %d - Wskaznik temp->prev == NULL\n", test);
    assert(heap_validate() == -9);
    printf("TEST WYKONANY\n\n");
    temp_s->prev = pages.head;
    temp_s->check_sum = check_sum_function(temp_s);
    pages.check_sum = check_sum_for_control();
    test++;

    temp1 = heap_calloc(sizeof(char), 8);
    printf("TEST %d - Alokacja za pomoca calloc(sizeof(int)*8);\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po calloc(sizeof(int)*8); - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    temp_s = pages.head->next;
    struct heap_block *temp_s12 = temp_s->next;
    temp_s->next = NULL;
    temp_s->check_sum = check_sum_function(temp_s);
    temp_s12->check_sum = check_sum_function(temp_s12);
    pages.check_sum = check_sum_for_control();
    printf("TEST %d - Wskaznik temp->next == NULL\n", test);
    assert(heap_validate() == -10);
    printf("TEST WYKONANY\n\n");
    temp_s->next = temp_s12;
    temp_s->check_sum = check_sum_function(temp_s);
    temp_s12->check_sum = check_sum_function(temp_s12);
    pages.check_sum = check_sum_for_control();
    test++;

    temp_s = pages.head->next;
    temp_s->left_control_fence = 1;
    printf("TEST %d - Zly lewy plotek bloku pamieci\n", test);
    assert(heap_validate() == -11);
    printf("TEST WYKONANY\n\n");
    temp_s->left_control_fence = 0;
    test++;

    temp_s = pages.head->next;
    temp_s->right_control_fence = 1;
    printf("TEST %d - Zly prawy plotek bloku pamieci\n", test);
    assert(heap_validate() == -12);
    printf("TEST WYKONANY\n\n");
    temp_s->right_control_fence = 0;
    test++;

    temp_s = pages.head->next;
    temp_s->check_sum = 0;
    printf("TEST %d - Zla suma kontrolna bloku pamieci\n", test);
    assert(heap_validate() == -13);
    printf("TEST WYKONANY\n\n");
    temp_s->check_sum = check_sum_for_control(temp_s);
    test++;

    temp_s = pages.head->next;
    int temp_size = temp_s->block_size;
    temp_s->block_size = 0;
    temp_s->check_sum = check_sum_function(temp_s);
    pages.check_sum = check_sum_for_control();
    printf("TEST %d - Zly rozmiar bloku pamieci\n", test);
    assert(heap_validate() == -14);
    printf("TEST WYKONANY\n\n");
    temp_s->block_size = temp_size;
    temp_s->check_sum = check_sum_function(temp_s);
    pages.check_sum = check_sum_for_control();
    test++;

    temp_s = pages.head->next;
    struct heap_block *temp_s2 = temp_s->next;
    temp_size = temp_s->block_size;
    int temp_size2 = temp_s2->block_size;
    temp_s->block_size = -5;
    temp_s2->block_size = -5;
    temp_s->check_sum = check_sum_function(temp_s);
    temp_s2->check_sum = check_sum_function(temp_s2);
    pages.check_sum = check_sum_for_control();
    printf("TEST %d - Dwa wolne bloki obok siebie\n", test);
    assert(heap_validate() == -15);
    printf("TEST WYKONANY\n\n");
    temp_s->block_size = temp_size;
    temp_s2->block_size = temp_size2;
    temp_s->check_sum = check_sum_function(temp_s);
    temp_s2->check_sum = check_sum_function(temp_s2);
    pages.check_sum = check_sum_for_control();
    test++;

    heap_dump_debug_information();
    printf("\n");
    
    temp_s = pages.head->next;
    temp_size = temp_s->block_size;
    temp_s->block_size = 12556;
    temp_s->check_sum = check_sum_function(temp_s);
    pages.check_sum = check_sum_for_control();
    printf("TEST %d - Zly rozmiar sterty\n", test);
    assert(heap_validate() == -16);
    printf("TEST WYKONANY\n\n");
    temp_s->block_size = temp_size;
    temp_s->check_sum = check_sum_function(temp_s);
    pages.check_sum = check_sum_for_control();
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(temp1);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(temp1) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    printf("TEST %d - get_pointer_type dla pointer_null\n", test);
    assert(get_pointer_type(NULL) == pointer_null);
    printf("TEST WYKONANY\n\n");
    test++;

    wsk = heap_malloc(sizeof(int)*8);
    printf("TEST %d - Alokacja za pomoca malloc(sizeof(int)*8);\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("Funkcja 'heap_validate' po malloc(sizeof(int)*8); - (%d)\n", heap_validate());
    assert(heap_validate() == 0);
    printf("TEST WYKONANY\n\n");
    test++;

    printf("TEST %d - get_pointer_type dla pointer_out_of_heap\n", test);
    assert(get_pointer_type(wsk+4096) == pointer_out_of_heap);
    printf("TEST WYKONANY\n\n");
    test++;

    printf("TEST %d - get_pointer_type dla pointer_control_block\n", test);
    assert(get_pointer_type(wsk - sizeof(struct heap_block)) == pointer_control_block);
    printf("TEST WYKONANY\n\n");
    test++;

    printf("TEST %d - get_pointer_type dla pointer_valid\n", test);
    assert(get_pointer_type(wsk) == pointer_valid);
    printf("TEST WYKONANY\n\n");
    test++;

    printf("TEST %d - get_pointer_type dla pointer_inside_data_block\n", test);
    assert(get_pointer_type(wsk + 1) == pointer_inside_data_block);
    printf("TEST WYKONANY\n\n");
    test++;

    heap_free(wsk);
    printf("TEST %d - Zwalnianie alokacji\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;

    printf("TEST %d - get_pointer_type dla pointer_unallocated\n", test);
    assert(get_pointer_type(wsk) == pointer_unallocated);
    printf("TEST WYKONANY\n\n");
    test++;



    printf("TEST %d - watki - 1\n", test);

    pthread_t th1, th2, th3;
    pthread_create(&th1, NULL, thread1, NULL);
    pthread_create(&th2, NULL, thread2, NULL);
    pthread_create(&th3, NULL, thread3, NULL);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    pthread_join(th3, NULL);

    printf("TEST WYKONANY\n\n");
    test++;

    heap_dump_debug_information();
    printf("\n");

    
    printf("TEST %d - watki - 2\n", test);

    pthread_t th4, th5, th6;
    pthread_create(&th4, NULL, thread4, NULL);
    pthread_create(&th5, NULL, thread5, NULL);
    pthread_create(&th6, NULL, thread6, NULL);

    pthread_join(th4, NULL);
    pthread_join(th5, NULL);
    pthread_join(th6, NULL);

    printf("TEST WYKONANY\n\n");
    test++;

    //heap_dump_debug_information();
    //printf("\n");

    void *ptr1 = heap_malloc(1000);
    void *ptr2 = heap_malloc(2000);
    void *ptr3 = heap_malloc(1000);

    heap_free(ptr2);
    ptr2 = heap_malloc(1990 - sizeof(struct heap_block));

    heap_dump_debug_information();
    printf("\n");

    
    char *ptr = heap_malloc(100);
    ptr[-7] = 100;
    assert(heap_validate() == -12);
    
    return 0;
}
