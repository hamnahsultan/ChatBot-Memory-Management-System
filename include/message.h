/*
 * ================================
 * CS200 Chatbot Memory Manager
 * Skeleton File
 * ================================
 */


#ifndef MESSAGE_H
#define MESSAGE_H

typedef struct {
    int id;
    char *text;
} Message;

Message *msg_create(int id, const char *raw);
void msg_destroy(Message *msg);
void msg_print(const Message *msg);
int msg_contains(const Message *msg, const char *word);
int msg_length(const Message *msg);
Message *msg_clone(const Message *src, int new_id);

#endif
