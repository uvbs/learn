/*************************************************************************
    > File Name: mini-heap.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Mon 05 Jan 2015 06:22:48 PM CST
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>

typedef struct min_heap
{
    int * p;
    unsigned capacity;
    unsigned size;
} min_heap_t;

static inline void min_heap_ctor(min_heap_t * s)
{
    s->p = NULL;
    s->capacity = 0;
    s->size = 0;
}

static inline void min_heap_dtor(min_heap_t * s)
{
    if (s->p != NULL) {
        free(s->p);
        s->p = NULL;
    }
    s->capacity = 0;
    s->size = 0;
}

static inline void min_heap_shift_up(min_heap_t * s, unsigned hole_index, int new_insert)
{
    unsigned parent = (hole_index - 1) >> 1;
    
    while (hole_index && s->p[parent] > new_insert) {
        s->p[hole_index] = s->p[parent];
        hole_index = parent;
        parent = (hole_index - 1) >> 1;
    }

    s->p[hole_index] = new_insert;
}

static inline void min_heap_shift_down(min_heap_t * s, unsigned hole_index, int new_value)
{
    unsigned min_child = (hole_index + 1) << 1;

    while (min_child <= s->size) {
        min_child -= (min_child == s->size || s->p[min_child] > s->p[min_child - 1]);
        if (new_value <= s->p[min_child]) {
            break;
        }

        s->p[hole_index] = s->p[min_child];
        hole_index = min_child;
        min_child = (hole_index + 1) << 1;
    }
    s->p[hole_index] = new_value;
}

int min_heap_reserve(min_heap_t * s, unsigned new_size)
{
    if (s->capacity < new_size) {
        int * new_ptr = NULL;
        unsigned new_capacity = s->capacity ? s->capacity * 2 : 8;
        if (new_capacity < new_size) {
            new_capacity = new_size;
        }

        new_ptr = (int *)realloc(s->p, new_capacity * sizeof(int));
        if (new_ptr == NULL) {
            return -1;
        }

        s->p = new_ptr;
        s->capacity = new_capacity;
    }
    return 0;
}

int min_heap_size(min_heap_t * s)
{
    return s->size;
}

int min_heap_empty(min_heap_t * s)
{
    return 0u == s->size;
}

int min_heap_top(min_heap_t * s)
{
    return s->size ? *(s->p) : 0;
}

int min_heap_push(min_heap_t * s, int insert)
{
    if (min_heap_reserve(s, s->size + 1)) {
        return -1;
    }
    min_heap_shift_up(s, s->size ++, insert);
    return 0;
}

int min_heap_pop(min_heap_t * s)
{
    if (s->size > 0) {
        int pop_value = * s->p;
        min_heap_shift_down(s, 0u, s->p[--s->size]);
        return pop_value;
    }
    return 0;
}

int main()
{
    min_heap_t min_heap;
    min_heap_ctor(&min_heap);
    printf("heap size = %d\n", min_heap_size(&min_heap));

    int value;
    while (scanf("%d", &value) && value) {
        min_heap_push(&min_heap, value);
    }

    printf("heap size = %d\n", min_heap_size(&min_heap));
    while (!min_heap_empty(&min_heap)) {
        printf("get top value:%d\n", min_heap_pop(&min_heap));
    }
    return 0;
}
