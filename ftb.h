#ifndef FTB_H_
#define FTB_H_

#ifdef _WIN32
    #define FTB_FS_SEP '\\'
    #define FTB_FS_SEP_OTHER '/'
    #include <windows.h>
    #include <malloc.h>
    #include <direct.h>
    #include <shellapi.h>
    #define FTB_IS_OS_WINDOWS true
    #define FTB_IS_OS_POSIX false
    #define ftb_alloca _alloca
#else /* _WIN32 */
    #define FTB_FS_SEP '/'
    #define FTB_FS_SEP_OTHER '\\'
    #define _XOPEN_SOURCE 500
    #define _GNU_SOURCE
    #include <ftw.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <limits.h>
    #include <errno.h>
    #include <dirent.h>
    #include <alloca.h>
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

#ifndef FTB_RAW
#define FTB_RAW 0
#endif /* FTBDEF */

#ifndef FTB_REALLOC
#define FTB_REALLOC realloc
#endif /* FTB_CALLOC */

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

#ifndef FTB_DA_INIT_CAPACITY
#define FTB_DA_INIT_CAPACITY 16
/* Calculation for the size is FTB_DA_INIT_CAPACITY*(sizeof(*da)) */
#endif /* FTB_DA_INIT_CAPACITY */

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


/* Warning / Error : For all of ftb_da_* macros
 * Dont put expression in macros just assing it to a variable then call it.
 */

#define ftb_da_header(da) ((da) ? (ftb_ptr_header_t*)(da) - 1 : 0)
#define ftb_da_addr(da) ((da) ? (ftb_da_header(da))->addr : 0)
#define ftb_da_count(da) ((da) ? (ftb_da_header(da))->count : 0)
#define ftb_da_capacity(da) ((ftb_da_header(da))->capacity)
#define ftb_da_elsize(da) ((ftb_da_header(da))->elsize)
#define ftb_da_max_count(da) (ftb_da_capacity(da) / ftb_da_elsize(da))
#define ftb_da_set_count(da,x) do{ (ftb_da_header(da))->count = (x);} while(0)
#define ftb_da_set_capacity(da,x) do{ (ftb_da_header(da))->capacity = (x);} while(0)
#define ftb_da_set_elsize(da,x) do{ (ftb_da_header(da))->elsize = (x);} while(0)
#define ftb_da_count_inc(da) ((ftb_da_header(da))->count++)
#define ftb_da_count_sum(da,x) ((ftb_da_header(da))->count+=(x))
#define ftb_da_count_sub(da,x) ((ftb_da_header(da))->count-=(x))
#define ftb_da_count_dec(da) ((ftb_da_header(da))->count--)
#define ftb_da_clamp_id(da,x) FTB_CLAMP((x), 0, (ftb_da_count(arr)-1))
#define ftb_da_foreach(type_ptr, it, da) \
    for (type_ptr it = (da); (da) != NULL && it < (da) + ftb_da_count(da); ++it)
#define ftb_da_empty(da) ((da) == NULL || ftb_da_count(da) == 0)
#define ftb_da_check_and_add_null(ctx,da)   \
    do {                                    \
        if((da) == NULL) {                  \
            ftb_da_append((ctx),(da),0);    \
        } else {                            \
            u32 count = ftb_da_count(da);   \
            if((da)[count-1] != 0) {        \
                ftb_da_append((ctx),(da),0);\
            }                               \
        }                                   \
    } while(0)
#define ftb_da_add_shadow_null(ctx,da) \
    do { ftb_da_append((ctx),(da),0);ftb_da_count_dec((da)); } while(0)
#define ftb_da_append_cstr(ctx,da,str) \
    do { ftb_da_appends((ctx),(da),str,strlen(str)); } while(0)

/* For ftb_da_reserve macro
 *   amount -> is in count in da's type which will be added with existing capacity
 *   ctx -> can be a pointer to context or be null for raw allocation
 *   da -> can be a null pointer or existing da pointer
 */
#define ftb_da_reserve(ctx,da,amount)                                            \
    do {                                                                         \
        ftb_ptr_header_t* header = 0;                                            \
        if((da) == NULL) {                                                       \
            usize da_s = (amount > FTB_DA_INIT_CAPACITY)                         \
            ? amount : FTB_DA_INIT_CAPACITY;                                     \
            da_s *= sizeof(*(da));                                               \
            if((ctx) == FTB_RAW)                                                 \
            {                                                                    \
                header = FTB_CALLOC(1, da_s + sizeof(ftb_ptr_header_t));         \
                (da) = (void*)(header + 1);                                      \
                header->addr = -1;                                               \
            }                                                                    \
            else                                                                 \
            { (da) = ftb_mem_zalloc((ctx),da_s); header = ftb_da_header((da)); } \
            ftb_da_set_count(da,0);                                              \
            ftb_da_set_capacity(da,da_s);                                        \
            ftb_da_set_elsize(da,sizeof(*(da)));                                 \
        } else {                                                                 \
            usize da_s = ftb_da_capacity((da)) + amount*sizeof(*(da));           \
            header = ftb_da_header((da));                                        \
            if((ctx) == FTB_RAW) {                                               \
                u32 count = ftb_da_count((da)), cap = ftb_da_capacity((da));     \
                header = FTB_REALLOC(header,da_s + sizeof(ftb_ptr_header_t));    \
                memset((u8*)header + sizeof(ftb_ptr_header_t) + cap,             \
                0, amount*sizeof(*(da)));                                        \
                header->elsize = sizeof(*(da));                                  \
                header->capacity = da_s;                                         \
                header->count = count;                                           \
                (da) = (void*)(header + 1);                                      \
            } else { (da) = ftb_mem_realloc((ctx),(da),da_s); }                  \
        }                                                                        \
    } while(0)

/* For ftb_da_append ftb_da_appends macro
 *   items_count -> is in count in da's type which will be added with existing capacity
 *   items -> a pointer to an array of element which will copied in size of items_count
 *   item -> an element which will copied
 *   ctx -> can be a pointer to context or be null for raw allocation
 *   da -> can be a null pointer or existing da pointer
 */
#define ftb_da_append(ctx,da,item)                          \
    do {                                                    \
        if((da) == NULL)                                    \
        {ftb_da_reserve((ctx),(da),FTB_DA_INIT_CAPACITY);}  \
        else{                                               \
            if(ftb_da_max_count((da)) < ftb_da_count((da)) + 1) \
            {ftb_da_reserve((ctx),(da),FTB_DA_INIT_CAPACITY);}  \
        }                                                   \
        (da)[ftb_da_count(da)] = (item);                    \
        ftb_da_count_inc((da));                             \
    } while(0)

#define ftb_da_appends(ctx,da,items,items_count)                                     \
    do {                                                                             \
        if((da) == NULL)                                                             \
        {ftb_da_reserve((ctx),(da),items_count);}                                    \
        if(ftb_da_max_count((da)) <= ftb_da_count((da)) + (items_count))             \
        {ftb_da_reserve((ctx),(da),(items_count));}                                  \
        memcpy((da) + ftb_da_count(da), (items), (items_count)*ftb_da_elsize((da))); \
        ftb_da_count_sum(da,items_count);                                            \
    } while(0)

#define TODO(msg) do { printf("%s:%d: To be done:%s\n",__FILE__,__LINE__,msg);abort();} while(0)
#define UNIMPLEMENTED                                                               \
    do {                                                                            \
        fprintf(stderr,"%s:%d: %s is not implemented!",__FILE__,__LINE__,__func__); \
        abort();                                                                    \
    } while(0);

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
#define FTB_CONTAINER_OF(ptr, type, member) ((type *)((char *)(ptr) - FTB_OFFSET_OF(type, member)))

#define ftb_scope(init, cleanup) for (int _i = ((init),0); !_i; ((cleanup),++_i))

#define ftb_time_scope(name, code_block) \
    do {                                 \
        u64 _start = ftb_time_now_us();  \
        code_block;                      \
        u64 _end = ftb_time_now_us();    \
        printf("[TIME] %s: %llu us\n", name, (unsigned long long)(_end-_start)); \
    } while(0)

#define ftb_scoped_ctx \
    for (ftb_ctx_t ctx = {0}, *pctx = &ctx, *_ftb_guard_##__LINE__ = &ctx; \
         _ftb_guard_##__LINE__; \
         ftb_mem_delete_ctx(pctx), _ftb_guard_##__LINE__ = NULL)

/* For ftb_ptr_header_t struct
 *   addr -> the index points to ctx pointer buffer for future changes
 *        -> it can -1 which indicates its a raw pointer
 *   count -> the count in pointers type
 *   capacity -> capacity of the array in bytes
 *   elsize -> amount of bytes every element takes
 *   (elsize * capacity + sizeof(ftb_da_header_t)) gives raw allocation size in bytes
 */
typedef struct {
    i32 addr;
    u32 count;
    u32 capacity;
    u32 elsize;
} ftb_ptr_header_t;

/* For ftb_mem_ptr_list_t struct
 *   marks -> a da list for storing the marks.Marks are an u32 which indexing via items' count.
 *   items -> a da list for allocated pointers which are will be managed by context.
 */
typedef struct {
    ftb_ptr_header_t** items;
    u32* marks;
} ftb_mem_ptr_list_t;

/* For ftb_ctx_log_level_t enum
 *   ftb_log_level_all -> Every log type is accepted and will be printted.
 *   ftb_log_level_info -> Debug logs are ingored and wont be printted.
 *   ftb_log_level_warn -> Only warn & error logs will be printted.
 *   ftb_log_level_error -> Just error logs will be printted.
 */
typedef enum {
    ftb_log_level_all = 0,
    ftb_log_level_info,
    ftb_log_level_warn,
    ftb_log_level_error
} ftb_ctx_log_level_t;

/* For ftb_ctx_logger_t struct
 *   log_file -> The file pointer to output the logs default is stdout it can be change.
 *   log_level -> The level of logs which can pass through.
 *   timestaps -> Turns on/off the log time for each printted log.
 *   __close_file -> Private.Determines will the log_file should close or not.
 */
typedef struct {
    FILE* log_file;
    bool __close_file;
    bool timestaps;
    ftb_ctx_log_level_t level;
} ftb_ctx_logger_t;

/* For ftb_ctx_t struct
 *   logger -> the library's log utils' config.
 *   ptrs -> stroes all of the marks to turn back and the pointer which allocated.
 */
typedef struct {
    ftb_mem_ptr_list_t ptrs;
    ftb_ctx_logger_t logger;
} ftb_ctx_t;

/* For ftb_test_t struct
 *   name -> The test suit's name.Can set manually but FTB_ADD_TEST automaticly sets it.
 *   res -> After the test ran the result stored here.
 *   time_usec -> How much time it take to run the test is stored here.
 *   func -> The test's pointer.Test should not take any argument and return a bool.
 */
typedef struct {
    const char* name;
    bool res;
    u64 time_usec;
    bool (*func)(void);
} ftb_test_t;

/* Rule
 * Every returned true is functions done succesfuly
 * Every returned false is functions has failed
 */

/* For ftb_mem_alloc ftb_mem_zalloc ftb_mem_realloc function
 *   bytes -> It should not contain the ftb_ptr_header_t size it is automaticly added.
 *          -> can be 0 which in that case it will return 0.
 *          -> takes bytes as in raw size in bytes(not as a count).
 *   ctx -> can not be 0 it will assert.
 *   ptr -> can be 0 which it will be same as zalloc.
 *   ret void* -> data pointer to new alloceted(*) memory or it can be 0.
 *             -> For realloc if size is same it will return same pointer.
 *   ! -> This functions never fail if they fail it will just assert.
 */
FTBDEF void* ftb_mem_alloc(ftb_ctx_t* ctx,usize bytes);
FTBDEF void* ftb_mem_zalloc(ftb_ctx_t* ctx,usize bytes);
FTBDEF void* ftb_mem_realloc(ftb_ctx_t* ctx,void* ptr,usize bytes);

/* For ftb_mem_free function
 *   ptr -> it can take both raw & ctx'ed pointers
 *   ctx -> if the pointer is raw it can be 0 but if the pointer is ctx'ed it should be set
 */
FTBDEF void ftb_mem_free(ftb_ctx_t* ctx,void* ptr);

/* For ftb_mem_delete_ctx function
 *   ctx -> pointer to context.Can be null it will instantly return
 *       -> when the function successfuly return the context will be set to 0
 *   Iterates over the items and frees and sets to 0.
 *   If a log file is open it closes.
 *   items & marks lists are freed and set to 0.
 */
FTBDEF void ftb_mem_delete_ctx(ftb_ctx_t* ctx);

/* For ftb_mem_set_mark function
 *   ctx -> pointer to context.Can not be 0 it will assert.
 *   Creates new mark based on items count .
 */
FTBDEF void ftb_mem_set_mark(ftb_ctx_t* ctx);

/* For ftb_mem_cleanup function
 *   ctx -> pointer to context.Can be null it will instantly return.
 *   Iterate over the pointers and frees untill it sees a mark.
 */
FTBDEF void ftb_mem_cleanup(ftb_ctx_t* ctx);

/* For ftb_time_now_ms function
 *   -dep -> Operating system
 *   ret u64 -> Current time in ms.
 */
FTBDEF u64 ftb_time_now_ms(void);

/* For ftb_time_now_us function
 *   -dep -> Operating system
 *   ret u64 -> Current time in us.
 */
FTBDEF u64 ftb_time_now_us(void);

/* For ftb_time_now_sec function
 *   -dep -> Operating system
 *   ret f64 -> Current time in sec.
 */
FTBDEF f64 ftb_time_now_sec(void);

/* For ftb_time_sleep_ms function
 *   -dep -> Operating system
 *   ms -> Amount of time to sleep in ms.
 */
FTBDEF void ftb_time_sleep_ms(u32 ms);

/* For ftb_test_run function
 *   tests -> The da list of ftb_test_t which are test will be run and
 *         -> will be reported the result with their run time
 *   ret bool -> If it encounters a vaild pointer it will fail with false.
 *            -> In success or 'tests' being null it will return true.
 */
FTBDEF bool ftb_test_run(ftb_test_t* tests);

/* For ftb_test_report function
 *   Same as ftb_test_report_redirect(tests,stdout)
 */
FTBDEF bool ftb_test_report(ftb_test_t* tests);

/* For ftb_test_report_redirect function
 *   tests -> The da list of ftb_test_t.Can not be 0 in that case it fails.
 *   fptr -> The pointer to file.Can not be 0 in that case it fails.
 *   ret bool -> Returns if all the tests passed or not.
 *   Prints the list of tests their name,result & time.
 */
FTBDEF bool ftb_test_report_redirect(ftb_test_t* tests,FILE* fptr);

/* For ftb_test_add macro
 *   tests -> in type of 'ftb_test_t' which is the tests da list.
 *   fn -> in type of 'bool (*fn)(void)' which is used in function pointer and test name.
 *   Appends the test to tests list.
 */
#define ftb_test_add(tests, fn)                     \
    do {                                            \
        ftb_test_t el = { .name = #fn,.func = fn }; \
        ftb_da_append(FTB_RAW,tests, el);           \
    } while (0)

/* For ftb_test_direct_assert macro
 *   cond -> if statement which if its false it will be triggered.
 *   msg -> a 'char*' which contains the message for the failure .
 *   If a condition fails it will exit the whole program with an error.
 */
#define ftb_test_direct_assert(cond,msg)          \
    do {                                          \
        if(!(cond)) {                             \
            fprintf(stderr,"[D/FAIL] %s:%d:%s: %s \n",__FILE__,__LINE__,__func__,msg); \
            exit(1);                              \
        }                                         \
    } while(0)

/* For ftb_test_result macro
 *   x -> Return x.If its true it means test is successful.If not it is failed.
 */
#define ftb_test_result(x) return x

/* For ftb_test_assert macro
 *   cond -> if statement which if its false it will be triggered.
 *   msg -> a 'char*' which contains the message for the failure .
 *   -dep -> FTB_TEST_CRASH
 *   If 'FTB_TEST_CRASH' defined it will exit program with an error
 *       with any condition which is failed.
 *   If 'FTB_TEST_CRASH' is not defined it will be return the failrue
 *       or continue the test.
 */
#ifdef FTB_TEST_CRASH
#define ftb_test_assert(cond,msg)               \
    do {                                        \
        if(!(cond)) {                           \
            fprintf(stderr,"[FAIL] %s:%d:%s: %s \n",__FILE__,__LINE__,__func__,msg); \
            exit(1);                            \
        }                                       \
    } while(0)
#else /* FTB_TEST_CRASH */
#define ftb_test_assert(cond, msg) do { if (!(cond)) { ftb_test_result(false); } } while (0)
#endif /* FTB_TEST_CRASH */

/* For __ftb_log function private
 *   ctx -> Pointer to context.
 *   tag -> The tag in c-str of the log which will be printted like [tag].
 *   fmt -> The format in c-str style which will be used in vprintf.
 *   ap  -> va_list the varible argumant list which will be taken from other functions.
 *   ret bool -> false or failure and it can be about null 'ctx','tag' or 'fmt' or
 *            -> vprintf have failed and true for success.
 */
FTBDEF bool __ftb_log
(ftb_ctx_t* ctx,const char* tag,const char* fmt,va_list ap);

/* For ftb_log ftb_log_info ftb_log_warn ftb_log_error ftb_log_debug function
 *   ctx -> Pointer to context.
 *   tag -> The tag which will be given automaticly by the function or user passes.
 *   fmt -> The format in c-str style which will be used in vprintf.
 *   ret bool -> false or failure and it can be about null 'ctx','tag' or 'fmt' or
 *            -> vprintf have failed and true for success.
 */
FTBDEF bool ftb_log(ftb_ctx_t* ctx,const char* tag,const char* fmt,...);
FTBDEF bool ftb_log_info(ftb_ctx_t* ctx,const char* fmt,...);
FTBDEF bool ftb_log_warn(ftb_ctx_t* ctx,const char* fmt,...);
FTBDEF bool ftb_log_error(ftb_ctx_t* ctx,const char* fmt,...);
FTBDEF bool ftb_log_debug(ftb_ctx_t* ctx,const char* fmt,...);

/* For ftb_log_set_timestap function
 *   ctx -> Pointer to context.
 *   x -> true/false to set timestap.
 *   ret bool -> false if 'ctx' is null otherwise true.
 */
FTBDEF bool ftb_log_set_timestap(ftb_ctx_t* ctx,bool x);

/* For ftb_log_toggle_timestap function
 *   ctx -> Pointer to context.
 *   ret bool -> false if 'ctx' is null otherwise true.
 */
FTBDEF bool ftb_log_toogle_timestap(ftb_ctx_t* ctx);

/* For ftb_log_set_log_file_path function
 *   ctx -> Pointer to context.
 *   path -> The path to the log file for printing.Which will be open with fopen("w").
 *   ret bool -> If 'ctx' or 'path' is null or it could not open the file it returns false.
 */
FTBDEF bool ftb_log_set_log_file_path(ftb_ctx_t* ctx,const char* path);

/* For ftb_log_set_log_file function
 *   ctx -> Pointer to context.
 *   file -> The file pointer to redirecting assuming it has write access.
 *   ret bool -> If 'ctx' or 'file' is null it returns false.Otherwise it returns true
 */
FTBDEF bool ftb_log_set_log_file(ftb_ctx_t* ctx,FILE* file);

/* For ftb_log_set_log_level function
 *   ctx -> Pointer to context.
 *   level -> The level of logging which determines by 'ftb_ctx_log_level_t'
 *   ret bool -> If 'ctx' is null or 'level' is out of bound is null it returns false.
 */
FTBDEF bool ftb_log_set_log_level(ftb_ctx_t* ctx,ftb_ctx_log_level_t level);

/* For ftb_bytes_t type
 *   A da list for u8 it is not null terminated.Use count instead.
 */
typedef u8* ftb_bytes_t;

/* For ftb_path_t type
 *   A da list for u8.
 */
typedef u8* ftb_path_t;

/* For ftb_str_t type
 *   A da list for u8.It is a string format which supports UTF-8.
 *   UTF8 character count can getable via 'ftb_str_get_char_count'.
 *   For printing like in printf you should use it with 'ftb_da_count'.
 *   For cstr style use you can add shadow null via ftb_da_add_shadow_null
 *   it doesnt increase the count but add a null to end it needs to be updated
 *   after modification before the usage.
 */
typedef u8* ftb_str_t;

/* For ftb_str_lowercase function
 *   str1 -> First str.Can be null.
 *   str2 -> Second str.Can be null.
 *   ret ftb_str_t -> Returns a true if both or same (in memory layout) or null.Otherwise false.
 */
FTBDEF bool ftb_str_cmp(const ftb_str_t str1,const ftb_str_t str2);

/* For ftb_str_lowercase function
 *   -dep -> May not contains some sequences.They should be implemented.
 *   ctx -> Pointing to the context.
 *   str -> A str which will be use in construction of the new str with trimmed only left.
 *   ret ftb_str_t -> Returns a null for null strs.Otherwise new str.
 */
FTBDEF ftb_str_t ftb_str_trim_left(ftb_ctx_t* ctx,const ftb_str_t src);

/* For ftb_str_lowercase function
 *   -dep -> May not contains some sequences.They should be implemented.
 *   ctx -> Pointing to the context.
 *   str -> A str which will be use in construction of the new str with trimmed only right.
 *   ret ftb_str_t -> Returns a null for null strs.Otherwise new str.
 */
FTBDEF ftb_str_t ftb_str_trim_right(ftb_ctx_t* ctx,const ftb_str_t src);

/* For ftb_str_lowercase function
 *   -dep -> May not contains some sequences.They should be implemented.
 *   ctx -> Pointing to the context.
 *   str -> A str which will be use in construction of the new str with trimmed both sides.
 *   ret ftb_str_t -> Returns a null for null strs.Otherwise new str.
 */
FTBDEF ftb_str_t ftb_str_trim(ftb_ctx_t* ctx,const ftb_str_t src);

/* For ftb_str_to_cstr function
 *   str -> A str which will be copied and append null terminator.
 *   ret bool -> Returns "\0" for null strs.Otherwise data + '\0'.
 */
FTBDEF char* ftb_str_to_cstr(ftb_ctx_t* ctx,const ftb_str_t str);

/* For ftb_str_clear function
 *   str -> A str which will be clear.Old buffer set to zero.Keeps the capacity.
 *   ret bool -> Returns a false for null strs.Otherwise true.
 */
FTBDEF bool ftb_str_clear(const ftb_str_t str);

/* For ftb_str_lowercase function
 *   -dep -> May not contains some sequences.They should be implemented.
 *   ctx -> Pointing to the context.
 *   str -> A str which will be use in construction of the new str with lowercases.
 *   ret ftb_str_t -> Returns a null for null strs.Otherwise new str.
 */
FTBDEF ftb_str_t ftb_str_lowercase(ftb_ctx_t* ctx,const ftb_str_t str);

/* For ftb_str_uppercase function
 *   -dep -> May not contains some sequences.They should be implemented.
 *   ctx -> Pointing to the context.
 *   str -> A str which will be use in construction of the new str with uppercases.
 *   ret ftb_str_t -> Returns a null for null strs.Otherwise new str.
 */
FTBDEF ftb_str_t ftb_str_uppercase(ftb_ctx_t* ctx,const ftb_str_t str);

/* For ftb_str_check_valid function
 *   str -> A str which will be validate.
 *   ret bool -> Returns a false for invalid.Otherwise true including empty-null strs.
 */
FTBDEF bool ftb_str_check_valid(const ftb_str_t str);

/* For ftb_str_get_char_count function
 *   str -> A str which will be counted.For empty-null strs return 0.
 *   ret bool -> Returns a -1 for invalid.Otherwise utf8-char-count.
 */
FTBDEF i64 ftb_str_get_char_count(const ftb_str_t str);

/* For ftb_str_get_char_count function
 *   str -> A str which will be counted.For empty-null strs return 0.Only check start bytes
 *   ret bool -> Returns a -1 for invalid.Otherwise utf8-char-count.
 */
FTBDEF i64 ftb_str_get_char_count_fast(const ftb_str_t str);

/* For ftb_path_make_from_cstr function
 *   path -> The path which will be copied and turn to 'ftb_path_t' type.
 *   ret ftb_path_t -> Returns a 'ftb_path_t' does not fail.
 */
FTBDEF ftb_path_t ftb_path_make_from_cstr(ftb_ctx_t* ctx,const char* str);

/* For ftb_path_is_dir function
 *   -dep -> Operating System.
 *   path -> The path to directory which is will checked.
 *   ret bool -> if 'path' is null it returns false.Otherwise it returns its status.
 */
FTBDEF bool ftb_path_is_dir(ftb_path_t path);

/* For ftb_path_is_file function
 *   -dep -> Operating System.
 *   path -> The path to file which is will checked.
 *   ret bool -> if 'path' is null it returns false.Otherwise it returns its status.
 */
FTBDEF bool ftb_path_is_file(ftb_path_t path);

/* For ftb_file_read function
 *   ctx -> Pointing to the context.
 *   path -> The path which is pointing to the file to read.
 *   ret ftb_bytes_t -> If 'ctx' or 'path' is null or if it could not read or open the file
 *                   -> it returns 0 otherwise it returns a da list which is ctx'ed
 */
FTBDEF ftb_bytes_t ftb_file_read(ftb_ctx_t* ctx,ftb_path_t path);

/* For ftb_file_size function
 *   -dep -> Operating System.
 *   path -> The path which is pointing to the file to get its size in bytes.
 *   ret i64 -> If 'path' is null or if it could not open the file it returns -1.
 */
FTBDEF i64 ftb_file_size(ftb_path_t path);

/* For ftb_file_remove function
 *   -dep -> Operating System.
 *   path -> The path which is pointing to the file to remove.
 *   ret bool -> If 'path' is null it return false.Otherwise it returns its status.
 */
FTBDEF bool ftb_file_remove(ftb_path_t path);

/* For ftb_file_exists function
 *   -dep -> Operating System.
 *   path -> The path which is pointing to the file to check.
 *   ret bool -> If 'path' is null it return false.Otherwise it returns its status.
 */
FTBDEF bool ftb_file_exists(ftb_path_t path);

/* For ftb_file_copy function
 *   from -> The file's path which is wanted to be copied.
 *   to -> The path to end address.
 *   ret bool -> If 'from' or 'to' null it returns false.Otherwise it returns its status.
 */
FTBDEF bool ftb_file_copy(ftb_path_t from,ftb_path_t to);

/* For ftb_file_mtime function
 *   -dep -> Operating System.
 *   path -> The path to file which will be taken its last modification time.
 *   ret bool -> if 'path' is null or in failure it returns -1.Otherwise
 */
FTBDEF i64 ftb_file_mtime(ftb_path_t path);

/* For ftb_dir_remove function
 *   -dep -> Operating System.
 *   path -> The path to directory which will removed it can contain other things.
 *   ret bool -> if 'path' is null it returns false.Otherwise it returns its status.
 */
FTBDEF bool ftb_dir_remove(ftb_path_t path);

/* For ftb_dir_exists function
 *   -dep -> Operating System.
 *   path -> The path to directory.
 *   ret bool -> if 'path' is null it returns false.Otherwise it returns its status.
 */
FTBDEF bool ftb_dir_exists(ftb_path_t path);

/* For ftb_dir_mkdir function
 *   -dep -> Operating System.
 *   path -> The path to directory which is will be created.
 *   ret bool -> if 'path' is null it returns false.Otherwise it returns its status.
 */
FTBDEF bool ftb_dir_mkdir(ftb_path_t path);

/* For ftb_dir_mkdir_ifnot_exists function
 *   -dep -> Operating System.
 *   path -> The path to directory which is will be created if it does not exists.
 *   ret bool -> if 'path' is null it returns false.Otherwise it returns its status.
 */
FTBDEF bool ftb_dir_mkdir_ifnot_exists(ftb_path_t path);

/* For ftb_dir_list_dir function
 *   -dep -> Operating System.
 *   path -> The path to directory which is will be iterated with the 'callback'.
 *   callback -> A function pointer which will be called every iteration
 *   arg -> A void* to argumant which will be given by the user and it will be passed to callback.
 *       -> If callback does not use arg it can be null.
 *   ret bool -> if 'path' or 'callback' is null it returns false.Otherwise it returns its status.
 */
FTBDEF bool ftb_dir_list_dir
(ftb_path_t path, void (*callback)(const char*,bool,void*), void* arg);

/* For ftb_file_write_n function
 *   path -> The path to the file which will be written.
 *   data -> An array for the data.Does not need to be null terminated.
 *   size -> The amount of bytes will be written to the file.
 *   ret bool -> If 'path' or 'data' is null or if file could not created or written
 *            -> it will return false.Otherwise it returns true (including size can be 0).
 */
FTBDEF bool ftb_file_write_n
(ftb_path_t path,const void* data, size_t size);

/* For ftb_file_write function
 *   path -> The path to the file which will be written.
 *   data -> The da list.For size it will use (da's count * da's elsize).
 *   ret bool -> If 'path' or 'data' is null or if file could not created or written
 *            -> it will return false.For successful operations it will return true.
 */
ALLFTBDEF bool ftb_file_write
(ftb_path_t path, const void* data)
{ return ftb_file_write_n(path,data,ftb_da_count(data)*ftb_da_elsize(data)); }

/* For ftb_file_write_cstr function
 *   path -> The path to the file which will be written.
 *   data -> The pointer to c-str.It needs to be null terminated.Size calculated with strlen.
 *   ret bool -> If 'path' or 'data' is null or if file could not created or written
 *            -> it will return false.For successful operations it will return true.
 */
ALLFTBDEF bool ftb_file_write_cstr
(ftb_path_t path, const char* data)
{ return ftb_file_write_n(path,data,strlen(data)); }

#endif /* FTB_H_ */

#ifdef FTB_IMPLEMENTATION
#ifndef FTB_FIRST_IMPLEMENTATION
#define FTB_FIRST_IMPLEMENTATION

FTBDEF void ftb_mem_free
(ftb_ctx_t* ctx,void* ptr)
{
    if(!ptr) return;
    i32 addr = ftb_da_addr(ptr);
    ftb_ptr_header_t* header = ftb_da_header(ptr);
    FTB_ZERO(*header);
    free(header);
    if(addr == -1) return;
    assert(ctx);
    assert(ctx->ptrs.items);
    ctx->ptrs.items[addr] = 0;
}

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
    ftb_da_append(FTB_RAW,ctx->ptrs.items,header);
    header->addr = ftb_da_count(ctx->ptrs.items) - 1;
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
    ftb_da_append(FTB_RAW,ctx->ptrs.items,header);
    header->addr = ftb_da_count(ctx->ptrs.items) - 1;
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
    assert(bytes >= cap);
    void* da = ftb_mem_zalloc(ctx,bytes);
    memcpy(da,ptr,cap);
    ftb_da_set_elsize(da,ftb_da_elsize(ptr));
    ftb_da_set_count(da,ftb_da_count(ptr));
    ftb_mem_free(ctx,ptr);
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
    ftb_da_append(FTB_RAW,ctx->ptrs.marks,mark);
}

FTBDEF void ftb_mem_delete_ctx
(ftb_ctx_t* ctx)
{
    if(!ctx) return;
    if(ctx->ptrs.items)
    {
        ftb_da_foreach(ftb_ptr_header_t**,it,ctx->ptrs.items)
        {
            if(!(*it)) {continue;}
            FTB_FREE(*it);
            *it = 0;
        }
        ftb_mem_free(FTB_RAW,ctx->ptrs.items);
    }
    if(ctx->ptrs.marks)
    { ftb_mem_free(FTB_RAW,ctx->ptrs.marks); }
    if(ctx->logger.__close_file)
    { fclose(ctx->logger.log_file); }
    FTB_ZERO(*ctx);
}

FTBDEF void ftb_mem_cleanup
(ftb_ctx_t* ctx)
{
    if(!ctx) return;
    if(!ctx->ptrs.items || !ctx->ptrs.marks) return;
    isize i = ftb_da_count(ctx->ptrs.items) - 1;
    assert(i >= 0);
    assert(ftb_da_count(ctx->ptrs.marks) > 0);
    isize stop = ctx->ptrs.marks[ftb_da_count(ctx->ptrs.marks)-1];
    for(;i >= stop;--i)
    {
        if(!ctx->ptrs.items[i]) {continue;}
        FTB_FREE(ctx->ptrs.items[i]);
        ctx->ptrs.items[i] = 0;
        ftb_da_count_dec(ctx->ptrs.items);
    }
}

FTBDEF bool ftb_test_run
(ftb_test_t* tests)
{
    u32 i = 0;
    if(!tests) return true;
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

FTBDEF bool ftb_test_report
(ftb_test_t* tests)
{
    return ftb_test_report_redirect(tests,stdout);
}

FTBDEF bool ftb_test_report_redirect
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

FTBDEF bool __ftb_log
(ftb_ctx_t* ctx,const char* tag,const char* fmt,va_list ap)
{
    if(!ctx || !tag || !fmt) return false;
    if(!ctx->logger.log_file) return true;
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
    if(ctx->logger.timestaps) {
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
    i32 err = vfprintf(ctx->logger.log_file,buf,ap);
    fflush(ctx->logger.log_file);
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
    if(ctx->logger.level > ftb_log_level_info) return true;
    va_list ap;
    va_start(ap,fmt);
    bool err = __ftb_log(ctx,"INFO",fmt,ap);
    va_end(ap);
    return err;
}

FTBDEF bool ftb_log_warn
(ftb_ctx_t* ctx,const char* fmt,...)
{
    if(ctx->logger.level > ftb_log_level_warn) return true;
    va_list ap;
    va_start(ap,fmt);
    bool err = __ftb_log(ctx,"WARN",fmt,ap);
    va_end(ap);
    return err;
}

FTBDEF bool ftb_log_error
(ftb_ctx_t* ctx,const char* fmt,...)
{
    if(ctx->logger.level > ftb_log_level_error) return true;
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
    if(ctx->logger.level > ftb_log_level_all) return true;
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
    FTB_UNUSED(ctx);
    FTB_UNUSED(fmt);
    return true;
}
#endif /* defined(DEBUG) || defined(FTB_DEBUG) */

FTBDEF bool ftb_log_set_timestap
(ftb_ctx_t* ctx,bool x)
{
    ftb_error_ret(!ctx,false);
    ctx->logger.timestaps = x;
    return true;
}

FTBDEF bool ftb_log_toogle_timestap
(ftb_ctx_t* ctx)
{
    ftb_error_ret(!ctx,false);
    ctx->logger.timestaps = !ctx->logger.timestaps;
    return true;
}

FTBDEF bool ftb_log_set_log_file_path
(ftb_ctx_t* ctx,const char* path)
{
    ftb_error_ret((!ctx || !path),false);
    FILE* fptr = 0;
    fptr = fopen(path,"w");
    if(!fptr) return false;
    ctx->logger.log_file = fptr;
    ctx->logger.__close_file = true;
    return true;
}

FTBDEF bool ftb_log_set_log_file
(ftb_ctx_t* ctx,FILE* file)
{
    ftb_error_ret((!ctx || !file),false);
    ctx->logger.log_file = file;
    return true;
}

FTBDEF bool ftb_log_set_log_level
(ftb_ctx_t* ctx,ftb_ctx_log_level_t level)
{
    ftb_error_ret(!ctx,false);
    if(level >= ftb_log_level_all && level <= ftb_log_level_error) {
        ctx->logger.level = level;
        return true;
    }
    return false;
}

static inline u32 __ftb_str_utf8_char_len
(const u8* str,const u32 remaining_bytes)
{
    if (remaining_bytes == 0) return 0;
    u8 c = str[0];
    if ((c & 0x80) == 0x00) {
        /* 1-byte sequence: 0xxxxxxx */
        return 1;
    }
    else if ((c & 0xE0) == 0xC0) {
        /* 2-byte sequence: 110xxxxx 10xxxxxx */
        if (remaining_bytes < 2 || (str[1] & 0xC0) != 0x80) return 0;
        return 2;
    }
    else if ((c & 0xF0) == 0xE0) {
        /* 3-byte sequence: 1110xxxx 10xxxxxx 10xxxxxx */
        if (remaining_bytes < 3 ||
           (str[1] & 0xC0) != 0x80 ||
           (str[2] & 0xC0) != 0x80) return 0;
        return 3;
    }
    else if ((c & 0xF8) == 0xF0) {
        /* 4-byte sequence: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx */
        if (remaining_bytes < 4 ||
           (str[1] & 0xC0) != 0x80 ||
           (str[2] & 0xC0) != 0x80 ||
           (str[3] & 0xC0) != 0x80) return 0;
        return 4;
    }
    /* Invalid start byte */
    return 0;
}

static inline bool __ftb_utf8_is_space(const u8* str, u32 char_len)
{
    if (char_len == 1) {
        u8 c = str[0];
        /* Standard ASCII space (0x20) or control spaces (\t, \n, \v, \f, \r) */
        return c == 0x20 || (c >= 0x09 && c <= 0x0D);
    }
    else if (char_len == 2) {
        /* NBSP (Non-Breaking Space) or NEL (Next Line) */
        if (str[0] == 0xC2)
        { return str[1] == 0xA0 || str[1] == 0x85; }
    }
    else if (char_len == 3) {
        /* Most Unicode spaces start with 0xE2 0x80 */
        if (str[0] == 0xE2) {
            if (str[1] == 0x80) {
                /* U+2000 to U+200A (En quad, Em quad, Hair space, etc.) */
                if (str[2] >= 0x80 && str[2] <= 0x8A) return true;
                /* U+2028 (Line Sep), U+2029 (Paragraph Sep) */
                if (str[2] == 0xA8 || str[2] == 0xA9) return true;
                /* U+202F (Narrow No-Break Space) */
                if (str[2] == 0xAF) return true;
            }
            else if (str[1] == 0x81 && str[2] == 0x9F)
            { return true; } /* U+205F (Medium Mathematical Space) */
        }
        else if (str[0] == 0xE3 && str[1] == 0x80 && str[2] == 0x80)
        { return true; } /* U+3000 (Ideographic Space - Fullwidth CJK space) */
        else if (str[0] == 0xE1 && str[1] == 0x9A && str[2] == 0x80)
        { return true; } /* U+1680 (Ogham Space Mark) */
    }
    /* Note: Unicode currently has no 4-byte whitespace characters. */
    return false;
}

static inline bool __ftb_str_trim_bounds
(const ftb_str_t str, u32* out_start, u32* out_end)
{
    u32 count = ftb_da_count(str);
    u32 start = 0;
    u32 end = 0;
    bool found_first_char = false;
    for(u32 i = 0; i < count;) {
        u32 len = __ftb_str_utf8_char_len(str + i, count - i);
        if(len == 0) return false;
        bool is_space = __ftb_utf8_is_space(str + i, len);
        if(!is_space) {
            if(!found_first_char) {
                start = i;
                found_first_char = true;
            }
            end = i + len;
        }
        i += len;
    }
    if(!found_first_char) {
        *out_start = 0;
        *out_end = 0;
    } else {
        *out_start = start;
        *out_end = end;
    }
    return true;
}

FTBDEF ftb_str_t ftb_str_trim_left
(ftb_ctx_t* ctx,const ftb_str_t src)
{
    ftb_error_ret(!src, NULL);
    u32 start, end;
    ftb_str_t trimmed = NULL;
    if(!__ftb_str_trim_bounds(src, &start, &end)) return 0;
    u32 count = ftb_da_count(src);
    if (count > start) {
        for(u32 i = start;i < count;++i)
             ftb_da_append(ctx,trimmed, src[i]);
    }
    return trimmed;
}

FTBDEF ftb_str_t ftb_str_trim_right
(ftb_ctx_t* ctx,const ftb_str_t src)
{
    ftb_error_ret(!src, NULL);
    u32 start, end;
    ftb_str_t trimmed = NULL;
    if(!__ftb_str_trim_bounds(src, &start, &end)) return false;
    if (end > 0) {
        for(u32 i = 0;i < end;++i)
             ftb_da_append(ctx,trimmed, src[i]);
//        ftb_da_appends(ctx,trimmed, src, end);
    }
    return trimmed;
}

FTBDEF ftb_str_t ftb_str_trim
(ftb_ctx_t* ctx,const ftb_str_t src)
{
    ftb_error_ret(!src, NULL);
    u32 start, end;
    ftb_str_t trimmed = NULL;
    if(!__ftb_str_trim_bounds(src, &start, &end)) return false;
    if (end > start) {
        ftb_da_appends(ctx,trimmed, (char*)src + start, end - start);
    }
    return trimmed;
}

static inline void __ftb_str_append_lowercase
(ftb_ctx_t* ctx,ftb_str_t* dest_str, const u8* str, u32 char_len)
{
    u32 cp = 0;
    if (char_len == 1) cp = str[0];
    else if (char_len == 2) cp = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
    else if (char_len == 3) cp = ((str[0] & 0x0F) << 12) |
        ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
    else if (char_len == 4) cp = ((str[0] & 0x07) << 18) |
        ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
    if (cp >= 'A' && cp <= 'Z') { cp += 32; }
    else if (cp >= 0x00C0 && cp <= 0x00D6) { cp += 0x20; }
    else if (cp >= 0x00D8 && cp <= 0x00DE) { cp += 0x20; }
    else if (cp == 0x0130) { cp = 0x0069; /* İ -> i */ }
    else if (cp == 0x011E) { cp = 0x011F; /* Ğ -> ğ */ }
    else if (cp == 0x015E) { cp = 0x015F; /* Ş -> ş */ }
    else if (cp == 0x0178) { cp = 0x00FF; /* Ÿ -> ÿ */ }
    else if (cp >= 0x0410 && cp <= 0x042F)
    { /* Cyrillic А-Я -> а-я */ cp += 0x20; }
    else if (cp >= 0x0391 && cp <= 0x03A9 && cp != 0x03A2)
    { /* Greek Α-Ω */ cp += 0x20; }
    else {
        /* General Latin Extended-A pairs (even -> odd) */
        if ((cp >= 0x0100 && cp <= 0x012F) || (cp >= 0x0132 && cp <= 0x0137)
            || (cp >= 0x014A && cp <= 0x0177)) {
            if ((cp % 2) == 0) cp += 1;
        } else if ((cp >= 0x0139 && cp <= 0x0148) || (cp >= 0x0179 && cp <= 0x017E)) {
            if ((cp % 2) != 0) cp += 1; /* odd -> even */
        }
    }
    if (cp <= 0x7F) {
        ftb_da_append(ctx, *dest_str, (u8)cp);
    } else if (cp <= 0x07FF) {
        ftb_da_append(ctx, *dest_str, 0xC0 | (cp >> 6));
        ftb_da_append(ctx, *dest_str, 0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        ftb_da_append(ctx, *dest_str, 0xE0 | (cp >> 12));
        ftb_da_append(ctx, *dest_str, 0x80 | ((cp >> 6) & 0x3F));
        ftb_da_append(ctx, *dest_str, 0x80 | (cp & 0x3F));
    } else {
        ftb_da_append(ctx, *dest_str, 0xF0 | (cp >> 18));
        ftb_da_append(ctx, *dest_str, 0x80 | ((cp >> 12) & 0x3F));
        ftb_da_append(ctx, *dest_str, 0x80 | ((cp >> 6) & 0x3F));
        ftb_da_append(ctx, *dest_str, 0x80 | (cp & 0x3F));
    }
}

static inline void __ftb_str_append_uppercase
(ftb_ctx_t* ctx,ftb_str_t* dest_str, const u8* str, u32 char_len)
{
    u32 cp = 0;
    if (char_len == 1) cp = str[0];
    else if (char_len == 2) cp = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
    else if (char_len == 3) cp = ((str[0] & 0x0F) << 12)
        | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
    else if (char_len == 4) cp = ((str[0] & 0x07) << 18)
        | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
    if (cp == 'i') { cp = 0x0130; /* 'i' -> 'İ' */ }
    else if (cp >= 'a' && cp <= 'z') { cp -= 32; }
    else if (cp >= 0x00E0 && cp <= 0x00F6) { cp -= 0x20; }
    else if (cp >= 0x00F8 && cp <= 0x00FE) { cp -= 0x20; }
    else if (cp == 0x00FF) { cp = 0x0178; /* ÿ -> Ÿ */ }
    else if (cp == 0x0131) { cp = 0x0049; /* 'ı' -> 'I' */ }
    else if (cp == 0x011F) { cp = 0x011E; /* ğ -> Ğ */ }
    else if (cp == 0x015F) { cp = 0x015E; /* ş -> Ş */ }
    else if (cp >= 0x0430 && cp <= 0x044F)
    { /* Cyrillic а-я -> А-Я */ cp -= 0x20; }
    else if (cp >= 0x03B1 && cp <= 0x03C9)
    { /* Greek α-ω */
        if (cp == 0x03C2) cp = 0x03A3; /* ς -> Σ */
        else cp -= 0x20;
    } else {
        /* General Latin Extended-A pairs (odd -> even) */
        if ((cp >= 0x0100 && cp <= 0x012F) || (cp >= 0x0132 && cp <= 0x0137)
            || (cp >= 0x014A && cp <= 0x0177)) {
            if ((cp % 2) != 0) cp -= 1;
        } else if ((cp >= 0x0139 && cp <= 0x0148) || (cp >= 0x0179 && cp <= 0x017E)) {
            if ((cp % 2) == 0) cp -= 1; /* even -> odd */
        }
    }
    if (cp <= 0x7F) {
        ftb_da_append(ctx, *dest_str, (u8)cp);
    } else if (cp <= 0x07FF) {
        ftb_da_append(ctx, *dest_str, 0xC0 | (cp >> 6));
        ftb_da_append(ctx, *dest_str, 0x80 | (cp & 0x3F));
    } else if (cp <= 0xFFFF) {
        ftb_da_append(ctx, *dest_str, 0xE0 | (cp >> 12));
        ftb_da_append(ctx, *dest_str, 0x80 | ((cp >> 6) & 0x3F));
        ftb_da_append(ctx, *dest_str, 0x80 | (cp & 0x3F));
    } else {
        ftb_da_append(ctx, *dest_str, 0xF0 | (cp >> 18));
        ftb_da_append(ctx, *dest_str, 0x80 | ((cp >> 12) & 0x3F));
        ftb_da_append(ctx, *dest_str, 0x80 | ((cp >> 6) & 0x3F));
        ftb_da_append(ctx, *dest_str, 0x80 | (cp & 0x3F));
    }
}

FTBDEF ftb_str_t ftb_str_uppercase
(ftb_ctx_t* ctx,const ftb_str_t src)
{
    ftb_error_ret(!src, NULL);
    ftb_str_t upper_str = NULL;
    u32 count = ftb_da_count(src);
    for (u32 i = 0; i < count; )
    {
        u32 len = __ftb_str_utf8_char_len(src + i, count - i);
        if (len == 0) { ftb_mem_free(ctx,upper_str); return 0; }
        __ftb_str_append_uppercase(ctx, &upper_str, src + i, len);
        i += len;
    }
    return upper_str;
}

FTBDEF ftb_str_t ftb_str_lowercase
(ftb_ctx_t* ctx,const ftb_str_t src)
{
    ftb_error_ret(!src, NULL);
    ftb_str_t lower_str = NULL;
    u32 count = ftb_da_count(src);
    for (u32 i = 0; i < count; )
    {
        u32 len = __ftb_str_utf8_char_len(src + i, count - i);
        if (len == 0) { ftb_mem_free(ctx,lower_str);return 0; }
        __ftb_str_append_lowercase(ctx,&lower_str, src + i, len);
        i += len;
    }
    return lower_str;
}

FTBDEF bool ftb_str_clear
(const ftb_str_t str)
{
    ftb_error_ret(!str,false);
    memset(str,0,ftb_da_count(str));
    ftb_da_set_count(str,0);
    return true;
}

FTBDEF char* ftb_str_to_cstr
(ftb_ctx_t* ctx,const ftb_str_t str)
{
    u32 count = 0;
    if(str) {
        count = ftb_da_count(str);
    }
    char* tmp = ftb_mem_alloc(ctx,count+1);
    memcpy(tmp,str,count);
    tmp[count] = 0;
    return tmp;
}

FTBDEF i64 ftb_str_get_char_count
(const ftb_str_t str)
{
    if(!str) return 0;
    u32 byte_count = ftb_da_count(str);
    u32 char_count = 0;
    for (u32 i = 0; i < byte_count; )
    {
        u32 len = __ftb_str_utf8_char_len(str + i, byte_count - i);
        if(len == 0) return -1;
        i += len;
        char_count++;
    }
    return char_count;
}

FTBDEF i64 ftb_str_get_char_count_fast
(const ftb_str_t str)
{
    if(!str) return 0;
    u32 byte_count = ftb_da_count(str);
    u32 char_count = 0;
    for (u32 i = 0; i < byte_count; ++i) {
        if ((str[i] & 0xC0) != 0x80) {
            char_count++;
        }
    }
    return char_count;
}

FTBDEF bool ftb_str_check_valid
(const ftb_str_t str)
{
    return (ftb_str_get_char_count(str) != -1);
}

FTBDEF bool ftb_str_cmp
(const ftb_str_t str1,const ftb_str_t str2)
{
    if(!str1 && !str2) return true;
    if(!str1) return false;
    if(!str2) return false;
    u32 c1 = ftb_da_count(str1);
    u32 c2 = ftb_da_count(str2);
    if(c1 != c2) return false;
    return memcmp(str1,str2,c1) == 0;
}


FTBDEF ftb_path_t ftb_path_make_from_cstr
(ftb_ctx_t* ctx,const char* str)
{
    ftb_path_t path = 0;
    u32 count = strlen(str);
    ftb_da_appends(ctx,path,str,count);
    ftb_da_set_count(path,count);
    return path;
}

static char* __ftb_tmp_safe_path
(const ftb_path_t path)
{
    // Todo: trim left and right
    char* tmp = 0;
    u32 count = ftb_da_count(path);
    tmp = malloc(count + 2);
    assert(tmp);
    memcpy(tmp,path,count);
    tmp[count] = 0;
    /* For windows */
    tmp[count + 1] = 0;
    return tmp;
}

#define __ftb_ret_free(tmp,cond) \
do { bool ok = (cond);free((tmp)); return ok;} while(0)

FTBDEF bool ftb_file_exists
(const ftb_path_t path)
{
    ftb_error_ret(!path, false);
    char* tmp = __ftb_tmp_safe_path(path);
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(path);
    __ftb_ret_free(tmp,(attr != INVALID_FILE_ATTRIBUTES && !(attr & FILE_ATTRIBUTE_DIRECTORY)));
#else
    __ftb_ret_free(tmp,access(tmp,F_OK)==0);
#endif
}

FTBDEF bool ftb_file_copy
(const ftb_path_t from,const ftb_path_t to)
{
    ftb_error_ret((!from || !to), false);
    bool ok = true;
    ftb_scoped_ctx {
        ftb_bytes_t data = ftb_file_read(pctx,from);
        if(!data) {ok = false;break;}
        ok = ftb_file_write(to,data);
    }
    return ok;
}

FTBDEF bool ftb_dir_exists
(const ftb_path_t path)
{
    ftb_error_ret(!path, false);
    char* tmp = __ftb_tmp_safe_path(path);
#ifdef _WIN32
    DWORD attr = GetFileAttributesA(tmp);
    __ftb_ret_free(tmp,(attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)));
#else
    struct stat s;
    __ftb_ret_free(tmp,(stat(tmp,&s)==0 && S_ISDIR(s.st_mode)));
#endif
}

FTBDEF bool ftb_dir_mkdir
(const ftb_path_t path)
{
    ftb_error_ret(!path, false);
    char* tmp = __ftb_tmp_safe_path(path);
#ifdef _WIN32
    __ftb_ret_free(tmp,(_mkdir(tmp)==0 || errno==EEXIST));
#else
    __ftb_ret_free(tmp,(mkdir(tmp,0755)==0 || errno==EEXIST));
#endif
}

FTBDEF bool ftb_dir_mkdir_ifnot_exists
(const ftb_path_t path)
{
    ftb_error_ret(!path, false);
    if(!ftb_dir_exists(path)) {
        bool ok = ftb_dir_mkdir(path);
        return ok;
    }
    return true;
}

FTBDEF bool ftb_file_remove
(const ftb_path_t path)
{
    ftb_error_ret(!path, false);
    char* tmp = __ftb_tmp_safe_path(path);
#ifdef _WIN32
    __ftb_ret_free(tmp,DeleteFileA(tmp));
#else
    __ftb_ret_free(tmp,unlink(tmp)==0);
#endif
}

FTBDEF i64 ftb_file_size(const ftb_path_t path)
{
    ftb_error_ret(!path, -1);
    char* tmp = __ftb_tmp_safe_path(path);
    i64 size = -1;
#if _WIN32
    struct __stat64 st;
    if (_stat64(tmp, &st) == 0) {
        size = (i64)st.st_size;
    }
#else
    struct stat st;
    if (stat(tmp, &st) == 0) {
        size = (i64)st.st_size;
    }
#endif
    free(tmp);
    return size;
}

FTBDEF ftb_bytes_t ftb_file_read
(ftb_ctx_t* ctx,const ftb_path_t path)
{
    ftb_error_ret((!ctx || !path), 0);
    ftb_bytes_t bytes = 0;
    char* tmp = __ftb_tmp_safe_path(path);
    FILE* fptr = fopen(tmp,"rb");
    free(tmp);
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
    ftb_da_set_elsize(bytes,sizeof(u8));
    if(fread(bytes,size,sizeof(u8),fptr) != 1) {
        bytes = 0;
    }
    fclose(fptr);
    return bytes;
}

#ifndef _WIN32
FTBDEF int __ftb_dir_remove_unlink_cb
(const char *fpath,const struct stat *sb,int typeflag,struct FTW *ftwbuf)
{
    FTB_UNUSED(sb);
    FTB_UNUSED(typeflag);
    FTB_UNUSED(ftwbuf);
    int rv = remove(fpath);
    if (rv) perror(fpath);
    return rv;
}
#endif

FTBDEF bool ftb_dir_remove
(const ftb_path_t path)
{
    ftb_error_ret(!path,false);
    char* tmp = __ftb_tmp_safe_path(path);
#ifdef _WIN32
    SHFILEOPSTRUCTA file_op = {
        NULL,
        FO_DELETE,
        NULL,
        NULL,
        FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT,
        FALSE,
        NULL,
        NULL
    };
    file_op.pFrom = tmp;
    __ftb_ret_free(tmp,(SHFileOperationA(&file_op) == 0));
#else
    __ftb_ret_free(tmp,nftw(tmp,__ftb_dir_remove_unlink_cb, 64, FTW_DEPTH | FTW_PHYS) == 0);
#endif
}

FTBDEF i64 ftb_file_mtime
(const ftb_path_t path)
{
    ftb_error_ret(!path, -1);
    char* tmp = __ftb_tmp_safe_path(path);
#ifdef _WIN32
    WIN32_FILE_ATTRIBUTE_DATA data;
    if(!GetFileAttributesExA(tmp,GetFileExInfoStandard,&data))
    { free(tmp); return -1;}
    ULARGE_INTEGER t;
    t.LowPart=data.ftLastWriteTime.dwLowDateTime;
    t.HighPart=data.ftLastWriteTime.dwHighDateTime;
    free(tmp);
    return (i64)(t.QuadPart/10000ULL);
#else
    struct stat s;
    if(stat(tmp,&s)<0) { free(tmp);return -1;}
    free(tmp);
    return (i64)s.st_mtime*1000;
#endif
}

FTBDEF bool ftb_file_write_n
(const ftb_path_t path,const void* data,size_t size)
{
    ftb_error_ret((!path || !data),false);
    char* tmp = __ftb_tmp_safe_path(path);
    FILE* fptr = fopen(tmp,"wb");
    free(tmp);
    if(!fptr) {return false;}
    if(size) {
        if(fwrite(data,size,1,fptr) != 1) {
            fclose(fptr);
            return false;
        }
    }
    fclose(fptr);
    return true;
}

FTBDEF bool ftb_dir_list_dir
(ftb_path_t path,void (*callback)(const char*,bool,void*),void* arg)
{
    ftb_error_ret((!path || !callback),false);
    char* tmp = __ftb_tmp_safe_path(path);
#ifdef _WIN32
    char search[512];
    snprintf(search,512,"%s\\*",tmp);
    free(tmp);
    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(search,&fd);
    if(h == INVALID_HANDLE_VALUE) {return false;}
    do {
        if(strcmp(fd.cFileName,".") && strcmp(fd.cFileName,".."))
        {callback(fd.cFileName,(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY)!=0,arg);}
    } while(FindNextFileA(h,&fd));
    FindClose(h);
#else
    DIR* d = opendir(tmp);
    free(tmp);
    if(!d) {return false;}
    struct dirent* e;
    while((e=readdir(d))) {
        if(strcmp(e->d_name,".") && strcmp(e->d_name,".."))
        {callback(e->d_name,(e->d_type==DT_DIR),arg);}
    }
    closedir(d);
#endif
    return true;
}

FTBDEF bool ftb_path_is_file
(ftb_path_t path)
{
    ftb_error_ret(!path,false);
    char* tmp = __ftb_tmp_safe_path(path);
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(tmp);
    __ftb_ret_free(tmp,(attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)));
#else
    struct stat st;
    __ftb_ret_free(tmp,(stat(tmp, &st) == 0 && S_ISREG(st.st_mode)));
#endif
}

FTBDEF bool ftb_path_is_dir
(ftb_path_t path)
{
    ftb_error_ret(!path,false);
    char* tmp = __ftb_tmp_safe_path(path);
#ifdef _WIN32
    DWORD attrs = GetFileAttributesA(tmo);
    __ftb_ret_free(tmp,(attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)));
#else
    struct stat st;
    __ftb_ret_free(tmp,(stat(tmp, &st) == 0 && S_ISDIR(st.st_mode)));
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
