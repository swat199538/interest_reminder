//
// Created by swat on 9/23/2021.
//

#ifndef INTEREST_REMINDER_ADLIST_H
#define INTEREST_REMINDER_ADLIST_H

#define AL_START_HEAD 0
#define AL_START_TAIL 1

typedef struct listNode{
    struct listNode *prev;
    struct listNode *next;
    void *value;
} listNode;

typedef struct listIter{
    listNode *next;
    int direction;
} listIter;

typedef struct list{
    listNode *head;
    listNode *tail;
    void *(*dup)(void *ptr);
    void (*free)(void *ptr);
    int (*match)(void *ptr, void *key);
    unsigned int len;
} list;

list *listCreate(void);
void listRelease(list *list);
list *listAddNodeHead(list *list, void *data);
list *listAddNodeTail(list *list, void *data);
void listDelNode(list *list, listNode *node);
listIter *listGetIterator(list *list, int direction);
void listReleaseIterator(listIter *iter);


#endif //INTEREST_REMINDER_ADLIST_H
