//
// Created by swat on 9/23/2021.
//

#include "adlist.h"
#include "zmalloc.h"

list *listCreate()
{
    list *l;
    if ( (l = zmalloc(sizeof(list))) == NULL)
        return NULL;
    l->head = l->tail = NULL;
    l->len = 0;
    l->free = NULL;
    l->dup = NULL;
    l->match = NULL;
    return l;
}

void listRelease(list *list)
{
    unsigned int len;
    listNode *current, *next;

    current = list->head;
    len = list->len;

    while (len--){
        next = current->next;
        if (list->free) list->free(current);
        zfree(current);
        current = next;
    }
    zfree(list);
}

list *listAddNodeHead(list *list, void *value)
{
    listNode *node;

    if ( (node = zmalloc(sizeof(listNode))) == NULL )
        return NULL;

    node->value = value;
    if (list->len==0){
        node->next = node->prev = NULL;
        list->head = list->tail = node;
    } else{
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->len++;
    return list;
}

list *listAddNodeTail(list *list, void *value)
{
    listNode *node;
    if ( (node = zmalloc(sizeof(listNode))) == NULL )
        return NULL;
    node->value = value;
    if (list->len == 0){
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else{
        node->next = NULL;
        list->tail->next = node;
        node->prev = list->tail;
        list->tail = node;
    }
    list->len++;
    return list;
}

void listDelNode(list *list, listNode *node)
{
    if (node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;

    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;

    if (list->free) list->free(node->value);
    zfree(node);
    list->len--;
}

listIter *listGetIterator(list *list, int direction)
{
    listIter *iter;
    if ( (iter = zmalloc(sizeof(listIter))) == NULL )
        return NULL;
    if (direction == AL_START_HEAD)
        iter->next = list->head;
    else
        iter->next = list->tail;
    iter->direction = direction;
    return iter;
}

void listReleaseIterator(listIter *iter) {
    zfree(iter);
}