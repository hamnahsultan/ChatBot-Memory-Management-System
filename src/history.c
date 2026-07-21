/*
 * ================================
 * CS200 Chatbot Memory Manager
 * History Layer Skeleton
 * ================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "history.h"
#include "parser.h"
#include "compaction.h"

/*
 * history_init
 *
 * Initializes an empty history.
 */
void history_init(History *h)
{



    if (h == NULL)
    {
        return;
    }

    h->capacity = 4;
    h->count = 0;
    h->array = malloc(h->capacity * sizeof(Message *));

    if (h->array == NULL)
    {
        h->capacity = 0;
        return;
    }

    h->head = NULL;
    h->tail = NULL;
}

/*
 * history_add
 *
 * Appends a new message.
 */
int history_add(History *h, Message *msg)
{



    if (h == NULL || msg == NULL)
    {
        return -1;
    }

    if (parser_check_policy(msg->text) <= 0)
    {
        return -1;
    }
    else
    {
        parser_apply_policy(msg->text);
    }

    if (h->count == h->capacity)
    {
        int newCap = (h->capacity == 0) ? 4 : h->capacity * 2;

        Message **temp = realloc(h->array,
                                 newCap * sizeof(Message *));

        if (temp == NULL)
            return -1;

        h->array = temp;
        h->capacity = newCap;
    }

    h->array[h->count] = msg;

    Node *node = malloc(sizeof(Node));
    if (node == NULL)
    {
        return -1;
    }

    node->msg = msg;
    node->next = NULL;

    if (h->head == NULL)
    {
        h->tail = node;
        h->head = h->tail;
    }
    else
    {
        h->tail->next = node;
        h->tail = node;
    }

    h->count++;

    return 0;
}

/*
 * history_get
 *
 * Retrieves message by index.
 */
Message *history_get(History *h, int index)
{



    if (h == NULL)
    {
        return NULL;
    }

    if (index < 0 || index >= h->count)
    {
        return NULL;
    }

    return h->array[index];
}

/*
 * history_delete_kth_last
 *
 * Removes the kth newest message.
 */
int history_delete_kth_last(History *h, int k)
{



    if (k <= 0 || k > h->count)
    {
        return -1;
    }

    int index = h->count - k;
    Message *target = h->array[index];

    Node *previous = NULL;
    Node *c = h->head;
    while (c && c->msg != target)
    {
        previous = c;
        c = c->next;
    }

    if (c == NULL)
    {
        return -1;
    }

    if (previous != NULL)
    {
        previous->next = c->next;
    }
    else
    {
        h->head = c->next;
    }

    if (c == h->tail)
    {
        h->tail = previous;
    }

    free(c);

    for (int i = index; i < h->count - 1; i++)
    {
        h->array[i] = h->array[i + 1];
    }

    h->count--;
    h->array[h->count] = NULL;
    msg_destroy(target);

    return 0;
}

/*
 * history_purge_word
 *
 * Removes all matching messages.
 */
int history_purge_word(History *h, const char *word)
{



    if (h == NULL || word == NULL)
        return -1;

    int removed = 0;

    for (int i = h->count - 1; i >= 0; i--)
    {
        if (strstr(h->array[i]->text, word) != NULL)
        {
            int k = h->count - i;

            if (history_delete_kth_last(h, k) == 0)
                removed++;
        }
    }

    return removed;
}

/*
 * history_shift_left
 *
 * Removes oldest message.
 */
void history_shift_left(History *h)
{



    if (h == NULL || h->count == 0)
    {
        return;
    }

    Message *previous = h->array[0];

    Node *temp = h->head;
    h->head = temp->next;

    if (h->head == NULL)
    {
        h->tail = NULL;
    }

    free(temp);

    for (int i = 0; i < h->count - 1; i++)
    {
        h->array[i] = h->array[i + 1];
    }

    h->count--;
    h->array[h->count] = NULL;
    msg_destroy(previous);
}

/*
 * history_rewind
 *
 * Removes last k messages.
 */
void history_rewind(History *h, int k)
{



    if (h == NULL || k <= 0)
    {
        return;
    }

    if (k > h->count)
    {
        k = h->count;
    }

    while (k--)
    {
        history_delete_kth_last(h, 1);
    }
}

/*
 * history_reset
 *
 * Clears all stored messages.
 */
void history_reset(History *h)
{



    if (h == NULL)
        return;

    Node *c = h->head;
    while (c)
    {
        Node *next = c->next;
        msg_destroy(c->msg);
        free(c);
        c = next;
    }

    free(h->array);

    h->array = NULL;
    h->head = NULL;
    h->tail = NULL;
    h->count = 0;
    h->capacity = 0;
}

/*
 * history_validate
 *
 * Checks internal consistency.
 */
int history_validate(History *h)
{



    if (h == NULL)
    {
        return -1;
    }

    int counter = 0;
    Node *c = h->head;
    int i = 0;

    while (c)
    {
        if (i >= h->count)
        {
            return -1;
        }

        if (h->array[i] != c->msg)
        {
            return -1;
        }

        c = c->next;

        i++;
        counter++;
    }

    if (counter != h->count)
    {
        return -1;
    }

    return 0;
}

/*
 * history_compact
 *
 * Triggers full reorganization.
 */
int history_compact(History *h)
{



    if (h == NULL)
    {
        return -1;
    }

    return compaction_run(h);
}
