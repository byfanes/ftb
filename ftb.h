#ifndef FTB_H_
#define FTB_H_

#ifdef _WIN32

#define FTB_FS_SEP '\\'
    #include <windows.h>
    #warning "Doesnt fully support windows and havent done any test for it yet!"
#else /* _WIN32 */
    #define FTB_FS_SEP '/'
    #include <sys/stat.h>
    #include <unistd.h>
    #include <limits.h>
#endif /* _WIN32 */

#ifndef FTBDEF
#define FTBDEF
#endif /* FTBDEF */

#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

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

#ifdef FTB_VERBOSE

#define _FTB_VERBOSE(msg)    \
    do {                     \
        fprintf(stdout,"[INFO]" msg); \
    } while(0);
#else /* FTB_VERBOSE */
#define _FTB_VERBOSE(msg)
#endif /* FTB_VERBOSE */

#ifndef FTB_DA_INIT_CAPACITY
#define FTB_DA_INIT_CAPACITY 16
#endif /* FTB_DA_INIT_CAPACITY */

#define ftb_da_header(da) ((ftb_ptr_header_t*)(da) - 1)
#define ftb_da_addr(da) ((ftb_da_header(da))->addr)
#define ftb_da_count(da) ((ftb_da_header(da))->count)
#define ftb_da_count_inc(da) ((ftb_da_header(da))->count++)
#define ftb_da_count_sum(da,x) ((ftb_da_header(da))->count+=(x))
#define ftb_da_count_sub(da,x) ((ftb_da_header(da))->count-=(x))
#define ftb_da_count_dec(da) ((ftb_da_header(da))->count--)
#define ftb_da_capacity(da) (ftb_da_header(da))->capacity
#define ftb_da_set_count(da,x) do{ (ftb_da_header(da))->count = (x);} while(0)
#define ftb_da_set_capacity(da,x) do{ (ftb_da_header(da))->capacity = (x);} while(0)
#define ftb_raw_da_free(da) free(ftb_da_header(da))
#define ftb_da_clamp_id(da,x) CLAMP((x), 0, (ftb_da_count(arr)-1))

#define __ftb_da_reserve(da,amount,func_alloc,func_realloc)  \
    do {                                                     \
        size_t _ftb_da_amt = (amount);                       \
        ftb_ptr_header_t *header = 0;                        \
        if((da) == NULL) {                                   \
            size_t _ftb_da_cap = 0;                          \
            _ftb_da_cap = FTB_DA_INIT_CAPACITY > _ftb_da_amt \
            ? FTB_DA_INIT_CAPACITY : _ftb_da_amt;            \
            func_alloc;                                      \
            header->count = 0;                               \
            header->capacity = _ftb_da_cap;                  \
            (da) = (void*)(header + 1);                      \
        }                                                    \
        header = ftb_da_header(da);                          \
        size_t _ftb_da_needed = header->count + _ftb_da_amt; \
        if (_ftb_da_needed > header->capacity) {             \
            size_t _ftb_da_new_cap = header->capacity * 2;   \
            if (_ftb_da_new_cap < _ftb_da_needed) {          \
                _ftb_da_new_cap = _ftb_da_needed;            \
            }                                                \
            func_realloc;                                    \
        }                                                    \
    } while(0)

#define ftb_da_reserve(ctx,da,amount)                                                         \
__ftb_da_reserve((da),(amount),                                                               \
{(header) = ftb_mem_zalloc((ctx), sizeof(*(da)) * _ftb_da_cap + sizeof(ftb_ptr_header_t));},  \
{da = ftb_mem_realloc((ctx), da, sizeof(*(da)) * _ftb_da_new_cap + sizeof(ftb_ptr_header_t)); \
 ftb_da_set_capacity(da,_ftb_da_new_cap);})

#define ftb_raw_da_reserve(da,amount)                                                         \
__ftb_da_reserve((da),(amount),                                                               \
{(header) = calloc(1, sizeof(*(da)) * _ftb_da_cap + sizeof(ftb_ptr_header_t));},              \
{(header)->capacity = _ftb_da_new_cap;                                                        \
 (header) = realloc(header, sizeof(*(da)) * header->capacity + sizeof(ftb_ptr_header_t));     \
 (da) = (void*)(header + 1);})

#define __ftb_da_append(da,item,func)        \
    do {                                     \
        func;                                \
        ftb_da_count_inc(da);                \
        (da)[ftb_da_count(da) - 1] = (item); \
    } while(0)

#define __ftb_da_appends(da,items,items_count,func)                    \
    do {                                                               \
        func;                                                          \
        memcpy((da) + ftb_da_count(da), (items), (items_count)*sizeof(*(da))); \
        ftb_da_count_sum(da,items_count);                              \
    } while(0)

#define ftb_da_append(ctx,da,item) __ftb_da_append(da,item,{ftb_da_reserve((ctx),(da),1);})
#define ftb_raw_da_append(da,item) __ftb_da_append(da,item,{ftb_raw_da_reserve((da),1);})

#define ftb_raw_da_appends(da,items,items_count) \
__ftb_da_appends(da,items,items_count,{ftb_raw_da_reserve((da),(items_count));})

#define ftb_da_appends(ctx,da,items,items_count) \
__ftb_da_appends(da,items,items_count,{ftb_da_reserve((ctx),(da),(items_count));})

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
do {                                            \
        if(!(cond)) {                           \
            fprintf(stderr,"[FAIL] %s:%d:%s: %s \n",__FILE__,__LINE__,__func__,msg); \
            exit(1);                            \
        }                                       \
    } while(0)

#else /* FTB_TEST_CRASH */
#define TEST_ASSERT(cond, msg) \
    do {                  \
        if (!(cond)) {    \
            return false; \
        }                 \
    } while (0)

#endif /* FTB_TEST_CRASH */

#define FTB_MIN(a,b) ((a) < (b) ? (a) : (b))
#define FTB_MAX(a,b) ((a) > (b) ? (a) : (b))
#define FTB_CLAMP(x,min,max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))
#define FTB_SWAP(type,a,b) do { type tmp = (a);(a) = (b);(b) = tmp;} while(0)
#define FTB_SWAP_PTR(a,b) do { void* tmp = (a); (a) = (b); (b) = tmp; } while(0)
#define FTB_ZERO(x) memset(&(x),0,sizeof(x))
#define FTB_UNUSED(x) (void)(x)
#define FTB_CONCAT2(a,b) a##b
#define FTB_CONCAT(a,b) CONCAT2(a,b)
#define FTB_STRINGIFY(x) #x
#define FTB_TOSTRING(x) STRINGIFY(x)
#define FTB_KB(x) ((x)*1024ULL)
#define FTB_MB(x) ((x)*1024ULL*1024ULL)
#define FTB_GB(x) ((x)*1024ULL*1024ULL*1024ULL)
#define FTB_BIT_SET(x,b) ((x) |= (1<<(b)))
#define FTB_BIT_CLR(x,b) ((x) &= ~(1<<(b)))
#define FTB_BIT_GET(x,b) (((x)>>(b))&1)
#define FTB_BIT_TOGGLE(x,b) ((x) ^= (1<<(b)))
#define FTB_BIT_MASK(x,mask) ((x) & (mask))
#define FTB_BIT_CLEAR_MASK(x,mask) ((x) &= ~(mask))
#define FTB_ALIGN_UP(x,a) (((x)+(a)-1) & ~((a)-1))
#define FTB_ALIGN_DOWN(x,a) ((x) & ~((a)-1))
#define FTB_ENUM_TO_STRING(val) #val

#define ftb_scope(init, cleanup) \
    for (int _i = ((init),0); !_i; ((cleanup),++_i))

#define ftb_time_scope(name, code_block) \
    do {                                 \
        u64 _start = ftb_time_now_us();  \
        code_block;                      \
        u64 _end = ftb_time_now_us();    \
        printf("[TIME] %s: %llu us\n", name, _end-_start); \
    } while(0)

typedef struct {
    u32 addr;
    u32 count;
    u32 capacity;
} ftb_ptr_header_t;

typedef struct {
    void** items;
    u32* marks;
} ftb_mem_ptr_list_t;

typedef enum {
    ftb_log_level_all = 0,
    ftb_log_level_info = 1,
    ftb_log_level_warn = 2,
    ftb_log_level_error = 3,
} ftb_ctx_log_level_t;

typedef struct {
    FILE* log_file;
    bool __close_file;
    bool timestaps;
    ftb_ctx_log_level_t level;
} ftb_ctx_loger_t;

typedef struct {
    ftb_mem_ptr_list_t ptrs;
    ftb_ctx_loger_t loger;
} ftb_ctx_t;

typedef struct {
    const char* name;
    bool res;
    u64 time_usec;
    bool (*func)(void);
} ftb_test_t;

/* Every returned true is functions done succesfuly
 * Every returned false is functions has failed
 * Except for enum returns
 */

#define TODO(msg) do { printf("%s:%d: To be done:%s\n",__FILE__,__LINE__,msg);abort();} while(0)
#define UNIMPLEMENTED                                                               \
    do {                                                                            \
        fprintf(stderr,"%s:%d: %s is not implemented!",__FILE__,__LINE__,__func__); \
        abort(0);                                                                   \
    } while(0);

#define scoped_ctx() \
    for (ftb_ctx_t ctx = {0}, *pctx = &ctx, *_ftb_guard_##__LINE__ = &ctx; \
         _ftb_guard_##__LINE__; \
         ftb_mem_delete_ctx(pctx), _ftb_guard_##__LINE__ = NULL)

FTBDEF bool ftb_tests_run(ftb_test_t* tests);
FTBDEF bool ftb_tests_report(ftb_test_t* tests);
FTBDEF bool ftb_tests_report_redirect(ftb_test_t* tests,FILE* fptr);

FTBDEF void ftb_mem_delete_ctx(ftb_ctx_t* ctx);
FTBDEF void* ftb_mem_alloc(ftb_ctx_t* ctx,usize bytes);
FTBDEF void* ftb_mem_zalloc(ftb_ctx_t* ctx,usize bytes);
FTBDEF void* ftb_mem_realloc(ftb_ctx_t* ctx,void* ptr,usize bytes);
FTBDEF void ftb_mem_free(ftb_ctx_t* ctx,void* ptr);
FTBDEF void ftb_mem_set_mark(ftb_ctx_t* ctx);
FTBDEF void ftb_mem_cleanup(ftb_ctx_t* ctx);

FTBDEF bool __ftb_log
(ftb_ctx_t* ctx,const char* tag,const char* fmt,va_list ap);
FTBDEF bool ftb_log(ftb_ctx_t* ctx,const char* tag,const char* fmt,...);
FTBDEF bool ftb_log_info(ftb_ctx_t* ctx,const char* fmt,...);
FTBDEF bool ftb_log_warn(ftb_ctx_t* ctx,const char* fmt,...);
FTBDEF bool ftb_log_error(ftb_ctx_t* ctx,const char* fmt,...);
FTBDEF bool ftb_log_debug(ftb_ctx_t* ctx,const char* fmt,...);
FTBDEF bool ftb_log_set_timestap(ftb_ctx_t* ctx,bool x);
FTBDEF bool ftb_log_toogle_timestap(ftb_ctx_t* ctx);
FTBDEF bool ftb_log_set_log_file_path(ftb_ctx_t* ctx,const char* path);
FTBDEF bool ftb_log_set_log_file(ftb_ctx_t* ctx,FILE* file);
FTBDEF bool ftb_log_set_log_level(ftb_ctx_t* ctx,ftb_ctx_log_level_t level);

typedef char* ftb_str_t;

FTBDEF ftb_str_t ftb_str_create(ftb_ctx_t* ctx);
FTBDEF bool ftb_str_clear(ftb_str_t str);
FTBDEF char* ftb_str_to_cstr(ftb_ctx_t* ctx,ftb_str_t str);
FTBDEF u32 ftb_str_len(ftb_str_t str);

#define ftb_str_len(str) ftb_da_count(str)
#define ftb_free_raw_str(str) ftb_da_free(str)

#define ftb_str_append_cstr_n(ctx,str,cstr,n)             \
    do {                                                  \
        ftb_error_ret(!cstr,false);                       \
        ftb_da_appends(ctx,str,cstr,n);                   \
    } while (0)

#define ftb_str_append_cstr(ctx,str,cstr)                 \
    do {                                                  \
        ftb_error_ret(!cstr,false);                       \
        ftb_da_appends(ctx,str,cstr,strlen(cstr));        \
    } while (0)

#define ftb_str_append_str(ctx,str1,str2)                 \
    do {                                                  \
        ftb_error_ret((!str2 || str1 == str2),false);     \
        ftb_da_appends(ctx,str1,str2,ftb_da_count(str2)); \
    } while(0)

#define ftb_str_printf(ctx,fmt,...) ({              \
    ftb_str_t _s = ftb_str_create(ctx);             \
    int _needed = snprintf(NULL,0,fmt,__VA_ARGS__); \
    ftb_da_reserve(ctx,_s,_needed+1);               \
    snprintf(_s, _needed+1, fmt, __VA_ARGS__);      \
    _s;                                             \
})

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

typedef char* ftb_path_t;

bool ftb_path_basename_cstr(ftb_ctx_t* ctx,ftb_path_t* out,const char* path,u32 len);
bool ftb_path_dirname_cstr(ftb_ctx_t* ctx,ftb_path_t* out,const char* path,u32 len);
bool ftb_path_extension_cstr(ftb_ctx_t* ctx,ftb_path_t* out,const char* path,u32 len);
bool ftb_path_stem_cstr(ftb_ctx_t* ctx,ftb_path_t* out,const char* path,u32 len);

bool ftb_path_basename(ftb_ctx_t* ctx,ftb_path_t* out,ftb_path_t path);
bool ftb_path_dirname(ftb_ctx_t* ctx,ftb_path_t* out,ftb_path_t path);
bool ftb_path_extension(ftb_ctx_t* ctx,ftb_path_t* out,ftb_path_t path);
bool ftb_path_stem(ftb_ctx_t* ctx,ftb_path_t* out,ftb_path_t path);

bool ftb_path_join_cstr(ftb_ctx_t* ctx, ftb_path_t* out, const char* p1, u32 len1, const char* p2, u32 len2);
bool ftb_path_join(ftb_ctx_t* ctx, ftb_path_t* out, ftb_path_t p1, ftb_path_t p2);
bool ftb_path_normalize_cstr(ftb_ctx_t* ctx, ftb_path_t* out, const char* path, u32 len);
bool ftb_path_normalize(ftb_ctx_t* ctx, ftb_path_t* out, ftb_path_t path);
bool ftb_path_with_extension_cstr(ftb_ctx_t* ctx, ftb_path_t* out, const char* path, u32 len, const char* ext, u32 ext_len);
bool ftb_path_with_extension(ftb_ctx_t* ctx, ftb_path_t* out, ftb_path_t path, ftb_path_t ext);

bool ftb_path_is_absolute_cstr(const char* path, u32 len);
bool ftb_path_is_absolute(ftb_path_t path);
bool ftb_path_is_relative_cstr(const char* path, u32 len);
bool ftb_path_is_relative(ftb_path_t path);
bool ftb_path_has_extension_cstr(const char* path, u32 len);
bool ftb_path_has_extension(ftb_path_t path);
bool ftb_path_exists_cstr(ftb_ctx_t* ctx, const char* path, u32 len);
bool ftb_path_exists(ftb_ctx_t* ctx, ftb_path_t path);
bool ftb_path_is_file_cstr(ftb_ctx_t* ctx, const char* path, u32 len);
bool ftb_path_is_file(ftb_ctx_t* ctx, ftb_path_t path);
bool ftb_path_is_dir_cstr(ftb_ctx_t* ctx, const char* path, u32 len);
bool ftb_path_is_dir(ftb_ctx_t* ctx, ftb_path_t path);
bool ftb_path_absolute_cstr(ftb_ctx_t* ctx, ftb_path_t* out, const char* path, u32 len);
bool ftb_path_absolute(ftb_ctx_t* ctx, ftb_path_t* out, ftb_path_t path);

bool ftb_path_cwd(ftb_ctx_t* ctx, ftb_path_t* out);

u64 ftb_time_now_ms(void);
u64 ftb_time_now_us(void);
f64 ftb_time_now_sec(void);
void ftb_time_sleep_ms(u32);

#endif /* FTB_H_ */

#ifdef FTB_IMPLEMENTATION
#ifndef FTB_FIRST_IMPLEMENTATION
#define FTB_FIRST_IMPLEMENTATION

FTBDEF void* ftb_mem_alloc
(ftb_ctx_t* ctx,usize bytes)
{
    assert(ctx);
    if(!bytes) return 0;
    ftb_ptr_header_t* header = malloc(bytes + sizeof(ftb_ptr_header_t));
    assert(header);
    void* ptr = ((ftb_ptr_header_t*)header + 1);
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
    ftb_ptr_header_t* header = calloc(1,bytes + sizeof(ftb_ptr_header_t));
    assert(header);
    void* ptr = ((ftb_ptr_header_t*)header + 1);
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
    u32 cap = ftb_da_capacity(ptr);
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
    ftb_ptr_header_t* a = ftb_da_header(ptr);
    u32 i = a->addr;
    free(a);
    ctx->ptrs.items[i-1] = 0;
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
    assert(ctx);
    if(ctx->ptrs.items)
    {
        for(u32 i = 0;i < ftb_da_count(ctx->ptrs.items);++i)
        {
            if(!ctx->ptrs.items[i]) {continue;}
            void* ptr = ftb_da_header(ctx->ptrs.items[i]);
            free(ptr);
        }
        ftb_raw_da_free(ctx->ptrs.items);
    }
    if(ctx->ptrs.marks)
    {ftb_raw_da_free(ctx->ptrs.marks);}
    if(ctx->loger.__close_file)
    {
        fclose(ctx->loger.log_file);
    }
    return;
}

FTBDEF void ftb_mem_cleanup
(ftb_ctx_t* ctx)
{
    assert(ctx);
    isize i = ftb_da_count(ctx->ptrs.items) - 1;
    assert(i >= 0);
    assert(ftb_da_count(ctx->ptrs.marks) > 0);
    isize stop = ctx->ptrs.marks[ftb_da_count(ctx->ptrs.marks)-1];
    for(;i >= stop;--i)
    {
        void* ptr = ftb_da_header(ctx->ptrs.items[i]);
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
    u32 i = 0;
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
    u32 i = 0;
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
    u32 len = strlen(cstr);
    if(ftb_da_count(str) < len) return false;
    return (strncmp(str,cstr,len) == 0);
}

FTBDEF bool ftb_str_ends_with_cstr
(ftb_str_t str,const char* cstr)
{
    ftb_error_ret((!str || !cstr),false);
    u32 len = strlen(cstr);
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
    u32 i = 0;
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
    u32 i = 0;
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
    u32 i = 0;
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
    u32 i = 0;
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
    if(start >= ftb_da_count(str)) {return true;}
    if(len == 0) {return true;}
    if(start + len > ftb_da_count(str)) {
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
        u64 usec = 0;
        u64 end_usec = 0;
        ftb_error_ret((!tests[i].func),false);
        usec = ftb_time_now_us();
        tests[i].res = tests[i].func();
        end_usec = ftb_time_now_us();
        tests[i].time_usec = end_usec - usec;
    }
    return true;
}

FTBDEF bool ftb_tests_report
(ftb_test_t* tests)
{
    return ftb_tests_report_redirect(tests,stdout);
}

FTBDEF bool ftb_tests_report_redirect
(ftb_test_t* tests,FILE* fptr)
{
    ftb_error_ret((!fptr || !tests),false);
    if(!ftb_da_count(tests)) return true;
    u32 i = 0;
    fprintf(fptr,"\n-----Results of a test group-----\n");
    bool pass = true;
    char green_s[] = "\033[32mS\033[0m";
    char red_f[]   = "\033[31mF\033[0m";
    char *s,*f;
    if(fptr == stdout || fptr == stderr) {
        s = green_s;
        f = red_f;
    } else {
        s = "S";
        f = "F";
    }
    for(;i < ftb_da_count(tests);++i)
    {
        ftb_test_t* test = &tests[i];
        pass &= test->res;
        char* c = (test->res) ? s : f;
        u64 us = test->time_usec;
        if (us < 1000)
        {fprintf(fptr," [%6lu us] [%s] <- %s\n",us,c,test->name);}
        else if (us < 1000000)
        {fprintf(fptr," [%6.2f ms] [%s] <- %s\n",us / 1000.0,c,test->name);}
        else
        {fprintf(fptr," [%6.2f  s] [%s] <- %s\n",us / 1000000.0,c,test->name);}
    }
    return pass;
}

FTBDEF bool __ftb_log
(ftb_ctx_t* ctx,const char* tag,const char* fmt,va_list ap)
{
    assert(ctx);
    if(!ctx->loger.log_file) return true;
    u32 tag_len = strlen(tag);
    u32 fmt_len = strlen(fmt);
    u32 len = fmt_len+tag_len;
    char buf[len+64];
    u32 index = 0;
    if(ctx->loger.timestaps) {
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        sprintf(buf,"[%02d:%02d:%02d] ", tm->tm_hour, tm->tm_min, tm->tm_sec);
        index = strlen(buf);
    }
    buf[index++] = '[';
    memcpy(&buf[index],tag,tag_len);
    index += tag_len;
    buf[index++] = ']';
    buf[index++] = ' ';
    memcpy(&buf[index],fmt,fmt_len);
    index += fmt_len;
    buf[index++] = '\n';
    buf[index++] = '\0';
    i32 err = vfprintf(ctx->loger.log_file,buf,ap);
    return (err < 0) ? false : true;
}

FTBDEF bool ftb_log
(ftb_ctx_t* ctx,const char* tag,const char* fmt,...)
{
    va_list ap = {0};
    va_start(ap,fmt);
    bool err = __ftb_log(ctx,tag,fmt,ap);
    va_end(ap);
    return err;
}

FTBDEF bool ftb_log_info
(ftb_ctx_t* ctx,const char* fmt,...)
{
    ftb_error_ret((!ctx || !fmt),false);
    if(ctx->loger.level > ftb_log_level_info) return true;
    va_list ap = {0};
    va_start(ap,fmt);
    bool err = __ftb_log(ctx,"INFO",fmt,ap);
    va_end(ap);
    return err;
}

FTBDEF bool ftb_log_warn
(ftb_ctx_t* ctx,const char* fmt,...)
{
    ftb_error_ret((!ctx || !fmt),false);
    if(ctx->loger.level > ftb_log_level_warn) return true;
    va_list ap = {0};
    va_start(ap,fmt);
    bool err = __ftb_log(ctx,"WARN",fmt,ap);
    va_end(ap);
    return err;
}

FTBDEF bool ftb_log_error
(ftb_ctx_t* ctx,const char* fmt,...)
{
    ftb_error_ret((!ctx || !fmt),false);
    if(ctx->loger.level > ftb_log_level_error) return true;
    va_list ap = {0};
    va_start(ap,fmt);
    bool err = __ftb_log(ctx,"ERROR",fmt,ap);
    va_end(ap);
    return err;
}

#if defined(DEBUG) || defined(FTB_DEBUG)
FTBDEF bool ftb_log_debug
(ftb_ctx_t* ctx,const char* fmt,...)
{
    ftb_error_ret((!ctx || !fmt),false);
    if(ctx->loger.level > ftb_log_debug) return true;
    va_list ap = {0};
    va_start(ap,fmt);
    bool err = __ftb_log(ctx,"DEBUG",fmt,ap);
    va_end(ap);
    return err;
}
#else 
FTBDEF bool ftb_log_debug
(ftb_ctx_t* ctx,const char* fmt,...)
{
    (void)ctx;
    (void)fmt;
    return true;
}
#endif /* defined(DEBUG) || defined(FTB_DEBUG) */

FTBDEF bool ftb_log_set_timestap
(ftb_ctx_t* ctx,bool x)
{
    ftb_error_ret(!ctx,false);
    ctx->loger.timestaps = x;
    return true;
}

FTBDEF bool ftb_log_toogle_timestap
(ftb_ctx_t* ctx)
{
    ftb_error_ret(!ctx,false);
    ctx->loger.timestaps = !ctx->loger.timestaps;
    return true;
}

FTBDEF bool ftb_log_set_log_file_path
(ftb_ctx_t* ctx,const char* path)
{
    ftb_error_ret(!ctx,false);
    FILE* fptr = 0;
    fptr = fopen(path,"w");
    if(!fptr) return false;
    ctx->loger.log_file = fptr;
    ctx->loger.__close_file = true;
    return true;
}

FTBDEF bool ftb_log_set_log_file
(ftb_ctx_t* ctx,FILE* file)
{
    ftb_error_ret(!ctx,false);
    ctx->loger.log_file = file;
    return true;
}

FTBDEF bool ftb_log_set_log_level
(ftb_ctx_t* ctx,ftb_ctx_log_level_t level)
{
    ftb_error_ret(!ctx,false);
    if(level >= ftb_log_level_all && level <= ftb_log_level_error) {
        ctx->loger.level = level;
        return true;
    }
    return false;
}

typedef struct {
    const char* ptr;
    u32 count;
} ftb_str_cut_t;

ftb_str_cut_t ftb_str_sepc_cstr
(char c, const char* path, u32 len)
{
    if(!path || !len) { return (ftb_str_cut_t){0}; }
    for(u32 i = 0; i < len; ++i) {
        if(path[i] == c) {
            return (ftb_str_cut_t){.ptr = path, .count = i};
        }
    }
    return (ftb_str_cut_t){.ptr = path, .count = len};
}

ftb_str_cut_t ftb_str_sepc_cstr_rev
(char c, const char* path, u32 len)
{
    if(!path || !len) { return (ftb_str_cut_t){0}; }
    u32 i = len;
    while(i-- > 0) {
        if(path[i] == c) {
            return (ftb_str_cut_t){.ptr = &path[i], .count = len - i};
        }
    }
    return (ftb_str_cut_t){.ptr = path, .count = len};
}

ftb_str_cut_t ftb_str_sepc_cstr_rev_wout
(char want, char wout, const char* path, u32 len)
{
    if(!path || !len) { return (ftb_str_cut_t){0}; }
    u32 i = len;
    while(i-- > 0) {
        if(path[i] == want) {
            ++i;
            return (ftb_str_cut_t){.ptr = &path[i], .count = len - i};
        }
        if(path[i] == wout) { 
            break; 
        }
    }
    return (ftb_str_cut_t){0};
}

ftb_str_cut_t ftb_str_sepc_cstr_wout
(char want, char wout, const char* path, u32 len)
{
    if(!path || !len) { return (ftb_str_cut_t){0}; }
    for(u32 i = 0; i < len; ++i) {
        if(path[i] == want) {
            return (ftb_str_cut_t){.ptr = path, .count = i};
        }
        if(path[i] == wout) { 
            break; 
        }
    }
    return (ftb_str_cut_t){0};
}

bool ftb_path_basename_cstr
(ftb_ctx_t* ctx,ftb_path_t* out,const char* path,u32 len)
{
    ftb_error_ret((!ctx || !path || !out),false);
    if(len == 0) {return false;}
    ftb_str_clear(*out);
    ftb_str_cut_t cut = ftb_str_sepc_cstr_rev(FTB_FS_SEP, path, len);
#ifdef _WIN32
    ftb_str_cut_t cut_fwd = ftb_str_sepc_cstr_rev('/', path, len);
    if (cut_fwd.ptr && (!cut.ptr || cut_fwd.ptr > cut.ptr)) {
        cut = cut_fwd;
    }
#endif
    if(!cut.ptr || cut.count == len) {
        ftb_str_append_cstr_n(ctx, *out, path, len);
        return true;
    }
    ftb_da_appends(ctx, *out, cut.ptr + 1, cut.count - 1);
    return true;
}

bool ftb_path_dirname_cstr
(ftb_ctx_t* ctx,ftb_path_t* out,const char* path,u32 len)
{
    ftb_error_ret((!ctx || !path || !out),false);
    if(len == 0) {return false;}
    ftb_str_clear(*out);
    ftb_str_cut_t cut = ftb_str_sepc_cstr_rev(FTB_FS_SEP, path, len);
    u32 dir_len = 0;
    if (cut.count == len) {
        if (cut.ptr[0] == FTB_FS_SEP) {
            dir_len = 1;
        } else {
            return true;
        }
    } else {
        dir_len = len - cut.count; 
    }
    if (dir_len > 0) {
        ftb_str_append_cstr_n(ctx, *out, path, dir_len);
    }
    return true;
}

bool ftb_path_extension_cstr
(ftb_ctx_t* ctx,ftb_path_t* out,const char* path,u32 len)
{
    ftb_error_ret((!ctx || !path || !out),false);
    ftb_str_clear(*out);
    if(len == 0) {return false;}
    ftb_str_cut_t cut = {0};
    cut = ftb_str_sepc_cstr_rev_wout('.',FTB_FS_SEP,path,len);
    if(!cut.ptr) {return false;}
    ftb_str_append_cstr_n(ctx,*out,cut.ptr,cut.count+1);
    return true;
}

bool ftb_path_stem_cstr
(ftb_ctx_t* ctx, ftb_path_t* out, const char* path, u32 len)
{
    ftb_error_ret((!ctx || !path || !out), false);
    ftb_str_clear(*out);
    if (len == 0) { return false; }
    ftb_str_cut_t dot = ftb_str_sepc_cstr_rev_wout('.', FTB_FS_SEP, path, len);
    ftb_str_cut_t sep = ftb_str_sepc_cstr_rev(FTB_FS_SEP, path, len);
    const char* start_ptr = path;
    u32 start_len = len;
    if (sep.count > 0 && sep.ptr[0] == FTB_FS_SEP) {
        start_ptr = sep.ptr + 1;
        start_len = sep.count - 1;
    } else {
        start_ptr = sep.ptr;
        start_len = sep.count;
    }
    u32 copy_len = start_len;
    if (dot.ptr != NULL) {
        copy_len = start_len - dot.count - 1;
    }
    ftb_str_append_cstr_n(ctx,*out,start_ptr,copy_len);
    return true;
}

bool ftb_path_basename
(ftb_ctx_t* ctx,ftb_path_t* out,ftb_path_t path)
{
    return ftb_path_basename_cstr(ctx,out,path,ftb_str_len(path));
}

bool ftb_path_dirname
(ftb_ctx_t* ctx,ftb_path_t* out,ftb_path_t path)
{
    return ftb_path_dirname_cstr(ctx,out,path,ftb_str_len(path));
}

bool ftb_path_extension
(ftb_ctx_t* ctx,ftb_path_t* out,ftb_path_t path)
{
    return ftb_path_extension_cstr(ctx,out,path,ftb_str_len(path));
}

bool ftb_path_stem
(ftb_ctx_t* ctx,ftb_path_t* out,ftb_path_t path)
{
    return ftb_path_stem_cstr(ctx,out,path,ftb_str_len(path));
}

bool ftb_path_join_cstr(ftb_ctx_t* ctx, ftb_path_t* out, const char* p1, u32 len1, const char* p2, u32 len2) {
    ftb_error_ret((!ctx || !out || (!p1 && !p2)), false);
    ftb_str_clear(*out);
    if (p1 && len1 > 0) {
        ftb_str_append_cstr_n(ctx, *out, p1, len1);
    }
    if (p2 && len2 > 0) {
        bool p1_has_sep = (len1 > 0 && p1[len1 - 1] == FTB_FS_SEP);
        bool p2_has_sep = (p2[0] == FTB_FS_SEP);
        if (p1_has_sep && p2_has_sep) {
            ftb_da_appends(ctx, *out, p2 + 1, len2 - 1);
        } else if (!p1_has_sep && !p2_has_sep && len1 > 0) {
            char sep = FTB_FS_SEP;
            ftb_da_appends(ctx, *out, &sep, 1);
            ftb_str_append_cstr_n(ctx, *out, p2, len2);
        } else {
            ftb_str_append_cstr_n(ctx, *out, p2, len2);
        }
    }
    return true;
}

bool ftb_path_join(ftb_ctx_t* ctx, ftb_path_t* out, ftb_path_t p1, ftb_path_t p2) {
    return ftb_path_join_cstr(ctx, out, p1, ftb_str_len(p1), p2, ftb_str_len(p2));
}

bool ftb_path_normalize_cstr(ftb_ctx_t* ctx, ftb_path_t* out, const char* path, u32 len) {
    ftb_error_ret((!ctx || !path || !out), false);
    ftb_str_clear(*out);
    if (len == 0) return true;
    typedef struct { const char* ptr; u32 len; } ftb_path_comp_t;
    ftb_path_comp_t* stack = NULL;
    u32 i = 0;
    bool is_abs = (path[0] == FTB_FS_SEP);
    while (i < len) {
        while (i < len && path[i] == FTB_FS_SEP) i++;
        if (i >= len) break;
        u32 start = i;
        while (i < len && path[i] != FTB_FS_SEP) i++;
        u32 comp_len = i - start;
        if (comp_len == 1 && path[start] == '.') {
            continue;
        } else if (comp_len == 2 && path[start] == '.' && path[start+1] == '.') {
            if (ftb_da_count(stack) > 0) {
                ftb_path_comp_t top = stack[ftb_da_count(stack) - 1];
                if (!(top.len == 2 && top.ptr[0] == '.' && top.ptr[1] == '.')) {
                    ftb_da_count_dec(stack);
                    continue;
                }
            }
            if (!is_abs) {
                ftb_path_comp_t c = {path + start, comp_len};
                ftb_raw_da_append(stack, c);
            }
        } else {
            ftb_path_comp_t c = {path + start, comp_len};
            ftb_raw_da_append(stack, c);
        }
    }
    if (is_abs) {
        char sep = FTB_FS_SEP;
        ftb_da_appends(ctx, *out, &sep, 1);
    }
    u32 count = ftb_da_count(stack);
    for (u32 j = 0; j < count; j++) {
        ftb_str_append_cstr_n(ctx, *out, stack[j].ptr, stack[j].len);
        if (j < count - 1 || (!is_abs && count == 0)) {
            char sep = FTB_FS_SEP;
            ftb_da_appends(ctx, *out, &sep, 1);
        }
    }
    if (ftb_da_count(*out) == 0 && !is_abs) {
        char dot = '.';
        ftb_da_appends(ctx, *out, &dot, 1);
    }
    if (stack) ftb_raw_da_free(stack);
    return true;
}

bool ftb_path_normalize(ftb_ctx_t* ctx, ftb_path_t* out, ftb_path_t path) {
    return ftb_path_normalize_cstr(ctx, out, path, ftb_str_len(path));
}

bool ftb_path_with_extension_cstr(ftb_ctx_t* ctx, ftb_path_t* out, const char* path, u32 len, const char* ext, u32 ext_len) {
    ftb_error_ret((!ctx || !path || !out || !ext), false);
    ftb_str_clear(*out);
    ftb_str_cut_t dot = ftb_str_sepc_cstr_rev_wout('.', FTB_FS_SEP, path, len);
    u32 base_len = dot.ptr ? (len - dot.count - 1) : len;
    ftb_str_append_cstr_n(ctx, *out, path, base_len);
    if (ext_len > 0) {
        if (ext[0] != '.') {
            char dot_char = '.';
            ftb_da_appends(ctx, *out, &dot_char, 1);
        }
        ftb_str_append_cstr_n(ctx, *out, ext, ext_len);
    }
    return true;
}

bool ftb_path_with_extension(ftb_ctx_t* ctx, ftb_path_t* out, ftb_path_t path, ftb_path_t ext) {
    return ftb_path_with_extension_cstr(ctx, out, path, ftb_str_len(path), ext, ftb_str_len(ext));
}

bool ftb_path_is_absolute_cstr(const char* path, u32 len) {
    if (!path || len == 0) return false;
    return path[0] == FTB_FS_SEP;
}

bool ftb_path_is_absolute(ftb_path_t path) {
    return ftb_path_is_absolute_cstr(path, ftb_str_len(path));
}

bool ftb_path_is_relative_cstr(const char* path, u32 len) {
    if (!path || len == 0) return true;
    return !ftb_path_is_absolute_cstr(path, len);
}

bool ftb_path_is_relative(ftb_path_t path) {
    return ftb_path_is_relative_cstr(path, ftb_str_len(path));
}

bool ftb_path_has_extension_cstr(const char* path, u32 len) {
    if (!path || len == 0) return false;
    ftb_str_cut_t dot = ftb_str_sepc_cstr_rev_wout('.', FTB_FS_SEP, path, len);
    return dot.ptr != NULL;
}

bool ftb_path_has_extension(ftb_path_t path) {
    return ftb_path_has_extension_cstr(path, ftb_str_len(path));
}

static char* _ftb_path_temp_cstr(ftb_ctx_t* ctx, const char* path, u32 len) {
    char* tmp = ftb_mem_alloc(ctx, len + 1);
    memcpy(tmp, path, len);
    tmp[len] = '\0';
    return tmp;
}

bool ftb_path_exists_cstr(ftb_ctx_t* ctx, const char* path, u32 len) {
    if (!ctx || !path || len == 0) return false;
    char* tmp = _ftb_path_temp_cstr(ctx, path, len);
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(tmp);
    ftb_mem_free(ctx, tmp);
    return (attrs != INVALID_FILE_ATTRIBUTES);
#else
    struct stat st;
    bool exists = (stat(tmp, &st) == 0);
    ftb_mem_free(ctx, tmp);
    return exists;
#endif
}

bool ftb_path_exists(ftb_ctx_t* ctx, ftb_path_t path) {
    return ftb_path_exists_cstr(ctx, path, ftb_str_len(path));
}

bool ftb_path_is_file_cstr(ftb_ctx_t* ctx, const char* path, u32 len) {
    if (!ctx || !path || len == 0) return false;
    char* tmp = _ftb_path_temp_cstr(ctx, path, len);
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(tmp);
    ftb_mem_free(ctx, tmp);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    bool is_f = (stat(tmp, &st) == 0 && S_ISREG(st.st_mode));
    ftb_mem_free(ctx, tmp);
    return is_f;
#endif
}

bool ftb_path_is_file(ftb_ctx_t* ctx, ftb_path_t path) {
    return ftb_path_is_file_cstr(ctx, path, ftb_str_len(path));
}

bool ftb_path_is_dir_cstr(ftb_ctx_t* ctx, const char* path, u32 len) {
    if (!ctx || !path || len == 0) return false;
    char* tmp = _ftb_path_temp_cstr(ctx, path, len);
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(tmp);
    ftb_mem_free(ctx, tmp);
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    bool is_d = (stat(tmp, &st) == 0 && S_ISDIR(st.st_mode));
    ftb_mem_free(ctx, tmp);
    return is_d;
#endif
}

bool ftb_path_is_dir(ftb_ctx_t* ctx, ftb_path_t path) {
    return ftb_path_is_dir_cstr(ctx, path, ftb_str_len(path));
}

bool ftb_path_absolute_cstr(ftb_ctx_t* ctx, ftb_path_t* out, const char* path, u32 len) {
    ftb_error_ret((!ctx || !path || !out), false);
    ftb_str_clear(*out);
    char* tmp = _ftb_path_temp_cstr(ctx, path, len);
#ifdef _WIN32
    char* resolved = _fullpath(NULL, tmp, 0);
#else
    char* resolved = realpath(tmp, NULL);
#endif
    ftb_mem_free(ctx, tmp);
    if (!resolved) return false;
#ifdef _WIN32
    for (int i = 0; resolved[i]; ++i) {
        if (resolved[i] == '\\') resolved[i] = FTB_FS_SEP;
    }
#endif
    ftb_str_append_cstr_n(ctx, *out, resolved, strlen(resolved));
    free(resolved); 
    return true;
}

bool ftb_path_absolute(ftb_ctx_t* ctx, ftb_path_t* out, ftb_path_t path) {
    return ftb_path_absolute_cstr(ctx, out, path, ftb_str_len(path));
}

bool ftb_path_cwd(ftb_ctx_t* ctx, ftb_path_t* out) {
    ftb_error_ret((!ctx || !out), false);
    ftb_str_clear(*out);
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetCurrentDirectoryA(MAX_PATH, buf);
    if (len > 0 && len < MAX_PATH) {
        for (DWORD i = 0; i < len; ++i) {
            if (buf[i] == '\\') buf[i] = FTB_FS_SEP; 
        }
        ftb_str_append_cstr_n(ctx, *out, buf, len);
        return true;
    }
    return false;
#else
    char buf[4096];
    if (getcwd(buf, sizeof(buf)) != NULL) {
        ftb_da_appends(ctx, *out, buf, strlen(buf));
        return true;
    }
    return false;
#endif
}

u64 ftb_time_now_ms
(void)
{
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts; clock_gettime(CLOCK_REALTIME,&ts);
    return (u64)ts.tv_sec*1000+ts.tv_nsec/1000000;
#endif
}

u64 ftb_time_now_us
(void)
{
#ifdef _WIN32
    LARGE_INTEGER freq, t;
    QueryPerformanceFrequency(&freq); QueryPerformanceCounter(&t);
    return (u64)(t.QuadPart*1000000/freq.QuadPart);
#else
    struct timespec ts; clock_gettime(CLOCK_REALTIME,&ts); return (u64)ts.tv_sec*1000000+ts.tv_nsec/1000;
#endif
}

f64 ftb_time_now_sec
(void)
{
    return (f64)ftb_time_now_ms()/1000.0;
}

void ftb_time_sleep_ms
(u32 ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms*1000);
#endif
}

#endif /* FTB_FIRST_IMPLEMENTATION */
#endif /* FTB_IMPLEMENTATION */

/*
 *  ftb - Fanes' C toolbox for development 
 *  Copyright (C) 2025 Menderes Sabaz <sabazmenders@proton.me>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
