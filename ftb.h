#ifndef FTB_H_
#define FTB_H_

#ifdef _WIN32
    #define FTB_FS_SEP '\\'
    #define FTB_FS_SEP_OTHER '/'
    #include <windows.h>
    #include <malloc.h>
    #include <direct.h>
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

#define FTB_INIT_CAP 16
#define FTB_STR_DEF_CAP 64
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

/* For ftb_da_reserve
 *   amount -> is in count in da's type which will be added with existing capacity
 *   ctx -> can be a pointer to context or be null for raw allocation
 *   da -> can be a null pointer or existing da pointer
 */
#define ftb_da_reserve(ctx,da,amount)                                                     \
    do {                                                                                  \
        ftb_ptr_header_t* header = 0;                                                     \
        if((da) == NULL) {                                                                \
            usize da_s = (amount > FTB_DA_INIT_CAPACITY) ? amount : FTB_DA_INIT_CAPACITY; \
            da_s *= sizeof(*(da));                                                        \
            if((ctx) == FTB_RAW)                                                          \
            {                                                                             \
                header = FTB_CALLOC(1, da_s + sizeof(ftb_ptr_header_t));                  \
                (da) = (void*)(header + 1);                                               \
                header->addr = -1;                                                        \
            }                                                                             \
            else                                                                          \
            { (da) = ftb_mem_zalloc((ctx),da_s); header = ftb_da_header((da)); }          \
            ftb_da_set_count(da,0);                                                       \
            ftb_da_set_capacity(da,da_s);                                                 \
            ftb_da_set_elsize(da,sizeof(*(da)));                                          \
        } else {                                                                          \
            usize da_s = ftb_da_capacity((da)) + amount*sizeof(*(da));                    \
            header = ftb_da_header((da));                                                 \
            if((ctx) == FTB_RAW) {                                                        \
                header = FTB_REALLOC(header,da_s + sizeof(ftb_ptr_header_t));             \
                memset(                                                                   \
                    header + sizeof(ftb_ptr_header_t) + ftb_da_capacity((da)),            \
                    0, amount*sizeof(*(da))                                               \
                    );                                                                    \
                header->elsize = sizeof(*(da));                                           \
                header->capacity = da_s;                                                  \
                header->count = ftb_da_count((da));                                       \
                (da) = (void*)(header + 1);                                               \
            } else { (da) = ftb_mem_realloc((ctx),(da),da_s); }                           \
        }                                                                                 \
    } while(0)

/* For ftb_da_append & ftb_da_appends
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
        if(ftb_da_max_count((da)) <= ftb_da_count((da)) + 1)\
        {ftb_da_reserve((ctx),(da),1);}                     \
        (da)[ftb_da_count(da)] = (item);                    \
        ftb_da_header((da))->count++;                       \
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
#define FTB_UNUSED(x) (void)x
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
 * marks -> a da list for storing the marks.Marks are an u32 which indexing via items' count.
 * items -> a da list for allocated pointers which are will be managed by context.
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
    ftb_log_level_error,
} ftb_ctx_log_level_t;

/* For ftb_ctx_loger_t
 * log_file -> The file pointer to output the logs default is stdout it can be change.
 * log_level -> The level of logs which can pass through.
 * timestaps -> Turns on/off the log time for each printted log.
 * __close_file -> Private.Determines will the log_file should close or not.
 */
typedef struct {
    FILE* log_file;
    bool __close_file;
    bool timestaps;
    ftb_ctx_log_level_t level;
} ftb_ctx_loger_t;

/* For ftb_ctx_t
 * loger -> the library's log utils' config
 * ptrs -> stroes all of the marks to turn back and the pointer which allocated
 */
typedef struct {
    ftb_mem_ptr_list_t ptrs;
    ftb_ctx_loger_t loger;
} ftb_ctx_t;

/* For ftb_test_t
 * name -> The test suit's name.Can set manually but FTB_ADD_TEST automaticly sets it.
 * res -> After the test ran the result stored here.
 * time_usec -> How much time it take to run the test is stored here.
 * func -> The test's pointer.Test should not take any argument and return a bool.
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
         
/* For ftb_mem_[_/z/re]alloc function
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

/* For ftb_mem_delete_ctx functions
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

/* For ftb_time_now_ms
 *   -dep -> Operating system
 *   ret u64 -> Current time in ms.
 */
FTBDEF u64 ftb_time_now_ms(void);

/* For ftb_time_now_us
 *   -dep -> Operating system
 *   ret u64 -> Current time in us.
 */
FTBDEF u64 ftb_time_now_us(void);

/* For ftb_time_now_sec
 *   -dep -> Operating system
 *   ret f64 -> Current time in sec.
 */
FTBDEF f64 ftb_time_now_sec(void);

/* For ftb_time_sleep_ms
 *   -dep -> Operating system
 *   ms -> Amount of time to sleep in ms.
 */
FTBDEF void ftb_time_sleep_ms(u32 ms);

/* For ftb_test_run function
 *   tests -> The da list of ftb_test_t which are test will be run and
 *         -> will be reported the result with their run time
 *   ret bool -> If it encounters a vaild pointer it will fail with false.
 *            -> In success or tests being null it will return true.
 */
FTBDEF bool ftb_test_run(ftb_test_t* tests);

/* For ftb_test_report function
 *   Same as ftb_test_report_redirect(tests,stdout)
 */
FTBDEF bool ftb_test_report(ftb_test_t* tests);

/* For ftb_test_report_redirect function
 *   tests -> The da list of ftb_test_t.Can not be 0 in that case it fails.
 *   fptr -> The pointer to FILE.Can not be 0 in that case it fails.
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

#endif /* FTB_H_ */

#ifdef FTB_IMPLEMENTATION
#ifndef FTB_FIRST_IMPLEMENTATION
#define FTB_FIRST_IMPLEMENTATION

FTBDEF void ftb_mem_free
(ftb_ctx_t* ctx,void* ptr)
{
    if(!ptr) return;
    u32 addr = ftb_da_addr(ptr);
    free(ftb_da_header(ptr));
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
    if(ctx->loger.__close_file)
    { fclose(ctx->loger.log_file); }
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
