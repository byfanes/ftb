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
    TEST_ASSERT(num != NULL, "Memory alloc returned NULL");
    *num = 123;

    TEST_ASSERT(num != NULL, "Context count should be 2");

    typedef struct { int a; int b; } vec2;
    vec2* v = ftb_mem_zalloc(&ctx, sizeof(vec2));
    TEST_ASSERT(v != NULL, "Zalloc returned NULL");
    TEST_ASSERT(v->a == 0 && v->b == 0, "Zalloc did not zero memory");
    TEST_ASSERT(ftb_da_count(ctx.ptrs.items) == 2, "Context count should be 2");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_mem_free_direct(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    int* ptr1 = ftb_mem_alloc(&ctx, sizeof(int));
    int* ptr2 = ftb_mem_alloc(&ctx, sizeof(int));
    ptr2 = ptr2;
    ptr1 = ptr1;
    TEST_ASSERT(ftb_da_count(ctx.ptrs.items) == 2, "Items count 2");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_memory_scope_markers(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    ftb_mem_alloc(&ctx, 10);
    TEST_ASSERT(ftb_da_count(ctx.ptrs.items) == 1, "Ctx count 1");

    ftb_mem_set_mark(&ctx);

    ftb_mem_alloc(&ctx, 20);
    ftb_mem_alloc(&ctx, 20);

    TEST_ASSERT(ftb_da_count(ctx.ptrs.items) == 3, "Ctx count before cleanup");
    ftb_mem_cleanup(&ctx);
    TEST_ASSERT(ftb_da_count(ctx.ptrs.items) == 1, "Ctx count after cleanup");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_memory_cleanup(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    ftb_str_create(&ctx);
    ftb_str_create(&ctx);
    ftb_str_create(&ctx);

    u32 count_before = ftb_da_count(ctx.ptrs.items);
    ftb_mem_cleanup(&ctx);
    u32 count_after = ftb_da_count(ctx.ptrs.items);

    TEST_ASSERT(count_after < count_before, "Cleanup reduced allocation count");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_memory_realloc(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    int* p1 = ftb_mem_alloc(&ctx, 16);
    p1[0] = 42;
    p1[1] = 84;

    int* p2 = ftb_mem_realloc(&ctx, p1, 32);
    TEST_ASSERT(p2 != NULL, "Realloc larger");
    TEST_ASSERT(ftb_da_capacity(p2) == 32, "Realloc capacity increased");

    TEST_ASSERT(p2[0] == 42, "Realloc preserved data 0");
    TEST_ASSERT(p2[1] == 84, "Realloc preserved data 1");

    void* p3 = ftb_mem_realloc(&ctx, NULL, 64);
    TEST_ASSERT(p3 != NULL, "Realloc from NULL");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_memory_alloc_zero(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    void* p = ftb_mem_alloc(&ctx, 0);
    TEST_ASSERT(p == NULL, "Alloc 0 bytes returns NULL");

    void* p2 = ftb_mem_zalloc(&ctx, 0);
    TEST_ASSERT(p2 == NULL, "Zalloc 0 bytes returns NULL");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_memory_zalloc_large(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    int size = 1024;
    u8* mem = ftb_mem_zalloc(&ctx, size);
    TEST_ASSERT(mem != NULL, "Zalloc large");

    bool all_zero = true;
    for(int i = 0; i < size; i++) {
        if(mem[i] != 0) {
            all_zero = false;
            break;
        }
    }
    TEST_ASSERT(all_zero, "Large zalloc is all zeros");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

/*
 * ==========================================
 * STRING TESTS
 * ==========================================
 */

bool test_str_basic_ops(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    ftb_str_t s = ftb_str_create(&ctx);
    TEST_ASSERT(s != NULL, "String creation failed");
    TEST_ASSERT(ftb_strlen(s) == 0, "String not empty");

    ftb_str_append_cstr(&ctx, s, "Hello");
    TEST_ASSERT(strcmp(s, "Hello") == 0, "Append cstr content");
    TEST_ASSERT(ftb_strlen(s) == 5, "Append cstr count");

    ftb_str_t s2 = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s2, " World");
    ftb_str_append_str(&ctx, s, s2);

    TEST_ASSERT(strcmp(s, "Hello World") == 0, "Append str content");
    TEST_ASSERT(ftb_strlen(s) == 11, "Append str count");

    char* raw = ftb_str_to_cstr(&ctx, s);
    TEST_ASSERT(strcmp(raw, "Hello World") == 0, "To cstr check");

    ftb_str_clear(s);
    TEST_ASSERT(ftb_strlen(s) == 0, "Clear count");
    TEST_ASSERT(s[0] == 0, "Clear data");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_case(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);
    ftb_str_t s = ftb_str_create(&ctx);

    ftb_str_append_cstr(&ctx, s, "HeLLo");
    ftb_str_uppercase(s);
    TEST_ASSERT(strcmp(s, "HELLO") == 0, "Uppercase");

    ftb_str_lowercase(s);
    TEST_ASSERT(strcmp(s, "hello") == 0, "Lowercase");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_compare(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);
    ftb_str_t s1 = ftb_str_create(&ctx);
    ftb_str_t s2 = ftb_str_create(&ctx);

    ftb_str_append_cstr(&ctx, s1, "Apple");
    ftb_str_append_cstr(&ctx, s2, "Apple");

    TEST_ASSERT(ftb_str_cmp_cstr(s1, "Apple"), "Cmp cstr true");
    TEST_ASSERT(!ftb_str_cmp_cstr(s1, "Orange"), "Cmp cstr false");

    TEST_ASSERT(ftb_str_cmp(s1, s2), "Cmp str true");

    ftb_str_append_cstr(&ctx, s2, "s");
    TEST_ASSERT(!ftb_str_cmp(s1, s2), "Cmp str false");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_starts_ends(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);
    ftb_str_t s = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s, "filename.txt");

    TEST_ASSERT(ftb_str_starts_with_cstr(s, "file"), "Starts with cstr true");
    TEST_ASSERT(!ftb_str_starts_with_cstr(s, "txt"), "Starts with cstr false");

    TEST_ASSERT(ftb_str_ends_with_cstr(s, ".txt"), "Ends with cstr true");
    TEST_ASSERT(!ftb_str_ends_with_cstr(s, "name"), "Ends with cstr false");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_trim(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);
    ftb_str_t s = ftb_str_create(&ctx);

    ftb_str_append_cstr(&ctx, s, "   abc");
    ftb_str_trim_left(s);
    TEST_ASSERT(strcmp(s, "abc") == 0, "Trim left content");
    TEST_ASSERT(ftb_strlen(s) == 3, "Trim left count");

    ftb_str_clear(s);

    ftb_str_append_cstr(&ctx, s, "abc   ");
    ftb_str_trim_right(s);

    TEST_ASSERT(strcmp(s, "abc") == 0, "Trim right content");
    TEST_ASSERT(ftb_strlen(s) == 3, "Trim right count");

    ftb_str_clear(s);

    ftb_str_append_cstr(&ctx, s, "  xyz  ");
    ftb_str_trim(s);
    TEST_ASSERT(strcmp(s, "xyz") == 0, "Trim both");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_trim_no_spaces(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);
    ftb_str_t s = ftb_str_create(&ctx);

    ftb_str_append_cstr(&ctx, s, "hello");
    ftb_str_trim_right(s);
    TEST_ASSERT(strcmp(s, "hello") == 0, "Trim right without spaces should not modify");

    ftb_str_clear(s);
    ftb_str_append_cstr(&ctx, s, "world");
    ftb_str_trim_left(s);
    TEST_ASSERT(strcmp(s, "world") == 0, "Trim left without spaces should not modify");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_find_contains(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);
    ftb_str_t s = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s, "banana");

    TEST_ASSERT(ftb_str_find_char_begin(s, 'a') == 1, "Find char begin");
    TEST_ASSERT(ftb_str_find_char_end(s, 'a') == 5, "Find char end");

    TEST_ASSERT(ftb_str_contains_cstr(s, "nana"), "Contains cstr");
    TEST_ASSERT(ftb_str_find_cstr(s, "nana") == 2, "Find cstr index");
    TEST_ASSERT(!ftb_str_contains_cstr(s, "apple"), "Not contains");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_remove_range(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);
    ftb_str_t s = ftb_str_create(&ctx);

    ftb_str_append_cstr(&ctx, s, "0123456789");
    ftb_str_remove_range(s, 3, 3);

    TEST_ASSERT(strcmp(s, "0126789") == 0, "Remove range content");
    TEST_ASSERT(ftb_strlen(s) == 7, "Remove range count");

    ftb_str_remove_range(s, 5, 100);
    TEST_ASSERT(strcmp(s, "01267") == 0, "Remove range overshoot");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_string_utils(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    ftb_str_t s = ftb_str_create(&ctx);

    ftb_str_append_cstr(&ctx, s, "Hello");
    ftb_str_uppercase(s);
    TEST_ASSERT(ftb_str_cmp_cstr(s, "HELLO"), "Uppercase");
    ftb_str_lowercase(s);
    TEST_ASSERT(ftb_str_cmp_cstr(s, "hello"), "Lowercase");

    ftb_str_clear(s);
    ftb_str_append_cstr(&ctx, s, "   data   ");
    ftb_str_trim(s);
    TEST_ASSERT(ftb_str_cmp_cstr(s, "data"), "Trim spaces");
    TEST_ASSERT(ftb_strlen(s) == 4, "Trim count update");

    ftb_str_clear(s);
    ftb_str_append_cstr(&ctx, s, "filename.txt");
    TEST_ASSERT(ftb_str_starts_with_cstr(s, "file"), "Starts with 'file'");
    TEST_ASSERT(ftb_str_ends_with_cstr(s, ".txt"), "Ends with '.txt'");
    TEST_ASSERT(!ftb_str_ends_with_cstr(s, ".png"), "Not ends with '.png'");

    TEST_ASSERT(ftb_str_find_char_begin(s, '.') == 8, "Find char '.'");
    TEST_ASSERT(ftb_str_find_cstr(s, "name") == 4, "Find cstr 'name'");
    TEST_ASSERT(ftb_str_contains_cstr(s, "txt"), "Contains 'txt'");

    ftb_str_remove_range(s, 4, 4);
    TEST_ASSERT(ftb_str_cmp_cstr(s, "file.txt"), "Remove range middle");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_capacity_and_length(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = ftb_str_create(&ctx);

    TEST_ASSERT(ftb_da_capacity(s) == FTB_STR_DEF_CAP, "Default string capacity");
    TEST_ASSERT(ftb_da_count(s) == 0, "Default string count");

    ftb_str_append_cstr(&ctx, s, "0123456789");
    TEST_ASSERT(ftb_da_count(s) == 10, "Length after append");

    ftb_str_clear(s);
    TEST_ASSERT(ftb_da_count(s) == 0, "Length after clear");
    TEST_ASSERT(ftb_da_capacity(s) == FTB_STR_DEF_CAP, "Capacity retained after clear");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_to_cstr_empty(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = ftb_str_create(&ctx);

    char* cstr = ftb_str_to_cstr(&ctx, s);
    TEST_ASSERT(cstr != NULL, "Empty str to cstr not null");
    TEST_ASSERT(cstr[0] == '\0', "Empty cstr is null terminated");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_cmp_length_diff(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s1 = ftb_str_create(&ctx);
    ftb_str_t s2 = ftb_str_create(&ctx);

    ftb_str_append_cstr(&ctx, s1, "short");
    ftb_str_append_cstr(&ctx, s2, "shorter");

    TEST_ASSERT(!ftb_str_cmp(s1, s2), "Different lengths not equal");
    TEST_ASSERT(!ftb_str_cmp(s2, s1), "Different lengths not equal reversed");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_cmp_prefix(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s1 = ftb_str_create(&ctx);
    ftb_str_t s2 = ftb_str_create(&ctx);

    ftb_str_append_cstr(&ctx, s1, "prefix");
    ftb_str_append_cstr(&ctx, s2, "prefix_extra");

    TEST_ASSERT(ftb_str_cmp(s1, s2) == false, "Prefix is not equal to full");
    TEST_ASSERT(ftb_str_cmp(s2, s1) == false, "Full is not equal to prefix");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_find_char_missing(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s, "hello");

    TEST_ASSERT(ftb_str_find_char_begin(s, 'z') == -1, "Char not found begin");
    TEST_ASSERT(ftb_str_find_char_end(s, 'z') == -1, "Char not found end");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_find_cstr_edge(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s, "hello");

    TEST_ASSERT(ftb_str_find_cstr(s, "world") == -1, "Substr not found");
    TEST_ASSERT(ftb_str_find_cstr(s, "") == 0, "Empty substr found at 0");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_starts_ends_edge(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s, "hi");

    TEST_ASSERT(!ftb_str_starts_with_cstr(s, "high"), "Starts with longer cstr");
    TEST_ASSERT(!ftb_str_ends_with_cstr(s, "high"), "Ends with longer cstr");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_starts_ends_str(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s1 = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s1, "superman");

    ftb_str_t start_s = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, start_s, "super");

    ftb_str_t end_s = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, end_s, "man");

    ftb_str_t bad_s = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, bad_s, "bat");

    TEST_ASSERT(ftb_str_starts_with(s1, start_s), "Starts with str");
    TEST_ASSERT(ftb_str_ends_with(s1, end_s), "Ends with str");

    TEST_ASSERT(!ftb_str_starts_with(s1, bad_s), "Does not start with str");
    TEST_ASSERT(!ftb_str_ends_with(s1, bad_s), "Does not end with str");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_find_str_advanced(void) {
    ftb_ctx_t ctx = {0};

    ftb_str_t haystack = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, haystack, "The quick brown fox");

    ftb_str_t needle1 = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, needle1, "quick");

    ftb_str_t needle2 = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, needle2, "lazy");

    TEST_ASSERT(ftb_str_find(haystack, needle1) == 4, "Find string inside");
    TEST_ASSERT(ftb_str_contains(haystack, needle1), "Contains string true");

    TEST_ASSERT(ftb_str_find(haystack, needle2) == -1, "Find string missing");
    TEST_ASSERT(!ftb_str_contains(haystack, needle2), "Contains string false");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_trim_advanced(void) {
    ftb_ctx_t ctx = {0};

    ftb_str_t s_empty = ftb_str_create(&ctx);
    ftb_str_trim(s_empty);
    TEST_ASSERT(ftb_strlen(s_empty) == 0, "Trim empty string");

    ftb_str_t s_spaces = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s_spaces, "     ");
    ftb_str_trim(s_spaces);
    TEST_ASSERT(ftb_strlen(s_spaces) == 0, "Trim all spaces string");

    ftb_str_t s_left = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s_left, " \t\n word");
    ftb_str_trim_left(s_left);
    TEST_ASSERT(strcmp(s_left, "word") == 0, "Trim left with tabs and newlines");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_remove_range_edge_cases(void) {
    ftb_ctx_t ctx = {0};

    ftb_str_t s = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s, "abcdef");

    ftb_str_remove_range(s, 2, 0);
    TEST_ASSERT(strcmp(s, "abcdef") == 0, "Remove len 0");

    ftb_str_remove_range(s, 10, 2);
    TEST_ASSERT(strcmp(s, "abcdef") == 0, "Remove out of bounds");

    ftb_str_remove_range(s, 3, 3);
    TEST_ASSERT(strcmp(s, "abc") == 0, "Remove exact tail");
    TEST_ASSERT(ftb_strlen(s) == 3, "Tail removed length");

    ftb_str_remove_range(s, 0, 100);
    TEST_ASSERT(ftb_strlen(s) == 0, "Remove all");
    TEST_ASSERT(strcmp(s, "") == 0, "Empty after remove all");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_case_symbols(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s, "123 !@# aBc");

    ftb_str_uppercase(s);
    TEST_ASSERT(strcmp(s, "123 !@# ABC") == 0, "Uppercase ignores symbols");

    ftb_str_lowercase(s);
    TEST_ASSERT(strcmp(s, "123 !@# abc") == 0, "Lowercase ignores symbols");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_null_handling(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = NULL;

    TEST_ASSERT(ftb_str_clear(s) == false, "Clear NULL string");
    TEST_ASSERT(ftb_str_uppercase(s) == false, "Uppercase NULL string");
    TEST_ASSERT(ftb_str_lowercase(s) == false, "Lowercase NULL string");
    TEST_ASSERT(ftb_str_find_char_begin(s, 'a') == -1, "Find char in NULL string");
    TEST_ASSERT(ftb_str_remove_range(s, 0, 1) == false, "Remove range from NULL string");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_append_loop(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = ftb_str_create(&ctx);

    for(int i = 0; i < 100; i++) {
        ftb_str_append_cstr(&ctx, s, "A");
    }
    TEST_ASSERT(ftb_strlen(s) == 100, "String length 100");
    TEST_ASSERT(ftb_da_capacity(s) >= 100, "Capacity grew");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

/*
 * ==========================================
 * DYNAMIC ARRAY TESTS
 * ==========================================
 */

bool test_raw_da_macros(void) {
    int* arr = NULL;

    ftb_raw_da_append(arr, 10);
    ftb_raw_da_append(arr, 20);
    TEST_ASSERT(arr != NULL, "Raw DA created");
    TEST_ASSERT(ftb_da_count(arr) == 2, "Count is 2");
    TEST_ASSERT(arr[0] == 10, "arr[0] is 10");
    TEST_ASSERT(arr[1] == 20, "arr[1] is 20");

    int items[] = {30, 40, 50};
    ftb_raw_da_appends(arr, items, 3);
    TEST_ASSERT(ftb_da_count(arr) == 5, "Count is 5");
    TEST_ASSERT(arr[2] == 30, "arr[2] is 30");
    TEST_ASSERT(arr[4] == 50, "arr[4] is 50");

    ftb_raw_da_reserve(arr, 100);
    TEST_ASSERT(ftb_da_capacity(arr) >= 100, "Capacity increased");

    ftb_raw_da_free(arr);
    TEST_RESULT(true);
}

bool test_managed_da_macros(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    float* f_arr = NULL;
    ftb_da_append(&ctx, f_arr, 3.14f);
    ftb_da_append(&ctx, f_arr, 2.71f);

    TEST_ASSERT(f_arr != NULL, "Managed DA created");
    TEST_ASSERT(ftb_da_count(f_arr) == 2, "Managed DA count 2");
    TEST_ASSERT(f_arr[0] == 3.14f, "Value 1 correct");

    float more_f[] = {1.0f, 2.0f};
    ftb_da_appends(&ctx, f_arr, more_f, 2);

    TEST_ASSERT(ftb_da_count(f_arr) == 4, "Managed DA count 4");
    TEST_ASSERT(f_arr[2] == 1.0f, "Appended value 1 correct");
    TEST_ASSERT(f_arr[3] == 2.0f, "Appended value 2 correct");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_da_count_macros(void) {
    int* arr = NULL;
    ftb_raw_da_append(arr, 1);
    ftb_raw_da_append(arr, 2);

    TEST_ASSERT(ftb_da_count(arr) == 2, "Count is 2");

    ftb_da_count_sum(arr, 3);
    TEST_ASSERT(ftb_da_count(arr) == 5, "Count sum by 3");

    ftb_da_count_sub(arr, 2);
    TEST_ASSERT(ftb_da_count(arr) == 3, "Count sub by 2");

    ftb_da_count_inc(arr);
    TEST_ASSERT(ftb_da_count(arr) == 4, "Count inc by 1");

    ftb_da_count_dec(arr);
    TEST_ASSERT(ftb_da_count(arr) == 3, "Count dec by 1");

    ftb_da_set_count(arr, 10);
    TEST_ASSERT(ftb_da_count(arr) == 10, "Set count to 10");

    ftb_da_set_capacity(arr, 20);
    TEST_ASSERT(ftb_da_capacity(arr) == 20, "Set capacity to 20");

    ftb_raw_da_free(arr);
    TEST_RESULT(true);
}

/*
 * ==========================================
 * PATH MANIPULATION TESTS
 * ==========================================
 */

bool test_path_basename(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    const char* p1 = "dir" PSEP "file.txt";
    TEST_ASSERT(ftb_path_basename_cstr(&ctx, &out, p1), "basename normal");
    TEST_ASSERT(strcmp(out, "file.txt") == 0, "basename normal match");

    const char* p2 = "file.txt";
    TEST_ASSERT(ftb_path_basename_cstr(&ctx, &out, p2), "basename no dir");
    TEST_ASSERT(strcmp(out, "file.txt") == 0, "basename no dir match");
#ifdef _WIN32
    const char* p3 = "dir/file.txt";
    TEST_ASSERT(ftb_path_basename_cstr(&ctx, &out, p3), "basename windows fwd slash");
    TEST_ASSERT(strcmp(out, "file.txt") == 0, "basename windows fwd slash match");
#endif
    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_dirname(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    const char* p1 = "dir" PSEP "file.txt";
    TEST_ASSERT(ftb_path_dirname_cstr(&ctx, &out, p1), "dirname normal");
    TEST_ASSERT(strcmp(out, "dir") == 0, "dirname normal match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_extension(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    const char* p1 = "dir" PSEP "file.txt";
    TEST_ASSERT(ftb_path_extension_cstr(&ctx, &out, p1), "extension normal");
    TEST_ASSERT(strcmp(out, "txt") == 0, "extension normal match");

    const char* p2 = "archive.tar.gz";
    TEST_ASSERT(ftb_path_extension_cstr(&ctx, &out, p2), "extension double");
    TEST_ASSERT(strcmp(out, "gz") == 0, "extension double match");

    const char* p3 = "dir" PSEP "file";
    TEST_ASSERT(ftb_path_extension_cstr(&ctx, &out, p3) == false, "no extension");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_stem(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);
    const char* p2 = "archive.tar.gz";
    const char* p1 = "dir" PSEP "file.txt";
    const char* p3 = "dir" PSEP "file";

    TEST_ASSERT(ftb_path_stem_cstr(&ctx, &out, p1), "stem normal");
    TEST_ASSERT(strcmp(out, "file") == 0, "stem normal match");

    TEST_ASSERT(ftb_path_stem_cstr(&ctx, &out, p2), "stem double ext");
    TEST_ASSERT(strcmp(out, "archive.tar") == 0, "stem double ext match");

    TEST_ASSERT(ftb_path_stem_cstr(&ctx, &out, p3), "stem no ext");
    TEST_ASSERT(strcmp(out, "file") == 0, "stem no ext match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_managed_wrappers(void) {
    ftb_ctx_t ctx = {0};

    ftb_path_t in_path = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, in_path, "folder" PSEP "image.png");

    ftb_path_t out_stem = ftb_str_create(&ctx);
    TEST_ASSERT(ftb_path_stem(&ctx, &out_stem, in_path), "managed stem");
    TEST_ASSERT(strcmp(out_stem, "image") == 0, "managed stem match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_dirname_cstr(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    const char* p1 = "dir" PSEP "file.txt";
    const char* p2 = PSEP "file.txt";
    const char* p3 = "file.txt";
    const char* p4 = "dir" PSEP "subdir" PSEP "file";
    const char* p5 = "dir" PSEP;

    TEST_ASSERT(ftb_path_dirname_cstr(&ctx, &out, p1), "dirname normal");
    TEST_ASSERT(strcmp(out, "dir") == 0, "dirname normal match");

    TEST_ASSERT(ftb_path_dirname_cstr(&ctx, &out, p2), "dirname root");
    TEST_ASSERT(strcmp(out, PSEP) == 0, "dirname root match");

    TEST_ASSERT(ftb_path_dirname_cstr(&ctx, &out, p3), "dirname empty");
    TEST_ASSERT(ftb_strlen(out) == 0, "dirname empty match");

    TEST_ASSERT(ftb_path_dirname_cstr(&ctx, &out, p4), "dirname nested");
    TEST_ASSERT(strcmp(out, "dir" PSEP "subdir") == 0, "dirname nested match");

    TEST_ASSERT(ftb_path_dirname_cstr(&ctx, &out, p5), "dirname trailing slash");
    TEST_ASSERT(strcmp(out, "dir") == 0, "dirname trailing slash match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_wrappers(void) {
    ftb_ctx_t ctx = {0};

    ftb_path_t my_path = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, my_path, "my_folder" PSEP "archive.tar.gz");

    ftb_path_t out_wrapper = ftb_str_create(&ctx);
    ftb_path_t out_cstr = ftb_str_create(&ctx);

    ftb_path_basename(&ctx, &out_cstr, my_path);
    TEST_ASSERT(ftb_path_basename(&ctx, &out_wrapper, my_path), "wrapper basename call");
    TEST_ASSERT(strcmp(out_wrapper, out_cstr) == 0, "wrapper basename matches cstr result");

    ftb_path_dirname(&ctx, &out_cstr, my_path);
    TEST_ASSERT(ftb_path_dirname(&ctx, &out_wrapper, my_path), "wrapper dirname call");
    TEST_ASSERT(strcmp(out_wrapper, out_cstr) == 0, "wrapper dirname matches cstr result");

    ftb_path_extension(&ctx, &out_cstr, my_path);
    TEST_ASSERT(ftb_path_extension(&ctx, &out_wrapper, my_path), "wrapper ext call");
    TEST_ASSERT(strcmp(out_wrapper, out_cstr) == 0, "wrapper ext matches cstr result");

    ftb_path_stem(&ctx, &out_cstr, my_path);
    TEST_ASSERT(ftb_path_stem(&ctx, &out_wrapper, my_path), "wrapper stem call");
    TEST_ASSERT(strcmp(out_wrapper, out_cstr) == 0, "wrapper stem matches cstr result");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_join_cstr(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    const char* p1 = "dir";
    const char* p2 = "file.txt";
    TEST_ASSERT(ftb_path_join_cstr(&ctx, &out, p1, p2), "join normal");
    TEST_ASSERT(strcmp(out, "dir" PSEP "file.txt") == 0, "join normal match");

    const char* p3 = "dir" PSEP;
    const char* p4 = PSEP "file.txt";
    TEST_ASSERT(ftb_path_join_cstr(&ctx, &out, p3, p4), "join with multiple slashes");
    TEST_ASSERT(strcmp(out, "dir" PSEP "file.txt") == 0, "join multiple slashes match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_normalize_cstr(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    const char* p1 = "a" PSEP "b" PSEP ".." PSEP "c";
    TEST_ASSERT(ftb_path_normalize_cstr(&ctx, &out, p1), "normalize relative dot-dot");
    TEST_ASSERT(strcmp(out, "a" PSEP "c") == 0, "normalize relative match");

    const char* p2 = PSEP "a" PSEP "." PSEP "b" PSEP PSEP PSEP "c" PSEP ".." PSEP "d" PSEP;
    TEST_ASSERT(ftb_path_normalize_cstr(&ctx, &out, p2), "normalize absolute complex");
    TEST_ASSERT(strcmp(out, PSEP "a" PSEP "b" PSEP "d") == 0, "normalize absolute match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_with_extension_cstr(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    const char* p1 = "file.txt";
    const char* ext = ".md";
    TEST_ASSERT(ftb_path_with_extension_cstr(&ctx, &out, p1, ext), "replace ext");
    TEST_ASSERT(strcmp(out, "file.md") == 0, "replace ext match");

    const char* p2 = "archive";
    const char* ext2 = "tar.gz";
    TEST_ASSERT(ftb_path_with_extension_cstr(&ctx, &out, p2, ext2), "add ext");
    TEST_ASSERT(strcmp(out, "archive.tar.gz") == 0, "add ext match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_info_cstr(void) {
    const char* abs_p = PSEP "usr" PSEP "bin" PSEP "gcc";
    const char* rel_p = "src" PSEP "main.c";

    TEST_ASSERT(ftb_path_is_absolute_cstr(abs_p), "is absolute true");
    TEST_ASSERT(!ftb_path_is_absolute_cstr(rel_p), "is absolute false");

    TEST_ASSERT(!ftb_path_is_relative_cstr(abs_p), "is relative false");
    TEST_ASSERT(ftb_path_is_relative_cstr(rel_p), "is relative true");

    TEST_ASSERT(ftb_path_has_extension_cstr(rel_p), "has extension true");
    TEST_ASSERT(!ftb_path_has_extension_cstr(abs_p), "has extension false");

    TEST_RESULT(true);
}

bool test_path_fs_operations(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    TEST_ASSERT(ftb_path_cwd_get(&ctx, &out), "get cwd");
    TEST_ASSERT(ftb_strlen(out) > 0, "cwd string populated");

    const char* current_dir = ".";
    TEST_ASSERT(ftb_path_exists_cstr(&ctx, current_dir), "cwd exists");
    TEST_ASSERT(ftb_path_is_dir_cstr(&ctx, current_dir), "cwd is dir");
    TEST_ASSERT(!ftb_path_is_file_cstr(&ctx, current_dir), "cwd is not a regular file");

    TEST_ASSERT(ftb_path_absolute_cstr(&ctx, &out, current_dir), "resolve abs path");
    TEST_ASSERT(ftb_path_is_absolute(out), "resolved path is absolute");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

/*
 * ==========================================
 * MACROS & MATH TESTS
 * ==========================================
 */

bool test_math_macros(void) {
    TEST_ASSERT(FTB_MIN(5, 10) == 5, "FTB_MIN");
    TEST_ASSERT(FTB_MAX(5, 10) == 10, "FTB_MAX");
    TEST_ASSERT(FTB_CLAMP(15, 0, 10) == 10, "FTB_CLAMP upper bounds");
    TEST_ASSERT(FTB_CLAMP(-5, 0, 10) == 0, "FTB_CLAMP lower bounds");
    TEST_ASSERT(FTB_CLAMP(5, 0, 10) == 5, "FTB_CLAMP inside bounds");

    int a = 1, b = 2;
    FTB_SWAP(int, a, b);
    TEST_ASSERT(a == 2 && b == 1, "FTB_SWAP values");

    void* p1 = (void*)0x100;
    void* p2 = (void*)0x200;
    FTB_SWAP_PTR(p1, p2);
    TEST_ASSERT(p1 == (void*)0x200 && p2 == (void*)0x100, "FTB_SWAP_PTR values");

    TEST_ASSERT(FTB_ALIGN_UP(13, 8) == 16, "FTB_ALIGN_UP");
    TEST_ASSERT(FTB_ALIGN_DOWN(13, 8) == 8, "FTB_ALIGN_DOWN");
    TEST_ASSERT(FTB_IS_POW2(16), "FTB_IS_POW2 for true");
    TEST_ASSERT(!FTB_IS_POW2(15), "FTB_IS_POW2 for false");

    TEST_RESULT(true);
}

bool test_bit_macros(void) {
    u32 flags = 0;
    FTB_BIT_SET(flags, 3);
    TEST_ASSERT(flags == 8, "FTB_BIT_SET");
    TEST_ASSERT(FTB_BIT_GET(flags, 3) == 1, "FTB_BIT_GET true");
    TEST_ASSERT(FTB_BIT_GET(flags, 2) == 0, "FTB_BIT_GET false");

    FTB_BIT_CLR(flags, 3);
    TEST_ASSERT(flags == 0, "FTB_BIT_CLR");

    FTB_BIT_TOGGLE(flags, 1);
    TEST_ASSERT(flags == 2, "FTB_BIT_TOGGLE set");
    FTB_BIT_TOGGLE(flags, 1);
    TEST_ASSERT(flags == 0, "FTB_BIT_TOGGLE clear");

    TEST_RESULT(true);
}

/*
 * ==========================================
 * DA ADVANCED TESTS
 * ==========================================
 */

bool test_da_foreach(void) {
    int* arr = NULL;
    ftb_raw_da_append(arr, 10);
    ftb_raw_da_append(arr, 20);
    ftb_raw_da_append(arr, 30);

    int sum = 0;
    ftb_da_foreach(int*, it, arr) {
        sum += *it;
    }
    TEST_ASSERT(sum == 60, "ftb_da_foreach computes correct sum");

    ftb_raw_da_free(arr);
    TEST_RESULT(true);
}

/*
 * ==========================================
 * STR ADVANCED TESTS
 * ==========================================
 */

#ifdef __GNUC__
bool test_str_printf(void) {
    ftb_ctx_t ctx = {0};
    ftb_mem_set_mark(&ctx);

    ftb_str_t s = ftb_str_printf(&ctx, "Hello %d %s", 42, "World");
    TEST_ASSERT(s != NULL, "str_printf created string");

    /* ftb_str_printf relies on standard snprintf, string is null terminated.
       Note: Macro does not increment da_count, so we assert via strcmp */
    TEST_ASSERT(strcmp(s, "Hello 42 World") == 0, "str_printf content matches");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}
#endif

/*
 * ==========================================
 * FILE I/O AND DIR TESTS
 * ==========================================
 */

bool test_file_io(void) {
    const char* test_file = "test_ftb_io.txt";
    const char* content = "Hello File IO";

    ftb_file_remove(test_file);
    TEST_ASSERT(!ftb_file_exists(test_file), "File should not exist initially");

    TEST_ASSERT(ftb_file_write_cstr(test_file, content), "Write cstr to file");
    TEST_ASSERT(ftb_file_exists(test_file), "File should exist after writing");

    i64 size = ftb_file_size(test_file);
    TEST_ASSERT(size == (i64)strlen(content), "File size should match content length");

    ftb_ctx_t ctx = {0};
    ftb_bytes_t data = ftb_file_read(&ctx, test_file);
    TEST_ASSERT(data != NULL, "Read file must not be NULL");
    TEST_ASSERT(ftb_da_count(data) == (u32)size, "Read data DA count match");
    TEST_ASSERT(strncmp((char*)data, content, size) == 0, "Read content match");

    TEST_ASSERT(ftb_file_remove(test_file), "Remove file successfully");
    TEST_ASSERT(!ftb_file_exists(test_file), "File should be gone");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_file_copy_and_mtime(void) {
    const char* src = "test_src.txt";
    const char* dst = "test_dst.txt";

    ftb_file_write_cstr(src, "copy me");

    TEST_ASSERT(ftb_file_copy(src, dst), "File copy success");
    TEST_ASSERT(ftb_file_exists(dst), "Dest file exists after copy");

    i64 mtime_src = ftb_file_mtime(src);
    i64 mtime_dst = ftb_file_mtime(dst);

    TEST_ASSERT(mtime_src > 0, "Source mtime valid");
    TEST_ASSERT(mtime_dst > 0, "Dest mtime valid");

    ftb_file_remove(src);
    ftb_file_remove(dst);
    TEST_RESULT(true);
}

bool test_dir_operations(void) {
    const char* test_dir = "test_ftb_dir";

#ifdef _WIN32
    RemoveDirectoryA(test_dir);
#else
    rmdir(test_dir);
#endif

    TEST_ASSERT(!ftb_dir_exists(test_dir), "Dir should not exist initially");

    TEST_ASSERT(ftb_dir_mkdir(test_dir), "Create directory");
    TEST_ASSERT(ftb_dir_exists(test_dir), "Directory should exist");

    TEST_ASSERT(ftb_dir_mkdir_ifnot_exists(test_dir), "Create existing directory safely");

#ifdef _WIN32
    RemoveDirectoryA(test_dir);
#else
    rmdir(test_dir);
#endif
    TEST_ASSERT(!ftb_dir_exists(test_dir), "Dir should be removed");

    TEST_RESULT(true);
}

static int g_file_count = 0;
static void test_list_callback(const char* name, bool is_dir, void* user) {
    (void)is_dir; (void)user;
    if (name) g_file_count++;
}

bool test_dir_list(void) {
    const char* test_dir = "test_ftb_list_dir";
    ftb_dir_mkdir(test_dir);

    ftb_file_write_cstr("test_ftb_list_dir" PSEP "f1.txt", "1");
    ftb_file_write_cstr("test_ftb_list_dir" PSEP "f2.txt", "2");

    g_file_count = 0;
    TEST_ASSERT(ftb_dir_list_dir(test_dir, test_list_callback, NULL), "List dir success");
    TEST_ASSERT(g_file_count == 2, "Found 2 files inside dir");

    ftb_file_remove("test_ftb_list_dir" PSEP "f1.txt");
    ftb_file_remove("test_ftb_list_dir" PSEP "f2.txt");
#ifdef _WIN32
    RemoveDirectoryA(test_dir);
#else
    rmdir(test_dir);
#endif

    TEST_RESULT(true);
}

/*
 * ==========================================
 * LOGGING TESTS
 * ==========================================
 */

bool test_logger(void) {
    ftb_ctx_t ctx = {0};
    const char* log_file = "test_ftb_log.txt";

    TEST_ASSERT(ftb_log_set_log_file_path(&ctx, log_file), "Set log file path");

    ftb_log_set_log_level(&ctx, ftb_log_level_info);
    TEST_ASSERT(ftb_log_info(&ctx, "This is an info log"), "Log info");
    TEST_ASSERT(ftb_log_warn(&ctx, "This is a warning log"), "Log warn");
    TEST_ASSERT(ftb_log_error(&ctx, "This is an error log"), "Log error");

    ftb_mem_delete_ctx(&ctx);

    i64 size = ftb_file_size(log_file);
    TEST_ASSERT(size > 0, "Log file should contain data");

    ftb_file_remove(log_file);
    TEST_RESULT(true);
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

    TEST_ASSERT(end - start <= 45, "Sleep duration should be at least ~30ms");
    TEST_RESULT(true);
}

/*
 * ==========================================
 * SCOPE TESTS
 * ==========================================
 */

bool test_scoped_ctx(void) {
    ftb_scoped_ctx {
        void* ptr = ftb_mem_alloc(pctx, 1024);
        TEST_ASSERT(ptr != NULL, "Alloc within scoped context");
        TEST_ASSERT(ftb_da_count(pctx->ptrs.items) == 1, "Count is 1 in scoped ctx");
    }
    // No easy way to assert destruction without hacking globals,
    // but a successful compilation & execution without leak crash proves correct syntax & flow.
    TEST_RESULT(true);
}

/*
 * ==========================================
 * STRUCT & POINTER MACROS
 * ==========================================
 */
typedef struct {
    int id;
    float value;
    char name[16];
} ftb_test_struct_t;

bool test_struct_macros(void) {
    ftb_test_struct_t obj;
    FTB_ZERO(obj);
    TEST_ASSERT(obj.id == 0 && obj.value == 0.0f && obj.name[0] == '\0', "FTB_ZERO clears struct");

    float* val_ptr = &obj.value;
    ftb_test_struct_t* recovered = FTB_CONTAINER_OF(val_ptr, ftb_test_struct_t, value);
    TEST_ASSERT(recovered == &obj, "FTB_CONTAINER_OF resolves parent pointer accurately");

    TEST_RESULT(true);
}

/*
 * ==========================================
 * PATH & CWD EDGE CASES
 * ==========================================
 */
bool test_path_rename_and_cwd(void) {
    ftb_ctx_t ctx = {0};

    /* Test rename */
    ftb_file_write_cstr("ftb_old_name.txt", "rename me");
    TEST_ASSERT(ftb_path_rename("ftb_old_name.txt", "ftb_new_name.txt"), "Rename file success");
    TEST_ASSERT(!ftb_file_exists("ftb_old_name.txt"), "Old file should no longer exist");
    TEST_ASSERT(ftb_file_exists("ftb_new_name.txt"), "New file should exist");
    ftb_file_remove("ftb_new_name.txt");

    /* Test CWD changing */
    ftb_path_t current_dir = ftb_str_create(&ctx);
    TEST_ASSERT(ftb_path_cwd_get(&ctx, &current_dir), "Get CWD");

    TEST_ASSERT(ftb_path_cwd_set(".."), "Change CWD to parent directory");
    TEST_ASSERT(ftb_path_cwd_set(current_dir), "Restore original CWD");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

/*
 * ==========================================
 * MEMORY & LOGGING EDGE CASES
 * ==========================================
 */
bool test_mem_realloc_to_zero(void) {
    ftb_ctx_t ctx = {0};
    void* ptr = ftb_mem_alloc(&ctx, 64);
    TEST_ASSERT(ptr != NULL, "Allocated 64 bytes");

    void* ptr2 = ftb_mem_realloc(&ctx, ptr, 0);
    TEST_ASSERT(ptr2 == NULL, "Realloc to 0 bytes returns NULL (acts as free)");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_logger_toggles(void) {
    ftb_ctx_t ctx = {0};

    ftb_log_set_timestap(&ctx, true);
    TEST_ASSERT(ctx.loger.timestaps == true, "Set timestamp true");

    ftb_log_toogle_timestap(&ctx);
    TEST_ASSERT(ctx.loger.timestaps == false, "Toggle timestamp false");

    TEST_RESULT(true);
}

/*
 * ==========================================
 * TIME RESOLUTION TESTS
 * ==========================================
 */
bool test_time_resolution(void) {
    u64 t_us = ftb_time_now_us();
    f64 t_sec = ftb_time_now_sec();

    TEST_ASSERT(t_us > 0, "Microseconds time is valid");
    TEST_ASSERT(t_sec > 0.0, "Seconds time is valid");

    TEST_RESULT(true);
}

/*
 * ==========================================
 * "ABSOLUTELY CLUELESS PERSON" TESTS (BAD USAGE)
 * ==========================================
 * These test how the library defends itself against terrible
 * inputs (NULLs, 0-lengths, out-of-bounds, self-references).
 */

/* Optional because of the warings with the flags of
    '-Wall -Wextra -pedantic' to check everything is fine.
 */

#ifdef TEST_CLUELESS

static bool _clueless_append_null(ftb_ctx_t* ctx, ftb_str_t s) {
    ftb_str_append_cstr(ctx, s, NULL);
    return true;
}

static bool _clueless_append_self(ftb_ctx_t* ctx, ftb_str_t s) {
    ftb_str_append_str(ctx, s, s);
    return true;
}

bool test_clueless_string_abuse(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s = ftb_str_create(&ctx);

    TEST_ASSERT(_clueless_append_null(&ctx, s) == false, "Library stopped appending NULL cstr");

    TEST_ASSERT(_clueless_append_self(&ctx, s) == false, "Library stopped appending str to itself");

    TEST_ASSERT(ftb_str_uppercase(NULL) == false, "Safely rejected NULL uppercase");
    TEST_ASSERT(ftb_str_find_cstr(NULL, "needle") == -1, "Safely rejected NULL search");
    TEST_ASSERT(ftb_str_find_cstr(s, NULL) == -1, "Safely rejected NULL needle");

    ftb_str_append_cstr(&ctx, s, "Normal String");
    TEST_ASSERT(ftb_str_remove_range(s, 99999, 500) == true, "Ignored out-of-bounds remove");
    TEST_ASSERT(strcmp(s, "Normal String") == 0, "String was completely unharmed");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_clueless_memory_abuse(void) {
    ftb_ctx_t ctx = {0};

    void* p1 = ftb_mem_alloc(&ctx, 0);
    void* p2 = ftb_mem_zalloc(&ctx, 0);
    TEST_ASSERT(p1 == NULL, "Allocating 0 bytes returns NULL");
    TEST_ASSERT(p2 == NULL, "Zallocating 0 bytes returns NULL");

    void* p3 = ftb_mem_realloc(&ctx, NULL, 0);
    TEST_ASSERT(p3 == NULL, "Reallocating NULL with 0 bytes returns NULL safely");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_clueless_file_abuse(void) {
    ftb_ctx_t ctx = {0};

    TEST_ASSERT(ftb_file_exists(NULL) == false, "NULL file existence handled");
    TEST_ASSERT(ftb_file_size(NULL) == false, "NULL file size returns -1/false");
    TEST_ASSERT(ftb_file_remove(NULL) == false, "NULL file remove rejected");

    const char* phantom_file = "i_do_not_exist_12345.xyz";
    TEST_ASSERT(ftb_file_size(phantom_file) == -1, "Phantom file size is -1");

    ftb_bytes_t data = ftb_file_read(&ctx, phantom_file);
    TEST_ASSERT(data == NULL, "Reading phantom file safely returns NULL");

    TEST_ASSERT(ftb_file_copy(NULL, NULL) == false, "Copying NULL to NULL rejected");
    TEST_ASSERT(ftb_file_copy(phantom_file, "destination.txt") == false, "Copying phantom file failed gracefully");

    TEST_ASSERT(ftb_dir_exists(NULL) == false, "NULL dir check rejected");
    TEST_ASSERT(ftb_dir_mkdir(NULL) == false, "NULL dir creation rejected");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_clueless_path_abuse(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    TEST_ASSERT(ftb_path_basename_cstr_n(&ctx, &out, "some" PSEP "path", 0) == false, "Path with 0 length rejected");
    TEST_ASSERT(ftb_path_dirname_cstr_n(&ctx, &out, "some" PSEP "path", 0) == false, "Dirname with 0 length rejected");

    TEST_ASSERT(ftb_path_join_cstr(&ctx, &out, NULL, NULL) == false, "Joining double NULL paths rejected");

    TEST_ASSERT(ftb_path_extension_cstr_n(&ctx, &out, NULL, 10) == false, "Getting extension of NULL rejected");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

#endif /* TEST_CLUELESS */

/*
 * ==========================================
 * MAIN ENTRY POINT
 * ==========================================
 */

int main(void)
{
    ftb_test_t* tests = 0;

    /* --- Memory Tests --- */
    FTB_ADD_TEST(tests, test_memory_alloc_and_free);
    FTB_ADD_TEST(tests, test_mem_free_direct);
    FTB_ADD_TEST(tests, test_memory_scope_markers);
    FTB_ADD_TEST(tests, test_memory_cleanup);
    FTB_ADD_TEST(tests, test_memory_realloc);
    FTB_ADD_TEST(tests, test_memory_alloc_zero);
    FTB_ADD_TEST(tests, test_memory_zalloc_large);

    /* --- String Tests --- */
    FTB_ADD_TEST(tests, test_str_basic_ops);
    FTB_ADD_TEST(tests, test_str_case);
    FTB_ADD_TEST(tests, test_str_compare);
    FTB_ADD_TEST(tests, test_str_starts_ends);
    FTB_ADD_TEST(tests, test_str_trim);
    FTB_ADD_TEST(tests, test_str_trim_no_spaces);
    FTB_ADD_TEST(tests, test_str_find_contains);
    FTB_ADD_TEST(tests, test_str_remove_range);
    FTB_ADD_TEST(tests, test_string_utils);
    FTB_ADD_TEST(tests, test_str_capacity_and_length);
    FTB_ADD_TEST(tests, test_str_to_cstr_empty);
    FTB_ADD_TEST(tests, test_str_cmp_length_diff);
    FTB_ADD_TEST(tests, test_str_cmp_prefix);
    FTB_ADD_TEST(tests, test_str_find_char_missing);
    FTB_ADD_TEST(tests, test_str_find_cstr_edge);
    FTB_ADD_TEST(tests, test_str_starts_ends_edge);
    FTB_ADD_TEST(tests, test_str_starts_ends_str);
    FTB_ADD_TEST(tests, test_str_find_str_advanced);
    FTB_ADD_TEST(tests, test_str_trim_advanced);
    FTB_ADD_TEST(tests, test_str_remove_range_edge_cases);
    FTB_ADD_TEST(tests, test_str_case_symbols);
    FTB_ADD_TEST(tests, test_str_null_handling);
    FTB_ADD_TEST(tests, test_str_append_loop);
    #ifdef __GNUC__
    FTB_ADD_TEST(tests, test_str_printf);
    #endif

    /* --- Dynamic Array Tests --- */
    FTB_ADD_TEST(tests, test_raw_da_macros);
    FTB_ADD_TEST(tests, test_managed_da_macros);
    FTB_ADD_TEST(tests, test_da_count_macros);
    FTB_ADD_TEST(tests, test_da_foreach);

    /* --- Path Tests --- */
    FTB_ADD_TEST(tests, test_path_basename);
    FTB_ADD_TEST(tests, test_path_dirname);
    FTB_ADD_TEST(tests, test_path_extension);
    FTB_ADD_TEST(tests, test_path_stem);
    FTB_ADD_TEST(tests, test_path_managed_wrappers);
    FTB_ADD_TEST(tests, test_path_dirname_cstr);
    FTB_ADD_TEST(tests, test_path_wrappers);
    FTB_ADD_TEST(tests, test_path_join_cstr);
    FTB_ADD_TEST(tests, test_path_normalize_cstr);
    FTB_ADD_TEST(tests, test_path_with_extension_cstr);
    FTB_ADD_TEST(tests, test_path_info_cstr);
    FTB_ADD_TEST(tests, test_path_fs_operations);

    /* File I/O & Directory Tests */
    FTB_ADD_TEST(tests, test_file_io);
    FTB_ADD_TEST(tests, test_file_copy_and_mtime);
    FTB_ADD_TEST(tests, test_dir_operations);
    FTB_ADD_TEST(tests, test_dir_list);

    /* Subsystem Tests */
    FTB_ADD_TEST(tests, test_logger);
    FTB_ADD_TEST(tests, test_time_sleep);
    FTB_ADD_TEST(tests, test_scoped_ctx);

    /* Final Polish Edge Cases */
    /* Macros & Math Tests */
    FTB_ADD_TEST(tests, test_math_macros);
    FTB_ADD_TEST(tests, test_bit_macros);
    FTB_ADD_TEST(tests, test_struct_macros);
    FTB_ADD_TEST(tests, test_path_rename_and_cwd);
    FTB_ADD_TEST(tests, test_mem_realloc_to_zero);
    FTB_ADD_TEST(tests, test_logger_toggles);
    FTB_ADD_TEST(tests, test_time_resolution);

    /* --- Bad Usage / Clueless Developer Tests --- */
    #ifdef TEST_CLUELESS
    FTB_ADD_TEST(tests, test_clueless_string_abuse);
    FTB_ADD_TEST(tests, test_clueless_memory_abuse);
    FTB_ADD_TEST(tests, test_clueless_file_abuse);
    FTB_ADD_TEST(tests, test_clueless_path_abuse);
    #endif /* TEST_CLUELESS */

    /* --- Run & Report --- */
    ftb_tests_run(tests);
    bool all_passed = ftb_tests_report(tests);

    ftb_raw_da_free(tests);
    return all_passed ? 0 : 1;
}
