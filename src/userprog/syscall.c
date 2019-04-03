#include "userprog/syscall.h"
#include <stdio.h>
#include <list.h>
#include <stdbool.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/interrupt.h"
#include "threads/init.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);
struct file_desc {
    struct file * fileloc;
    int fd;
    struct list_elem threadelem;
};

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
    thread_exit();
    process_exit();
    return;
  }

  if(i==0 && *((int *) f->esp) == SYS_HALT)
  {
    shutdown_power_off();
    return;
  }

  if(i==0 && *((int *) f->esp) == SYS_OPEN)
  {
    void * stack_value = safe_acc(*(char **) arg1);
    if(stack_value == NULL)
    return; 

    struct file * temp = filesys_open(*(char **) arg1);
    if(temp == NULL)
    {
      f->eax = -1;
      return;
    }

    struct file_desc ins;
    ins.fd = thread_current()->max_fd;
    thread_current()->max_fd++;
    ins.fileloc = temp;
    list_push_back(&thread_current()->files, &ins.threadelem);
    f->eax = thread_current()->max_fd - 1;
    return;
  }

  if(i==0 && *((int *) f->esp) == SYS_REMOVE)
  {
    void * stack_value = safe_acc(arg1);
    bool temp = filesys_remove(*(char **) arg1);
    if(!temp)
    {
      f->eax = false;
      return;
    }
    f->eax = true;
    return;
  }

  if(i==0 && *((int *) f->esp) == SYS_CREATE)
  {
    void * stack_value = safe_acc(arg1);
    if(stack_value == NULL)
      return; 
    bool temp = filesys_create(*(char **) arg1, * (unsigned *) arg2);
    if(!temp)
    {
      f->eax = false;
      return;
    }
    f->eax = true;
    return;
  }
  
  if(i==0 && *((int *) f->esp) == SYS_FILESIZE)
  {
      for(struct list_elem * index = list_begin(&thread_current()->files);
	  (index) != list_end(&thread_current()->files);
	  index = list_next(index))
      {
	 if(list_entry(index, struct file_desc, threadelem)->fd == *(int *) arg1)
           {
             f->eax = file_length(list_entry(index, struct file_desc, threadelem)->fileloc);
           }
      }
    return;
  }

  if(i==0 && *((int *) f->esp) == SYS_REMOVE)
  {
    bool temp = filesys_create(*(char **) arg1, * (unsigned *) arg2);
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
    else
    {
      for(struct list_elem * index = list_begin(&thread_current()->files);
	  (index) != list_end(&thread_current()->files);
	  index = list_next(index))
      {
	 if(list_entry(index, struct file_desc, threadelem)->fd == *(int *) arg1)
           {
             f->eax = file_write(list_entry(index, struct file_desc, threadelem)->fileloc, *(char **) arg2, *(int *) arg3);
           }
      }
      return;
    }
  }

  if(*((int *) f->esp) == SYS_READ)
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
    else
    {
      for(struct list_elem * index = list_begin(&thread_current()->files);
	  (index) != list_end(&thread_current()->files);
	  index = list_next(index))
      {
	 if(list_entry(index, struct file_desc, threadelem)->fd == *(int *) arg1)
           {
             f->eax = file_read(list_entry(index, struct file_desc, threadelem)->fileloc, *(char **) arg2, *(unsigned *) arg3);
           }
      }
      return;
    }
  }
}


void * 
safe_acc(const void * f)
{
  if(f==NULL)
  {
    process_exit();
    thread_exit();
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

