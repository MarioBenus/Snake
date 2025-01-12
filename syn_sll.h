#ifndef SNAKE_SYN_SLL
#define SNAKE_SYN_SLL

#include "sll.h"
#include <pthread.h>
#include <stdbool.h>

typedef struct syn_sll
{
    sll sll;
    pthread_mutex_t mutex;
} syn_sll;

void syn_sll_init(syn_sll* this, size_t dataSize);
void syn_sll_clear(syn_sll* this);
size_t syn_sll_get_size(syn_sll* this);
bool syn_sll_get(syn_sll* this, size_t index, void* out);
bool syn_sll_add(syn_sll* this, void* data);
void syn_sll_for_each(syn_sll* this, void(*process_item)(void*, void*, void*, void*),
                      void* in, void* out, void* err);
bool syn_sll_set(syn_sll* this, size_t index, void* data);
bool syn_sll_insert(syn_sll* this, size_t index, void* data);
bool syn_sll_remove(syn_sll* this, size_t index);

#endif