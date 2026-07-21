#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"

/* ============================================
   COLORS
   ============================================ */

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"

/* ============================================
   COUNTERS
   ============================================ */

static int RUN  = 0;
static int PASS = 0;

/* ============================================
   ASSERTION HELPER
   ============================================ */

#define ASSERT(cond, msg)                                      \
    do {                                                       \
        if (!(cond)) {                                        \
            printf(RED "   [ASSERT FAIL] %s\n\n" RESET, msg); \
            return;                                           \
        }                                                      \
    } while (0)

/* ============================================
   FRAMEWORK
   ============================================ */

void begin(const char *name)
{
    printf(BLUE "[%02d] %s\n" RESET,
           ++RUN, name);
}

void pass(void)
{
    printf(GREEN "   [PASS]\n\n" RESET);
    PASS++;
}

void fail(const char *exp,
          const char *got)
{
    printf(RED "   [FAIL]\n" RESET);

    printf(YELLOW "   Expected: \"%s\"\n" RESET,
           exp ? exp : "NULL");

    printf(YELLOW "   Given:    \"%s\"\n\n" RESET,
           got ? got : "NULL");
}

/* ============================================
   CORE CHECK (VALUE + OWNERSHIP)
   ============================================ */

void check(const char *name,
           const char *in,
           const char *exp)
{
    begin(name);

    char *out = parser_process(in);

    /* Both NULL = PASS */
    if (!exp && !out) {
        pass();
        return;
    }

    /* Both valid and equal */
    if (exp && out &&
        strcmp(exp, out) == 0) {

        pass();
        parser_free(out);
        return;
    }

    fail(exp, out);
    parser_free(out);
}

/* ============================================
   BASIC PIPELINE
   ============================================ */

void test_basic(void)
{
    check("Trim + normalize",
          "   hi   there   ",
          "hi there");

    check("Preserve normal text",
          "hello world",
          "hello world");

    check("Multiple spaces",
          "a     b     c",
          "a b c");
}

/* ============================================
   CHEATING CENSOR
   ============================================ */

void test_cheating(void)
{
    check("Lowercase cheating",
          "I was cheating",
          "I was ********");

    check("Uppercase CHEATING",
          "CHEATING is bad",
          "******** is bad");

    check("Mixed case",
          "cHeAtInG detected",
          "******** detected");

    check("Multiple cheating",
          "cheating cheating cheating",
          "******** ******** ********");

    check("With spaces",
          " cheating ",
          "********");
}

/* ============================================
   HARD BLOCK
   ============================================ */

void test_blocked(void)
{
    check("Block plagiarise",
          "they plagiarise work",
          NULL);

    check("Block plagiarism",
          "this is plagiarism",
          NULL);

    check("Block plagiarising",
          "plagiarising assignment",
          NULL);

    check("Mixed case",
          "PlAgIaRiSe found",
          NULL);
}

/* ============================================
   MIXED POLICY
   ============================================ */

void test_mixed(void)
{
    check("Cheating allowed",
          "no cheating here",
          "no ******** here");

    check("Blocked dominates",
          "plagiarise cheating",
          NULL);
}

/* ============================================
   INPUT IMMUTABILITY
   ============================================ */

void test_input_immutability(void)
{
    begin("Input immutability");

    char input[]    = " cheating ";
    char original[] = " cheating ";

    char *out = parser_process(input);

    ASSERT(strcmp(input, original) == 0,
           "parser modified input buffer");

    ASSERT(out != NULL,
           "unexpected NULL output");

    ASSERT(strcmp(out, "********") == 0,
           "incorrect output");

    parser_free(out);
    pass();
}

/* ============================================
   FRESH ALLOCATION
   ============================================ */

void test_fresh_allocation(void)
{
    begin("Fresh allocation per call");

    char *a = parser_process(" cheating ");
    char *b = parser_process(" cheating ");

    ASSERT(a && b, "NULL returned");
    ASSERT(a != b, "static or reused buffer detected");
    ASSERT(strcmp(a, b) == 0, "outputs differ");

    parser_free(a);
    parser_free(b);
    pass();
}

/* ============================================
   ROLLBACK + CLEAN FAILURE
   ============================================ */

void test_rollback(void)
{
    begin("Rollback semantics");

    for (int i = 0; i < 300; i++) {
        char *s = parser_process("this is plagiarism");
        ASSERT(s == NULL,
               "blocked input returned memory");
    }

    char *ok = parser_process(" cheating ");

    ASSERT(ok != NULL,
           "parser broken after failures");

    ASSERT(strcmp(ok, "********") == 0,
           "incorrect output after failures");

    parser_free(ok);
    pass();
}

/* ============================================
   LIFETIME VALIDITY
   ============================================ */

void test_lifetime_validity(void)
{
    begin("Lifetime validity");

    char *s = parser_process(" cheating ");

    ASSERT(s != NULL, "NULL returned");
    ASSERT(strlen(s) == 8, "buffer corrupted");
    ASSERT(strcmp(s, "********") == 0,
           "unexpected mutation");

    parser_free(s);
    pass();
}

/* ============================================
   parser_free(NULL) — EXPLICIT
   ============================================ */

void test_parser_free_null(void)
{
    begin("parser_free(NULL) safety");

    parser_free(NULL);
    parser_free(NULL);
    parser_free(NULL);

    pass();
}

/* ============================================
   parser_free(NULL) INTERLEAVED
   ============================================ */

void test_parser_free_null_interleaved(void)
{
    begin("parser_free(NULL) interleaved");

    parser_free(NULL);

    char *s = parser_process(" cheating ");
    ASSERT(s != NULL,
           "parser_process failed after NULL free");

    parser_free(NULL);
    parser_free(s);

    pass();
}

/* ============================================
   REPEATED STABILITY
   ============================================ */

void test_repeat(void)
{
    begin("Repeated allocation + cleanup");

    for (int i = 0; i < 300; i++) {
        char *s = parser_process(" cheating ");
        ASSERT(s != NULL, "NULL returned");
        ASSERT(strcmp(s, "********") == 0,
               "incorrect output");
        parser_free(s);
    }

    pass();
}

/* ============================================
   MAIN
   ============================================ */

int main(void)
{
    printf("\n=================================\n");
    printf("   CS200 PARSER POLICY TESTS\n");
    printf("=================================\n\n");

    test_basic();
    test_cheating();
    test_blocked();
    test_mixed();

    /* Destructor correctness */
    test_parser_free_null();
    test_parser_free_null_interleaved();

    test_input_immutability();
    test_fresh_allocation();
    test_rollback();
    test_lifetime_validity();
    test_repeat();

    printf("---------------------------------\n");
    printf("Passed: %d / %d\n", PASS, RUN);
    printf("---------------------------------\n\n");

    return 0;
}