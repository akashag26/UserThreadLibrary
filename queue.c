//
//  queue.c
//  MyThread
//
//  Created by Akash  Agrawal on 1/7/16.
//  Copyright © 2016 Akash  Agrawal. All rights reserved.
//
//#include<stdio.h>
#include<stdlib.h>
#include "queue.h"
#define handle_error(msg) \
do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef int bool;
#define TRUE  1
#define FALSE 0


struct Node *Create_New_Node(struct Thread *thread)
{
    struct Node *ptr = (struct Node *)malloc(sizeof(struct Node));
    if(ptr==NULL)
        	handle_error("Could not Allocate Memory");

    ptr->thread = thread;
    ptr->next = NULL;
    return ptr;
}

void enQueue(struct Queue *q, struct Thread *thread) {
    struct Node *newptr = Create_New_Node(thread);
    
    if(q->front==NULL)
    {
        q->front=newptr;
        q->rear=newptr;
    }
    else {
        q->rear->next = newptr;
        q->rear = newptr;
    }	
}

int isPresent(struct Queue *q, struct Thread *thread) {
    struct Node *p = q->front;
    while(p != NULL) {
        if(p->thread == thread)
            return 1;
        p = p-> next;
    }	
    return 0;
}


struct Thread *deQueue(struct Queue *q) {
    if(q->front==NULL)
        return NULL;
    
    struct Node *ptr = q->front;
    struct Thread *thread = q->front->thread;
    
    if(q->front==q->rear){
        free(ptr);
        q->front= NULL;
        q->rear =NULL;
        return thread;
    }
    
        q->front = q->front->next;
        free(ptr);
        return thread;
}

void Display_Queue(struct Queue *q){
    struct Node *ptr = q->front;
    while(ptr!=NULL){
        printf("\n%p", ptr->thread);
        ptr = ptr->next;
    }
}

bool deleteFromQueue(struct Queue *q, struct Thread *thread){
    
    bool isSuccess = FALSE;
    if(q->front==NULL)
        return isSuccess;
    
    struct Node *temp = q->front;
    struct Node *beforeDelete = q->front;
    
    while(temp != NULL) {
        if(temp->thread == thread) {
            //If the thread is the only thread present in the queue.
            if(temp == q->front && q->front == q->rear)
            {
                q->front=NULL;
                q->rear=NULL;
            }
            else if(temp == q->front)               //If the thread is present at the head
            {
                q->front = temp->next;
            }
            else if(temp == q->rear)                //If the thread is present at the tail
            {
		q->rear = beforeDelete;
                beforeDelete->next = NULL;
            }
            else
            {
                    beforeDelete->next = temp->next;           // If the thread is present in between
            }
            
         free(temp);
            isSuccess = TRUE;
            return isSuccess;
        }	
        beforeDelete = temp;
        temp = temp-> next;
    }	
    
    return isSuccess;
}
