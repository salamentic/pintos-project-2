#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
struct semaphore * get_execsema();
void process_activate (void);
int exec_status;

#endif /* userprog/process.h */
