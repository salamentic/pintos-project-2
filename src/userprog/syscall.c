#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/interrupt.h"
#include "threads/init.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"

static void syscall_handler (struct intr_frame *);

void * 
safe_acc(const void * f);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

/*static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
}*/

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  void * stack_pos = f->esp;
  int i=0;
  int args = 1;
  int arg1 = 1;
  void *  arg2;
  void *  arg3;
  arg1 = (f->esp) + 4;
  arg2 = (f->esp) + 8;
  arg3 = (f->esp) + 12;

  if(i==0 && *((int *) f->esp) == SYS_EXIT)
  {
    process_exit();
    thread_exit();
    return;
  }

  if(i==0 && *((int *) f->esp) == SYS_HALT)
  {
    shutdown_power_off();
    return;
  }

  if(i==0 && *((int *) f->esp) == SYS_OPEN)
  {
    struct file * temp = file_reopen(*(struct file **) arg1);
    if(temp == NULL)
    {
      f->eax = -1;
      return;
    }
    temp->fd = thread_current()->max_fd;
    thread_current()->max_fd++;
    list_push_back(&thread_current()->files, &temp->threadelem);
    f->eax = max_fd - 1;
    return;
  }

  if(i==0 && *((int *) f->esp) == SYS_CREATE)
  {
    struct file * temp = filesys_create(*(char **) arg1, * (unsigned *) arg2);
    if(temp == NULL)
    {
      f->eax = false;
      return;
    }
    f->eax = true;
    return;
  }
  
  if(i==0 && *((int *) f->esp) == SYS_REMOVE)
  {
    struct file * temp = filesys_create(*(char **) arg1, * (unsigned *) arg2);
    if(temp == NULL)
    {
      f->eax = false;
      return;
    }
    f->eax = true;
    return;
  }
  if(*((int *) f->esp) == SYS_WRITE)
  {
    if(*(int *) arg1 == 1)
    {
      void * stack_value = safe_acc(arg2);
      if(stack_value == NULL)
      return; 
      putbuf((*(char **) arg2), *(int *) arg3);
      f->eax = *(int *) arg3;
      return;
    }
    else return;
  }
}


void * 
safe_acc(const void * f)
{
  if(f==NULL)
  {
    process_exit();
    return NULL;
  }
  if(is_user_vaddr(f))
  {
    void * ret_ptr = pagedir_get_page(thread_current()->pagedir, f);
    if(ret_ptr != NULL)
      return ret_ptr;
    process_exit();
    thread_exit();
    return NULL;
  }
}

