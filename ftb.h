#ifndef FTB_H_
#define FTB_H_

#ifdef _WIN32
    #define FTB_FS_SEP '\\'
    #define FTB_FS_SEP_OTHER '/'
    #include <windows.h>
    #include <malloc.h>
    #include <direct.h>
    #include <libloaderapi.h>
    #define FTB_IS_OS_WINDOWS true
    #define FTB_IS_OS_POSIX false
    #define ftb_alloca _alloca
#else /* _WIN32 */
    #define FTB_FS_SEP '/'
    #define FTB_FS_SEP_OTHER '\\'
    #include <sys/stat.h>
    #include <unistd.h>
    #include <limits.h>
    #include <errno.h>
    #include <dirent.h>
    #include <alloca.h>
    #include <dlfcn.h>
    #define FTB_IS_OS_WINDOWS false
    #define FTB_IS_OS_POSIX true
    #define ftb_alloca alloca
#endif /* _WIN32 */

#if defined(_WIN32)
    #define FTB_PLATFORM_NAME "Windows"
#elif defined(__APPLE__)
    #define FTB_PLATFORM_NAME "macOS"
#elif defined(__linux__)
    #define FTB_PLATFORM_NAME "Linux"
#else
    #define FTB_PLATFORM_NAME "Unknown"
#endif /* FTB_PLATFORM_NAME */

#if defined(__x86_64__) || defined(_M_X64)
    #define FTB_ARCH_NAME "x86_64"
#elif defined(__aarch64__)
    #define FTB_ARCH_NAME "arm64"
#elif defined(__i386__) || defined(_M_IX86)
    #define FTB_ARCH_NAME "x86"
#else
    #define FTB_ARCH_NAME "unknown"
#endif /* FTB_ARCH_NAME */

#ifndef FTB_RESTRICT
    #if defined(_MSC_VER)
        #define FTB_RESTRICT __restrict
    #elif defined(__GNUC__) || defined(__clang__)
        #define FTB_RESTRICT __restrict__
    #elif __STDC_VERSION__ >= 199901L
        #define FTB_RESTRICT restrict
    #else
        #define FTB_RESTRICT
    #endif /* FTB_RESTRICT */
#endif /* FTB_RESTRICT */

#ifndef FTBDEF
#define FTBDEF
#endif /* FTBDEF */

#ifndef ALLFTBDEF
#define ALLFTBDEF static inline
#endif /* ALLFTBDEF */

#ifndef FTB_CALLOC
#define FTB_CALLOC calloc
#endif /* FTB_CALLOC */

#ifndef FTB_MALLOC
#define FTB_MALLOC malloc
#endif /* FTB_MALLOC */

#ifndef FTB_FREE
#define FTB_FREE free
#endif /* FTB_FREE */

#include <stdbool.h>
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

#ifndef FTB_DA_INIT_CAPACITY
#define FTB_DA_INIT_CAPACITY 16
#endif /* FTB_DA_INIT_CAPACITY */

#define ftb_da_header(da) ((da) ? (ftb_ptr_header_t*)(da) - 1 : 0)
#define ftb_da_addr(da) ((da) ? (ftb_da_header(da))->addr : 0)
#define ftb_da_count(da) ((ftb_da_header(da))->count)
#define ftb_da_capacity(da) ((ftb_da_header(da))->capacity)
#define ftb_da_set_count(da,x) do{ (ftb_da_header(da))->count = (x);} while(0)
#define ftb_da_set_capacity(da,x) do{ (ftb_da_header(da))->capacity = (x);} while(0)
#define ftb_da_count_inc(da) ((ftb_da_header(da))->count++)
#define ftb_da_count_sum(da,x) ((ftb_da_header(da))->count+=(x))
#define ftb_da_count_sub(da,x) ((ftb_da_header(da))->count-=(x))
#define ftb_da_count_dec(da) ((ftb_da_header(da))->count--)
#define ftb_raw_da_free(da) free(ftb_da_header(da))
#define ftb_da_clamp_id(da,x) FTB_CLAMP((x), 0, (ftb_da_count(arr)-1))
#define ftb_da_foreach(type_ptr, it, da) \
    for (type_ptr it = (da); (da) != NULL && it < (da) + ftb_da_count(da); ++it)
#define ftb_da_empty(da) ((da) == NULL || ftb_da_count(da) == 0)

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
{da = ftb_mem_realloc((ctx), da, sizeof(*(da)) * _ftb_da_new_cap);                            \
 ftb_da_set_capacity(da,_ftb_da_new_cap);})

#define ftb_raw_da_reserve(da,amount)                                                         \
__ftb_da_reserve((da),(amount),                                                               \
{(header) = FTB_CALLOC(1, sizeof(*(da)) * _ftb_da_cap + sizeof(ftb_ptr_header_t));},          \
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
#define FTB_CONCAT(a,b) FTB_CONCAT2(a,b)
#define FTB_STRINGIFY(x) #x
#define FTB_TOSTRING(x) FTB_STRINGIFY(x)
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

#define FTB_IS_POW2(x) (((x) != 0) && (((x) & ((x) - 1)) == 0))
#define FTB_OFFSET_OF(type, member) ((usize)&(((type*)0)->member))
#define FTB_CONTAINER_OF(ptr, type, member) \
    ((type *)((char *)(ptr) - FTB_OFFSET_OF(type, member)))

#define ftb_scope(init, cleanup) \
    for (int _i = ((init),0); !_i; ((cleanup),++_i))

#define ftb_time_scope(name, code_block) \
    do {                                 \
        u64 _start = ftb_time_now_us();  \
        code_block;                      \
        u64 _end = ftb_time_now_us();    \
        printf("[TIME] %s: %llu us\n", name, (unsigned long long)(_end-_start)); \
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

#define UNUSED(x) (void)x
#define TODO(msg) do { printf("%s:%d: To be done:%s\n",__FILE__,__LINE__,msg);abort();} while(0)
#define UNIMPLEMENTED                                                               \
    do {                                                                            \
        fprintf(stderr,"%s:%d: %s is not implemented!",__FILE__,__LINE__,__func__); \
        abort();                                                                    \
    } while(0);

#define ftb_scoped_ctx \
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

#define ftb_strlen(str) ftb_da_count(str)
#define ftb_free_raw_str(str) ftb_da_free(str)

#define __ftb_da(da) (da),ftb_da_count(da)
#define __ftb_str(str) (str),((str) ? (u32)(strlen((str))) : 0)

#define ftb_str_append_cstr_n(ctx,str,cstr,n) \
    do {                                      \
        ftb_error_ret(!cstr,false);           \
        ftb_da_appends(ctx,str,cstr,n);       \
    } while (0)

#define ftb_str_append_cstr(ctx,str,cstr)          \
    do {                                           \
        ftb_error_ret(!cstr,false);                \
        ftb_da_appends(ctx,str,cstr,strlen(cstr)); \
    } while (0)

#define ftb_str_append_str(ctx,str1,str2)                 \
    do {                                                  \
        ftb_error_ret((!str2 || str1 == str2),false);     \
        ftb_da_appends(ctx,str1,str2,ftb_da_count(str2)); \
    } while(0)

FTBDEF ftb_str_t ftb_str_printf(ftb_ctx_t* ctx, const char* fmt, ...);
FTBDEF bool ftb_str_remove_range(ftb_str_t str, u32 start, u32 len);
FTBDEF bool ftb_str_uppercase(ftb_str_t str);
FTBDEF bool ftb_str_lowercase(ftb_str_t str);
FTBDEF bool ftb_str_trim_left(ftb_str_t str);
FTBDEF bool ftb_str_trim_right(ftb_str_t str);
FTBDEF bool ftb_str_trim(ftb_str_t str);
FTBDEF i32 ftb_str_find_char_end(const ftb_str_t str, char c);
FTBDEF i32 ftb_str_find_char_begin(const ftb_str_t str, char c);

FTBDEF bool ftb_str_cmp_cstr_n
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT cstr, const u32 len);
ALLFTBDEF bool ftb_str_cmp
(const ftb_str_t FTB_RESTRICT str1, const ftb_str_t FTB_RESTRICT str2)
{ return ftb_str_cmp_cstr_n(str1, __ftb_da(str2)); }
ALLFTBDEF bool ftb_str_cmp_cstr
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT cstr)
{ return ftb_str_cmp_cstr_n(str, __ftb_str(cstr)); }

FTBDEF bool ftb_str_starts_with_cstr_n
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT cstr, u32 len);
ALLFTBDEF bool ftb_str_starts_with
(const ftb_str_t FTB_RESTRICT str1, const ftb_str_t FTB_RESTRICT str2)
{ return ftb_str_starts_with_cstr_n(str1, __ftb_da(str2)); }
ALLFTBDEF bool ftb_str_starts_with_cstr
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT cstr)
{ return ftb_str_starts_with_cstr_n(str, __ftb_str(cstr)); }

FTBDEF bool ftb_str_ends_with_cstr_n
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT cstr, u32 len);
ALLFTBDEF bool ftb_str_ends_with
(const ftb_str_t FTB_RESTRICT str1, const ftb_str_t FTB_RESTRICT str2)
{ return ftb_str_ends_with_cstr_n(str1, __ftb_da(str2)); }
ALLFTBDEF bool ftb_str_ends_with_cstr
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT cstr)
{ return ftb_str_ends_with_cstr_n(str, __ftb_str(cstr)); }

FTBDEF bool ftb_str_contains_cstr_n
(const ftb_str_t FTB_RESTRICT haystack, const char* FTB_RESTRICT needle, u32 len);
ALLFTBDEF bool ftb_str_contains
(const ftb_str_t FTB_RESTRICT str1,const ftb_str_t FTB_RESTRICT str2)
{ return ftb_str_contains_cstr_n(str1, __ftb_da(str2)); }
ALLFTBDEF bool ftb_str_contains_cstr
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT cstr)
{ return ftb_str_contains_cstr_n(str, __ftb_str(cstr)); }

FTBDEF i32 ftb_str_find_cstr_n
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT needle, u32 len);
ALLFTBDEF i32 ftb_str_find
(const ftb_str_t FTB_RESTRICT str1, const ftb_str_t FTB_RESTRICT str2)
{ return ftb_str_find_cstr_n(str1, __ftb_da(str2)); }
ALLFTBDEF i32 ftb_str_find_cstr
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT cstr)
{ return ftb_str_find_cstr_n(str, __ftb_str(cstr)); }

typedef char* ftb_path_t;

FTBDEF bool ftb_path_cwd_get(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out);
FTBDEF bool ftb_path_cwd_set(const char* path);
FTBDEF bool ftb_path_rename(const char* FTB_RESTRICT from, const char* FTB_RESTRICT to);
FTBDEF bool ftb_path_change_sep(ftb_path_t path, char cur_sep, char new_sep);

FTBDEF bool ftb_path_basename_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out,
const char* FTB_RESTRICT path, const u32 len);
ALLFTBDEF bool ftb_path_basename
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, ftb_path_t FTB_RESTRICT path)
{ return ftb_path_basename_cstr_n(ctx, out, __ftb_da(path)); }
ALLFTBDEF bool ftb_path_basename_cstr
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path)
{ return ftb_path_basename_cstr_n(ctx, out, __ftb_str(path)); }

FTBDEF bool ftb_path_dirname_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out,
const char* FTB_RESTRICT path, u32 len);
ALLFTBDEF bool ftb_path_dirname
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, ftb_path_t FTB_RESTRICT path)
{ return ftb_path_dirname_cstr_n(ctx, out, __ftb_da(path)); }
ALLFTBDEF bool ftb_path_dirname_cstr
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path)
{ return ftb_path_dirname_cstr_n(ctx, out, __ftb_str(path)); }

FTBDEF bool ftb_path_extension_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out,
const char* FTB_RESTRICT path, u32 len);
ALLFTBDEF bool ftb_path_extension
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, ftb_path_t FTB_RESTRICT path)
{ return ftb_path_extension_cstr_n(ctx, out, __ftb_da(path)); }
ALLFTBDEF bool ftb_path_extension_cstr
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path)
{ return ftb_path_extension_cstr_n(ctx, out, __ftb_str(path)); }

FTBDEF bool ftb_path_stem_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out,
const char* FTB_RESTRICT path, u32 len);
ALLFTBDEF bool ftb_path_stem
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, ftb_path_t FTB_RESTRICT path)
{ return ftb_path_stem_cstr_n(ctx, out, __ftb_da(path)); }
ALLFTBDEF bool ftb_path_stem_cstr
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path)
{ return ftb_path_stem_cstr_n(ctx, out, __ftb_str(path)); }

FTBDEF bool ftb_path_join_cstr_n(
ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out,
const char* FTB_RESTRICT p1, u32 len1, const char* FTB_RESTRICT p2, u32 len2);
ALLFTBDEF bool ftb_path_join
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out,
ftb_path_t FTB_RESTRICT p1, ftb_path_t FTB_RESTRICT p2)
{ return ftb_path_join_cstr_n(ctx, out, __ftb_da(p1), __ftb_da(p2)); }
ALLFTBDEF bool ftb_path_join_cstr
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out,
const char* FTB_RESTRICT p1, const char* FTB_RESTRICT p2)
{ return ftb_path_join_cstr_n(ctx, out, __ftb_str(p1), __ftb_str(p2)); }

FTBDEF bool ftb_path_normalize_cstr_n(ftb_ctx_t* FTB_RESTRICT ctx,
ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path, u32 len);
ALLFTBDEF bool ftb_path_normalize(ftb_ctx_t* FTB_RESTRICT ctx,
ftb_path_t* FTB_RESTRICT out, ftb_path_t FTB_RESTRICT path)
{ return ftb_path_normalize_cstr_n(ctx, out, __ftb_da(path)); }
ALLFTBDEF bool ftb_path_normalize_cstr(ftb_ctx_t* FTB_RESTRICT ctx,
ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path)
{ return ftb_path_normalize_cstr_n(ctx, out, __ftb_str(path)); }

FTBDEF bool ftb_path_with_extension_cstr_n(ftb_ctx_t* FTB_RESTRICT ctx,
ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path,
u32 len, const char* FTB_RESTRICT ext, u32 ext_len);
ALLFTBDEF bool ftb_path_with_extension(ftb_ctx_t* FTB_RESTRICT ctx,
ftb_path_t* FTB_RESTRICT out, ftb_path_t FTB_RESTRICT path, ftb_path_t FTB_RESTRICT ext)
{ return ftb_path_with_extension_cstr_n(ctx, out, __ftb_da(path), __ftb_da(ext)); }
ALLFTBDEF bool ftb_path_with_extension_cstr(ftb_ctx_t* FTB_RESTRICT ctx,
ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path, const char* FTB_RESTRICT ext)
{ return ftb_path_with_extension_cstr_n(ctx, out, __ftb_str(path), __ftb_str(ext)); }

FTBDEF bool ftb_path_is_absolute_cstr_n(const char* FTB_RESTRICT path, u32 len);
ALLFTBDEF bool ftb_path_is_absolute(ftb_path_t FTB_RESTRICT path)
{ return ftb_path_is_absolute_cstr_n(__ftb_da(path)); }
ALLFTBDEF bool ftb_path_is_absolute_cstr(const char* FTB_RESTRICT path)
{ return ftb_path_is_absolute_cstr_n(__ftb_str(path)); }

FTBDEF bool ftb_path_is_relative_cstr_n(const char* FTB_RESTRICT path, u32 len);
ALLFTBDEF bool ftb_path_is_relative(ftb_path_t FTB_RESTRICT path)
{ return ftb_path_is_relative_cstr_n(__ftb_da(path)); }
ALLFTBDEF bool ftb_path_is_relative_cstr(const char* FTB_RESTRICT path)
{ return ftb_path_is_relative_cstr_n(__ftb_str(path)); }

FTBDEF bool ftb_path_has_extension_cstr_n(const char* FTB_RESTRICT path, u32 len);
ALLFTBDEF bool ftb_path_has_extension(ftb_path_t FTB_RESTRICT path)
{ return ftb_path_has_extension_cstr_n(__ftb_da(path)); }
ALLFTBDEF bool ftb_path_has_extension_cstr(const char* FTB_RESTRICT path)
{ return ftb_path_has_extension_cstr_n(__ftb_str(path)); }

FTBDEF bool ftb_path_exists_cstr_n(ftb_ctx_t* FTB_RESTRICT ctx,
const char* FTB_RESTRICT path, u32 len);
ALLFTBDEF bool ftb_path_exists(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t FTB_RESTRICT path)
{ return ftb_path_exists_cstr_n(ctx, __ftb_da(path)); }
ALLFTBDEF bool ftb_path_exists_cstr(ftb_ctx_t* FTB_RESTRICT ctx, const char* FTB_RESTRICT path)
{ return ftb_path_exists_cstr_n(ctx, __ftb_str(path)); }

FTBDEF bool ftb_path_is_file_cstr_n(ftb_ctx_t* FTB_RESTRICT ctx,
const char* FTB_RESTRICT path, u32 len);
ALLFTBDEF bool ftb_path_is_file(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t FTB_RESTRICT path)
{ return ftb_path_is_file_cstr_n(ctx, __ftb_da(path)); }
ALLFTBDEF bool ftb_path_is_file_cstr(ftb_ctx_t* FTB_RESTRICT ctx, const char* FTB_RESTRICT path)
{ return ftb_path_is_file_cstr_n(ctx, __ftb_str(path)); }

FTBDEF bool ftb_path_is_dir_cstr_n(ftb_ctx_t* FTB_RESTRICT ctx,
const char* FTB_RESTRICT path, u32 len);
ALLFTBDEF bool ftb_path_is_dir(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t FTB_RESTRICT path)
{ return ftb_path_is_dir_cstr_n(ctx, __ftb_da(path)); }
ALLFTBDEF bool ftb_path_is_dir_cstr(ftb_ctx_t* FTB_RESTRICT ctx, const char* FTB_RESTRICT path)
{ return ftb_path_is_dir_cstr_n(ctx, __ftb_str(path)); }

FTBDEF bool ftb_path_absolute_cstr_n(ftb_ctx_t* FTB_RESTRICT ctx,
ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path, u32 len);
ALLFTBDEF bool ftb_path_absolute(ftb_ctx_t* FTB_RESTRICT ctx,
ftb_path_t* FTB_RESTRICT out, ftb_path_t FTB_RESTRICT path)
{ return ftb_path_absolute_cstr_n(ctx, out, __ftb_da(path)); }
ALLFTBDEF bool ftb_path_absolute_cstr(ftb_ctx_t* FTB_RESTRICT ctx,
ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path)
{ return ftb_path_absolute_cstr_n(ctx, out, __ftb_str(path)); }

FTBDEF u64 ftb_time_now_ms(void);
FTBDEF u64 ftb_time_now_us(void);
FTBDEF f64 ftb_time_now_sec(void);
FTBDEF void ftb_time_sleep_ms(u32);

typedef u8* ftb_bytes_t;

FTBDEF ftb_bytes_t ftb_file_read(ftb_ctx_t* FTB_RESTRICT ctx, const char* FTB_RESTRICT path);

FTBDEF bool ftb_file_write_cstr_n(const char* FTB_RESTRICT path,
const void* FTB_RESTRICT data, size_t size);
ALLFTBDEF bool ftb_file_write(const char* FTB_RESTRICT path, const ftb_bytes_t FTB_RESTRICT data)
{ return ftb_file_write_cstr_n(path, __ftb_da(data)); }
ALLFTBDEF bool ftb_file_write_cstr(const char* FTB_RESTRICT path, const char* FTB_RESTRICT data)
{ return ftb_file_write_cstr_n(path, __ftb_str(data)); }

FTBDEF i64 ftb_file_size(const char* path);
FTBDEF bool ftb_file_remove(const char* path);
FTBDEF bool ftb_file_exists(const char* path);
FTBDEF i64 ftb_file_mtime(const char* path);
FTBDEF bool ftb_file_copy(const char* FTB_RESTRICT from, const char* FTB_RESTRICT to);
FTBDEF bool ftb_dir_exists(const char* path);
FTBDEF bool ftb_dir_mkdir(const char* path);
FTBDEF bool ftb_dir_mkdir_ifnot_exists(const char* path);
FTBDEF bool ftb_dir_list_dir
(const char* path, void (*callback)(const char*,bool,void*), void* user);

#endif /* FTB_H_ */

#ifdef FTB_IMPLEMENTATION
#ifndef FTB_FIRST_IMPLEMENTATION
#define FTB_FIRST_IMPLEMENTATION

#ifdef _WIN32
    #define _FTB_IS_SEP(c) ((c) == FTB_FS_SEP || (c) == FTB_FS_SEP_OTHER)
#else
    #define _FTB_IS_SEP(c) ((c) == FTB_FS_SEP)
#endif

FTBDEF void* ftb_mem_alloc
(ftb_ctx_t* ctx,usize bytes)
{
    assert(ctx);
    if(!bytes) return 0;
    ftb_ptr_header_t* header = FTB_MALLOC(bytes + sizeof(ftb_ptr_header_t));
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
    ftb_ptr_header_t* header = FTB_CALLOC(1,bytes + sizeof(ftb_ptr_header_t));
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
    if(!bytes && ptr) {return 0;}
    if(!ptr && bytes) {return ftb_mem_zalloc(ctx,bytes);}
    u32 cap = ftb_da_capacity(ptr);
    if(cap == bytes) {return ptr;}
    void* da = ftb_mem_zalloc(ctx,bytes);
    usize copy_size = (cap < bytes) ? cap : bytes;
    memcpy(da,ptr,copy_size);
    ftb_da_set_count(da,ftb_da_count(ptr));
    return da;
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
            FTB_FREE(ptr);
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
    if(!ctx->ptrs.items || !ctx->ptrs.marks) return;
    isize i = ftb_da_count(ctx->ptrs.items) - 1;
    assert(i >= 0);
    assert(ftb_da_count(ctx->ptrs.marks) > 0);
    isize stop = ctx->ptrs.marks[ftb_da_count(ctx->ptrs.marks)-1];
    for(;i >= stop;--i)
    {
        if(!ctx->ptrs.items[i]) {continue;}
        void* ptr = ftb_da_header(ctx->ptrs.items[i]);
        FTB_FREE(ptr);
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

FTBDEF ftb_str_t ftb_str_printf
(ftb_ctx_t* ctx, const char* fmt, ...)
{
    ftb_str_t _s = ftb_str_create(ctx);
    va_list args1, args2;
    va_start(args1, fmt);
    va_copy(args2, args1);
    int _needed = vsnprintf(NULL, 0, fmt, args1);
    va_end(args1);

    ftb_da_reserve(ctx, _s, _needed + 1);
    vsnprintf(_s, _needed + 1, fmt, args2);
    va_end(args2);
    ftb_da_set_count(_s,_needed + 1);
    return _s;
}

FTBDEF bool ftb_str_cmp_cstr_n
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT cstr, const u32 len)
{
    ftb_error_ret((!str || !cstr),false);
    if(ftb_strlen(str) != len) return false;
    return (strncmp(str,cstr,len) == 0);
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

FTBDEF bool ftb_str_starts_with_cstr_n
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT cstr, u32 len)
{
    ftb_error_ret((!str || !cstr),false);
    if(ftb_da_count(str) < len) return false;
    return (strncmp(str,cstr,len) == 0);
}

FTBDEF bool ftb_str_ends_with_cstr_n
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT cstr, u32 len)
{
    ftb_error_ret((!str || !cstr),false);
    if(ftb_da_count(str) < len) return false;
    char* start = &str[ftb_da_count(str)-len];
    return (strncmp(start,cstr,len) == 0);
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
    if(count == 0) {return true;}
    i32 i = count - 1;
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
(const ftb_str_t str, char c)
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
(const ftb_str_t str, char c)
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

FTBDEF i32 ftb_str_find_cstr_n
(const ftb_str_t FTB_RESTRICT str, const char* FTB_RESTRICT needle, u32 len)
{
    ftb_error_ret((!str || !needle),-1);
    u32 i = 0;
    for(;i < ftb_da_count(str);++i)
    {
        if(strncmp(&str[i],needle,len) == 0)
        {
            return i;
        }
    }
    return -1;
}

FTBDEF bool ftb_str_contains_cstr_n
(const ftb_str_t FTB_RESTRICT haystack, const char* FTB_RESTRICT needle, u32 len)
{
    return (ftb_str_find_cstr_n(haystack,needle,len) != -1);
}

FTBDEF bool ftb_str_remove_range
(ftb_str_t str, u32 start, u32 len)
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
        {fprintf(fptr," [%6llu us] [%s] <- %s\n",(unsigned long long)us,c,test->name);}
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
    u32 len = fmt_len + tag_len + 64;
    char* buf = 0;
    if(len >= FTB_KB(10)) {
        buf = (char*)FTB_MALLOC(len);
        assert(buf);
    } else {
        buf = (char*)ftb_alloca(len);
    }
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
    if(len >= FTB_KB(10)) { FTB_FREE(buf); }
    return (err < 0) ? false : true;
}

FTBDEF bool ftb_log
(ftb_ctx_t* ctx,const char* tag,const char* fmt,...)
{
    va_list ap;
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
    va_list ap;
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
    va_list ap;
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
    va_list ap;
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
    if(ctx->loger.level > ftb_log_level_all) return true;
    va_list ap;
    va_start(ap,fmt);
    bool err = __ftb_log(ctx,"DEBUG",fmt,ap);
    va_end(ap);
    return err;
}
#else
FTBDEF bool ftb_log_debug
(ftb_ctx_t* ctx,const char* fmt,...)
{
    UNUSED(ctx);
    UNUSED(fmt);
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

static ftb_str_cut_t _ftb_path_basename_cut(const char* path, u32 len) {
    if(!path || !len) { return (ftb_str_cut_t){0}; }
    u32 i = len;
    while(i-- > 0) {
        if(_FTB_IS_SEP(path[i])) {
            return (ftb_str_cut_t){.ptr = &path[i+1], .count = len - i - 1};
        }
    }
    return (ftb_str_cut_t){.ptr = path, .count = len};
}

static ftb_str_cut_t _ftb_path_find_ext_dot(const char* path, u32 len) {
    if(!path || !len) { return (ftb_str_cut_t){0}; }
    u32 i = len;
    while(i-- > 0) {
        if(path[i] == '.') {
            return (ftb_str_cut_t){.ptr = &path[i+1], .count = len - i - 1};
        }
        if(_FTB_IS_SEP(path[i])) {
            break;
        }
    }
    return (ftb_str_cut_t){0};
}

static char* _ftb_path_temp_cstr(ftb_ctx_t* FTB_RESTRICT ctx, const char* FTB_RESTRICT path, u32 len)
{
    char* tmp = ftb_mem_alloc(ctx, len + 1);
    memcpy(tmp, path, len);
    tmp[len] = '\0';
    return tmp;
}

FTBDEF bool ftb_path_basename_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path, const u32 len)
{
    ftb_error_ret((!ctx || !path || !out),false);
    if(len == 0) {return false;}
    ftb_str_clear(*out);
    ftb_str_cut_t cut = _ftb_path_basename_cut(path, len);
    ftb_str_append_cstr_n(ctx, *out, cut.ptr, cut.count);
    return true;
}

FTBDEF bool ftb_path_change_sep
(ftb_path_t path, char cur_sep, char new_sep)
{
    ftb_error_ret((!path || (cur_sep == new_sep) || !new_sep || !cur_sep),false);
    u32 i = 0;
    for(;i < ftb_da_count(path);i++)
    {
        if(path[i] == cur_sep) {path[i] = new_sep;}
    }
    return true;
}

FTBDEF bool ftb_path_dirname_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path, u32 len)
{
    ftb_error_ret((!ctx || !path || !out),false);
    if(len == 0) {return false;}
    ftb_str_clear(*out);
    ftb_str_cut_t cut = _ftb_path_basename_cut(path, len);
    u32 dir_len = len - cut.count;
    if (dir_len > 0) {
        if (!(dir_len == 1 && _FTB_IS_SEP(path[0]))) {
            dir_len--;
        }
        ftb_str_append_cstr_n(ctx, *out, path, dir_len);
    }
    return true;
}

FTBDEF bool ftb_path_extension_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path, u32 len)
{
    ftb_error_ret((!ctx || !path || !out),false);
    ftb_str_clear(*out);
    if(len == 0) {return false;}
    ftb_str_cut_t dot = _ftb_path_find_ext_dot(path, len);
    if(!dot.ptr) {return false;}
    ftb_str_append_cstr_n(ctx,*out,dot.ptr,dot.count);
    return true;
}

FTBDEF bool ftb_path_stem_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path, u32 len)
{
    ftb_error_ret((!ctx || !path || !out), false);
    ftb_str_clear(*out);
    if (len == 0) { return false; }
    ftb_str_cut_t base = _ftb_path_basename_cut(path, len);
    ftb_str_cut_t dot = _ftb_path_find_ext_dot(base.ptr, base.count);
    u32 copy_len = base.count;
    if (dot.ptr != NULL) {
        copy_len = base.count - dot.count - 1;
    }
    ftb_str_append_cstr_n(ctx,*out,base.ptr,copy_len);
    return true;
}

FTBDEF bool ftb_path_join_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT p1, u32 len1, const char* FTB_RESTRICT p2, u32 len2)
{
    ftb_error_ret((!ctx || !out || (!p1 && !p2)), false);
    ftb_str_clear(*out);
    if (p1 && len1 > 0) {
        ftb_str_append_cstr_n(ctx, *out, p1, len1);
    }
    if (p2 && len2 > 0) {
        bool p1_has_sep = (len1 > 0 && _FTB_IS_SEP(p1[len1 - 1]));
        bool p2_has_sep = _FTB_IS_SEP(p2[0]);
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

FTBDEF bool ftb_path_normalize_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path, u32 len)
{
    ftb_error_ret((!ctx || !path || !out), false);
    ftb_str_clear(*out);
    if (len == 0) return true;
    typedef struct { const char* ptr; u32 len; } ftb_path_comp_t;
    ftb_path_comp_t* stack = NULL;
    u32 i = 0;
    bool is_abs = _FTB_IS_SEP(path[0]);
    while (i < len) {
        while (i < len && _FTB_IS_SEP(path[i])) i++;
        if (i >= len) break;
        u32 start = i;
        while (i < len && !_FTB_IS_SEP(path[i])) i++;
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

FTBDEF bool ftb_path_with_extension_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path, u32 len, const char* FTB_RESTRICT ext, u32 ext_len)
{
    ftb_error_ret((!ctx || !path || !out || !ext), false);
    ftb_str_clear(*out);
    ftb_str_cut_t dot = _ftb_path_find_ext_dot(path, len);
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

FTBDEF bool ftb_path_is_absolute_cstr_n
(const char* FTB_RESTRICT path, u32 len)
{
    if (!path || len == 0) return false;
#ifdef _WIN32
    if (len >= 3 && isalpha((unsigned char)path[0]) && path[1] == ':' && _FTB_IS_SEP(path[2])) {
        return true;
    }
#endif
    return _FTB_IS_SEP(path[0]);
}

FTBDEF bool ftb_path_is_relative_cstr_n
(const char* FTB_RESTRICT path, u32 len)
{
    return !ftb_path_is_absolute_cstr_n(path, len);
}

FTBDEF bool ftb_path_has_extension_cstr_n
(const char* FTB_RESTRICT path, u32 len)
{
    if (!path || len == 0) return false;
    ftb_str_cut_t dot = _ftb_path_find_ext_dot(path, len);
    return dot.ptr != NULL;
}

FTBDEF bool ftb_path_exists_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, const char* FTB_RESTRICT path, u32 len)
{
    if (!ctx || !path || len == 0) return false;
    char* tmp = _ftb_path_temp_cstr(ctx, path, len);
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(tmp);
    return (attrs != INVALID_FILE_ATTRIBUTES);
#else
    struct stat st;
    bool exists = (stat(tmp, &st) == 0);
    return exists;
#endif
}

FTBDEF bool ftb_path_is_file_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, const char* FTB_RESTRICT path, u32 len)
{
    if (!ctx || !path || len == 0) return false;
    char* tmp = _ftb_path_temp_cstr(ctx, path, len);
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(tmp);
    return (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    bool is_f = (stat(tmp, &st) == 0 && S_ISREG(st.st_mode));
    return is_f;
#endif
}

FTBDEF bool ftb_path_is_dir_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, const char* FTB_RESTRICT path, u32 len)
{
    if (!ctx || !path || len == 0) return false;
    char* tmp = _ftb_path_temp_cstr(ctx, path, len);
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(tmp);
    return (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat st;
    bool is_d = (stat(tmp, &st) == 0 && S_ISDIR(st.st_mode));
    return is_d;
#endif
}

FTBDEF bool ftb_path_absolute_cstr_n
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out, const char* FTB_RESTRICT path, u32 len)
{
    ftb_error_ret((!ctx || !path || !out), false);
    ftb_str_clear(*out);
    char* tmp = _ftb_path_temp_cstr(ctx, path, len);
#ifdef _WIN32
    char* resolved = _fullpath(NULL, tmp, 0);
#else
    char* resolved = realpath(tmp, NULL);
#endif
    if (!resolved) return false;
    for (int i = 0; resolved[i]; ++i) {
        if (resolved[i] == FTB_FS_SEP_OTHER) {
            resolved[i] = FTB_FS_SEP;
        }
    }
    ftb_str_append_cstr_n(ctx, *out, resolved, strlen(resolved));
    FTB_FREE(resolved);
    return true;
}

FTBDEF bool ftb_path_rename
(const char* FTB_RESTRICT from, const char* FTB_RESTRICT to)
{
    ftb_error_ret((!from || !to), false);
    return rename(from,to)==0;
}

FTBDEF bool ftb_path_cwd_set
(const char* path)
{
#ifdef _WIN32
    return _chdir(path)==0;
#else
    return chdir(path)==0;
#endif
}

FTBDEF bool ftb_path_cwd_get
(ftb_ctx_t* FTB_RESTRICT ctx, ftb_path_t* FTB_RESTRICT out)
{
    ftb_error_ret((!ctx || !out), false);
    ftb_str_clear(*out);
#ifdef _WIN32
    char buf[MAX_PATH];
    DWORD len = GetCurrentDirectoryA(MAX_PATH, buf);
    if (len > 0 && len < MAX_PATH) {
        for (DWORD i = 0; i < len; ++i) {
            if (buf[i] == '\\') buf[i] = FTB_FS_SEP;
        }
        ftb_da_appends(ctx, *out, buf, len);
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

FTBDEF u64 ftb_time_now_ms
(void)
{
#ifdef _WIN32
    return GetTickCount64();
#else
    struct timespec ts; clock_gettime(CLOCK_REALTIME,&ts);
    return (u64)ts.tv_sec*1000+ts.tv_nsec/1000000;
#endif
}

FTBDEF u64 ftb_time_now_us
(void)
{
#ifdef _WIN32
    LARGE_INTEGER freq, t;
    QueryPerformanceFrequency(&freq); QueryPerformanceCounter(&t);
    return (u64)(t.QuadPart*1000000/freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME,&ts);
    return (u64)ts.tv_sec*1000000+ts.tv_nsec/1000;
#endif
}

FTBDEF f64 ftb_time_now_sec
(void)
{
    return (f64)ftb_time_now_ms()/1000.0;
}

FTBDEF void ftb_time_sleep_ms
(u32 ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms*1000);
#endif
}

FTBDEF bool ftb_file_exists
(const char* path)
{
    ftb_error_ret(!path, false);
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY));
#else
    return access(path,F_OK)==0;
#endif
}

FTBDEF bool ftb_file_copy
(const char* FTB_RESTRICT from, const char* FTB_RESTRICT to)
{
    ftb_error_ret((!from || !to), false);
    bool ok = true;
    ftb_scoped_ctx{
        ftb_bytes_t data = ftb_file_read(pctx,from);
        if(!data) {ok = false;break;}
        ok = ftb_file_write(to,data);
    }
    return ok;
}

FTBDEF bool ftb_dir_exists
(const char* path)
{
    ftb_error_ret(!path, false);
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat s;
    return stat(path,&s)==0 && S_ISDIR(s.st_mode);
#endif
}

FTBDEF bool ftb_dir_mkdir
(const char* path)
{
    ftb_error_ret(!path, false);
#ifdef _WIN32
    return _mkdir(path)==0 || errno==EEXIST;
#else
    return mkdir(path,0755)==0 || errno==EEXIST;
#endif
}

FTBDEF bool ftb_dir_mkdir_ifnot_exists
(const char* path)
{
    ftb_error_ret(!path, false);
    if(!ftb_dir_exists(path)) {
        return ftb_dir_mkdir(path);
    }
    return true;
}

FTBDEF bool ftb_file_remove
(const char* path)
{
    ftb_error_ret(!path, false);
#ifdef _WIN32
    return DeleteFileA(path);
#else
    return unlink(path)==0;
#endif
}

FTBDEF i64 ftb_file_size
(const char* path)
{
    ftb_error_ret(!path, false);
    FILE* f = fopen(path,"rb");
    if(!f) {return -1;}
    fseek(f,0,SEEK_END);
    i64 size=ftell(f);
    fclose(f);
    return size;
}

FTBDEF ftb_bytes_t ftb_file_read
(ftb_ctx_t* FTB_RESTRICT ctx, const char* FTB_RESTRICT path)
{
    ftb_error_ret((!ctx || !path), false);
    ftb_bytes_t bytes = 0;
    FILE* fptr = fopen(path,"rb");
    if(!fptr) {return 0;}
    fseek(fptr,0,SEEK_END);
    i64 size=ftell(fptr);
    rewind(fptr);
    if(size == -1) {
        fclose(fptr);
        return 0;
    }
    ftb_da_reserve(ctx,bytes,size);
    ftb_da_set_count(bytes,size);
    if(fread(bytes,size,1,fptr) != 1) {
        bytes = 0;
    }
    fclose(fptr);
    return bytes;
}

FTBDEF bool ftb_file_write_cstr_n
(const char* FTB_RESTRICT path, const void* FTB_RESTRICT data, size_t size)
{
    ftb_error_ret((!path || !data),false);
    if(size == 0) return true;
    FILE* fptr = fopen(path,"wb");
    if(!fptr) {return false;}
    if(fwrite(data,size,1,fptr) != 1) {
        fclose(fptr);
        return false;
    }
    fclose(fptr);
    return true;
}

FTBDEF i64 ftb_file_mtime
(const char* path)
{
    ftb_error_ret(!path, -1);
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA data;
    if(!GetFileAttributesExA(path,GetFileExInfoStandard,&data))
    {return -1;}
    ULARGE_INTEGER t;
    t.LowPart=data.ftLastWriteTime.dwLowDateTime;
    t.HighPart=data.ftLastWriteTime.dwHighDateTime;
    return (i64)(t.QuadPart/10000ULL);
#else
    struct stat s;
    if(stat(path,&s)<0) {return -1;}
    return (i64)s.st_mtime*1000;
#endif
}

FTBDEF bool ftb_dir_list_dir
(const char* path, void (*callback)(const char*,bool,void*), void* user)
{
#ifdef _WIN32
    char search[512];
    snprintf(search,512,"%s\\*",path);
    WIN32_FIND_DATAA fd;
    HANDLE h=FindFirstFileA(search,&fd);
    if(h==INVALID_HANDLE_VALUE) {return false;}
    do {
        if(strcmp(fd.cFileName,".") && strcmp(fd.cFileName,".."))
        {callback(fd.cFileName,(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0,user);}
    } while(FindNextFileA(h,&fd));
    FindClose(h);
#else
    DIR* d=opendir(path);
    if(!d) {return false;}
    struct dirent* e;
    while((e=readdir(d))) {
        if(strcmp(e->d_name,".") && strcmp(e->d_name,".."))
        {callback(e->d_name,(e->d_type==DT_DIR),user);}
    }
    closedir(d);
#endif
    return true;
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
