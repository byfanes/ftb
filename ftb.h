#ifndef FTB_H_
#define FTB_H_

#ifdef _WIN32
#error "Doesnt support windows yet!"
#endif

#ifndef FTBDEF
#define FTBDEF
#endif /* FTBDEF */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>

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
#define FTB_STR_DEF_CAP 64

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

typedef enum {
    fptr_func = -1,
    fptr_str_ptr = -2,
} ftb_mem_fat_ptr_type_t;

typedef struct s_ftb_mem_fat_ptr_t{
    char* ptr;
    i32 count;
    i32 capacity;
    // for cap < 0 values equal to:
    // ftb_mem_fat_ptr_type_t type;
} ftb_mem_fat_ptr_t;

typedef struct {
    ftb_mem_fat_ptr_t* items;
    u32 count;
    u32 capacity;
} ftb_mem_ptr_list_t;

typedef ftb_mem_fat_ptr_t* ftb_str_t;

typedef struct {
    ftb_mem_ptr_list_t ptr_list;
} ftb_ctx_t;

typedef enum {
    fec_success = 0,
    fec_noptr,
    fec_coandcap,
    fec_unknownfunc,
    fec_end,
} ftb_free_ec_t;

FTBDEF ftb_free_ec_t ftb_mem_free_item
(ftb_ctx_t* ctx,ftb_mem_fat_ptr_t* ptr,void* func);
FTBDEF void ftb_mem_delete_ctx(ftb_ctx_t* ctx);
FTBDEF void* ftb_mem_alloc(ftb_ctx_t* ctx,usize bytes);
FTBDEF void* ftb_mem_zalloc(ftb_ctx_t* ctx,usize bytes);
FTBDEF void ftb_mem_free(ftb_ctx_t* ctx,void* ptr);
#define ftb_mem_set_mark(ctx) _ftb_mem_set_mark(ctx,(void*)__func__)
#define ftb_mem_cleanup(ctx) _ftb_mem_cleanup(ctx,(void*)__func__)
FTBDEF void _ftb_mem_set_mark(ftb_ctx_t* ctx,void* func);
FTBDEF void _ftb_mem_cleanup(ftb_ctx_t* ctx,void* func);

FTBDEF ftb_str_t ftb_str_create(ftb_ctx_t* ctx);
FTBDEF void ftb_str_clear(ftb_str_t str);
FTBDEF char* ftb_str_to_cstr(ftb_ctx_t* ctx,ftb_str_t str);

FTBDEF void ftb_str_append_cstr(ftb_str_t str,char* cstr);
FTBDEF void ftb_str_append_str(ftb_str_t str,ftb_str_t str2);
FTBDEF bool ftb_str_cmp_cstr(ftb_str_t str,char* cstr);
FTBDEF bool ftb_str_cmp_str(ftb_str_t str,ftb_str_t cstr);

//FTBDEF void ftb_str_append_fmt(ftb_str_t str,const char* fmt,...);
FTBDEF void ftb_str_uppercase(ftb_str_t str);
FTBDEF void ftb_str_lowercase(ftb_str_t str);

FTBDEF bool ftb_str_starts_with_cstr(ftb_str_t str,const char* cstr);
FTBDEF bool ftb_str_ends_with_cstr(ftb_str_t str,const char* cstr);
FTBDEF bool ftb_str_starts_with_str(ftb_str_t str1,ftb_str_t str2);
FTBDEF bool ftb_str_ends_with_str(ftb_str_t str1,ftb_str_t str2);

FTBDEF void ftb_str_trim_left(ftb_str_t str);
FTBDEF void ftb_str_trim_right(ftb_str_t str);
FTBDEF void ftb_str_trim(ftb_str_t str);

FTBDEF i32 ftb_str_find_char_end(ftb_str_t str,char c);
FTBDEF i32 ftb_str_find_char_begin(ftb_str_t str,char c);
FTBDEF i32 ftb_str_find_cstr(ftb_str_t str,char* needle);
FTBDEF i32 ftb_str_find_str(ftb_str_t str,ftb_str_t needle);
FTBDEF bool ftb_str_contains_cstr(ftb_str_t str,char* needle);
FTBDEF bool ftb_str_contains_str(ftb_str_t str,ftb_str_t needle);

FTBDEF void ftb_str_remove_range(ftb_str_t str, u32 start, u32 len);

#endif /* FTB_H_ */

#ifdef FTB_IMPLEMENTATION
#ifndef FTB_FIRST_IMPLEMENTATION
#define FTB_FIRST_IMPLEMENTATION

FTBDEF void* ftb_mem_alloc
(ftb_ctx_t* ctx,usize bytes)
{
    assert(ctx);
    if(!bytes) return 0;
    ftb_mem_fat_ptr_t ptr = {
        .count = bytes,
        .capacity = bytes,
        .ptr = malloc(bytes),
    };
    assert(ptr.ptr);
    ftb_da_append(&ctx->ptr_list,ptr);
    return ptr.ptr;
}

FTBDEF void* ftb_mem_zalloc
(ftb_ctx_t* ctx,usize bytes)
{
    assert(ctx);
    if(!bytes) return 0;
    ftb_mem_fat_ptr_t ptr = {
        .count = bytes,
        .capacity = bytes,
        .ptr = calloc(1,bytes),
    };
    assert(ptr.ptr);
    ftb_da_append(&ctx->ptr_list,ptr);
    return ptr.ptr;
}

FTBDEF void ftb_mem_free
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
            if(fptr->capacity >= 0)
            {
                free(fptr->ptr);
                fptr->ptr = 0;
                return;
            }
            if(fptr->count != -1)
            {
                printf("Unknow dangle!\nSkipping...\n");
                return;
            }
            if(fptr->capacity == fptr_func)
            {
                printf("System depends on function marks can't free it\n");
                return;
            }
            if(fptr->capacity == fptr_str_ptr)
            {
                ftb_mem_fat_ptr_t* str = (ftb_mem_fat_ptr_t*)fptr->ptr;
                free(str->ptr);
                free(str);
                fptr->ptr = 0;
                return;
            }
        }
    }
    assert(0);
}

FTBDEF void _ftb_mem_set_mark
(ftb_ctx_t* ctx,void* func)
{
    assert(ctx);
    ftb_mem_fat_ptr_t ptr = {
        .count = -1,
        .capacity = fptr_func,
        .ptr = func,
    };
    ftb_da_append(&ctx->ptr_list,ptr);
}

FTBDEF void ftb_mem_delete_ctx
(ftb_ctx_t* ctx)
{
    assert(ctx);
    ftb_mem_fat_ptr_t* ptr = 0;
    ftb_free_ec_t ec = 0;
    i32 i = ctx->ptr_list.count;
    for(;i >= 0;--i)
    {
        ptr = &ctx->ptr_list.items[i];
        ec = ftb_mem_free_item(ctx,ptr,(void*)-1);
        if(ec != fec_end && ec != fec_success)
        {printf("Ec error: %d\n",ec);}
    }
    free(ctx->ptr_list.items);
    return;
}

FTBDEF void _ftb_mem_cleanup
(ftb_ctx_t* ctx,void* func)
{
    assert(ctx);
    isize i = ctx->ptr_list.count - 1;
    assert(i >= 0);
    ftb_mem_fat_ptr_t* ptr = 0;
    ftb_free_ec_t ec = 0;
    for(;i >= 0;--i)
    {
        ptr = &ctx->ptr_list.items[i];
        ec = ftb_mem_free_item(ctx,ptr,func);
        if(ec == fec_end) {return;}
    }
    assert(0);
}

FTBDEF ftb_free_ec_t ftb_mem_free_item
(ftb_ctx_t* ctx,ftb_mem_fat_ptr_t* ptr,void* func)
{
    if(!ctx) {return fec_noptr;}
    if(!ptr) {return fec_noptr;}
    if(ptr->capacity >= 0)
    {
        free(ptr->ptr);
        --ctx->ptr_list.count;
        return fec_success;
    }
    if(ptr->count != -1)
    {
        --ctx->ptr_list.count;
        return fec_coandcap;
    }
    if(ptr->capacity == fptr_func)
    {
        if(ptr->ptr == func)
        {
            --ctx->ptr_list.count;
            return fec_end;
        }
        if((isize)func == -1)
        {
            --ctx->ptr_list.count;
            return fec_success;
        }
        return fec_unknownfunc;
    }
    if(ptr->capacity == fptr_str_ptr)
    {
        ftb_mem_fat_ptr_t* str = (ftb_mem_fat_ptr_t*)ptr->ptr;
        if(!str) {return fec_noptr;}
        if(!str->ptr) {return fec_noptr;}
        free(str->ptr);
        free(str);
        --ctx->ptr_list.count;
        return fec_success;
    }
    if((isize)func == -1) {return fec_end;}
    assert(0);
}

FTBDEF ftb_str_t ftb_str_create
(ftb_ctx_t* ctx)
{
    assert(ctx);
    ftb_mem_fat_ptr_t* sptr = calloc(1,sizeof(ftb_mem_fat_ptr_t));
    assert(sptr);
    sptr->ptr = calloc(1,FTB_STR_DEF_CAP);
    sptr->count = 0;
    sptr->capacity = FTB_STR_DEF_CAP;
    ftb_mem_fat_ptr_t fptr = {
        .capacity = fptr_str_ptr,
        .count = -1,
        .ptr = (void*)sptr,
    };
    ftb_da_append(&ctx->ptr_list,fptr);
    return sptr;
}

FTBDEF void ftb_str_clear
(ftb_str_t str)
{
    memset(str->ptr,0,str->capacity);
    str->count = 0;
}

FTBDEF char* ftb_str_to_cstr
(ftb_ctx_t* ctx,ftb_str_t str)
{
    u32 len = str->count + 1;
    char* s = strndup(str->ptr,len);
    assert(s);
    s[str->count] = 0;
    ftb_mem_fat_ptr_t fptr = {
        .capacity = len,
        .count = len,
        .ptr = s,
    };
    ftb_da_append(&ctx->ptr_list,fptr);
    return s;
}

FTBDEF void ftb_str_append_cstr
(ftb_str_t str,char* cstr)
{
    assert(str);
    assert(cstr);
    u32 empty = str->capacity - str->count;
    u32 len = strlen(cstr);
    if(empty <= len)
    {
        void* ptr = calloc(1,((len / str->capacity) + 2) * str->capacity);
        assert(ptr);
        memcpy(ptr,(str->ptr),str->count);
        free(str->ptr);
        str->ptr = ptr;
    }
    memcpy(&str->ptr[str->count],cstr,len);
    str->count += len;
}

/* For functions ftb_str_appends_[c]str
 * if(empty <= len) must <= and it must not change
 * using different approach will destory last zero
 * and its depend for clib functions
 */

FTBDEF void ftb_str_append_str
(ftb_str_t str1,ftb_str_t str2)
{
    assert(str1);
    assert(str2);
    u32 empty = str1->capacity - str1->count;
    u32 len = str2->count;
    if(empty <= len)
    {
        void* ptr = calloc(1,((len / str1->capacity) + 2) * str1->capacity);
        assert(ptr);
        memcpy(ptr,str1->ptr,str1->count);
        free(str1->ptr);
        str1->ptr = ptr;
    }
    memcpy(&str1->ptr[str1->count],str2->ptr,len);
    str1->count += len;
}

FTBDEF bool ftb_str_cmp_cstr
(ftb_str_t str,char* cstr)
{
    assert(str);
    assert(cstr);
    return (strncmp(str->ptr,cstr,str->count) == 0);
}

FTBDEF bool ftb_str_cmp_str
(ftb_str_t str1,ftb_str_t str2)
{
    assert(str1);
    assert(str2);
    if(str1->count != str2->count) return false;
    return (strncmp(str1->ptr,str2->ptr,str1->count) == 0);
}

FTBDEF void ftb_str_uppercase
(ftb_str_t str)
{
    assert(str);
    i32 i = 0;
    for(;i < str->count;++i)
    {
        char c = str->ptr[i];
        if('a' <= c && c <= 'z')
        {
            str->ptr[i] = c - 32;
        }
    }
}

FTBDEF void ftb_str_lowercase
(ftb_str_t str)
{
    assert(str);
    i32 i = 0;
    for(;i < str->count;++i)
    {
        char c = str->ptr[i];
        if('A' <= c && c <= 'Z')
        {
            str->ptr[i] = c + 32;
        }
    }
}

FTBDEF bool ftb_str_starts_with_cstr
(ftb_str_t str,const char* cstr)
{
    assert(str);
    assert(cstr);
    i32 len = strlen(cstr);
    if(str->count < len) return false;
    return (strncmp(str->ptr,cstr,len) == 0);
}

FTBDEF bool ftb_str_ends_with_cstr
(ftb_str_t str,const char* cstr)
{
    assert(str);
    assert(cstr);
    i32 len = strlen(cstr);
    if(str->count < len) return false;
    char* start = &str->ptr[str->count-len];
    return (strncmp(start,cstr,len) == 0);
}

FTBDEF bool ftb_str_starts_with_str
(ftb_str_t str1,ftb_str_t str2)
{
    assert(str1);
    assert(str2);
    if(str1->count < str2->count) return false;
    return (strncmp(str1->ptr,str2->ptr,str2->count) == 0);
}

FTBDEF bool ftb_str_ends_with_str
(ftb_str_t str1,ftb_str_t str2)
{
    assert(str1);
    assert(str2);
    if(str1->count < str2->count) return false;
    u32 len = str2->count;
    char* start = &str1->ptr[str1->count-len];
    return (strncmp(start,str2->ptr,len) == 0);
}

FTBDEF void ftb_str_trim_left
(ftb_str_t str)
{
    assert(str);
    i32 i = 0;
    for(;i < str->count;++i)
    {
        if(!isblank(str->ptr[i]))
        {
            memmove(str->ptr,&str->ptr[i],str->count-i);
            str->count -= i;
            return;
        }
    }
    ftb_str_clear(str);
}

FTBDEF void ftb_str_trim_right
(ftb_str_t str)
{
    assert(str);
    i32 i = str->count - 1;
    for(;i >= 0;--i)
    {
        if(!isblank(str->ptr[i]))
        {
            memset(&str->ptr[i + 1],0,str->count - i + 1);
            str->count -= i;
            return;
        }
    }
}

FTBDEF void ftb_str_trim
(ftb_str_t str)
{
    assert(str);
    ftb_str_trim_left(str);
    ftb_str_trim_right(str);
}

FTBDEF i32 ftb_str_find_char_begin
(ftb_str_t str,char c)
{
    assert(str);
    i32 i = 0;
    for(;i < str->count;++i)
    {
        if(str->ptr[i] == c)
        {
            return i;
        }
    }
    return -1;
}

FTBDEF i32 ftb_str_find_char_end
(ftb_str_t str,char c)
{
    assert(str);
    i32 i = str->count - 1;
    for(;i >= 0;--i)
    {
        if(str->ptr[i] == c)
        {
            return i;
        }
    }
    return -1;
}

FTBDEF i32 ftb_str_find_cstr
(ftb_str_t str,char* needle)
{
    assert(str);
    assert(needle);
    i32 i = 0;
    u32 len = strlen(needle);
    for(;i < str->count;++i)
    {
        if(strncmp(&str->ptr[i],needle,len) == 0)
        {
            return i;
        }
    }
    return -1;
}

FTBDEF bool ftb_str_contains_cstr
(ftb_str_t str,char* needle)
{
    assert(str);
    assert(needle);
    return (ftb_str_find_cstr(str,needle) != -1);
}

FTBDEF i32 ftb_str_find_str
(ftb_str_t str,ftb_str_t needle)
{
    assert(str);
    assert(needle);
    i32 i = 0;
    for(;i < str->count;++i)
    {
        if(strncmp(&str->ptr[i],needle->ptr,needle->count) == 0)
        {
            return i;
        }
    }
    return -1;
}


FTBDEF bool ftb_str_contains_str
(ftb_str_t str,ftb_str_t needle)
{
    assert(str);
    assert(needle);
    return (ftb_str_find_str(str,needle) != -1);
}

FTBDEF void ftb_str_remove_range(ftb_str_t str, u32 start, u32 len)
{
    assert(str);
    if ((i32)start >= str->count) return;
    if (len == 0) return;
    if ((i32)start + (i32)len > str->count) {
        len = str->count - start;
    }
    char* data = str->ptr;
    u32 end_of_gap = start + len;
    u32 bytes_to_move = str->count - end_of_gap;
    if (bytes_to_move > 0) {
        memmove(data + start, data + end_of_gap, bytes_to_move);
    }
    str->count -= len;
    memset(&str->ptr[str->count],0,str->capacity - str->count);
}

FTBDEF void ftb_str_append_fmt(ftb_str_t str,const char* fmt,...);

#endif /* FTB_FIRST_IMPLEMENTATION */
#endif /* FTB_IMPLEMENTATION */
