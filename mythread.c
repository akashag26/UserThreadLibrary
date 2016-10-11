//
//  mythread.c
//  MyThread
//
//  Created by Akash  Agrawal on 1/7/16.
//  Copyright © 2016 Akash  Agrawal. All rights reserved.
//
#include "mythreadextra.h"
#include "mythread.h"
#include "queue.h"
#include "stdlib.h"
#define STACK_SIZE 8192

#define handle_error(msg) \
do { perror(msg); exit(EXIT_FAILURE); } while (0)

struct Queue *readyQueue, *blockedQueue;
struct Thread *runningThread, *mainThread;
ucontext_t contextControl;
ucontext_t exitContext;
int extra=0;

// ****** THREAD OPERATIONS ******
// Create a new thread.

struct Thread *newThread(void (*start_funct)(void *), void *args){
    //Initialise a new thread
	if(exitContext.uc_stack.ss_sp==NULL){
		ucontext_t temp;
	
		if(getcontext(&temp) == -1)
        		handle_error("Could not get context");
		temp.uc_stack.ss_sp = malloc(8192);
	
		if(temp.uc_stack.ss_sp==NULL)
			handle_error("Could not Allocate Memory");
	
		temp.uc_stack.ss_size = STACK_SIZE;    
		temp.uc_link = NULL;
		makecontext(&temp, MyThreadExit,0 );
    		exitContext= temp;
    }
    struct Thread *thread = (struct Thread *)malloc(sizeof(struct Thread));
	if(thread==NULL)
	handle_error("Could not Allocate Memory"); 
   	thread->parent = NULL;
    	thread->waitForChild = NULL;
    	thread->child = (struct Queue *)malloc(sizeof(struct Queue));
	
	if(thread->child==NULL)
        	handle_error("Could not Allocate Memory");
    	thread->child->front=NULL;
    	thread->child->rear = NULL;
    
   	if(getcontext(&thread->ucontext_ptr) == -1)
        	handle_error("Could not get context");
    
    	void *stack = (void *)malloc(STACK_SIZE);
	if(stack==NULL)
        	handle_error("Could not Allocate Memory");
  
  	(thread->ucontext_ptr.uc_stack).ss_sp = stack;
    	(thread->ucontext_ptr.uc_stack).ss_size = STACK_SIZE;
    
   	thread->ucontext_ptr.uc_link = &exitContext;
   	makecontext(&thread->ucontext_ptr, (void (*)()) start_funct, 1, args);
	return thread;
}


void *MyThreadCreate(void(*start_funct)(void *), void *args){
    
    //Initialise a new thread
    struct Thread *thread = newThread(start_funct, args);
    enQueue(runningThread->child, thread);
    thread->parent = runningThread;
    enQueue(readyQueue, thread); 
    return (void *)thread;
}

struct Thread *nextThread(){
    struct Thread *next = deQueue(readyQueue);
    if(next == NULL)                   //Check if Ready Queue is Empty.
  	  {
        if(setcontext(&contextControl)==-1)
		handle_error("Could not set context");
        return NULL;
   	 }
    	else
    	{
        return next;
    	}
}


// Yield invoking thread
void MyThreadYield(void){
 struct Thread *currentThread = runningThread;
 enQueue(readyQueue, runningThread);
 
 if((runningThread = nextThread())!=NULL){
   if(swapcontext(&(currentThread->ucontext_ptr), &(runningThread->ucontext_ptr))==-1)
	handle_error("Could not swap context");
   }
}

// Join with a child thread
int MyThreadJoin(void *thread){

	if(thread==NULL)
		return -1;
	

	struct Thread *child = (struct Thread *) thread;
if(isPresent(runningThread->child, child)!=1)
	//if(runningThread != child -> parent)       //If the thread is not the immediate child then return -1.
 		  {
	  	  return -1;
   		  }
  
    	if(isPresent(runningThread->child, child)==1)
    		{
        	runningThread->waitForChild = child;
        	enQueue(blockedQueue, runningThread);
        	if((runningThread = nextThread())!=NULL)
  if(swapcontext(&(child->parent->ucontext_ptr), &(runningThread->ucontext_ptr))==-1)
			handle_error("Could not swap context");
    		}

        return 0;
}


// Join with all children
void MyThreadJoinAll(void){	
	if(runningThread->child->front!=NULL){
 	        struct Thread *temp = runningThread;
        	enQueue(blockedQueue, temp);
    		if((runningThread = nextThread())!=NULL)
		{    
if(swapcontext(&(temp->ucontext_ptr), &(runningThread->ucontext_ptr))==-1)
			handle_error("Could not swap context");
		}
	}
}

// Terminate invoking thread
void MyThreadExit(void){
	int i=1;	
 	struct Thread *temp = runningThread;
  	struct Thread *parent = temp->parent;

    
	if(isPresent(blockedQueue,parent)==1)
	{
		deleteFromQueue(parent->child, temp);
      if((parent->child->front==NULL) || (parent->waitForChild == temp)) {
            		deleteFromQueue(blockedQueue, parent);
            		parent->waitForChild = NULL;
			i=0;
            		enQueue(readyQueue, parent);
        }
	}    

	if(parent!=NULL)
	{
	 if(parent->child!=NULL && (isPresent(parent->child,temp)==1))
        {	
	       deleteFromQueue(parent->child, temp);
        }	
	}


	free(temp->child);
    	free((temp->ucontext_ptr).uc_stack.ss_sp);
    	temp->parent = NULL;
  	temp->waitForChild=NULL;
    	temp->child = NULL;
    	free(temp);
    	temp = NULL;

    	runningThread = deQueue(readyQueue);
    	if(runningThread != NULL)
        	{
		if(setcontext(&runningThread->ucontext_ptr)==-1)
			handle_error("Could not set context");
		}
    	else if( extra ==1)
		;
	else
		{
        	if(setcontext(&contextControl)==-1)
			handle_error("Could not set context");
		}
}

// ****** SEMAPHORE OPERATIONS ******
// Create a semaphore
void *MySemaphoreInit(int initialValue)
{
    
    	if(initialValue<0)
        	return NULL;
    
 struct Semaphore *sem = (struct Semaphore *) malloc(sizeof(struct Semaphore));
 	if(sem==NULL)
        	handle_error("Could not Allocate Memory");
   
    	sem->val = initialValue;
    	sem->waitQueue = (struct Queue *)malloc(sizeof(struct Queue));
	if(sem->waitQueue==NULL)
        handle_error("Could not Allocate Memory");
 

   	sem->waitQueue->front =NULL;
    	sem->waitQueue->rear =NULL;
    	return (void *)sem;
}

// Signal a semaphore
void MySemaphoreSignal(MySemaphore sem){
 	if(sem==NULL)
        	return;
   	struct Semaphore *semaphore = sem;
    	semaphore->val+=1;
    	if(semaphore->val <= 0) {
        	struct Thread *thread = deQueue(semaphore->waitQueue);
        	enQueue(readyQueue, thread);
    	}
    
}


// Wait on a semaphore
void MySemaphoreWait(MySemaphore sem){
 	if(sem==NULL)
        	return;

   	struct Semaphore *semaphore = sem;
    	semaphore->val-=1;
    	if(semaphore->val < 0) {
        	struct Thread *currentThread = runningThread;
        	enQueue(semaphore->waitQueue, runningThread);
        	if((runningThread = nextThread())!=NULL){
 if(swapcontext(&(currentThread->ucontext_ptr), &(runningThread->ucontext_ptr))==-1)
		handle_error("Could not swap context");
    	}	
    }
}


// Destroy on a semaphore
int MySemaphoreDestroy(MySemaphore sem){
	if(sem==NULL)
        	return 0;

	struct Semaphore *semaphore = sem;
    	if(semaphore->val >= 0) {
        	free(semaphore->waitQueue);
        	free(semaphore);
        	return 0;
    	}	
    	return -1;
}

// ****** CALLS ONLY FOR UNIX PROCESS ******
// Create and run the "main" thread
void MyThreadInit(void(*start_funct)(void *), void *args){
    	readyQueue = (struct Queue *)malloc(sizeof(struct Queue));
	if(readyQueue==NULL)
        	handle_error("Could not Allocate Memory");
 
   	readyQueue->front = NULL;
    	readyQueue->rear = NULL;
    	blockedQueue = (struct Queue *)malloc(sizeof(struct Queue));
	if(blockedQueue==NULL)
        	handle_error("Could not Allocate Memory");
   
 	blockedQueue->front = NULL;
    	blockedQueue->rear = NULL;
    
    	struct Thread *thread = newThread(start_funct, args);
	runningThread = thread;
    	if(swapcontext(&contextControl, &(runningThread->ucontext_ptr))==-1)
		handle_error("Could not swap context");
    	return;
}


int MyThreadInitExtra(){
	extra = 1;
	readyQueue = (struct Queue *)malloc(sizeof(struct Queue));
	if(readyQueue==NULL)
        	handle_error("Could not Allocate Memory");

   	readyQueue->front = NULL;
    	readyQueue->rear = NULL;
    	blockedQueue = (struct Queue *)malloc(sizeof(struct Queue));
        if(blockedQueue==NULL)
        	handle_error("Could not Allocate Memory");

 	blockedQueue->front = NULL;
    	blockedQueue->rear = NULL;

        if(getcontext(&exitContext) == -1)
        	handle_error("Could not get context");
        exitContext.uc_stack.ss_sp = malloc(8192);

        if(exitContext.uc_stack.ss_sp==NULL)
        	handle_error("Could not Allocate Memory");

        exitContext.uc_stack.ss_size = STACK_SIZE;
        exitContext.uc_link = NULL;
        makecontext(&exitContext, MyThreadExit,0 );

	struct Thread *thread = (struct Thread *)malloc(sizeof(struct Thread));
        if(thread==NULL)
        	handle_error("Could not Allocate Memory");
   	thread->parent = NULL;
    	thread->waitForChild = NULL;
    	thread->child = (struct Queue *)malloc(sizeof(struct Queue));
	if(thread->child==NULL)
        	handle_error("Could not Allocate Memory");
    	thread->child->front=NULL;
    	thread->child->rear = NULL;

    	if(getcontext(&thread->ucontext_ptr) == -1)
        	handle_error("Could not get context");

 	runningThread = thread;
	return 0;
}
