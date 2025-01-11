#include "syn_sll.h"
#include <stdbool.h>

void syn_sll_init(syn_sll* this, size_t dataSize)
{
    sll_init(&this->sll, dataSize);
    pthread_mutex_init(&this->mutex, NULL);
}

void syn_sll_clear(syn_sll* this)
{
    pthread_mutex_lock(&this->mutex);
    sll_clear(&this->sll);
    pthread_mutex_unlock(&this->mutex);
}

size_t syn_sll_get_size(syn_sll* this)
{
    pthread_mutex_lock(&this->mutex);
    size_t result = sll_get_size(&this->sll);
    pthread_mutex_unlock(&this->mutex);
    return result;
}

bool syn_sll_get(syn_sll* this, size_t index, void* out)
{
    pthread_mutex_lock(&this->mutex);
    bool result = sll_get(&this->sll, index, out);
    pthread_mutex_unlock(&this->mutex);
    return result;
}

bool syn_sll_add(syn_sll* this, void* data)
{
    pthread_mutex_lock(&this->mutex);
    bool result = sll_add(&this->sll, data);
    pthread_mutex_unlock(&this->mutex);
    return result;
}

void syn_sll_for_each(syn_sll* this, void(*process_item)(void*, void*, void*, void*),
                      void* in, void* out, void* err)
{
    pthread_mutex_lock(&this->mutex);
    sll_for_each(&this->sll, process_item, in, out, err);
    pthread_mutex_unlock(&this->mutex);
}

bool syn_sll_set(syn_sll* this, size_t index, void* data)
{
    pthread_mutex_lock(&this->mutex);
    bool result = sll_set(&this->sll, index, data);
    pthread_mutex_unlock(&this->mutex);
    return result;
}

bool syn_sll_insert(syn_sll* this, size_t index, void* data)
{
    pthread_mutex_lock(&this->mutex);
    bool result = sll_insert(&this->sll, index, data);
    pthread_mutex_unlock(&this->mutex);
    return result;
}

bool syn_sll_remove(syn_sll* this, size_t index)
{
    pthread_mutex_lock(&this->mutex);
    bool result = sll_remove(&this->sll, index);
    pthread_mutex_unlock(&this->mutex);
    return result;
}
