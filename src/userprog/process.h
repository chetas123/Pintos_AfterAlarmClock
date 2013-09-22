#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);

struct stack_elem
{
  void *value;
  struct stack_elem *next;
};

struct stack
{
  struct stack_elem *top;
};

struct stack *init_stack (void);
void push (struct stack *sp, void *elem);
void *pop (struct stack *sp);

#endif /* userprog/process.h */
