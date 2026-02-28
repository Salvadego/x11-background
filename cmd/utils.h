
#ifndef UTILS_H_WJGZ8ES6
#define UTILS_H_WJGZ8ES6

#define OffsetOfMember(T, m) offsetof(T, m)
#define SizeOfType(T)        sizeof(T)
#define AlignOfType(T)                                                         \
        (sizeof(struct {                                                       \
                 char c;                                                       \
                 T    x;                                                       \
         }) -                                                                  \
         sizeof(T))

#define trace(msg) fprintf(stderr, "%s\n", msg)
#define null       NULL
#define Statement(x)                                                           \
        do {                                                                   \
                x                                                              \
        } while (0)

#define Bytes(n)     ((n) * 1)
#define Kilobytes(n) ((n) * Bytes(1024))
#define Megabytes(n) ((n) * Kilobytes(1024))
#define Gigabytes(n) ((n) * Megabytes(1024))

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct Arena {
        uint8_t *base;
        size_t   capacity;
        size_t   offset;
} Arena;

static Arena arena_create(size_t capacity) {
        Arena a;
        a.base     = (uint8_t *)malloc(capacity);
        a.capacity = capacity;
        a.offset   = 0;
        return a;
}

static inline void arena_destroy(Arena *a) {
        free(a->base);
        a->base     = NULL;
        a->capacity = 0;
        a->offset   = 0;
}

static inline void arena_reset(Arena *a) {
        a->offset = 0;
}

static void *arena_alloc(Arena *a, size_t size, size_t align) {
        size_t current    = (size_t)(a->base + a->offset);
        size_t aligned    = (current + (align - 1)) & ~(align - 1);
        size_t new_offset = (aligned - (size_t)a->base) + size;

        if (new_offset > a->capacity)
                return NULL;

        void *ptr = a->base + (aligned - (size_t)a->base);
        a->offset = new_offset;
        return ptr;
}

typedef struct {
        size_t size;
        size_t capacity;
        size_t elem_size;
        Arena *arena;
} arr_header_t;

#define arr_header(v) ((arr_header_t *)(v) - 1)

static inline void *
arr_new_impl(Arena *arena, size_t cap, size_t elem_size, size_t align) {
        size_t header_size = sizeof(arr_header_t);
        size_t data_size   = elem_size * cap;
        size_t total_size  = header_size + data_size;

        arr_header_t *h = arena_alloc(arena, total_size, align);
        if (!h)
                return NULL;

        h->size      = 0;
        h->capacity  = cap;
        h->elem_size = elem_size;
        h->arena     = arena;

        return (void *)(h + 1);
}

static inline void *arr_grow(void *v) {
        arr_header_t *old          = arr_header(v);
        size_t        new_capacity = old->capacity * 2;

        size_t header_size = sizeof(arr_header_t);
        size_t data_size   = new_capacity * old->elem_size;
        size_t total_size  = header_size + data_size;

        arr_header_t *new_h =
            arena_alloc(old->arena, total_size, old->elem_size);
        if (!new_h)
                return NULL;

        new_h->size      = old->size;
        new_h->capacity  = new_capacity;
        new_h->elem_size = old->elem_size;
        new_h->arena     = old->arena;

        memcpy(new_h + 1, v, old->size * old->elem_size);

        return (void *)(new_h + 1);
}

#define arr_len(v) (arr_header(v)->size)
#define arr_cap(v) (arr_header(v)->capacity)

#define ARR_INITIAL_CAPACITY 256

#define arr_new(alloc, type)                                                   \
        (type *)arr_new_impl((alloc),                                          \
                             ARR_INITIAL_CAPACITY,                             \
                             SizeOfType(type),                                 \
                             AlignOfType(type))

#define arr_new_cap(alloc, cap, type)                                          \
        (type *)arr_new_impl(                                                  \
            (alloc), (cap), SizeOfType(type), AlignOfType(type))

#define arr(alloc, type) arr_new(alloc, type)

#define arr_get(v, i)                                                          \
        (arr_header(v)->size <= (i) ? (trace("Out of Bounds"), (v)[0])         \
                                    : (v)[(i)])

#define arr_append(v, val)                                                     \
        Statement(arr_header_t *_h = arr_header(v);                            \
                  if (_h->size == _h->capacity) {                              \
                          void *_new = arr_grow(v);                            \
                          if (!_new)                                           \
                                  break;                                       \
                          v  = _new;                                           \
                          _h = arr_header(v);                                  \
                  } memcpy((uint8_t *)v + _h->size * _h->elem_size,            \
                           &(val),                                             \
                           _h->elem_size);                                     \
                  _h->size++;)

#endif /* end of include guard: UTILS_H_WJGZ8ES6 */
