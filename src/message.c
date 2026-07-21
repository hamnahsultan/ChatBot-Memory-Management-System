/*
 * ================================
 * CS200 Chatbot Memory Manager
 * Message Layer Skeleton
 * ================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"
#include <ctype.h>


Message *msg_create(int id, const char *raw){


    if(raw == NULL) 
    {
        return NULL;
    }

    Message* msg = malloc(sizeof(Message));
    if(msg == NULL) 
    {
        return NULL;
    }

    msg ->text = malloc(strlen(raw) + 1);
    
    if((msg ->text) == NULL)
    {
        free(msg);
        return NULL;
    }

    strcpy(msg ->text,raw);

    msg ->id = id;

    return msg;
}


void msg_destroy(Message *msg){


    if(msg == NULL)
    {
        return;
    }

    free(msg ->text);
    free(msg);
}


void msg_print(const Message *msg){


    if(msg == NULL)
    {
        return;
    }

    printf("ID = %d\n %s\n", msg ->id, msg ->text);
}


int msg_contains(const Message *msg, const char *word){


    if(word == NULL || msg == NULL)
    {
        return -1;
    }

    char* lowertext = strdup(msg ->text);
    char* lowerword = strdup(word);

    for(int i = 0; i < (int)strlen(lowertext); i++)
    {
        lowertext[i] = tolower(lowertext[i]);
    }

    for(int i = 0; i < (int)strlen(lowerword); i++)
    {
        lowerword[i] = tolower(lowerword[i]);
    }

    char* found = strstr(lowertext,lowerword);

    free(lowertext);
    free(lowerword);

    return found != NULL;
}


int msg_length(const Message *msg){


    if(msg == NULL || (msg ->text) == NULL)
    {
        return -1;
    }

    int length = 0;
    for (int i = 0; msg ->text[i] != '\0'; i++)
    {
        if(msg ->text[i] == ' ')
        {
            continue;
        }

        length++;
    }

    return length;
}


Message *msg_clone(const Message *src, int new_id){


    if(src == NULL || (src ->text) == NULL)
    {
        return NULL;
    }

    Message* clonedmsg = malloc(sizeof(Message));
    if(clonedmsg == NULL)
    {
        return NULL;
    }

    int length = strlen(src ->text);
    clonedmsg ->text = malloc(length + 1);
    
    if((clonedmsg ->text) == NULL)
    {
        free(clonedmsg);
        return NULL;
    }

    strcpy(clonedmsg ->text,src ->text);

    clonedmsg ->id = new_id;

    return clonedmsg;
}
