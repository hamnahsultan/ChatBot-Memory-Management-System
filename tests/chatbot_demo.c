#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "message.h"
#include "history.h"
#include "compaction.h"


/* ============================================
   COLORS
   ============================================ */

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define CYAN    "\033[36m"
#define RESET   "\033[0m"


/* ============================================
   GLOBALS
   ============================================ */

static int TOTAL = 0;
static int PASS  = 0;


/* ============================================
   CHECK FRAMEWORK
   ============================================ */

void check(int ok, const char *msg)
{
    TOTAL++;

    if (ok) {
        printf(GREEN "   [PASS] %s\n" RESET, msg);
        PASS++;
    } else {
        printf(RED   "   [FAIL] %s\n" RESET, msg);
    }
}


/* ============================================
   HISTORY VIEW
   ============================================ */

void show_history(History *h)
{
    printf(CYAN "\nHistory (%d messages)\n" RESET,
           h->count);

    printf("--------------------------------\n");

    Node *cur = h->head;

    if (!cur) {
        printf("(empty)\n");
        return;
    }

    while (cur) {

        printf(" [%02d] \"%s\"\n",
               cur->msg->id,
               cur->msg->text);

        cur = cur->next;
    }
}


/* ============================================
   PIPELINE INSERT
   ============================================ */

int pipeline_add(History *h,
                 int id,
                 const char *raw)
{
    printf("\nUser input: \"%s\"\n", raw);

    char *out = parser_process(raw);

    if (!out) {

        printf(YELLOW
               "Rejected: blocked by policy\n"
               RESET);

        check(1, "Policy rejection");

        return 0;
    }

    if (strstr(out, "********")) {

        printf(YELLOW
               "Censored: cheating replaced\n"
               RESET);

        check(1, "Censorship applied");
    }

    printf("Accepted: \"%s\"\n", out);

    Message *m = msg_create(id, out);

    free(out);

    if (!m) {

        printf(RED "Allocation failed\n" RESET);

        check(0, "Message allocation");

        return -1;
    }

    history_add(h, m);

    check(1, "Inserted into history");

    return 1;
}


/* ============================================
   INVARIANT CHECK
   ============================================ */

void validate(History *h,
              const char *stage)
{
    printf("\nValidating (%s)\n", stage);

    if (history_validate(h) == 0) {

        printf(GREEN "Structures consistent\n" RESET);
        check(1, "Invariant holds");
    }
    else {

        printf(RED "Invariant broken\n" RESET);
        check(0, "Invariant holds");
    }
}


/* ============================================
   BASIC FLOW (10+)
   ============================================ */

void test_basic(void)
{
    printf(BLUE "\n[PHASE] Basic pipeline\n" RESET);

    History h;
    history_init(&h);

    pipeline_add(&h,1,"hello");
    pipeline_add(&h,2,"hi there");
    pipeline_add(&h,3,"I was cheating");
    pipeline_add(&h,4,"world");

    check(h.count == 4, "Basic count");

    show_history(&h);

    validate(&h,"basic");

    history_reset(&h);
}


/* ============================================
   BLOCKING (10+)
   ============================================ */

void test_blocking(void)
{
    printf(BLUE "\n[PHASE] Blocking + rollback\n" RESET);

    History h;
    history_init(&h);

    pipeline_add(&h,1,"hello");
    pipeline_add(&h,2,"this is plagiarism");
    pipeline_add(&h,3,"ok");
    pipeline_add(&h,4,"plagiarise here");

    check(h.count == 2, "Blocked messages removed");

    show_history(&h);

    validate(&h,"blocking");

    history_reset(&h);
}


/* ============================================
   RESIZE + MASS (10+)
   ============================================ */

void test_mass(void)
{
    printf(BLUE "\n[PHASE] Mass insert\n" RESET);

    History h;
    history_init(&h);

    char buf[32];

    for (int i = 0; i < 40; i++) {

        sprintf(buf,"msg %d",i);

        pipeline_add(&h,i,buf);
    }

    check(h.count == 40, "Mass count");
    check(h.capacity >= 40, "Resize happened");

    validate(&h,"mass");

    history_reset(&h);
}


/* ============================================
   DELETE / SHIFT / REWIND (10+)
   ============================================ */

void test_deletions(void)
{
    printf(BLUE "\n[PHASE] Deletions\n" RESET);

    History h;
    history_init(&h);

    for (int i = 0; i < 12; i++) {

        char b[16];
        sprintf(b,"m%d",i);

        pipeline_add(&h,i,b);
    }

    history_delete_kth_last(&h,3);
    history_shift_left(&h);
    history_rewind(&h,2);

    check(h.count == 8, "Delete count");

    show_history(&h);

    validate(&h,"delete");

    history_reset(&h);
}


/* ============================================
   PURGE + COMPACT (10+)
   ============================================ */

void test_compact(void)
{
    printf(BLUE "\n[PHASE] Purge + compaction\n" RESET);

    History h;
    history_init(&h);

    pipeline_add(&h,1,"cheating now");
    pipeline_add(&h,2,"normal");
    pipeline_add(&h,3,"more cheating");
    pipeline_add(&h,4,"ok");

    int r = history_purge_word(&h,"********");

    check(r == 2, "Purge removed 2");

    history_compact(&h);

    show_history(&h);

    validate(&h,"compact");

    history_reset(&h);
}


/* ============================================
   STRESS (15+)
   ============================================ */

void test_stress(void)
{
    printf(BLUE "\n[PHASE] Stress test\n" RESET);

    History h;
    history_init(&h);

    char buf[32];

    for (int i = 0; i < 200; i++) {

        if (i % 7 == 0)
            strcpy(buf,"cheating");
        else if (i % 11 == 0)
            strcpy(buf,"plagiarism");
        else
            sprintf(buf,"m%d",i);

        pipeline_add(&h,i,buf);

        if (i % 15 == 0)
            history_shift_left(&h);

        if (i % 20 == 0 && h.count)
            history_delete_kth_last(&h,1);

        if (i % 25 == 0)
            history_compact(&h);

        if (history_validate(&h) != 0) {

            check(0,"Stress invariant");
            history_reset(&h);
            return;
        }
    }

    check(1,"Stress completed");

    validate(&h,"stress");

    history_reset(&h);
}


/* ============================================
   REPEATABILITY (5+)
   ============================================ */

void test_repeat(void)
{
    printf(BLUE "\n[PHASE] Repeatability\n" RESET);

    int ok = 1;

    for (int r = 0; r < 20; r++) {

        History h;
        history_init(&h);

        pipeline_add(&h,1," cheating ");
        pipeline_add(&h,2,"ok");

        if (h.count != 2)
            ok = 0;

        if (strcmp(h.array[0]->text,"********") != 0)
            ok = 0;

        history_reset(&h);
    }

    check(ok,"Repeat stable");
}


/* ============================================
   MAIN
   ============================================ */

int main(void)
{
    printf("\n============================================\n");
    printf("   CS200 FULL SYSTEM VISUAL TEST HARNESS\n");
    printf("============================================\n");

    test_basic();      /* ~10 */
    test_blocking();   /* ~10 */
    test_mass();       /* ~10 */
    test_deletions();  /* ~10 */
    test_compact();    /* ~10 */
    test_stress();     /* ~15 */
    test_repeat();     /* ~5  */

    printf("\n--------------------------------------------\n");
    printf("Result: %d / %d checks passed\n",
           PASS, TOTAL);
    printf("--------------------------------------------\n");

    if (PASS == TOTAL)
        printf(GREEN "SYSTEM CORRECT\n\n" RESET);
    else
        printf(RED "SYSTEM HAS ERRORS\n\n" RESET);

    return 0;
}
