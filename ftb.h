#ifndef FTB_H_
#define FTB_H_

#ifdef _WIN32
#error "Doesnt support windows yet!"
#endif

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef intptr_t    isize;
typedef uintptr_t   usize;

#define FTB_INIT_CAP 16

#define ftb_da_alloc(list, exp_cap)           \
    do {                                        \
        if ((exp_cap) > (list)->capacity) {     \
            if ((list)->capacity == 0) {        \
                (list)->capacity = FTB_INIT_CAP;\
            }                                   \
            while ((exp_cap) > (list)->capacity) { \
                (list)->capacity *= 2;             \
            }                                      \
            (list)->items = realloc((list)->items,(list)->capacity * sizeof(*(list)->items)); \
            assert((list)->items != NULL && "Ram is not enough!"); \
        } \
    } while (0)

#define ftb_da_append(list, element)            \
    do {                                       \
        ftb_da_alloc((list), (list)->count + 1); \
        (list)->items[(list)->count++] = (element);   \
    } while (0)

#define ftb_da_append_many(list, new_el, new_el_count)          \
    do {                                                          \
        ftb_da_alloc((list), (list)->count + (new_el_count)); \
        memcpy((list)->items + (list)->count, (new_el), (new_el_count)*sizeof(*(list)->items)); \
        (list)->count += (new_el_count); \
    } while (0)

#define ftb_da_append_null(list) ftb_da_append(list,0)
#define ftb_da_append_cstr(list,str) ftb_da_append_many(list,str,strlen(str))
#define ftb_da_clear(list) \
do {memset((list)->items,0,(list)->capacity); (list)->count = 0; } while(0)
#define ftb_da_free(list) \
do { free((list)->items); (list)->items = NULL; (list)->count = 0; (list)->capacity = 0; } while(0)

typedef struct {
    void* ptr;
    isize bytes;
} ftb_mem_fat_ptr_t;

typedef struct {
    ftb_mem_fat_ptr_t* items;
    u32 count;
    u32 capacity;
} ftb_mem_ptr_list_t;

typedef struct {
    ftb_mem_ptr_list_t ptr_list;
} ftb_ctx_t;

void* ftb_mem_alloc(ftb_ctx_t* ctx,usize bytes);
void* ftb_mem_zalloc(ftb_ctx_t* ctx,usize bytes);
void ftb_mem_free(ftb_ctx_t* ctx,void* ptr);
#define ftb_mem_set_mark(ctx) _ftb_mem_set_mark(ctx,(void*)__func__)
#define ftb_mem_cleanup(ctx) _ftb_mem_cleanup(ctx,(void*)__func__)
void _ftb_mem_set_mark(ftb_ctx_t* ctx,void* func);
void _ftb_mem_cleanup(ftb_ctx_t* ctx,void* func);

#endif // FTB_H_

#ifdef FTB_IMPLEMENTATION
#ifndef FTB_FIRST_IMPLEMENTATION
#define FTB_FIRST_IMPLEMENTATION

void* ftb_mem_alloc
(ftb_ctx_t* ctx,usize bytes)
{
    assert(ctx);
    ftb_mem_fat_ptr_t ptr = {
        .bytes = bytes,
        .ptr = malloc(bytes),
    };
    assert(ptr.ptr);
    ftb_da_append(&ctx->ptr_list,ptr);
    return ptr.ptr;
}

void* ftb_mem_zalloc
(ftb_ctx_t* ctx,usize bytes)
{
    assert(ctx);
    ftb_mem_fat_ptr_t ptr = {
        .bytes = bytes,
        .ptr = calloc(1,bytes),
    };
    assert(ptr.ptr);
    ftb_da_append(&ctx->ptr_list,ptr);
    return ptr.ptr;
}

void ftb_mem_free
(ftb_ctx_t* ctx,void* ptr)
{
    if(!ptr) return;
    assert(ctx);
    isize i = ctx->ptr_list.count - 1;
    assert(i > 0);
    ftb_mem_fat_ptr_t* fptr = 0;
    for(;i >= 0;--i)
    {
        fptr = &ctx->ptr_list.items[i];
        if(fptr->ptr == ptr) {
            free(fptr->ptr);
            fptr->ptr = 0;
            return;
        }
    }
    assert(0);
    
}

void _ftb_mem_set_mark
(ftb_ctx_t* ctx,void* func)
{
    assert(ctx);
    ftb_mem_fat_ptr_t ptr = {
        .bytes = -1,
        .ptr = func,
    };
    ftb_da_append(&ctx->ptr_list,ptr);
}

void _ftb_mem_cleanup
(ftb_ctx_t* ctx,void* func)
{
    assert(ctx);
    isize i = ctx->ptr_list.count - 1;
    assert(i >= 0);
    ftb_mem_fat_ptr_t* ptr = 0;
    for(;i >= 0;--i)
    {
        ptr = &ctx->ptr_list.items[i];
        if(ptr->ptr == func && ptr->bytes == -1) {
            --ctx->ptr_list.count;
            return;
        }
        free(ptr->ptr);
        --ctx->ptr_list.count;
    }
    assert(0);
}



#endif // FTB_FIRST_IMPLEMENTATION
#endif // FTB_IMPLEMENTATION
