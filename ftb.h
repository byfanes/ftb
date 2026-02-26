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

#ifdef FTB_WERROR

#define ftb_error_ret(cond,val)           \
   do {                                   \
       if(cond) {                         \
       printf("A ftb function has failed! \nAborting...\n"); \
       printf("%s:%d:%s: Function got invalid argumants\n\n",__FILE__,__LINE__,__func__); \
       abort();}                          \
   } while(0)

#else /* FTB_WERROR */

#define ftb_error_ret(cond,val) \
   do { if(cond) {return val;} } while(0)

#endif /* FTB_WERROR */

#ifndef FTB_DA_INIT_CAPACITY
#define FTB_DA_INIT_CAPACITY 16
#endif // FTB_DA_INIT_CAPACITY

#define ftb_da_count(da) ((ftb_ptr_header_t*)(da) - 1)->count
#define ftb_da_count_inc(da) (((ftb_ptr_header_t*)(da) - 1)->count++)
#define ftb_da_count_sum(da,x) (((ftb_ptr_header_t*)(da) - 1)->count+=(x))
#define ftb_da_count_sub(da,x) (((ftb_ptr_header_t*)(da) - 1)->count-=(x))
#define ftb_da_count_dec(da) (((ftb_ptr_header_t*)(da) - 1)->count--)
#define ftb_da_capacity(da) ((ftb_ptr_header_t*)(da) - 1)->capacity
#define ftb_da_set_count(da,x) do{ ((ftb_ptr_header_t*)(da) - 1)->count = (x);} while(0)
#define ftb_da_set_capacity(da,x) do{ ((ftb_ptr_header_t*)(da) - 1)->capacity = (x);} while(0)
#define ftb_raw_da_free(da) free((ftb_ptr_header_t*)(da) - 1)

#define ftb_da_reserve(ctx, da, amount)                                                                                  \
    do {                                                                                                                 \
        size_t _ftb_da_amt = (amount);                                                                                   \
        if ((da) == NULL) {                                                                                              \
            size_t _ftb_da_cap = FTB_DA_INIT_CAPACITY > _ftb_da_amt ? FTB_DA_INIT_CAPACITY : _ftb_da_amt;                \
            ftb_ptr_header_t *header = ftb_mem_zalloc((ctx), sizeof(*(da)) * _ftb_da_cap + sizeof(ftb_ptr_header_t));    \
            header->count = 0;                                                                                           \
            header->capacity = _ftb_da_cap;                                                                              \
            (da) = (void*)(header + 1);                                                                                  \
        }                                                                                                                \
        ftb_ptr_header_t *header = (ftb_ptr_header_t*)(da) - 1;                                                          \
        size_t _ftb_da_needed = header->count + _ftb_da_amt;                                                             \
        if (_ftb_da_needed > header->capacity) {                                                                         \
            size_t _ftb_da_new_cap = header->capacity * 2;                                                               \
            if (_ftb_da_new_cap < _ftb_da_needed) {                                                                      \
                _ftb_da_new_cap = _ftb_da_needed;                                                                        \
            }                                                                                                            \
            header->capacity = _ftb_da_new_cap;                                                                          \
            da = ftb_mem_realloc((ctx), da, sizeof(*(da)) * header->capacity + sizeof(ftb_ptr_header_t));        \
        }                                                                                                                \
    } while (0)

#define ftb_raw_da_reserve(da, amount)                                                                                   \
    do {                                                                                                                 \
        size_t _ftb_da_amt = (amount);                                                                                   \
        if ((da) == NULL) {                                                                                              \
            size_t _ftb_da_cap = FTB_DA_INIT_CAPACITY > _ftb_da_amt ? FTB_DA_INIT_CAPACITY : _ftb_da_amt;                \
            ftb_ptr_header_t *header = calloc(1, sizeof(*(da)) * _ftb_da_cap + sizeof(ftb_ptr_header_t));                \
            header->count = 0;                                                                                           \
            header->capacity = _ftb_da_cap;                                                                              \
            (da) = (void*)(header + 1);                                                                                  \
        }                                                                                                                \
        ftb_ptr_header_t *header = (ftb_ptr_header_t*)(da) - 1;                                                          \
        size_t _ftb_da_needed = header->count + _ftb_da_amt;                                                             \
        if (_ftb_da_needed > header->capacity) {                                                                         \
            size_t _ftb_da_new_cap = header->capacity * 2;                                                               \
            if (_ftb_da_new_cap < _ftb_da_needed) {                                                                      \
                _ftb_da_new_cap = _ftb_da_needed;                                                                        \
            }                                                                                                            \
            header->capacity = _ftb_da_new_cap;                                                                          \
            header = realloc(header, sizeof(*(da)) * header->capacity + sizeof(ftb_ptr_header_t));                       \
            (da) = (void*)(header + 1);                                                                                  \
        }                                                                                                                \
    } while (0)

#define ftb_da_append(ctx,da,item)                           \
    do {                                                     \
        ftb_da_reserve((ctx),(da),1);                        \
        (da)[((ftb_ptr_header_t*)(da)-1)->count++] = (item); \
    } while(0)

#define ftb_raw_da_append(da,item)                           \
    do {                                                     \
        ftb_raw_da_reserve((da),1);                          \
        (da)[((ftb_ptr_header_t*)(da)-1)->count++] = (item); \
    } while(0)

#define ftb_raw_da_appends(da,items,items_count)                      \
    do {                                                              \
        ftb_raw_da_reserve((da),(items_count));                       \
        u32 da_count = ((ftb_ptr_header_t*)(da)-1)->count;            \
        memcpy((da) + da_count, (items), (items_count)*sizeof(*(da)));\
        ((ftb_ptr_header_t*)(da)-1)->count += (items_count);          \
    } while(0)

#define ftb_da_appends(ctx,da,items,items_count)                      \
    do {                                                              \
        ftb_da_reserve((ctx),(da),(items_count));                     \
        u32 da_count = ((ftb_ptr_header_t*)(da)-1)->count;            \
        memcpy((da) + da_count, (items), (items_count)*sizeof(*(da)));\
        ((ftb_ptr_header_t*)(da)-1)->count += (items_count);          \
    } while(0)

#define FTB_ADD_TEST(tests, fn)       \
    do {                              \
        ftb_test_t el = {             \
            .name = #fn,              \
            .func = fn,               \
        };                            \
        ftb_raw_da_append(tests, el); \
    } while (0)

#define TEST_DIRECT_ASSERT(cond,msg)              \
    do {                                          \
        if(!(cond)) {                             \
            fprintf(stderr,"[D/FAIL] %s:%d:%s: %s \n",__FILE__,__LINE__,__func__,msg); \
            exit(1);                              \
        }                                         \
    } while(0)

#define TEST_RESULT(x) return x

#ifdef FTB_TEST_CRASH
#define TEST_ASSERT(cond,msg)                   \
do {                                        \
        if(!(cond)) {                           \
            fprintf(stderr,"[FAIL] %s:%d:%s: %s \n",__FILE__,__LINE__,__func__,msg); \
            exit(1);                            \
        }                                       \
    } while(0)

#else // FTB_TEST_CRASH
#define TEST_ASSERT(cond, msg) \
    do {                  \
        if (!(cond)) {    \
            return false; \
        }                 \
    } while (0)

#endif // FTB_TEST_CRASH

typedef struct {
    u32 addr;
    u32 count;
    u32 capacity;
} ftb_ptr_header_t;

typedef struct {
    void** items;
    u32* marks;
} ftb_mem_ptr_list_t;

typedef struct {
    ftb_mem_ptr_list_t ptrs;
} ftb_ctx_t;

typedef struct {
    char* name;
    bool res;
    // TBD: time
    bool (*func)(void);
} ftb_test_t;

/* Every returned true is functions done succesfuly
 * Every returned false is functions has failed
 * Except for enum returns
 */

#define TODO(msg) do { printf("%s:%d: To be done:%s\n",__FILE__,__LINE__,msg);abort();} while(0)

FTBDEF bool ftb_tests_run(ftb_test_t* tests);
FTBDEF bool ftb_tests_report(ftb_test_t* tests);

FTBDEF void ftb_mem_delete_ctx(ftb_ctx_t* ctx);
FTBDEF void* ftb_mem_alloc(ftb_ctx_t* ctx,usize bytes);
FTBDEF void* ftb_mem_zalloc(ftb_ctx_t* ctx,usize bytes);
FTBDEF void* ftb_mem_realloc(ftb_ctx_t* ctx,void* ptr,usize bytes);
FTBDEF void ftb_mem_free(ftb_ctx_t* ctx,void* ptr);
FTBDEF void ftb_mem_set_mark(ftb_ctx_t* ctx);
FTBDEF void ftb_mem_cleanup(ftb_ctx_t* ctx);

typedef char* ftb_str_t;

FTBDEF ftb_str_t ftb_str_create(ftb_ctx_t* ctx);
FTBDEF bool ftb_str_clear(ftb_str_t str);
FTBDEF char* ftb_str_to_cstr(ftb_ctx_t* ctx,ftb_str_t str);
FTBDEF u32 ftb_str_len(ftb_str_t str);

#define ftb_str_len(str) ftb_da_count(str)

#define ftb_str_append_cstr(ctx,str,cstr)                      \
    do {                                                       \
        ftb_error_ret((!str || !cstr),false);                  \
        ftb_da_appends(ctx,str,cstr,strlen(cstr));             \
    } while (0)

#define ftb_str_append_str(ctx,str1,str2)                      \
    do {                                                       \
        ftb_error_ret((!str1 || !str2 || str1 == str2),false); \
        ftb_da_appends(ctx,str1,str2,ftb_da_count(str2));      \
    } while(0)


//FTBDEF bool ftb_str_append_cstr(ftb_ctx_t* ctx,ftb_str_t str,char* cstr);
//FTBDEF bool ftb_str_append_str(ftb_ctx_t* ctx,ftb_str_t str,ftb_str_t str2);
FTBDEF bool ftb_str_cmp_cstr(ftb_str_t str,char* cstr);
FTBDEF bool ftb_str_cmp_str(ftb_str_t str,ftb_str_t cstr);

FTBDEF bool ftb_str_uppercase(ftb_str_t str);
FTBDEF bool ftb_str_lowercase(ftb_str_t str);

FTBDEF bool ftb_str_starts_with_cstr(ftb_str_t str,const char* cstr);
FTBDEF bool ftb_str_ends_with_cstr(ftb_str_t str,const char* cstr);
FTBDEF bool ftb_str_starts_with_str(ftb_str_t str1,ftb_str_t str2);
FTBDEF bool ftb_str_ends_with_str(ftb_str_t str1,ftb_str_t str2);

FTBDEF bool ftb_str_trim_left(ftb_str_t str);
FTBDEF bool ftb_str_trim_right(ftb_str_t str);
FTBDEF bool ftb_str_trim(ftb_str_t str);

FTBDEF i32 ftb_str_find_char_end(ftb_str_t str,char c);
FTBDEF i32 ftb_str_find_char_begin(ftb_str_t str,char c);
FTBDEF i32 ftb_str_find_cstr(ftb_str_t str,char* needle);
FTBDEF i32 ftb_str_find_str(ftb_str_t str,ftb_str_t needle);
FTBDEF bool ftb_str_contains_cstr(ftb_str_t str,char* needle);
FTBDEF bool ftb_str_contains_str(ftb_str_t str,ftb_str_t needle);

FTBDEF bool ftb_str_remove_range(ftb_str_t str, u32 start, u32 len);

#endif /* FTB_H_ */

#ifdef FTB_IMPLEMENTATION
#ifndef FTB_FIRST_IMPLEMENTATION
#define FTB_FIRST_IMPLEMENTATION

FTBDEF void* ftb_mem_alloc
(ftb_ctx_t* ctx,usize bytes)
{
    assert(ctx);
    if(!bytes) return 0;
    void* dangle = malloc(bytes + sizeof(ftb_ptr_header_t));
    assert(dangle);
    ftb_ptr_header_t* header = (ftb_ptr_header_t*)dangle;
    void* ptr = ((ftb_ptr_header_t*)dangle + 1);
    header->capacity = bytes;
    header->count = 0;
    ftb_raw_da_append(ctx->ptrs.items,ptr);
    header->addr = ftb_da_count(ctx->ptrs.items);
    return ptr;
}

FTBDEF void* ftb_mem_zalloc
(ftb_ctx_t* ctx,usize bytes)
{
    assert(ctx);
    if(!bytes) return 0;
    void* dangle = calloc(1,bytes + sizeof(ftb_ptr_header_t));
    assert(dangle);
    ftb_ptr_header_t* header = (ftb_ptr_header_t*)dangle;
    void* ptr = ((ftb_ptr_header_t*)dangle + 1);
    header->capacity = bytes;
    header->count = 0;
    ftb_raw_da_append(ctx->ptrs.items,ptr);
    header->addr = ftb_da_count(ctx->ptrs.items);
    return ptr;
}

FTBDEF void* ftb_mem_realloc
(ftb_ctx_t* ctx,void* ptr,usize bytes)
{
    assert(ctx);
    if(!bytes && !ptr) {return 0;}
    if(!bytes && ptr) {ftb_mem_free(ctx,ptr); return 0;}
    if(!ptr && bytes) {return ftb_mem_zalloc(ctx,bytes);}
    u32 cap = ((ftb_ptr_header_t*)ptr-1)->capacity;
    if(cap == bytes) {return ptr;}
    void* da = 0;
    if(cap < bytes) {
        da = ftb_mem_zalloc(ctx,bytes);
        memcpy(da,ptr,ftb_da_capacity(ptr));
        ftb_da_set_count(da,ftb_da_count(ptr));
        ftb_mem_free(ctx,ptr);
        return da;
    }
    ftb_error_ret(cap > bytes,0);
    assert(0);
}

FTBDEF void ftb_mem_free
(ftb_ctx_t* ctx,void* ptr)
{
    assert(ctx);
    if(!ptr) return;
    ftb_ptr_header_t* a = ((ftb_ptr_header_t*)ptr - 1);
    u32 i = a->addr;
    free(a);
    ctx->ptrs.items[i] = 0;
}

FTBDEF void ftb_mem_set_mark
(ftb_ctx_t* ctx)
{
    assert(ctx);
    u32 mark = 0;
    if(ctx->ptrs.items) {
        mark = ftb_da_count(ctx->ptrs.items);
    }
    ftb_raw_da_append(ctx->ptrs.marks,mark);
}

FTBDEF void ftb_mem_delete_ctx
(ftb_ctx_t* ctx)
{
    return;
    assert(ctx);
    for(i32 i = 0;i < ftb_da_count(ctx->ptrs.items);++i)
    {
        if(!ctx->ptrs.items[i]) {continue;}
        void* ptr = (ftb_ptr_header_t*)ctx->ptrs.items[i] - 1;
        free(ptr);
    }
    ftb_raw_da_free(ctx->ptrs.items);
    ftb_raw_da_free(ctx->ptrs.marks);
    return;
}

FTBDEF void ftb_mem_cleanup
(ftb_ctx_t* ctx)
{
    assert(ctx);
    isize i = ftb_da_count(ctx->ptrs.items) - 1;
    assert(i >= 0);
    assert(ftb_da_count(ctx->ptrs.marks) >= 0);
    isize stop = ctx->ptrs.marks[ftb_da_count(ctx->ptrs.marks)-1];
    for(;i >= stop;--i)
    {
        void* ptr = (ftb_ptr_header_t*)ctx->ptrs.items[i] - 1;
        free(ptr);
        ctx->ptrs.items[i] = 0;
        ftb_da_count_dec(ctx->ptrs.items);
    }
}

FTBDEF ftb_str_t ftb_str_create
(ftb_ctx_t* ctx)
{
    assert(ctx);
    return ftb_mem_zalloc(ctx,FTB_STR_DEF_CAP);
}

FTBDEF bool ftb_str_clear
(ftb_str_t str)
{
    ftb_error_ret(!str,false);
    memset(str,0,ftb_da_capacity(str));
    ftb_da_set_count(str,0);
    return true;
}

FTBDEF char* ftb_str_to_cstr
(ftb_ctx_t* ctx,ftb_str_t str)
{
    u32 len = ftb_da_count(str) + 1;
    char* s = 0;
    ftb_da_reserve(ctx,s,len);
    assert(s);
    memcpy(s,str,len);
    return s;
}

FTBDEF bool ftb_str_cmp_cstr
(ftb_str_t str,char* cstr)
{
    ftb_error_ret((!str || !cstr),false);
    return (strcmp(cstr,str) == 0);
}

FTBDEF bool ftb_str_cmp_str
(ftb_str_t str1,ftb_str_t str2)
{
    ftb_error_ret((!str1 || !str2),false);
    if(ftb_da_count(str1) != ftb_da_count(str2)) return false;
    return (strncmp(str1,str2,ftb_da_count(str2)) == 0);
}

FTBDEF bool ftb_str_uppercase
(ftb_str_t str)
{
    ftb_error_ret(!str,false);
    i32 i = 0;
    for(;i < ftb_da_count(str);++i)
    {
        char c = str[i];
        if('a' <= c && c <= 'z')
        {
            str[i] = c - 32;
        }
    }
    return true;
}

FTBDEF bool ftb_str_lowercase
(ftb_str_t str)
{
    ftb_error_ret(!str,false);
    i32 i = 0;
    for(;i < ftb_da_count(str);++i)
    {
        char c = str[i];
        if('A' <= c && c <= 'Z')
        {
            str[i] = c + 32;
        }
    }
    return true;
}

FTBDEF bool ftb_str_starts_with_cstr
(ftb_str_t str,const char* cstr)
{
    ftb_error_ret((!str || !cstr),false);
    i32 len = strlen(cstr);
    if(ftb_da_count(str) < len) return false;
    return (strncmp(str,cstr,len) == 0);
}

FTBDEF bool ftb_str_ends_with_cstr
(ftb_str_t str,const char* cstr)
{
    ftb_error_ret((!str || !cstr),false);
    i32 len = strlen(cstr);
    if(ftb_da_count(str) < len) return false;
    char* start = &str[ftb_da_count(str)-len];
    return (strncmp(start,cstr,len) == 0);
}

FTBDEF bool ftb_str_starts_with_str
(ftb_str_t str1,ftb_str_t str2)
{
    ftb_error_ret((!str1 || !str2),false);
    if(ftb_da_count(str1) < ftb_da_count(str2)) return false;
    return (strncmp(str1,str2,ftb_da_count(str2)) == 0);
}

FTBDEF bool ftb_str_ends_with_str
(ftb_str_t str1,ftb_str_t str2)
{
    ftb_error_ret((!str1 || !str2),false);
    if(ftb_da_count(str1) < ftb_da_count(str2)) return false;
    u32 len = ftb_da_count(str2);
    char* start = &str1[ftb_da_count(str1)-len];
    return (strncmp(start,str2,len) == 0);
}

FTBDEF bool ftb_str_trim_left
(ftb_str_t str)
{
    ftb_error_ret(!str,false);
    i32 i = 0;
    for(;i < ftb_da_count(str);++i)
    {
        if(!isspace(str[i]))
        {
            memmove(str,&str[i],ftb_da_count(str)-i);
            ftb_da_count_sub(str,i);
            memset(&str[ftb_da_count(str)],0,ftb_da_capacity(str) - ftb_da_count(str) -1);
            return true;
        }
    }
    return ftb_str_clear(str);
}

FTBDEF bool ftb_str_trim_right
(ftb_str_t str)
{
    ftb_error_ret(!str,false);
    u32 count = ftb_da_count(str);
    i32 i = count - 2;
    if(!isspace(str[i])) {return true;}
    for(;i >= 0;--i)
    {
        if(!isspace(str[i]))
        {
            ++i;
            memset(&str[i],0,count - i);
            ftb_da_count_sub(str,count-i);
            return true;
        }
    }
    return true;
}

FTBDEF bool ftb_str_trim
(ftb_str_t str)
{
    bool ret = true;
    ret &= ftb_str_trim_right(str);
    ret &= ftb_str_trim_left(str);
    return ret;
}

FTBDEF i32 ftb_str_find_char_begin
(ftb_str_t str,char c)
{
    ftb_error_ret(!str,-1);
    i32 i = 0;
    for(;i < ftb_da_count(str);++i)
    {
        if(str[i] == c)
        {
            return i;
        }
    }
    return -1;
}

FTBDEF i32 ftb_str_find_char_end
(ftb_str_t str,char c)
{
    ftb_error_ret(!str,-1);
    i32 i = ftb_da_count(str) - 1;
    for(;i >= 0;--i)
    {
        if(str[i] == c)
        {
            return i;
        }
    }
    return -1;
}

FTBDEF i32 ftb_str_find_cstr
(ftb_str_t str,char* needle)
{
    ftb_error_ret((!str || !needle),-1);
    i32 i = 0;
    u32 len = strlen(needle);
    for(;i < ftb_da_count(str);++i)
    {
        if(strncmp(&str[i],needle,len) == 0)
        {
            return i;
        }
    }
    return -1;
}

FTBDEF bool ftb_str_contains_cstr
(ftb_str_t str,char* needle)
{
    return (ftb_str_find_cstr(str,needle) != -1);
}

FTBDEF i32 ftb_str_find_str
(ftb_str_t str,ftb_str_t needle)
{
    ftb_error_ret((!str || !needle),-1);
    i32 i = 0;
    for(;i < ftb_da_count(str);++i)
    {
        if(strncmp(&str[i],needle,ftb_da_count(needle)) == 0)
        {
            return i;
        }
    }
    return -1;
}

FTBDEF bool ftb_str_contains_str
(ftb_str_t str,ftb_str_t needle)
{
    return (ftb_str_find_str(str,needle) != -1);
}

FTBDEF bool ftb_str_remove_range(ftb_str_t str, u32 start, u32 len)
{
    ftb_error_ret(!str,false);
    if((i32)start >= ftb_da_count(str)) {return true;}
    if(len == 0) {return true;}
    if((i32)start + (i32)len > ftb_da_count(str)) {
        len = ftb_da_count(str) - start;
    }
    char* data = str;
    u32 end_of_gap = start + len;
    u32 bytes_to_move = ftb_da_count(str) - end_of_gap;
    if (bytes_to_move > 0) {
        memmove(data + start, data + end_of_gap, bytes_to_move);
    }
    ftb_da_count_sub(str,len);
    memset(&str[ftb_da_count(str)],0,ftb_da_capacity(str) - ftb_da_count(str));
    return true;
}

FTBDEF bool ftb_tests_run
(ftb_test_t* tests)
{
    u32 i = 0;
    for(;i < ftb_da_count(tests);++i)
    {
        ftb_error_ret((!tests[i].func),false);
        tests[i].res = tests[i].func();
    }
    return true;
}

FTBDEF bool ftb_tests_report
(ftb_test_t* tests)
{
    if(!ftb_da_count(tests)) return true;
    u32 i = 0;
    printf("\n-----Results of a test group-----\n");
    bool pass = true;
    for(;i < ftb_da_count(tests);++i)
    {
        pass &= tests[i].res;
        char s[] = "\033[32mS\033[0m";
        char f[]   = "\033[31mF\033[0m";
        char* c = (tests[i].res) ? s : f;
        printf("  [%s] <- %s\n",c,tests[i].name);
    }
    return pass;
}

#endif /* FTB_FIRST_IMPLEMENTATION */
#endif /* FTB_IMPLEMENTATION */
