#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <setjmp.h>

#include "message.h"




/* =====================================================
   COLORS (may be ignored on some Windows terminals)
   ===================================================== */

#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define CYAN    "\033[1;36m"
#define RESET   "\033[0m"


/* =====================================================
   GLOBAL STATE
   ===================================================== */

static int total_tests  = 0;
static int passed_tests = 0;

static jmp_buf jump_env;


/* =====================================================
   SEGFAULT HANDLER
   ===================================================== */

void segv_handler(int sig) {

    (void)sig;

    printf(RED "\n[CRASH] SEGMENTATION FAULT\n" RESET);
    printf(YELLOW
           "-> Invalid memory access detected\n"
           "-> Possible NULL dereference\n"
           "-> Possible use-after-free\n"
           "-> Possible buffer overflow\n\n"
           RESET);

    longjmp(jump_env, 1);
}


/* =====================================================
   TEST FRAMEWORK
   ===================================================== */

void begin_test(const char *name) {

    total_tests++;

    printf(BLUE "\n[%02d] %s\n" RESET,
           total_tests, name);
}


void pass_test(void) {

    passed_tests++;

    printf(GREEN "   [PASS]\n" RESET);
}


/* =====================================================
   EXPECTED vs GIVEN
   ===================================================== */

#define FAIL_INT(exp, got, msg)                         \
    do {                                                \
        printf(RED "   [FAIL]\n" RESET);                \
        printf(YELLOW "   -> %s\n", msg);               \
        printf("   -> Expected: %d\n", exp);            \
        printf("   -> Given:    %d\n\n" RESET, got);    \
        return;                                        \
    } while (0)


#define FAIL_PTR(exp, got, msg)                         \
    do {                                                \
        printf(RED "   [FAIL]\n" RESET);                \
        printf(YELLOW "   -> %s\n", msg);               \
        printf("   -> Expected: %p\n", exp);            \
        printf("   -> Given:    %p\n\n" RESET, got);    \
        return;                                        \
    } while (0)


#define FAIL_STR(exp, got, msg)                         \
    do {                                                \
        printf(RED "   [FAIL]\n" RESET);                \
        printf(YELLOW "   -> %s\n", msg);               \
        printf("   -> Expected: \"%s\"\n", exp);        \
        printf("   -> Given:    \"%s\"\n\n" RESET, got);\
        return;                                        \
    } while (0)



/* =====================================================
   MEMORY CHECKERS
   ===================================================== */

void check_bytes(const char *exp,
                 const char *got,
                 const char *ctx) {

    if (!exp || !got) {

        printf(RED "   [FAIL]\n" RESET);
        printf(YELLOW "   -> %s\n", ctx);
        printf("   -> NULL pointer in comparison\n\n" RESET);
        return;
    }

    size_t el = strlen(exp);
    size_t gl = strlen(got);

    if (el != gl) {

        printf(RED "   [FAIL]\n" RESET);
        printf(YELLOW "   -> %s\n", ctx);

        printf("   -> Length mismatch\n");
        printf("   -> Expected: %lu\n", (unsigned long) el);
        printf("   -> Given:    %lu\n\n" RESET,
               (unsigned long) gl);

        return;
    }

    if (memcmp(exp, got, el + 1) != 0) {

        printf(RED "   [FAIL]\n" RESET);
        printf(YELLOW "   -> %s\n", ctx);

        printf("   -> Raw bytes differ\n");
        printf("   -> Expected: \"%s\"\n", exp);
        printf("   -> Given:    \"%s\"\n\n" RESET, got);
    }
}


void check_null_term(const char *s,
                     const char *ctx) {

    if (!s) {

        printf(RED "   [FAIL]\n" RESET);
        printf(YELLOW "   -> %s\n", ctx);
        printf("   -> NULL string pointer\n\n" RESET);

        return;
    }

    size_t len = strlen(s);

    if (s[len] != '\0') {

        printf(RED "   [FAIL]\n" RESET);
        printf(YELLOW "   -> %s\n", ctx);

        printf("   -> Missing null terminator\n");
        printf("   -> Buffer overflow likely\n\n" RESET);
    }
}


/* =====================================================
   SAFE RUNNER
   ===================================================== */

int safe_run(void (*fn)(void)) {

    if (setjmp(jump_env) == 0) {

        fn();
        return 1;
    }

    return 0;
}


/* =====================================================
   HELPERS
   ===================================================== */

Message *make_msg(int id, const char *s) {

    Message *m = msg_create(id, s);

    if (!m) {

        printf(RED "FATAL: msg_create failed\n" RESET);
        exit(1);
    }

    return m;
}


/* =====================================================
   CREATE TESTS
   ===================================================== */

void test_create_basic(void) {

    begin_test("Create: normal");

    Message *m = msg_create(1, "hello");

    if (!m)
        FAIL_PTR((void*)1, NULL, "Returned NULL");

    if (m->id != 1)
        FAIL_INT(1, m->id, "ID mismatch");

    check_bytes("hello", m->text,
                "Text copy incorrect");

    check_null_term(m->text,
                    "Text not null-terminated");

    msg_destroy(m);
    pass_test();
}


void test_create_null(void) {

    begin_test("Create: NULL input");

    Message *m = msg_create(2, NULL);

    if (m != NULL)
        FAIL_PTR(NULL, m, "Should reject NULL");

    pass_test();
}


void test_create_empty(void) {

    begin_test("Create: empty");

    Message *m = make_msg(3, "");

    check_bytes("", m->text,
                "Empty string wrong");

    msg_destroy(m);
    pass_test();
}


void test_create_long(void) {

    begin_test("Create: long");

    char buf[4000];

    memset(buf, 'A', 3999);
    buf[3999] = 0;

    Message *m = make_msg(4, buf);

    check_bytes(buf, m->text,
                "Long copy failed");

    msg_destroy(m);
    pass_test();
}


void test_create_independent(void) {

    begin_test("Create: deep copy");

    char buf[32] = "hello";

    Message *m = make_msg(5, buf);

    buf[0] = 'X';

    check_bytes("hello", m->text,
                "Shallow copy detected");

    msg_destroy(m);
    pass_test();
}


/* =====================================================
   DESTROY TESTS
   ===================================================== */

void test_destroy_null(void) {

    begin_test("Destroy: NULL");

    msg_destroy(NULL);

    pass_test();
}


void test_destroy_once(void) {

    begin_test("Destroy: single free");

    Message *m = make_msg(6, "hi");

    msg_destroy(m);

    pass_test();
}


void test_destroy_after_clone(void) {

    begin_test("Destroy: after clone");

    Message *a = make_msg(7, "x");
    Message *b = msg_clone(a, 8);

    msg_destroy(a);
    msg_destroy(b);

    pass_test();
}


/* =====================================================
   PRINT TESTS
   ===================================================== */

void test_print_basic(void) {

    begin_test("Print: normal");

    Message *m = make_msg(9, "print");

    msg_print(m);

    msg_destroy(m);

    pass_test();
}


void test_print_null(void) {

    begin_test("Print: NULL");

    msg_print(NULL);

    pass_test();
}


/* =====================================================
   CONTAINS TESTS
   ===================================================== */

void test_contains_yes(void) {

    begin_test("Contains: found");

    Message *m = make_msg(10, "hello world");

    int r = msg_contains(m, "world");

    if (r != 1)
        FAIL_INT(1, r, "Word not detected");

    msg_destroy(m);
    pass_test();
}


void test_contains_no(void) {

    begin_test("Contains: missing");

    Message *m = make_msg(11, "hello");

    int r = msg_contains(m, "abc");

    if (r != 0)
        FAIL_INT(0, r, "False positive");

    msg_destroy(m);
    pass_test();
}


void test_contains_null(void) {

    begin_test("Contains: NULL args");

    Message *m = make_msg(12, "hi");

    int r1 = msg_contains(NULL, "hi");
    int r2 = msg_contains(m, NULL);

    if (r1 != -1)
        FAIL_INT(-1, r1, "NULL msg not handled");

    if (r2 != -1)
        FAIL_INT(-1, r2, "NULL word not handled");

    msg_destroy(m);
    pass_test();
}


/* =====================================================
   LENGTH TESTS
   ===================================================== */

void test_length_basic(void) {

    begin_test("Length: normal");

    Message *m = make_msg(13, "hello");

    int l = msg_length(m);

    if (l != 5)
        FAIL_INT(5, l, "Wrong length");

    msg_destroy(m);
    pass_test();
}


void test_length_empty(void) {

    begin_test("Length: empty");

    Message *m = make_msg(14, "");

    int l = msg_length(m);

    if (l != 0)
        FAIL_INT(0, l, "Empty wrong");

    msg_destroy(m);
    pass_test();
}


void test_length_null(void) {

    begin_test("Length: NULL");

    int l = msg_length(NULL);

    if (l != -1)
        FAIL_INT(-1, l, "NULL not handled");

    pass_test();
}


/* =====================================================
   CLONE TESTS
   ===================================================== */

void test_clone_basic(void) {

    begin_test("Clone: basic");

    Message *a = make_msg(15, "clone");

    Message *b = msg_clone(a, 99);

    if (!b)
        FAIL_PTR((void*)1, NULL, "Clone NULL");

    if (b->id != 99)
        FAIL_INT(99, b->id, "ID wrong");

    check_bytes("clone", b->text,
                "Clone text wrong");

    msg_destroy(a);
    msg_destroy(b);

    pass_test();
}


void test_clone_deep(void) {

    begin_test("Clone: deep");

    Message *a = make_msg(16, "orig");
    Message *b = msg_clone(a, 17);

    b->text[0] = 'X';

    check_bytes("orig", a->text,
                "Shared buffer");

    msg_destroy(a);
    msg_destroy(b);

    pass_test();
}


void test_clone_null(void) {

    begin_test("Clone: NULL");

    Message *m = msg_clone(NULL, 1);

    if (m != NULL)
        FAIL_PTR(NULL, m, "Should reject NULL");

    pass_test();
}


/* =====================================================
   STRESS + FUZZ
   ===================================================== */

void test_mass_alloc(void) {

    begin_test("Stress: alloc/free");

    for (int i = 0; i < 20000; i++) {

        Message *m = msg_create(i, "stress");

        if (!m)
            FAIL_PTR((void*)1, NULL, "Alloc failed");

        msg_destroy(m);
    }

    pass_test();
}


void test_mass_clone(void) {

    begin_test("Stress: clone");

    Message *b = make_msg(20, "base");

    for (int i = 0; i < 5000; i++) {

        Message *c = msg_clone(b, i);

        if (!c)
            FAIL_PTR((void*)1, NULL, "Clone failed");

        msg_destroy(c);
    }

    msg_destroy(b);

    pass_test();
}


void test_fuzz(void) {

    begin_test("Fuzz: random");

    char buf[256];

    for (int i = 0; i < 3000; i++) {

        int len = rand() % 255;

        for (int j = 0; j < len; j++)
            buf[j] = 'a' + rand() % 26;

        buf[len] = 0;

        Message *m = msg_create(i, buf);

        if (!m)
            FAIL_PTR((void*)1, NULL, "Fuzz failed");

        check_bytes(buf, m->text,
                    "Fuzz mismatch");

        msg_destroy(m);
    }

    pass_test();
}


/* =====================================================
   MAIN
   ===================================================== */

int main(void) {

    srand(42);

    signal(SIGSEGV, segv_handler);

    printf(CYAN "\n=================================\n");
    printf("   CS200 MESSAGE TEST HARNESS\n");
    printf("=================================\n" RESET);


    safe_run(test_create_basic);
    test_create_null();
    test_create_empty();
    test_create_long();
    test_create_independent();

    test_destroy_null();
    test_destroy_once();
    test_destroy_after_clone();

    test_print_basic();
    test_print_null();

    test_contains_yes();
    test_contains_no();
    test_contains_null();

    test_length_basic();
    test_length_empty();
    test_length_null();

    test_clone_basic();
    test_clone_deep();
    test_clone_null();

    test_mass_alloc();
    test_mass_clone();
    test_fuzz();


    printf(CYAN "\n=================================\n");
    printf("RESULT: %d / %d PASSED\n",
           passed_tests, total_tests);
    printf("=================================\n\n" RESET);


    if (passed_tests == total_tests) {

        printf(GREEN "ALL TESTS PASSED - MEMORY SAFE\n\n" RESET);

    } else {

        printf(RED "FIX FAILURES BEFORE SUBMITTING\n\n" RESET);
    }

    return 0;
}
