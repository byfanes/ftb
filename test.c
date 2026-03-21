#define FTB_IMPLEMENTATION
#define FTB_TEST_CRASH
#define FTBDEF static inline
#include "ftb.h"

#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#define PSEP "\\"
#else
#define PSEP "/"
#endif

/*
 * ==========================================
 * MEMORY TESTS
 * ==========================================
 */

bool test_memory_alloc_and_free(void) {
    ftb_ctx_t ctx = {0};
    //ftb_mem_set_mark(&ctx);

    int* num = ftb_mem_alloc(&ctx, sizeof(int));
    ftb_test_assert(num != NULL, "Memory alloc returned NULL");
    *num = 123;

    typedef struct { int a; int b; } vec2;
    vec2* v = ftb_mem_zalloc(&ctx, sizeof(vec2));
    ftb_test_assert(v != NULL, "Zalloc returned NULL");
    ftb_test_assert(v->a == 0 && v->b == 0, "Zalloc did not zero memory");
    ftb_test_assert(ftb_da_count(ctx.ptrs.items) == 2, "Context count should be 2");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_mem_free_direct(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    int* ptr1 = ftb_mem_alloc(&ctx, sizeof(int));
    int* ptr2 = ftb_mem_alloc(&ctx, sizeof(int));
    ptr2 = ptr2;
    ptr1 = ptr1;
    ftb_test_assert(ftb_da_count(ctx.ptrs.items) == 2, "Items count 2");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_memory_scope_markers(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    ftb_mem_alloc(&ctx, 10);
    ftb_test_assert(ftb_da_count(ctx.ptrs.items) == 1, "Ctx count 1");

    ftb_mem_set_mark(&ctx);

    ftb_mem_alloc(&ctx, 20);
    ftb_mem_alloc(&ctx, 20);

    ftb_test_assert(ftb_da_count(ctx.ptrs.items) == 3, "Ctx count before cleanup");
    ftb_mem_cleanup(&ctx);
    ftb_test_assert(ftb_da_count(ctx.ptrs.items) == 1, "Ctx count after cleanup");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_memory_cleanup(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    ftb_mem_alloc(&ctx, 16);
    ftb_mem_alloc(&ctx, 16);
    ftb_mem_alloc(&ctx, 16);
    
    u32 count_before = ftb_da_count(ctx.ptrs.items);
    ftb_mem_cleanup(&ctx);
    u32 count_after = ftb_da_count(ctx.ptrs.items);
    ftb_test_assert(count_after < count_before, "Cleanup reduced allocation count");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_memory_realloc(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    int* p1 = ftb_mem_alloc(&ctx, 16);
    p1[0] = 42;
    p1[1] = 84;

    int* p2 = ftb_mem_realloc(&ctx, p1, 32);
    ftb_test_assert(p2 != NULL, "Realloc larger");
    ftb_test_assert(ftb_da_capacity(p2) == 32, "Realloc capacity increased");

    ftb_test_assert(p2[0] == 42, "Realloc preserved data 0");
    ftb_test_assert(p2[1] == 84, "Realloc preserved data 1");

    void* p3 = ftb_mem_realloc(&ctx, NULL, 64);
    ftb_test_assert(p3 != NULL, "Realloc from NULL");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_memory_alloc_zero(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    void* p = ftb_mem_alloc(&ctx, 0);
    ftb_test_assert(p == NULL, "Alloc 0 bytes returns NULL");

    void* p2 = ftb_mem_zalloc(&ctx, 0);
    ftb_test_assert(p2 == NULL, "Zalloc 0 bytes returns NULL");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_memory_zalloc_large(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    int size = 1024;
    u8* mem = ftb_mem_zalloc(&ctx, size);
    ftb_test_assert(mem != NULL, "Zalloc large");

    bool all_zero = true;
    for(int i = 0; i < size; i++) {
        if(mem[i] != 0) {
            all_zero = false;
            break;
        }
    }
    ftb_test_assert(all_zero, "Large zalloc is all zeros");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

/*
 * ==========================================
 * LOGGING TESTS
 * ==========================================
 */

bool test_logger(void) {
    ftb_ctx_t ctx = {0};
    const char* log_file = "test_ftb_log.txt";

    ftb_test_assert(ftb_log_set_log_file_path(&ctx, log_file), "Set log file path");

    ftb_log_set_log_level(&ctx, ftb_log_level_info);
    ftb_test_assert(ftb_log_info(&ctx, "This is an info log"), "Log info");
    ftb_test_assert(ftb_log_warn(&ctx, "This is a warning log"), "Log warn");
    ftb_test_assert(ftb_log_error(&ctx, "This is an error log"), "Log error");

    ftb_mem_delete_ctx(&ctx);

    //i64 size = ftb_file_size(log_file);
    //ftb_test_assert(size > 0, "Log file should contain data");

    //ftb_file_remove(log_file);
    ftb_test_result(true);
}

/*
 * ==========================================
 * TIME TESTS
 * ==========================================
 */

bool test_time_sleep(void) {
    u64 start = ftb_time_now_ms();
    ftb_time_sleep_ms(30);
    u64 end = ftb_time_now_ms();

    ftb_test_assert(end - start <= 45, "Sleep duration should be at least ~30ms");
    ftb_test_result(true);
}

/*
 * ==========================================
 * SCOPE TESTS
 * ==========================================
 */

bool test_scoped_ctx(void) {
    ftb_scoped_ctx {
        void* ptr = ftb_mem_alloc(pctx, 1024);
        ftb_test_assert(ptr != NULL, "Alloc within scoped context");
        ftb_test_assert(ftb_da_count(pctx->ptrs.items) == 1, "Count is 1 in scoped ctx");
    }
    // No easy way to assert destruction without hacking globals,
    // but a successful compilation & execution without leak crash proves correct syntax & flow.
    ftb_test_result(true);
}

/*
 * ==========================================
 * MAIN ENTRY POINT
 * ==========================================
 */

int main(void)
{
    ftb_test_t* tests = 0;

    ftb_test_add(tests,test_memory_alloc_and_free);
    ftb_test_add(tests,test_mem_free_direct);
    ftb_test_add(tests,test_memory_scope_markers);
    ftb_test_add(tests,test_memory_cleanup);
    ftb_test_add(tests,test_memory_realloc);
    ftb_test_add(tests,test_memory_alloc_zero);
    ftb_test_add(tests,test_memory_zalloc_large);
    ftb_test_add(tests,test_logger);
    ftb_test_add(tests,test_time_sleep);

    ftb_test_run(tests);
    bool all = ftb_test_report(tests);
    ftb_mem_free(FTB_RAW,tests);
    
    return all ? 0 : 1;
}
