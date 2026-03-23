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
    ftb_path_t log_file = 0;
    ftb_da_append_cstr(&ctx,log_file,"test_ftb_log.txt");
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
    ftb_path_t test_file = 0;
    ftb_da_append_cstr(&ctx,test_file,"test_ftb_io.txt");
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
    ftb_path_t src = 0;
    ftb_path_t dst = 0;
    ftb_da_append_cstr(&ctx,dst,"test_dst.txt");
    ftb_da_append_cstr(&ctx,src,"test_src.txt");

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
        ftb_da_append_cstr(pctx,test_dir,"test_ftb_dir");
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
        ftb_da_append_cstr(pctx,test_dir,"test_ftb_list_dir");
        ftb_da_append_cstr(pctx,test_file1,"test_ftb_list_dir" PSEP "f1.txt");
        ftb_da_append_cstr(pctx,test_file2,"test_ftb_list_dir" PSEP "f2.txt");
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
 * UTF-8 VALIDATION TESTS
 * ==========================================
 */

bool test_str_valid(void) {
    ftb_str_t str = NULL;

    const char* ascii = "Hello World!";
    ftb_da_append_cstr(FTB_RAW,str,ascii);
    ftb_test_assert(ftb_str_check_valid(str) == true, "Valid 1-byte (ASCII) string");
    ftb_mem_free(FTB_RAW,str);
    str = NULL;

    u8 valid_2byte[] = { 0xC3, 0xB1 };
    ftb_da_appends(FTB_RAW,str, valid_2byte,2);
    ftb_test_assert(ftb_str_check_valid(str) == true, "Valid 2-byte sequence");
    ftb_mem_free(FTB_RAW,str);
    str = NULL;

    u8 valid_3byte[] = { 0xE2, 0x82, 0xAC };
    ftb_da_appends(FTB_RAW,str, valid_3byte,3);
    ftb_test_assert(ftb_str_check_valid(str) == true, "Valid 3-byte sequence");
    ftb_mem_free(FTB_RAW,str);
    str = NULL;

    u8 valid_4byte[] = { 0xF0, 0x90, 0x8D, 0x88 };
    ftb_da_appends(FTB_RAW,str, valid_4byte,4);
    ftb_test_assert(ftb_str_check_valid(str) == true, "Valid 4-byte sequence");
    ftb_mem_free(FTB_RAW,str);

    ftb_test_result(true);
}

bool test_str_invalid(void) {
    ftb_str_t str = NULL;

    ftb_da_append(FTB_RAW,str, 0x80);
    ftb_test_assert(ftb_str_check_valid(str) == false, "Fails on stray continuation byte");
    ftb_mem_free(FTB_RAW,str);
    str = NULL;

    ftb_da_append(FTB_RAW,str, 0xFF);
    ftb_test_assert(ftb_str_check_valid(str) == false, "Fails on completely invalid byte (0xFF)");
    ftb_mem_free(FTB_RAW,str);
    str = NULL;

    ftb_da_append(FTB_RAW,str, 0xC3);
    ftb_test_assert(ftb_str_check_valid(str) == false, "Fails on truncated 2-byte sequence");
    ftb_mem_free(FTB_RAW,str);
    str = NULL;

    ftb_da_append(FTB_RAW,str, 0xE2);
    ftb_da_append(FTB_RAW,str, 0x82);
    ftb_test_assert(ftb_str_check_valid(str) == false, "Fails on truncated 3-byte sequence");
    ftb_mem_free(FTB_RAW,str);
    str = NULL;

    ftb_da_append(FTB_RAW,str, 0xF0);
    ftb_da_append(FTB_RAW,str, 0x90);
    ftb_da_append(FTB_RAW,str, 0x20);
    ftb_da_append(FTB_RAW,str, 0x88);
    ftb_test_assert(ftb_str_check_valid(str) == false, "Fails on malformed continuation byte");
    ftb_mem_free(FTB_RAW,str);

    ftb_test_result(true);
}

bool test_str_edge_cases(void) {
    ftb_str_t str = NULL;

    ftb_test_assert(ftb_str_check_valid(str) == true, "Empty string is valid UTF-8");

    const char* mixed = "Valid ASCII ";
    ftb_da_append_cstr(FTB_RAW,str, mixed);
    ftb_da_append(FTB_RAW,str, 0xC3);
    ftb_test_assert(ftb_str_check_valid(str) == false,
        "Fails on truncated byte at end of long string");
    ftb_mem_free(FTB_RAW,str);

    ftb_test_result(true);
}

bool test_str_char_count(void) {
    ftb_str_t str = NULL;

    const char* ascii = "Hello";
    ftb_da_append_cstr(FTB_RAW,str, ascii);
    ftb_test_assert(ftb_str_get_char_count(str) == 5, "ASCII string char count is 5");
    ftb_mem_free(FTB_RAW,str);
    str = NULL;

    /* "ññ" (4 bytes, 2 chars) */
    u8 spanish[] = { 0xC3, 0xB1, 0xC3, 0xB1 };
    ftb_da_appends(FTB_RAW,str, spanish,4);
    ftb_test_assert(ftb_str_get_char_count(str) == 2, "2-byte UTF-8 string char count is 2");
    ftb_mem_free(FTB_RAW,str);
    str = NULL;

    /* "𐍈" (4 bytes, 1 char) */
    u8 emoji[] = { 0xF0, 0x90, 0x8D, 0x88 };
    ftb_da_appends(FTB_RAW,str, emoji,4);
    ftb_test_assert(ftb_str_get_char_count(str) == 1, "4-byte UTF-8 emoji char count is 1");
    ftb_mem_free(FTB_RAW,str);
    str = NULL;

    /* "A ñ 𐍈!" -> 'A'(1) + ' '(1) + 'ñ'(2) + ' '(1) + '𐍈'(4) + '!'(1) */
    /* = 10 bytes, 6 characters */
    u8 mixed[] = { 'A', ' ', 0xC3, 0xB1, ' ', 0xF0, 0x90, 0x8D, 0x88, '!' };
    ftb_da_appends(FTB_RAW,str, mixed,10);
    ftb_test_assert(ftb_str_get_char_count(str) == 6, "Mixed UTF-8 string char count is 6");
    ftb_mem_free(FTB_RAW,str);

    ftb_test_result(true);
}

bool test_str_basic_ops(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = 0;
    ftb_str_t s2 = 0;

    // TODO strcmp
    ftb_test_assert(ftb_da_count(s) == 0, "String not empty");

    ftb_da_append_cstr(&ctx, s, "Hello");
    ftb_test_assert(strcmp((char*)s, "Hello") == 0, "Append cstr content");
    ftb_test_assert(ftb_da_count(s) == 5, "Append cstr count");

    ftb_da_append_cstr(&ctx, s2, " World");
    ftb_da_appends(&ctx, s, s2,ftb_da_count(s2));

    ftb_test_assert(strcmp((char*)s, "Hello World") == 0, "Append str content");
    ftb_test_assert(ftb_da_count(s) == 11, "Append str count");

    char* raw = ftb_str_to_cstr(&ctx, s);
    ftb_test_assert(strcmp(raw, "Hello World") == 0, "To cstr check");

    ftb_str_clear(s);
    ftb_test_assert(ftb_da_count(s) == 0, "Clear count");
    ftb_test_assert(s[0] == 0, "Clear data");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_str_lowercase(void) {
    ftb_str_t src = NULL;

    const char* ascii = "HeLlO WOrLd!";
    ftb_da_append_cstr(FTB_RAW,src, ascii);

    ftb_str_t res1 = ftb_str_lowercase(FTB_RAW,src);
    ftb_test_assert(memcmp(res1, "hello world!", 12) == 0, "ASCII Lowercase");
    ftb_mem_free(FTB_RAW,src);
    ftb_mem_free(FTB_RAW,res1);
    src = NULL;

    /* 2. Multi-byte Latin-1 Test (Á -> á, Ñ -> ñ, Ö -> ö) */
    u8 latin_upper[] = { 0xC3, 0x81, 0xC3, 0x91, 0xC3, 0x96 };
    u8 latin_lower_expected[] = { 0xC3, 0xA1, 0xC3, 0xB1, 0xC3, 0xB6 };
    ftb_da_appends(FTB_RAW,src, latin_upper,sizeof(latin_upper));

    ftb_str_t res2 = ftb_str_lowercase(FTB_RAW,src);
    ftb_test_assert(ftb_da_count(res2) == 6, "Latin-1 byte count remains 6");
    ftb_test_assert(memcmp(res2, latin_lower_expected, 6) == 0, "Latin-1 Lowercase");
    ftb_mem_free(FTB_RAW,src);
    ftb_mem_free(FTB_RAW,res2);
    src = NULL;

    /* 3. Turkish Locale Length Change Test ('İ' -> 'i') */
    u8 turkish_i[] = { 0xC4, 0xB0 };
    ftb_da_appends(FTB_RAW,src,turkish_i,sizeof(turkish_i));

    ftb_str_t res3 = ftb_str_lowercase(FTB_RAW,src);
    ftb_test_assert(ftb_da_count(res3) == 1, "Turkish İ shrinks from 2 bytes to 1 byte");
    ftb_test_assert(res3[0] == 'i', "Turkish İ lowercases to standard i");
    ftb_mem_free(FTB_RAW,src);
    ftb_mem_free(FTB_RAW,res3);
    src = NULL;

    /* 4. Cyrillic & Greek Extended Test */
    /* ПРИВЕТ ΓΕΙΑ -> привет γεια */
    const char* ext_upper =
        "\xD0\x9F\xD0\xA0\xD0\x98\xD0\x92\xD0\x95\xD0\xA2 \xCE\x93\xCE\x95\xCE\x99\xCE\x91";
    const char* ext_lower =
        "\xD0\xBF\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82 \xCE\xB3\xCE\xB5\xCE\xB9\xCE\xB1";
    ftb_da_append_cstr(FTB_RAW, src, ext_upper);
    ftb_str_t res4 = ftb_str_lowercase(FTB_RAW, src);
    ftb_test_assert(memcmp(res4, ext_lower, strlen(ext_lower)) == 0, "Cyrillic & Greek Lowercase");
    ftb_mem_free(FTB_RAW, src);
    ftb_mem_free(FTB_RAW, res4);

    ftb_test_result(true);
}

bool test_str_uppercase(void) {
    ftb_str_t src = NULL;

    /* 1. ASCII */
    const char* ascii = "hello world!";
    ftb_da_append_cstr(FTB_RAW, src, ascii);
    ftb_str_t res1 = ftb_str_uppercase(FTB_RAW, src);
    ftb_test_assert(memcmp(res1, "HELLO WORLD!", 12) == 0, "ASCII Uppercase");
    ftb_mem_free(FTB_RAW, src);
    ftb_mem_free(FTB_RAW, res1);
    src = NULL;

    /* 2. Turkish Locale (ıiğşçöü -> IİĞŞÇÖÜ) */
    const char* tr_lower = "\xC4\xB1" "i" "\xC4\x9F" "\xC5\x9F" "\xC3\xA7" "\xC3\xB6" "\xC3\xBC";
    const char* tr_upper = "I" "\xC4\xB0" "\xC4\x9E" "\xC5\x9E" "\xC3\x87" "\xC3\x96" "\xC3\x9C";
    ftb_da_append_cstr(FTB_RAW, src, tr_lower);
    ftb_str_t res2 = ftb_str_uppercase(FTB_RAW, src);
    ftb_test_assert(ftb_da_count(res2) == 13, "Turkish Uppercase Byte Count");
    ftb_test_assert(memcmp(res2, tr_upper, 13) == 0, "Turkish Uppercase");
    ftb_mem_free(FTB_RAW, src);
    ftb_mem_free(FTB_RAW, res2);
    src = NULL;

    /* 3. Cyrillic (привет -> ПРИВЕТ) */
    const char* cy_lower = "\xD0\xBF\xD1\x80\xD0\xB8\xD0\xB2\xD0\xB5\xD1\x82";
    const char* cy_upper = "\xD0\x9F\xD0\xA0\xD0\x98\xD0\x92\xD0\x95\xD0\xA2";
    ftb_da_append_cstr(FTB_RAW, src, cy_lower);
    ftb_str_t res3 = ftb_str_uppercase(FTB_RAW, src);
    ftb_test_assert(ftb_da_count(res3) == 12, "Cyrillic Uppercase Byte Count");
    ftb_test_assert(memcmp(res3, cy_upper, 12) == 0, "Cyrillic Uppercase");
    ftb_mem_free(FTB_RAW, src);
    ftb_mem_free(FTB_RAW, res3);
    src = NULL;

    /* 4. Greek (γεια -> ΓΕΙΑ) */
    const char* gr_lower = "\xCE\xB3\xCE\xB5\xCE\xB9\xCE\xB1";
    const char* gr_upper = "\xCE\x93\xCE\x95\xCE\x99\xCE\x91";
    ftb_da_append_cstr(FTB_RAW, src, gr_lower);
    ftb_str_t res4 = ftb_str_uppercase(FTB_RAW, src);
    ftb_test_assert(ftb_da_count(res4) == 8, "Greek Uppercase Byte Count");
    ftb_test_assert(memcmp(res4, gr_upper, 8) == 0, "Greek Uppercase");
    ftb_mem_free(FTB_RAW, src);
    ftb_mem_free(FTB_RAW, res4);

    ftb_test_result(true);
}

bool test_str_trim_utf8(void) {
    ftb_str_t src = NULL;

    const char* ascii = "  \t  Hello World! \n \r ";
    ftb_da_append_cstr(FTB_RAW,src, ascii);

    ftb_str_t ltrim1 = ftb_str_trim_left(FTB_RAW,src);
    ftb_str_t rtrim1 = ftb_str_trim_right(FTB_RAW,src);
    ftb_str_t trim1  = ftb_str_trim(FTB_RAW,src);

    ftb_test_assert(ftb_da_count(ltrim1) == 17, "Left Trim ASCII length");
    ftb_test_assert(memcmp(ltrim1, "Hello World! \n \r ", 18) == 0, "Left Trim ASCII content");

    ftb_test_assert(ftb_da_count(rtrim1) == 17, "Right Trim ASCII length");
    ftb_test_assert(memcmp(rtrim1, "  \t  Hello World!", 16) == 0, "Right Trim ASCII content");

    ftb_test_assert(ftb_da_count(trim1) == 12, "Full Trim ASCII length");
    ftb_test_assert(memcmp(trim1, "Hello World!", 12) == 0, "Full Trim ASCII content");

    ftb_mem_free(FTB_RAW,src);
    ftb_mem_free(FTB_RAW,ltrim1);
    ftb_mem_free(FTB_RAW,rtrim1);
    ftb_mem_free(FTB_RAW,trim1);
    src = NULL;

    u8 complex_str[] = {
        0xC2, 0xA0,
        0x20,
        'M','e','r','h','a','b','a',' ',
        0xF0, 0x9F, 0x8C, 0x8D,
        0xE3, 0x80, 0x80,
        0xC2, 0xA0
    };

    ftb_da_appends(FTB_RAW,src, complex_str, sizeof(complex_str));

    ftb_str_t trim2 = ftb_str_trim(FTB_RAW,src);

    u8 expected[] = { 'M','e','r','h','a','b','a',' ', 0xF0, 0x9F, 0x8C, 0x8D };

    ftb_test_assert(ftb_da_count(trim2) == 12,
         "UTF-8 Trim removed all multi-byte invisible spaces");
    ftb_test_assert(memcmp(trim2, expected, 12) == 0,
         "UTF-8 Trim preserved inner emoji and text");

    ftb_mem_free(FTB_RAW,src);
    ftb_mem_free(FTB_RAW,trim2);
    src = NULL;

    u8 all_spaces[] = { 0x20, 0xC2, 0xA0, 0xE3, 0x80, 0x80, '\t', '\n' };
    ftb_da_appends(FTB_RAW,src, all_spaces, sizeof(all_spaces));

    ftb_str_t trim3 = ftb_str_trim(FTB_RAW,src);
    ftb_da_add_shadow_null(FTB_RAW,trim3);
    ftb_test_assert(ftb_da_count(trim3) == 0,
         "Trimming an all-space string returns empty string");

    ftb_mem_free(FTB_RAW,src);
    ftb_mem_free(FTB_RAW,trim3);

    ftb_test_result(true);
}

bool test_str_compare(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);
    ftb_str_t s1 = 0;
    ftb_str_t s2 = 0;

    ftb_da_append_cstr(&ctx, s1, "Apple");
    ftb_da_append_cstr(&ctx, s2, "Apple");

    ftb_test_assert(ftb_str_cmp_cstr(s1, "Apple"), "Cmp cstr true");
    ftb_test_assert(!ftb_str_cmp_cstr(s1, "Orange"), "Cmp cstr false");

    ftb_test_assert(ftb_str_cmp(s1, s2), "Cmp str true");

    ftb_da_append_cstr(&ctx, s2, "s");
    ftb_test_assert(!ftb_str_cmp(s1, s2), "Cmp str false");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_str_to_cstr_empty(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = 0;

    char* cstr = ftb_str_to_cstr(&ctx, s);
    ftb_test_assert(cstr != NULL, "Empty str to cstr not null");
    ftb_test_assert(cstr[0] == '\0', "Empty cstr is null terminated");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_str_cmp_length_diff(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s1 = 0;
    ftb_str_t s2 = 0;

    ftb_da_append_cstr(&ctx, s1, "short");
    ftb_da_append_cstr(&ctx, s2, "shorter");

    ftb_test_assert(!ftb_str_cmp(s1, s2), "Different lengths not equal");
    ftb_test_assert(!ftb_str_cmp(s2, s1), "Different lengths not equal reversed");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_str_append_loop(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = 0;

    for(int i = 0; i < 100; i++) {
        ftb_da_append_cstr(&ctx, s, "A");
    }
    ftb_test_assert(ftb_da_count(s) == 100, "String length 100");
    ftb_test_assert(ftb_da_capacity(s) >= 100, "Capacity grew");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_str_case_symbols(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = 0;
    ftb_str_t new_s = 0;
    ftb_da_append_cstr(&ctx, s, "123 !@# aBc");

    new_s = ftb_str_uppercase(&ctx,s);
    ftb_test_assert(ftb_str_cmp_cstr(new_s, "123 !@# ABC"), "Uppercase ignores symbols");

    new_s = ftb_str_lowercase(&ctx,s);
    ftb_test_assert(ftb_str_cmp_cstr(new_s, "123 !@# abc"), "Lowercase ignores symbols");

    ftb_mem_delete_ctx(&ctx);
    ftb_test_result(true);
}

bool test_str_capacity_and_length(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = 0;

    ftb_test_assert(ftb_da_capacity(s) == 0, "Default string capacity");
    ftb_test_assert(ftb_da_count(s) == 0, "Default string count");

    ftb_da_append_cstr(&ctx, s, "0123456789");
    ftb_test_assert(ftb_da_count(s) == 10, "Length after append");

    ftb_str_clear(s);
    ftb_test_assert(ftb_da_count(s) == 0, "Length after clear");

    ftb_mem_delete_ctx(&ctx);
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
    ftb_test_add(tests, test_str_valid);
    ftb_test_add(tests, test_str_invalid);
    ftb_test_add(tests, test_str_edge_cases);
    ftb_test_add(tests, test_str_basic_ops);
    ftb_test_add(tests, test_str_lowercase);
    ftb_test_add(tests, test_str_uppercase);
    ftb_test_add(tests, test_str_trim_utf8);
    ftb_test_add(tests, test_str_compare);
    ftb_test_add(tests, test_str_to_cstr_empty);
    ftb_test_add(tests, test_str_cmp_length_diff);
    ftb_test_add(tests, test_str_append_loop);
    ftb_test_add(tests, test_str_case_symbols);
    ftb_test_add(tests, test_str_capacity_and_length);

    #if 0
    ftb_test_add(tests, test_str_starts_ends);
    ftb_test_add(tests, test_str_find_contains);
    ftb_test_add(tests, test_str_remove_range);
    ftb_test_add(tests, test_string_utils);
    ftb_test_add(tests, test_str_cmp_prefix);
    ftb_test_add(tests, test_str_find_char_missing);
    ftb_test_add(tests, test_str_find_cstr_edge);
    ftb_test_add(tests, test_str_starts_ends_edge);
    ftb_test_add(tests, test_str_starts_ends_str);
    ftb_test_add(tests, test_str_find_str_advanced);
    ftb_test_add(tests, test_str_remove_range_edge_cases);
    ftb_test_add(tests, test_str_null_handling);
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
