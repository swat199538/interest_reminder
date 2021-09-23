//
// Created by swat on 9/23/2021.
//

#ifndef INTEREST_REMINDER_ADLIST_H
#define INTEREST_REMINDER_ADLIST_H

typedef struct listNode{
    struct listNode *prev;
    struct listNode *next;
    void *data;
} listNode;

typedef struct listIter{
    listNode *next;
    int direction;
} listIter;

typedef struct list{
    listNode *head;
    listNode *tail;
    void *(*dup)(void *ptr);
} list;

#endif //INTEREST_REMINDER_ADLIST_H
