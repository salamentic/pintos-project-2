#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "threads/synch.h"

struct waitelem
{
  int pid;
  struct semaphore  wait;
  struct list_elem wait_elem;
  int ex;
};

int exec;

tid_t process_execute (char *file_name);
int process_wait (tid_t);
void process_exit (void);
struct semaphore * get_execsema();
void process_activate (void);
int exec_status;
struct list waitlist;

#endif /* userprog/process.h */
