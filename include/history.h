/*
 * ================================
 * CS200 Chatbot Memory Manager
 * Skeleton File
 * ================================
 */


#ifndef HISTORY_H
#define HISTORY_H

#include "message.h"

typedef struct Node {
    Message *msg;
    struct Node *next;
} Node;

typedef struct {
    Message **array;
    int count;
    int capacity;
    Node *head;
    Node *tail;
} History;

void history_init(History *h);
int history_add(History *h, Message *msg);
Message *history_get(History *h, int index);

int history_delete_kth_last(History *h, int k);
int history_purge_word(History *h, const char *word);

void history_shift_left(History *h);
void history_rewind(History *h, int k);
void history_reset(History *h);

int history_validate(History *h);
int history_compact(History *h);

#endif
