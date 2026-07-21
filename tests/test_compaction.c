#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "history.h"
#include "message.h"
#include "compaction.h"


/* ============================================
   COLORS
   ============================================ */

#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define RESET   "\033[0m"


/* ============================================
   GLOBALS
   ============================================ */

static int RUN = 0;
static int PASS = 0;


/* ============================================
   FRAMEWORK
   ============================================ */

void begin(const char *name)
{
    printf(BLUE "[%02d] %s\n" RESET, ++RUN, name);
}


void pass(void)
{
    printf(GREEN "   [PASS]\n\n" RESET);
    PASS++;
}


void fail(const char *msg, int exp, int got)
{
    printf(RED "   [FAIL]\n" RESET);

    printf("   %s\n", msg);

    printf(YELLOW "   Expected: %d\n" RESET, exp);
    printf(YELLOW "   Given:    %d\n\n" RESET, got);
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

        snprintf(buf,sizeof(buf),"m%d",i);

        history_add(h, M(i, buf));
    }
}


/* ============================================
   INVARIANT
   ============================================ */

int check_history(History *h,
                  int n,
                  const int *ids)
{
    if (h->count != n) return -1;

    for (int i = 0; i < n; i++) {

        if (!h->array[i]) return -2;

        if (h->array[i]->id != ids[i])
            return -3;
    }

    Node *cur = h->head;

    for (int i = 0; i < n; i++) {

        if (!cur) return -4;

        if (!cur->msg) return -5;

        if (cur->msg->id != ids[i])
            return -6;

        cur = cur->next;
    }

    if (cur) return -7;

    if (n == 0 && h->tail)
        return -8;

    if (n > 0 &&
        h->tail->msg->id != ids[n-1])
        return -9;

    if (history_validate(h) != 0)
        return -10;

    return 0;
}


/* ============================================
   TESTS
   ============================================ */

void test_basic()
{
    begin("Basic compaction");

    History h;
    history_init(&h);

    fill(&h,5);

    int r = compaction_run(&h);

    int ids[]={0,1,2,3,4};

    int c = check_history(&h,5,ids);

    if (r==0 && c==0)
        pass();
    else
        fail("Basic failed",0,r);

    history_reset(&h);
}


void test_capacity_shrink()
{
    begin("Capacity shrink");

    History h;
    history_init(&h);

    fill(&h,20);

    int old = h.capacity;

    compaction_run(&h);

    if (h.capacity == h.count &&
        h.capacity < old)
        pass();
    else
        fail("Capacity not shrunk",
             h.count,h.capacity);

    history_reset(&h);
}


void test_pointer_change()
{
    begin("Old pointers invalidated");

    History h;
    history_init(&h);

    fill(&h,6);

    Node *old_head = h.head;
    Message **old_arr = h.array;

    compaction_run(&h);

    if (h.head != old_head &&
        h.array != old_arr)
        pass();
    else
        fail("Reused old storage",1,0);

    history_reset(&h);
}


void test_repeat()
{
    begin("Repeat compaction");

    History h;
    history_init(&h);

    fill(&h,8);

    compaction_run(&h);
    compaction_run(&h);
    compaction_run(&h);

    int ids[]={0,1,2,3,4,5,6,7};

    int c=check_history(&h,8,ids);

    if(c==0) pass();
    else fail("Repeat corrupt",0,c);

    history_reset(&h);
}


void test_after_delete()
{
    begin("Compact after delete");

    History h;
    history_init(&h);

    fill(&h,7);

    history_delete_kth_last(&h,3);

    compaction_run(&h);

    int ids[]={0,1,2,3,5,6};

    int c=check_history(&h,6,ids);

    if(c==0) pass();
    else fail("Delete+compact bad",0,c);

    history_reset(&h);
}


void test_after_purge()
{
    begin("Compact after purge");

    History h;
    history_init(&h);

    history_add(&h,M(1,"hi"));
    history_add(&h,M(2,"bye"));
    history_add(&h,M(3,"hi again"));

    history_purge_word(&h,"hi");

    compaction_run(&h);

    int ids[]={2};

    int c=check_history(&h,1,ids);

    if(c==0) pass();
    else fail("Purge+compact bad",0,c);

    history_reset(&h);
}


void test_after_shift()
{
    begin("Compact after shift");

    History h;
    history_init(&h);

    fill(&h,5);

    history_shift_left(&h);

    compaction_run(&h);

    int ids[]={1,2,3,4};

    int c=check_history(&h,4,ids);

    if(c==0) pass();
    else fail("Shift+compact bad",0,c);

    history_reset(&h);
}


void test_after_rewind()
{
    begin("Compact after rewind");

    History h;
    history_init(&h);

    fill(&h,6);

    history_rewind(&h,4);

    compaction_run(&h);

    int ids[]={0,1};

    int c=check_history(&h,2,ids);

    if(c==0) pass();
    else fail("Rewind+compact bad",0,c);

    history_reset(&h);
}


void test_stress()
{
    begin("Stress compaction");

    History h;
    history_init(&h);

    for(int i=0;i<2000;i++)
        history_add(&h,M(i,"x"));

    for(int i=0;i<20;i++)
        compaction_run(&h);

    if(history_validate(&h)==0)
        pass();
    else
        fail("Stress corrupt",0,1);

    history_reset(&h);
}


/* ============================================
   MAIN
   ============================================ */

int main(void)
{
    printf("\n=================================\n");
    printf("   CS200 COMPACTION TEST HARNESS\n");
    printf("=================================\n\n");

    test_basic();
    test_capacity_shrink();
    test_pointer_change();
    test_repeat();
    test_after_delete();
    test_after_purge();
    test_after_shift();
    test_after_rewind();
    test_stress();

    printf("---------------------------------\n");
    printf("Passed: %d / %d\n", PASS, RUN);
    printf("---------------------------------\n\n");

    return 0;
}
