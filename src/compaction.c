/*
 * ================================
 * CS200 Chatbot Memory Manager
 * Compaction Subsystem Skeleton
 * ================================
 */

#include "compaction.h"
#include "message.h"
#include <stdlib.h>


Message **compaction_build_array(History *h){

    if(h == NULL)
    {
        return NULL;
    }

    if(h ->count == 0)
    {
        return NULL;
    }

    Message** arr1 = malloc(h ->count * sizeof(Message*));
    if(arr1 == NULL)
    {
        return NULL;
    }

    for(int i = 0; i < h ->count; i++)
    {
        arr1[i] = h ->array[i];
    }

    return arr1;
}


Node *compaction_build_list(Message **arr, int count){


    if(count == 0)
    {
        return NULL;
    }

    Node* head = NULL;
    Node* tail = NULL;

    for(int i = 0; i < count; i++)
    {
        Node* node = malloc(sizeof(Node));
        if(node == NULL)
        {
            Node* c = head;
            while(c)
            {
                Node* temp = c;
                c = c ->next;
                free(temp);
            }
            return NULL;
        }

        node ->msg = arr[i];
        node ->next = NULL;

        if(head == NULL)
        {
            head = node;
            tail = node;
        }
        else
        {
            tail ->next = node;
            tail = node;
        }
    }

    return head;
}


void compaction_free_old_storage(History *h){


    if(h == NULL)
    {
        return;
    }

    Node* c = h ->head;
    while(c)
    {
        Node* temp = c;
        c = c ->next;
        free(temp);
    }

    free(h ->array);
}


int compaction_replace_storage(History *h,
                               Message **arr,
                               Node *list){

    if(h == NULL)
    {
        return -1;
    }

    h ->array = arr;
    h ->head = list;

    if(list == NULL)
    {
        h ->tail = NULL;
    }
    else
    {
        Node* c = list;
        while(c ->next)
        {
            c = c ->next;
        }

        h ->tail = c;
    }
    
    h ->capacity = h ->count;

    return 0;
}


int compaction_run(History *h){


    if(h == NULL)
    {
        return -1;
    }

    if(h ->count == 0)
    {
        compaction_free_old_storage(h);

        h ->array = NULL;
        h ->head = NULL;
        h ->tail = NULL;
        h ->capacity = 0;

        return 0;
    }

    Message **arr = compaction_build_array(h);
    if (arr == NULL)
    {
        return -1;
    }

    Node *list = compaction_build_list(arr, h->count);
    if (list == NULL) 
    {
        free(arr);
        return -1;
    }

    compaction_free_old_storage(h);

    if (compaction_replace_storage(h, arr, list) < 0)
    {
        free(arr);
        return -1;
    }

    return 0;
}
