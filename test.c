#define FTB_IMPLEMENTATION
#define FTB_TEST_CRASH
#include "ftb.h"

#include <stdio.h>
#include <string.h>

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

    ftb_mem_free(&ctx, ptr1);

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
    TEST_ASSERT(ftb_str_len(s) == 0, "String not empty");

    ftb_str_append_cstr(&ctx, s, "Hello");
    TEST_ASSERT(strcmp(s, "Hello") == 0, "Append cstr content");
    TEST_ASSERT(ftb_str_len(s) == 5, "Append cstr count");

    ftb_str_t s2 = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s2, " World");
    ftb_str_append_str(&ctx, s, s2);

    TEST_ASSERT(strcmp(s, "Hello World") == 0, "Append str content");
    TEST_ASSERT(ftb_str_len(s) == 11, "Append str count");

    char* raw = ftb_str_to_cstr(&ctx, s);
    TEST_ASSERT(strcmp(raw, "Hello World") == 0, "To cstr check");

    ftb_str_clear(s);
    TEST_ASSERT(ftb_str_len(s) == 0, "Clear count");
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

    TEST_ASSERT(ftb_str_cmp_str(s1, s2), "Cmp str true");

    ftb_str_append_cstr(&ctx, s2, "s");
    TEST_ASSERT(!ftb_str_cmp_str(s1, s2), "Cmp str false");

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
    TEST_ASSERT(ftb_str_len(s) == 3, "Trim left count");

    ftb_str_clear(s);

    ftb_str_append_cstr(&ctx, s, "abc   ");
    ftb_str_trim_right(s);

    TEST_ASSERT(strcmp(s, "abc") == 0, "Trim right content");
    TEST_ASSERT(ftb_str_len(s) == 3, "Trim right count");

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
    TEST_ASSERT(ftb_str_len(s) == 7, "Remove range count");

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
    TEST_ASSERT(ftb_str_len(s) == 4, "Trim count update");

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

    TEST_ASSERT(!ftb_str_cmp_str(s1, s2), "Different lengths not equal");
    TEST_ASSERT(!ftb_str_cmp_str(s2, s1), "Different lengths not equal reversed");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_cmp_prefix(void) {
    ftb_ctx_t ctx = {0};
    ftb_str_t s1 = ftb_str_create(&ctx);
    ftb_str_t s2 = ftb_str_create(&ctx);

    ftb_str_append_cstr(&ctx, s1, "prefix");
    ftb_str_append_cstr(&ctx, s2, "prefix_extra");

    TEST_ASSERT(ftb_str_cmp_str(s1, s2) == false, "Prefix is not equal to full");
    TEST_ASSERT(ftb_str_cmp_str(s2, s1) == false, "Full is not equal to prefix");

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

    TEST_ASSERT(ftb_str_starts_with_str(s1, start_s), "Starts with str");
    TEST_ASSERT(ftb_str_ends_with_str(s1, end_s), "Ends with str");

    TEST_ASSERT(!ftb_str_starts_with_str(s1, bad_s), "Does not start with str");
    TEST_ASSERT(!ftb_str_ends_with_str(s1, bad_s), "Does not end with str");

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

    TEST_ASSERT(ftb_str_find_str(haystack, needle1) == 4, "Find string inside");
    TEST_ASSERT(ftb_str_contains_str(haystack, needle1), "Contains string true");

    TEST_ASSERT(ftb_str_find_str(haystack, needle2) == -1, "Find string missing");
    TEST_ASSERT(!ftb_str_contains_str(haystack, needle2), "Contains string false");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_str_trim_advanced(void) {
    ftb_ctx_t ctx = {0};

    ftb_str_t s_empty = ftb_str_create(&ctx);
    ftb_str_trim(s_empty);
    TEST_ASSERT(ftb_str_len(s_empty) == 0, "Trim empty string");

    ftb_str_t s_spaces = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, s_spaces, "     ");
    ftb_str_trim(s_spaces);
    TEST_ASSERT(ftb_str_len(s_spaces) == 0, "Trim all spaces string");

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
    TEST_ASSERT(ftb_str_len(s) == 3, "Tail removed length");

    ftb_str_remove_range(s, 0, 100);
    TEST_ASSERT(ftb_str_len(s) == 0, "Remove all");
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
    TEST_ASSERT(ftb_str_len(s) == 100, "String length 100");
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

    const char* p1 = "dir/file.txt";
    TEST_ASSERT(ftb_path_basename_cstr(&ctx, &out, p1, strlen(p1)), "basename normal");
    TEST_ASSERT(strcmp(out, "file.txt") == 0, "basename normal match");

    const char* p2 = "file.txt";
    TEST_ASSERT(ftb_path_basename_cstr(&ctx, &out, p2, strlen(p2)), "basename no dir");
    TEST_ASSERT(strcmp(out, "file.txt") == 0, "basename no dir match");
#ifdef _WIN32
    const char* p3 = "dir\\file.txt";
    TEST_ASSERT(ftb_path_basename_cstr(&ctx, &out, p3, strlen(p3)), "basename windows backslash");
    TEST_ASSERT(strcmp(out, "file.txt") == 0, "basename windows backslash match");
#endif
    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_dirname(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = NULL;

    const char* p1 = "dir/file.txt";
    TEST_ASSERT(ftb_path_dirname_cstr(&ctx, &out, p1, strlen(p1)), "dirname normal");
    TEST_ASSERT(strcmp(out, "dir") == 0, "dirname normal match");
    
    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_extension(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = NULL;

    const char* p1 = "dir/file.txt";
    TEST_ASSERT(ftb_path_extension_cstr(&ctx, &out, p1, strlen(p1)), "extension normal");
    TEST_ASSERT(strcmp(out, "txt") == 0, "extension normal match");

    const char* p2 = "archive.tar.gz";
    TEST_ASSERT(ftb_path_extension_cstr(&ctx, &out, p2, strlen(p2)), "extension double");
    TEST_ASSERT(strcmp(out, "gz") == 0, "extension double match");

    const char* p3 = "dir/file";
    TEST_ASSERT(ftb_path_extension_cstr(&ctx, &out, p3, strlen(p3)) == false, "no extension");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_stem(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = NULL;

    const char* p1 = "dir/file.txt";
    TEST_ASSERT(ftb_path_stem_cstr(&ctx, &out, p1, strlen(p1)), "stem normal");
    TEST_ASSERT(strcmp(out, "file") == 0, "stem normal match");

    const char* p2 = "archive.tar.gz";
    TEST_ASSERT(ftb_path_stem_cstr(&ctx, &out, p2, strlen(p2)), "stem double ext");
    TEST_ASSERT(strcmp(out, "archive.tar") == 0, "stem double ext match");

    const char* p3 = "dir/file";
    TEST_ASSERT(ftb_path_stem_cstr(&ctx, &out, p3, strlen(p3)), "stem no ext");
    TEST_ASSERT(strcmp(out, "file") == 0, "stem no ext match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_managed_wrappers(void) {
    ftb_ctx_t ctx = {0};
    
    ftb_path_t in_path = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, in_path, "folder/image.png");

    ftb_path_t out_stem = NULL;
    TEST_ASSERT(ftb_path_stem(&ctx, &out_stem, in_path), "managed stem");
    TEST_ASSERT(strcmp(out_stem, "image") == 0, "managed stem match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_dirname_cstr(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    const char* p1 = "dir/file.txt";
    TEST_ASSERT(ftb_path_dirname_cstr(&ctx, &out, p1, strlen(p1)), "dirname normal");
    TEST_ASSERT(strcmp(out, "dir") == 0, "dirname normal match");

    const char* p2 = "/file.txt";
    TEST_ASSERT(ftb_path_dirname_cstr(&ctx, &out, p2, strlen(p2)), "dirname root");
    TEST_ASSERT(strcmp(out, "/") == 0, "dirname root match");

    const char* p3 = "file.txt";
    TEST_ASSERT(ftb_path_dirname_cstr(&ctx, &out, p3, strlen(p3)), "dirname empty");
    TEST_ASSERT(ftb_str_len(out) == 0, "dirname empty match");

    const char* p4 = "dir/subdir/file";
    TEST_ASSERT(ftb_path_dirname_cstr(&ctx, &out, p4, strlen(p4)), "dirname nested");
    TEST_ASSERT(strcmp(out, "dir/subdir") == 0, "dirname nested match");

    const char* p5 = "dir/";
    TEST_ASSERT(ftb_path_dirname_cstr(&ctx, &out, p5, strlen(p5)), "dirname trailing slash");
    TEST_ASSERT(strcmp(out, "dir") == 0, "dirname trailing slash match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_wrappers(void) {
    ftb_ctx_t ctx = {0};

    ftb_path_t my_path = ftb_str_create(&ctx);
    ftb_str_append_cstr(&ctx, my_path, "my_folder/archive.tar.gz");

    ftb_path_t out_wrapper = ftb_str_create(&ctx);
    ftb_path_t out_cstr = ftb_str_create(&ctx);

    ftb_path_basename_cstr(&ctx, &out_cstr, my_path, ftb_str_len(my_path));
    TEST_ASSERT(ftb_path_basename(&ctx, &out_wrapper, my_path), "wrapper basename call");
    TEST_ASSERT(strcmp(out_wrapper, out_cstr) == 0, "wrapper basename matches cstr result");

    ftb_path_dirname_cstr(&ctx, &out_cstr, my_path, ftb_str_len(my_path));
    TEST_ASSERT(ftb_path_dirname(&ctx, &out_wrapper, my_path), "wrapper dirname call");
    TEST_ASSERT(strcmp(out_wrapper, out_cstr) == 0, "wrapper dirname matches cstr result");

    ftb_path_extension_cstr(&ctx, &out_cstr, my_path, ftb_str_len(my_path));
    TEST_ASSERT(ftb_path_extension(&ctx, &out_wrapper, my_path), "wrapper ext call");
    TEST_ASSERT(strcmp(out_wrapper, out_cstr) == 0, "wrapper ext matches cstr result");

    ftb_path_stem_cstr(&ctx, &out_cstr, my_path, ftb_str_len(my_path));
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
    TEST_ASSERT(strcmp(out, "dir/file.txt") == 0, "join normal match");

    const char* p3 = "dir/";
    const char* p4 = "/file.txt";
    TEST_ASSERT(ftb_path_join_cstr(&ctx, &out, p3, p4), "join with multiple slashes");
    TEST_ASSERT(strcmp(out, "dir/file.txt") == 0, "join multiple slashes match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_normalize_cstr(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    const char* p1 = "a/b/../c";
    TEST_ASSERT(ftb_path_normalize_cstr(&ctx, &out, p1, strlen(p1)), "normalize relative dot-dot");
    TEST_ASSERT(strcmp(out, "a/c") == 0, "normalize relative match");

    const char* p2 = "/a/./b///c/../d/";
    TEST_ASSERT(ftb_path_normalize_cstr(&ctx, &out, p2, strlen(p2)), "normalize absolute complex");
    TEST_ASSERT(strcmp(out, "/a/b/d") == 0, "normalize absolute match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_with_extension_cstr(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    const char* p1 = "file.txt";
    const char* ext = ".md";
    TEST_ASSERT(ftb_path_with_extension_cstr(&ctx, &out, p1, strlen(p1), ext, strlen(ext)), "replace ext");
    TEST_ASSERT(strcmp(out, "file.md") == 0, "replace ext match");

    const char* p2 = "archive";
    const char* ext2 = "tar.gz";
    TEST_ASSERT(ftb_path_with_extension_cstr(&ctx, &out, p2, strlen(p2), ext2, strlen(ext2)), "add ext");
    TEST_ASSERT(strcmp(out, "archive.tar.gz") == 0, "add ext match");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
}

bool test_path_info_cstr(void) {
    const char* abs_p = "/usr/bin/gcc";
    const char* rel_p = "src/main.c";

    TEST_ASSERT(ftb_path_is_absolute_cstr(abs_p, strlen(abs_p)), "is absolute true");
    TEST_ASSERT(!ftb_path_is_absolute_cstr(rel_p, strlen(rel_p)), "is absolute false");

    TEST_ASSERT(!ftb_path_is_relative_cstr(abs_p, strlen(abs_p)), "is relative false");
    TEST_ASSERT(ftb_path_is_relative_cstr(rel_p, strlen(rel_p)), "is relative true");

    TEST_ASSERT(ftb_path_has_extension_cstr(rel_p, strlen(rel_p)), "has extension true");
    TEST_ASSERT(!ftb_path_has_extension_cstr(abs_p, strlen(abs_p)), "has extension false");

    TEST_RESULT(true);
}

bool test_path_fs_operations(void) {
    ftb_ctx_t ctx = {0};
    ftb_path_t out = ftb_str_create(&ctx);

    TEST_ASSERT(ftb_path_cwd_get(&ctx, &out), "get cwd");
    TEST_ASSERT(ftb_str_len(out) > 0, "cwd string populated");

    const char* current_dir = ".";
    TEST_ASSERT(ftb_path_exists_cstr(&ctx, current_dir, 1), "cwd exists");
    TEST_ASSERT(ftb_path_is_dir_cstr(&ctx, current_dir, 1), "cwd is dir");
    TEST_ASSERT(!ftb_path_is_file_cstr(&ctx, current_dir, 1), "cwd is not a regular file");

    TEST_ASSERT(ftb_path_absolute_cstr(&ctx, &out, current_dir, 1), "resolve abs path");
    TEST_ASSERT(ftb_path_is_absolute_cstr(out, ftb_str_len(out)), "resolved path is absolute");

    ftb_mem_delete_ctx(&ctx);
    TEST_RESULT(true);
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

    /* --- Dynamic Array Tests --- */
    FTB_ADD_TEST(tests, test_raw_da_macros);
    FTB_ADD_TEST(tests, test_managed_da_macros);
    FTB_ADD_TEST(tests, test_da_count_macros);

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

    /* --- Run & Report --- */
    ftb_tests_run(tests);
    bool all_passed = ftb_tests_report(tests);

    ftb_raw_da_free(tests);
    return all_passed ? 0 : 1;
}
