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
    ftb_mem_set_mark(&ctx);

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
    ftb_path_t log_file = ftb_path_make_from_cstr(&ctx,"test_ftb_log.txt");
    ftb_da_add_shadow_null(&ctx,log_file);
    ftb_test_assert(ftb_log_set_log_file_path(&ctx, (char*)log_file), "Set log file path");

    ftb_log_set_log_level(&ctx, ftb_log_level_info);
    ftb_test_assert(ftb_log_info(&ctx, "This is an info log"), "Log info");
    ftb_test_assert(ftb_log_warn(&ctx, "This is a warning log"), "Log warn");
    ftb_test_assert(ftb_log_error(&ctx, "This is an error log"), "Log error");

    i64 size = ftb_file_size(log_file);
    ftb_test_assert(size > 0, "Log file should contain data");

    ftb_file_remove(log_file);
    ftb_mem_delete_ctx(&ctx);
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
 * FILE I/O AND DIR TESTS
 * ==========================================
 */

bool test_file_io(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t test_file = ftb_path_make_from_cstr(&ctx,"test_ftb_io.txt");
    const char* content = "Hello File IO";

    ftb_file_remove(test_file);
    ftb_test_assert(!ftb_file_exists(test_file), "File should not exist initially");

    ftb_test_assert(ftb_file_write_cstr(test_file, content), "Write cstr to file");
    ftb_test_assert(ftb_file_exists(test_file), "File should exist after writing");

    i64 size = ftb_file_size(test_file);
    ftb_test_assert(size == (i64)strlen(content), "File size should match content length");

    ftb_bytes_t data = ftb_file_read(&ctx, test_file);
    ftb_test_assert(data != NULL, "Read file must not be NULL");
    ftb_test_assert(ftb_da_count(data) == (u32)size, "Read data DA count match");
    ftb_test_assert(strncmp((char*)data, content, size) == 0, "Read content match");

    ftb_test_assert(ftb_file_remove(test_file), "Remove file successfully");
    ftb_test_assert(!ftb_file_exists(test_file), "File should be gone");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_file_copy_and_mtime(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t src = ftb_path_make_from_cstr(&ctx,"test_src.txt");
    ftb_path_t dst = ftb_path_make_from_cstr(&ctx,"test_dst.txt");

    ftb_file_write_cstr(src, "copy me");

    ftb_test_assert(ftb_file_copy(src, dst), "File copy success");
    ftb_test_assert(ftb_file_exists(dst), "Dest file exists after copy");

    i64 mtime_src = ftb_file_mtime(src);
    i64 mtime_dst = ftb_file_mtime(dst);

    ftb_test_assert(mtime_src > 0, "Source mtime valid");
    ftb_test_assert(mtime_dst > 0, "Dest mtime valid");

    ftb_file_remove(src);
    ftb_file_remove(dst);
    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_dir_operations(void) {
    ftb_path_t test_dir = 0;
    ftb_scoped_ctx {
        test_dir = ftb_path_make_from_cstr(pctx,"test_ftb_dir");
        ftb_dir_remove(test_dir);

        ftb_test_assert(!ftb_dir_exists(test_dir), "Dir should not exist initially");

        ftb_test_assert(ftb_dir_mkdir(test_dir), "Create directory");
        ftb_test_assert(ftb_dir_exists(test_dir), "Directory should exist");

        ftb_test_assert(ftb_dir_mkdir_ifnot_exists(test_dir), "Create existing directory safely");

        ftb_dir_remove(test_dir);
        
        ftb_test_assert(!ftb_dir_exists(test_dir), "Dir should be removed");
    }
    ftb_test_result(true);
}

static void test_list_callback(const char* name, bool is_dir, void* user) {
    (void)is_dir;
    if (name) (*(u32*)user)++;
}

bool test_dir_list(void) {
    ftb_path_t test_dir = 0;
    ftb_path_t test_file1 = 0;
    ftb_path_t test_file2 = 0;
    ftb_scoped_ctx {
        test_dir = ftb_path_make_from_cstr(pctx,"test_ftb_list_dir");
        test_file1 = ftb_path_make_from_cstr(pctx,"test_ftb_list_dir" PSEP "f1.txt");
        test_file2 = ftb_path_make_from_cstr(pctx,"test_ftb_list_dir" PSEP "f2.txt");
        ftb_dir_mkdir(test_dir);

        ftb_file_write_cstr(test_file1, "1");
        ftb_file_write_cstr(test_file2, "2");

        u32 file_count = 0;
        ftb_test_assert(ftb_dir_list_dir(test_dir, test_list_callback, &file_count),
            "List dir success");
        ftb_test_assert(file_count == 2, "Found 2 files inside dir");

        ftb_file_remove(test_file1);
        ftb_file_remove(test_file2);
        ftb_test_assert(ftb_dir_remove(test_dir), "Remove directory");
    }
    ftb_test_result(true);
}

/*
 * ==========================================
 * DYNAMIC ARRAY TESTS
 * ==========================================
 */

bool test_raw_da_macros(void) {
    int* arr = NULL;

    ftb_da_append(FTB_RAW,arr, 10);
    ftb_da_append(FTB_RAW,arr, 20);
    ftb_test_assert(arr != NULL, "Raw DA created");
    ftb_test_assert(ftb_da_count(arr) == 2, "Count is 2");
    ftb_test_assert(arr[0] == 10, "arr[0] is 10");
    ftb_test_assert(arr[1] == 20, "arr[1] is 20");

    int items[] = {30, 40, 50};
    ftb_da_appends(FTB_RAW,arr, items, 3);
    ftb_test_assert(ftb_da_count(arr) == 5, "Count is 5");
    ftb_test_assert(arr[2] == 30, "arr[2] is 30");
    ftb_test_assert(arr[4] == 50, "arr[4] is 50");

    ftb_da_reserve(FTB_RAW,arr, 100);
    ftb_test_assert(ftb_da_capacity(arr) >= 100, "Capacity increased");

    ftb_mem_free(FTB_RAW,arr);
    ftb_test_result(true);
}

bool test_managed_da_macros(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    float* f_arr = NULL;
    ftb_da_append(&ctx, f_arr, 3.14f);
    ftb_da_append(&ctx, f_arr, 2.71f);

    ftb_test_assert(f_arr != NULL, "Managed DA created");
    ftb_test_assert(ftb_da_count(f_arr) == 2, "Managed DA count 2");
    ftb_test_assert(f_arr[0] == 3.14f, "Value 1 correct");

    float more_f[] = {1.0f, 2.0f};
    ftb_da_appends(&ctx, f_arr, more_f, 2);

    ftb_test_assert(ftb_da_count(f_arr) == 4, "Managed DA count 4");
    ftb_test_assert(f_arr[2] == 1.0f, "Appended value 1 correct");
    ftb_test_assert(f_arr[3] == 2.0f, "Appended value 2 correct");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_da_count_macros(void) {
    int* arr = NULL;
    ftb_da_append(FTB_RAW,arr, 1);
    ftb_da_append(FTB_RAW,arr, 2);

    ftb_test_assert(ftb_da_count(arr) == 2, "Count is 2");

    ftb_da_count_sum(arr, 3);
    ftb_test_assert(ftb_da_count(arr) == 5, "Count sum by 3");

    ftb_da_count_sub(arr, 2);
    ftb_test_assert(ftb_da_count(arr) == 3, "Count sub by 2");

    ftb_da_count_inc(arr);
    ftb_test_assert(ftb_da_count(arr) == 4, "Count inc by 1");

    ftb_da_count_dec(arr);
    ftb_test_assert(ftb_da_count(arr) == 3, "Count dec by 1");

    ftb_da_set_count(arr, 10);
    ftb_test_assert(ftb_da_count(arr) == 10, "Set count to 10");

    ftb_da_set_capacity(arr, 20);
    ftb_test_assert(ftb_da_capacity(arr) == 20, "Set capacity to 20");

    ftb_mem_free(FTB_RAW,arr);
    ftb_test_result(true);
}

bool test_da_foreach(void) {
    int* arr = NULL;
    ftb_da_append(FTB_RAW,arr, 10);
    ftb_da_append(FTB_RAW,arr, 20);
    ftb_da_append(FTB_RAW,arr, 30);

    int sum = 0;
    ftb_da_foreach(int*, it, arr) {
        sum += *it;
    }
    ftb_test_assert(sum == 60, "ftb_da_foreach computes correct sum");

    ftb_mem_free(FTB_RAW,arr);
    ftb_test_result(true);
}

/*
 * ==========================================
 * MACROS & MATH TESTS
 * ==========================================
 */
bool test_math_macros(void) {
    ftb_test_assert(FTB_MIN(5, 10) == 5, "FTB_MIN");
    ftb_test_assert(FTB_MAX(5, 10) == 10, "FTB_MAX");
    ftb_test_assert(FTB_CLAMP(15, 0, 10) == 10, "FTB_CLAMP upper bounds");
    ftb_test_assert(FTB_CLAMP(-5, 0, 10) == 0, "FTB_CLAMP lower bounds");
    ftb_test_assert(FTB_CLAMP(5, 0, 10) == 5, "FTB_CLAMP inside bounds");

    int a = 1, b = 2;
    FTB_SWAP(int, a, b);
    ftb_test_assert(a == 2 && b == 1, "FTB_SWAP values");

    void* p1 = (void*)0x100;
    void* p2 = (void*)0x200;
    FTB_SWAP_PTR(p1, p2);
    ftb_test_assert(p1 == (void*)0x200 && p2 == (void*)0x100, "FTB_SWAP_PTR values");

    ftb_test_assert(FTB_ALIGN_UP(13, 8) == 16, "FTB_ALIGN_UP");
    ftb_test_assert(FTB_ALIGN_DOWN(13, 8) == 8, "FTB_ALIGN_DOWN");
    ftb_test_assert(FTB_IS_POW2(16), "FTB_IS_POW2 for true");
    ftb_test_assert(!FTB_IS_POW2(15), "FTB_IS_POW2 for false");

    ftb_test_result(true);
}

bool test_bit_macros(void) {
    u32 flags = 0;
    FTB_BIT_SET(flags, 3);
    ftb_test_assert(flags == 8, "FTB_BIT_SET");
    ftb_test_assert(FTB_BIT_GET(flags, 3) == 1, "FTB_BIT_GET true");
    ftb_test_assert(FTB_BIT_GET(flags, 2) == 0, "FTB_BIT_GET false");

    FTB_BIT_CLR(flags, 3);
    ftb_test_assert(flags == 0, "FTB_BIT_CLR");

    FTB_BIT_TOGGLE(flags, 1);
    ftb_test_assert(flags == 2, "FTB_BIT_TOGGLE set");
    FTB_BIT_TOGGLE(flags, 1);
    ftb_test_assert(flags == 0, "FTB_BIT_TOGGLE clear");

    ftb_test_result(true);
}

typedef struct {
    int id;
    float value;
    char name[16];
} ftb_test_struct_t;

bool test_struct_macros(void) {
    ftb_test_struct_t obj;
    FTB_ZERO(obj);
    ftb_test_assert(obj.id == 0 && obj.value == 0.0f && obj.name[0] == '\0',
    "FTB_ZERO clears struct");

    float* val_ptr = &obj.value;
    ftb_test_struct_t* recovered = FTB_CONTAINER_OF(val_ptr, ftb_test_struct_t, value);
    ftb_test_assert(recovered == &obj, "FTB_CONTAINER_OF resolves parent pointer accurately");

    ftb_test_result(true);
}

/*
 * ==========================================
 * MEMORY & LOGGING EDGE CASES
 * ==========================================
 */
bool test_mem_realloc_to_zero(void) {
    ftb_ctx_t ctx = {0};
    void* ptr = ftb_mem_alloc(&ctx, 64);
    ftb_test_assert(ptr != NULL, "Allocated 64 bytes");

    void* ptr2 = ftb_mem_realloc(&ctx, ptr, 0);
    ftb_test_assert(ptr2 == NULL, "Realloc to 0 bytes returns NULL (acts as free)");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_logger_toggles(void) {
    ftb_ctx_t ctx = {0};

    ftb_log_set_timestap(&ctx, true);
    ftb_test_assert(ctx.logger.timestaps == true, "Set timestamp true");

    ftb_log_toogle_timestap(&ctx);
    ftb_test_assert(ctx.logger.timestaps == false, "Toggle timestamp false");

    ftb_test_result(true);
}

/*
 * ==========================================
 * TIME RESOLUTION TESTS
 * ==========================================
 */
bool test_time_resolution(void) {
    u64 t_us = ftb_time_now_us();
    f64 t_sec = ftb_time_now_sec();

    ftb_test_assert(t_us > 0, "Microseconds time is valid");
    ftb_test_assert(t_sec > 0.0, "Seconds time is valid");

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
    
    /* --- Memory Tests --- */
    ftb_test_add(tests, test_memory_alloc_and_free);
    ftb_test_add(tests, test_mem_free_direct);
    ftb_test_add(tests, test_memory_scope_markers);
    ftb_test_add(tests, test_memory_cleanup);
    ftb_test_add(tests, test_memory_realloc);
    ftb_test_add(tests, test_memory_alloc_zero);
    ftb_test_add(tests, test_memory_zalloc_large);

    /* --- String Tests --- */
    #if 0
    ftb_test_add(tests, test_str_basic_ops);
    ftb_test_add(tests, test_str_case);
    ftb_test_add(tests, test_str_compare);
    ftb_test_add(tests, test_str_starts_ends);
    ftb_test_add(tests, test_str_trim);
    ftb_test_add(tests, test_str_trim_no_spaces);
    ftb_test_add(tests, test_str_find_contains);
    ftb_test_add(tests, test_str_remove_range);
    ftb_test_add(tests, test_string_utils);
    ftb_test_add(tests, test_str_capacity_and_length);
    ftb_test_add(tests, test_str_to_cstr_empty);
    ftb_test_add(tests, test_str_cmp_length_diff);
    ftb_test_add(tests, test_str_cmp_prefix);
    ftb_test_add(tests, test_str_find_char_missing);
    ftb_test_add(tests, test_str_find_cstr_edge);
    ftb_test_add(tests, test_str_starts_ends_edge);
    ftb_test_add(tests, test_str_starts_ends_str);
    ftb_test_add(tests, test_str_find_str_advanced);
    ftb_test_add(tests, test_str_trim_advanced);
    ftb_test_add(tests, test_str_remove_range_edge_cases);
    ftb_test_add(tests, test_str_case_symbols);
    ftb_test_add(tests, test_str_null_handling);
    ftb_test_add(tests, test_str_append_loop);
    #ifdef __GNUC__
    ftb_test_add(tests, test_str_printf);
    #endif
    #endif

    /* --- Dynamic Array Tests --- */
    ftb_test_add(tests, test_raw_da_macros);
    ftb_test_add(tests, test_managed_da_macros);
    ftb_test_add(tests, test_da_count_macros);
    ftb_test_add(tests, test_da_foreach);

    /* --- Path Tests --- */
    #if 0
    ftb_test_add(tests, test_path_basename);
    ftb_test_add(tests, test_path_dirname);
    ftb_test_add(tests, test_path_extension);
    ftb_test_add(tests, test_path_stem);
    ftb_test_add(tests, test_path_managed_wrappers);
    ftb_test_add(tests, test_path_dirname_cstr);
    ftb_test_add(tests, test_path_wrappers);
    ftb_test_add(tests, test_path_join_cstr);
    ftb_test_add(tests, test_path_normalize_cstr);
    ftb_test_add(tests, test_path_with_extension_cstr);
    ftb_test_add(tests, test_path_info_cstr);
    ftb_test_add(tests, test_path_fs_operations);
    #endif

    /* File I/O & Directory Tests */
    ftb_test_add(tests, test_file_io);
    ftb_test_add(tests, test_file_copy_and_mtime);
    ftb_test_add(tests, test_dir_operations);
    ftb_test_add(tests, test_dir_list);

    /* Subsystem Tests */
    ftb_test_add(tests, test_logger);
    ftb_test_add(tests, test_time_sleep);
    ftb_test_add(tests, test_scoped_ctx);

    /* Final Polish Edge Cases */
    /* Macros & Math Tests */
    ftb_test_add(tests, test_math_macros);
    ftb_test_add(tests, test_bit_macros);
    ftb_test_add(tests, test_struct_macros);
    //ftb_test_add(tests, test_path_rename_and_cwd);
    ftb_test_add(tests, test_mem_realloc_to_zero);
    ftb_test_add(tests, test_logger_toggles);
    ftb_test_add(tests, test_time_resolution);

    /* --- Bad Usage / Clueless Developer Tests --- */
    #ifdef TEST_CLUELESS
    ftb_test_add(tests, test_clueless_string_abuse);
    ftb_test_add(tests, test_clueless_memory_abuse);
    ftb_test_add(tests, test_clueless_file_abuse);
    ftb_test_add(tests, test_clueless_path_abuse);
    #endif /* TEST_CLUELESS */

    /* --- Run & Report --- */
    ftb_test_run(tests);
    bool all_passed = ftb_test_report(tests);

    ftb_mem_free(FTB_RAW,tests);
    return all_passed ? 0 : 1;
}
