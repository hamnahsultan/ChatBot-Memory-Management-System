#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "history.h"
#include "message.h"

/* ============================================
   COLORS
   ============================================ */

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"

/* ============================================
   GLOBAL COUNTERS
   ============================================ */

static int TESTS_RUN = 0;
static int TESTS_PASS = 0;

/* ============================================
   TEST FRAMEWORK
   ============================================ */

void begin(const char *name)
{
    printf(BLUE "[%02d] %s\n" RESET, ++TESTS_RUN, name);
}

void pass(void)
{
    printf(GREEN "   [PASS]\n\n" RESET);
    TESTS_PASS++;
}

void fail_int(const char *msg, int exp, int got)
{
    printf(RED "   [FAIL]\n" RESET);
    printf("   %s\n", msg);
    printf(YELLOW "   Expected: %d\n" RESET, exp);
    printf(YELLOW "   Given:    %d\n\n" RESET, got);
}

void fail_str(const char *msg, const char *exp, const char *got)
{
    printf(RED "   [FAIL]\n" RESET);
    printf("   %s\n", msg);
    printf(YELLOW "   Expected: %s\n" RESET, exp);
    printf(YELLOW "   Given:    %s\n\n" RESET, got);
}

/* ============================================
   HELPERS
   ============================================ */

Message *M(int id, const char *s)
{
    return msg_create(id, s);
}

void fill(History *h, int n)
{
    char buf[64];
    for (int i = 0; i < n; i++) {
        snprintf(buf, sizeof(buf), "m%d", i);
        history_add(h, M(i, buf));
    }
}

/* ============================================
   INVARIANT CHECKER
   ============================================ */

int check_history(History *h, int n, const int *ids)
{
    if (h->count != n) return -1;

    /* Array */
    for (int i = 0; i < n; i++) {
        if (!h->array[i]) return -2;
        if (h->array[i]->id != ids[i]) return -3;
    }

    /* List */
    Node *cur = h->head;
    for (int i = 0; i < n; i++) {
        if (!cur) return -4;
        if (!cur->msg) return -5;
        if (cur->msg->id != ids[i]) return -6;
        cur = cur->next;
    }
    if (cur) return -7;

    /* Tail */
    if (n == 0 && h->tail) return -8;
    if (n > 0 && h->tail->msg->id != ids[n-1]) return -9;

    /* Validation must AGREE */
    if (history_validate(h) != 0) return -10;

    return 0;
}

/* ============================================
   TESTS
   ============================================ */

void test_init()
{
    begin("Init empty history");

    History h;
    history_init(&h);

    int ids[] = {};
    int r = check_history(&h, 0, ids);

    if (r == 0) pass();
    else fail_int("Invalid empty state", 0, r);

    history_reset(&h);
}

/* ---------- ADD ---------- */

void test_add_basic()
{
    begin("Add 5 messages");

    History h;
    history_init(&h);
    fill(&h, 5);

    int ids[] = {0,1,2,3,4};
    int r = check_history(&h, 5, ids);

    if (r == 0) pass();
    else fail_int("Add failed", 0, r);

    history_reset(&h);
}

void test_add_resize()
{
    begin("Add triggers resize");

    History h;
    history_init(&h);
    fill(&h, 40);

    int ids[40];
    for (int i = 0; i < 40; i++) ids[i] = i;

    int r = check_history(&h, 40, ids);

    if (r == 0) pass();
    else fail_int("Resize broken", 0, r);

    history_reset(&h);
}

void test_add_blocked()
{
    begin("Blocked message never inserted");

    History h;
    history_init(&h);

    Message *m = M(1, "this is plagiarism");
    int r = history_add(&h, m);

    if (r >= 0)
        fail_int("Blocked message accepted", -1, r);
    else {
        int ids[] = {};
        int chk = check_history(&h, 0, ids);
        if (chk == 0) pass();
        else fail_int("Blocked message corrupted history", 0, chk);
    }

    history_reset(&h);
}

void test_add_censorship()
{
    begin("Censorship replaces cheating");

    History h;
    history_init(&h);

    history_add(&h, M(1, "I am cheating here"));

    if (strcmp(h.array[0]->text, "I am ******** here") == 0)
        pass();
    else
        fail_str("Censorship failed",
                 "I am ******** here",
                 h.array[0]->text);

    history_reset(&h);
}

/* ---------- GET ---------- */

void test_get()
{
    begin("Get by index");

    History h;
    history_init(&h);
    fill(&h, 4);

    int ok = 1;
    for (int i = 0; i < 4; i++)
        if (!history_get(&h, i) || history_get(&h, i)->id != i)
            ok = 0;

    if (ok) pass();
    else fail_int("Wrong element", 1, 0);

    history_reset(&h);
}

/* ---------- DELETE ---------- */

void test_delete_middle()
{
    begin("Delete middle");

    History h;
    history_init(&h);
    fill(&h, 6);

    history_delete_kth_last(&h, 3);

    int ids[] = {0,1,2,4,5};
    int r = check_history(&h, 5, ids);

    if (r == 0) pass();
    else fail_int("Middle delete wrong", 0, r);

    history_reset(&h);
}

void test_delete_first()
{
    begin("Delete oldest");

    History h;
    history_init(&h);
    fill(&h, 4);

    history_delete_kth_last(&h, 4);

    int ids[] = {1,2,3};
    int r = check_history(&h, 3, ids);

    if (r == 0) pass();
    else fail_int("First delete wrong", 0, r);

    history_reset(&h);
}

void test_delete_last()
{
    begin("Delete newest");

    History h;
    history_init(&h);
    fill(&h, 4);

    history_delete_kth_last(&h, 1);

    int ids[] = {0,1,2};
    int r = check_history(&h, 3, ids);

    if (r == 0) pass();
    else fail_int("Last delete wrong", 0, r);

    history_reset(&h);
}

/* ---------- SHIFT ---------- */

void test_shift()
{
    begin("Shift left");

    History h;
    history_init(&h);
    fill(&h, 5);

    history_shift_left(&h);

    int ids[] = {1,2,3,4};
    int r = check_history(&h, 4, ids);

    if (r == 0) pass();
    else fail_int("Shift failed", 0, r);

    history_reset(&h);
}

/* ---------- REWIND ---------- */

void test_rewind()
{
    begin("Rewind 3");

    History h;
    history_init(&h);
    fill(&h, 6);

    history_rewind(&h, 3);

    int ids[] = {0,1,2};
    int r = check_history(&h, 3, ids);

    if (r == 0) pass();
    else fail_int("Rewind wrong", 0, r);

    history_reset(&h);
}

void test_rewind_overflow()
{
    begin("Rewind overflow");

    History h;
    history_init(&h);
    fill(&h, 3);

    history_rewind(&h, 10);

    int ids[] = {};
    int r = check_history(&h, 0, ids);

    if (r == 0) pass();
    else fail_int("Overflow rewind wrong", 0, r);

    history_reset(&h);
}

/* ---------- PURGE ---------- */

void test_purge()
{
    begin("Purge word");

    History h;
    history_init(&h);

    history_add(&h, M(1,"hi"));
    history_add(&h, M(2,"bye"));
    history_add(&h, M(3,"hi again"));
    history_add(&h, M(4,"ok"));
    history_add(&h, M(5,"hi"));

    history_purge_word(&h, "hi");

    int ids[] = {2,4};
    int r = check_history(&h, 2, ids);

    if (r == 0) pass();
    else fail_int("Purge wrong", 0, r);

    history_reset(&h);
}

/* ---------- VALIDATION ---------- */

void test_validate_detects_corruption()
{
    begin("history_validate detects corruption");

    History h;
    history_init(&h);
    fill(&h, 3);

    h.head->next = NULL;  // corrupt list

    if (history_validate(&h) != 0)
        pass();
    else
        fail_int("Validate missed corruption", 1, 0);

    history_reset(&h);
}

/* ---------- STRESS ---------- */

void test_stress()
{
    begin("Stress + blocked messages");

    History h;
    history_init(&h);

    for (int i = 0; i < 2000; i++)
        history_add(&h, M(i, "x"));

    for (int i = 0; i < 200; i++)
        history_add(&h, M(10000+i, "this is plagiarism"));

    for (int i = 0; i < 500; i++)
        history_shift_left(&h);

    for (int i = 0; i < 300; i++)
        history_delete_kth_last(&h, 1);

    if (history_validate(&h) == 0)
        pass();
    else
        fail_int("Corruption detected", 0, 1);

    history_reset(&h);
}

/* ============================================
   MAIN
   ============================================ */

int main(void)
{
    printf("\n=================================\n");
    printf("   CS200 HISTORY TEST HARNESS\n");
    printf("=================================\n\n");

    test_init();

    test_add_basic();
    test_add_resize();
    test_add_blocked();
    test_add_censorship();

    test_get();

    test_delete_middle();
    test_delete_first();
    test_delete_last();

    test_shift();

    test_rewind();
    test_rewind_overflow();

   test_validate_detects_corruption();
    test_purge();


    test_stress();

    printf("---------------------------------\n");
    printf("Passed: %d / %d\n", TESTS_PASS, TESTS_RUN);
    printf("---------------------------------\n\n");

    return 0;
}