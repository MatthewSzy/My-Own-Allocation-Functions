#ifndef MATI_STERTA_H
#define MATI_STERTA_H

#include <stdint.h>
#include "custom_unistd.h"

/**
 * pointer_type_t informuje co znajduje sie pod podanym wskaźnikiem
 * pointer_null podany wskaźnik jest pusty
 * pointer_out_of_heap podany wskaźnik poza stertą
 * pointer_control_block podany wskaźnik jest w gdzies w stercie ale jest w strukturze wewnętrznej
 * pointer_inside_data_block podany wskaźnik jest w zaalokowanym miejscu
 * pointer_unallocated podany wskaźnik jest w miejscu w którym nie dokonala się alokacja
 * pointer_valid podany wskaźnik jest poprawny
 */
enum pointer_type_t
{
    pointer_null,
    pointer_out_of_heap,
    pointer_control_block,
    pointer_inside_data_block,
    pointer_unallocated,
    pointer_valid
};

/**
 * heap_block to struktura przechowująca zaalokowaną część pamięć działa jak fragment listy dwukierunkowej
 * ----------------------------------------------------------------
 * left_control_fence - blok kontrolny z lewej strony na początku
 * next - wskaźnik do następnego bloku pamięci
 * prev - wskaźnik do poprzedniego blocku pamięci
 * block_size - rozmiar przydzielonej pamięci(ujemna kiedy jest pusta(wolna), dodatnia kiedy jest zajęta)
 * code_line - linia w której został została stworzona struktura
 * program_name - program w którym została stworzona struktura(sterta.c)
 * control_sum - wartosć określająca czy dokonały sie zmiany w bloku pamięci
 * right_control_fence - blok kontrolny z prawej strony na końcu
*/
struct heap_block
{
    int left_control_fence;
    struct heap_block *next;
    struct heap_block *prev;
    int block_size;
    int code_line;
    const char* program_name;
    size_t check_sum;
    int right_control_fence;
};

/**
 * heap_control to struktura która zawiera calą długość strony w której przydzielamy bloki pamięci
 * -----------------------------------------------------------------
 * left_control_fence - blok kontrolny z lewej strony na początku
 * head - wskazuje na pierwszą przydzieloną pamięć na stronie
 * tail - wskazuje na ostatnią przydzieloną pamięć na stronie
 * init - określa czy blok jest zainicjalizowany
 * control_sum - wartosć określająca czy dokonały sie zmiany w bloku pamięci
 * right_control_fence - blok kontrolny z prawej strony na końcu
*/
struct heap_control
{
    int left_control_fence;
    struct heap_block *head;
    struct heap_block *tail;
    int init;
    size_t check_sum;
    int right_control_fence;
};

int heap_setup(void);

void * heap_malloc(size_t count);
void * heap_calloc(size_t number, size_t size);
void heap_free(void * memblock);
void * heap_realloc(void * memblock, size_t size);

void * heap_malloc_debug(size_t count, int fileline, const char * filename);
void * heap_calloc_debug(size_t number, size_t size, int fileline, const char * filename);
void * heap_realloc_debug(void * memblock, size_t size, int fileline, const char * filename);

void * heap_malloc_aligned(size_t count);
void * heap_calloc_aligned(size_t number, size_t size);
void * heap_realloc_aligned(void * memblock, size_t size);

void * heap_malloc_aligned_debug(size_t count, int fileline, const char * filename);
void * heap_calloc_aligned_debug(size_t number, size_t size, int fileline, const char * filename);
void * heap_realloc_aligned_debug(void * memblock, size_t size, int fileline, const char * filename);

size_t heap_get_used_space(void);
size_t heap_get_largest_used_block_size(void);
uint64_t heap_get_used_blocks_count(void);
size_t heap_get_free_space(void);
size_t heap_get_largest_free_area(void);
uint64_t heap_get_free_gaps_count(void);

enum pointer_type_t get_pointer_type(const const void * pointer);
void * heap_get_data_block_start(const void * pointer);
size_t heap_get_block_size(const const void * memblock);
int heap_validate(void);
void heap_dump_debug_information(void);

int add_to_word(size_t size);
int check_heap_size(size_t number);
int add_new_page();
int delete_page();
int aligned_help_function(intptr_t check, size_t size);
void connect_memory();
size_t check_sum_function(struct heap_block *x);
size_t check_sum_for_control();

#endif //MATI_STERTA_H

