//
//  queue.h
//  MyThread
//
//  Created by Akash  Agrawal on 1/7/16.
//  Copyright © 2016 Akash  Agrawal. All rights reserved.
//

#define _XOPEN_SOURCE 600
#ifndef queue_h
#define queue_h

#include <stdio.h>
#include <ucontext.h>

struct Thread {
    ucontext_t ucontext_ptr;
    struct Thread *parent;
    struct Thread *waitForChild;
    struct Queue *child;
};

struct Node {
    struct Thread *thread;
    struct Node *next;
};

struct  Queue{
    struct Node *front;
    struct Node *rear;
};

struct Semaphore{
    int val;
    struct Queue *waitQueue;
};

struct Node *Create_New_Node(struct Thread *thread);
void enQueue(struct Queue *q, struct Thread *thread);
struct Thread *deQueue(struct Queue *q);
void Display_Queue(struct Queue *q);
int deleteFromQueue(struct Queue *q, struct Thread *thread);
int isPresent(struct Queue *q, struct Thread *thread);
#endif /* queue_h */
