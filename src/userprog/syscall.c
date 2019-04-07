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
#include "threads/synch.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);
struct file_desc {
    struct file * fileloc;
    int fd;
    struct list_elem threadelem;
};
static struct semaphore filesys_sema;

void * 
safe_acc(const void * f);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  sema_init(&filesys_sema, 1);
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

  
  void * stack_value = safe_acc(f->esp);
  if(stack_value == NULL)
    return; 

  if(*((int *) f->esp) == SYS_EXIT)
  {
    void * stack_value = safe_acc(arg1);
    if(stack_value == NULL)
      return; 
    thread_current()->exit = *((int *) arg1);
    thread_exit();
    f->eax = *((int *) arg1);
    return;
  }

  if(*((int *) f->esp) == SYS_HALT)
  {
    shutdown_power_off();
    return;
  }

  if(*((int *) f->esp) == SYS_OPEN)
  {
    void * stack_value = safe_acc((char **) arg1);
    if(stack_value == NULL)
    return; 

    stack_value = safe_acc(*(char **) arg1);
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

  if(*((int *) f->esp) == SYS_REMOVE)
  {
    void * stack_value = safe_acc((char **) arg1);
    if(stack_value == NULL)
    return; 

    stack_value = safe_acc(*(char **) arg1);
    if(stack_value == NULL)
    return; 


    sema_down(&filesys_sema);
    bool temp = filesys_remove(*(char **) arg1);
    sema_up(&filesys_sema);
    if(!temp)
    {
      f->eax = false;
      return;
    }
    f->eax = true;
    return;
  }

  if( *((int *) f->esp) == SYS_CREATE)
  {
    //void * stack_value = safe_acc(arg1);
    //if(stack_value == NULL)
    //  return; 

    stack_value = safe_acc(*(char **) arg1);
    if(stack_value == NULL)
    return; 

    stack_value = safe_acc(arg2);
    if(stack_value == NULL)
      return; 

    sema_down(&filesys_sema);
    bool temp = filesys_create(*(char **) arg1, * (unsigned *) arg2);
    sema_up(&filesys_sema);
    if(!temp)
    {
      f->eax = false;
      return;
    }
    f->eax = true;
    return;
  }
  
  if(*((int *) f->esp) == SYS_FILESIZE)
  {
      void * stack_value = safe_acc(arg1);
      if(stack_value == NULL)
        return; 

      for(struct list_elem * index = list_begin(&thread_current()->files);
	  (index) != list_end(&thread_current()->files);
	  index = list_next(index))
      {
	 if(list_entry(index, struct file_desc, threadelem)->fd == *(int *) arg1)
           {
             sema_down(&filesys_sema);
             f->eax = file_length(list_entry(index, struct file_desc, threadelem)->fileloc);
             sema_up(&filesys_sema);
             return;
           }
      }
    return;
  }

  if(*((int *) f->esp) == SYS_TELL)
  {
      void * stack_value = safe_acc(arg1);
      if(stack_value == NULL)
        return; 

      for(struct list_elem * index = list_begin(&thread_current()->files);
	  (index) != list_end(&thread_current()->files);
	  index = list_next(index))
      {
	 if(list_entry(index, struct file_desc, threadelem)->fd == *(int *) arg1)
           {
             sema_down(&filesys_sema);
             f->eax = file_tell(list_entry(index, struct file_desc, threadelem)->fileloc);
             sema_up(&filesys_sema);
             return;
           }
      }
    return;
  }

  if(*((int *) f->esp) == SYS_SEEK)
  {
      void * stack_value = safe_acc(arg1);
      if(stack_value == NULL)
        return; 
      for(struct list_elem * index = list_begin(&thread_current()->files);
	  (index) != list_end(&thread_current()->files);
	  index = list_next(index))
      {
	 if(list_entry(index, struct file_desc, threadelem)->fd == *(int *) arg1)
           {
             sema_down(&filesys_sema);
             file_seek(list_entry(index, struct file_desc, threadelem)->fileloc, *(unsigned *) arg2);
             sema_up(&filesys_sema);
             return;
           }
      }
    return;
  }

  if(*((int *) f->esp) == SYS_EXEC)
  {
      void * stack_value = safe_acc(arg1);
      if(stack_value == NULL)
        return; 

      stack_value = safe_acc(*(char **) arg1);
      if(stack_value == NULL)
        return; 
    
    sema_init(get_execsema(), 0);

    tid_t temp = process_execute(*(char **) arg1);
    sema_down(get_execsema());
    if(temp == TID_ERROR)
	f->eax = -1;
    else
    {
        if(exec_status == -1)
           f->eax=-1;
        else
	f->eax = temp;
    }
    return;
  }

 /* if(*((int *) f->esp) == SYS_REMOVE)
  {
    void * stack_value = safe_acc(arg1);
    if(stack_value == NULL)
      return; 

    stack_value = safe_acc(arg2);
    if(stack_value == NULL)
      return; 

    sema_down(&filesys_sema);
    bool temp = filesys_remove(*(char **) arg1, * (unsigned *) arg2);
    sema_up(&filesys_sema);
    if(temp == NULL)
    {
      f->eax = false;
      return;
    }
    f->eax = true;
    return;
  }*/

  if(*((int *) f->esp) == SYS_WRITE)
  {
    if(*(int *) arg1 == 1)
    {
      void * stack_value = safe_acc(arg2);
      if(stack_value == NULL)
      return; 

      stack_value = safe_acc(*(char **) arg2);
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
             void * stack_value = safe_acc(arg2);
             if(stack_value == NULL)
       		return; 

             stack_value = safe_acc(*(char **) arg2);
             if(stack_value == NULL)
      		return; 
             sema_down(&filesys_sema);
             f->eax = file_write(list_entry(index, struct file_desc, threadelem)->fileloc, *(char **) arg2, *(int *) arg3);
             sema_up(&filesys_sema); 
             return;
           }
      }
      return;
    }
  }

  if(*((int *) f->esp) == SYS_READ)
  {
    if(*(int *) arg1 == 0)
    {
      void * stack_value = safe_acc(arg2);
      if(stack_value == NULL)
      return; 

      stack_value = safe_acc(*(char **) arg2);
      if(stack_value == NULL)
      return; 

      input_getc((*(char **) arg2), *(int *) arg3);
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
             void * stack_value = safe_acc(arg1);
             if(stack_value == NULL)
       		return; 

             stack_value = safe_acc(arg3);
             if(stack_value == NULL)
       		return; 

             stack_value = safe_acc(arg2);
             if(stack_value == NULL)
       		return; 

             stack_value = safe_acc(*(char **) arg2);
             if(stack_value == NULL)
      		return; 

             sema_down(&filesys_sema);
             f->eax = file_read(list_entry(index, struct file_desc, threadelem)->fileloc,(void *) *(char **) arg2, *(int *) arg3);
             sema_up(&filesys_sema);
             return;
           }
      }
      return;
    }
  }

  if(*((int *) f->esp) == SYS_CLOSE)
  {
      void * stack_value = safe_acc(arg2);
      if(stack_value == NULL)
      return; 

      stack_value = safe_acc(*(char **) arg2);
      if(stack_value == NULL)
      return; 

      for(struct list_elem * index = list_begin(&thread_current()->files);
	  (index) != list_end(&thread_current()->files);
	  index = list_next(index))
      {
	 if(list_entry(index, struct file_desc, threadelem)->fd == *(int *) arg1)
           {
             stack_value = safe_acc(arg2);
             if(stack_value == NULL)
      		return; 

             stack_value = safe_acc(*(char **) arg2);
             if(stack_value == NULL)
      		return; 

             sema_down(&filesys_sema);
             f->eax = file_close(list_entry(index, struct file_desc, threadelem)->fileloc);
             sema_up(&filesys_sema);
             list_remove(index);
             return;
           }
      }
      return;
  }
}


void * 
safe_acc(const void * f)
{
  if(f==NULL)
  {
    thread_current()->exit = -1;
    thread_exit();
    return NULL;
  }
  if(is_user_vaddr(f))
  {
    void * ret_ptr = pagedir_get_page(thread_current()->pagedir, f);
    if(ret_ptr != NULL)
      return ret_ptr;
    thread_current()->exit = -1;
    thread_exit();
    return NULL;
  }
  thread_current()->exit = (-1);
  thread_exit();
  return NULL;
}

