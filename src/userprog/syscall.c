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


static struct semaphore filesys_sema;

void * 
safe_acc(const void * f);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
  sema_init(&filesys_sema, 1);
}

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


    sema_down(&filesys_sema);
    struct file * temp = filesys_open(*(char **) arg1);
    if(temp == NULL)
    {
      f->eax = -1;
      sema_up(&filesys_sema);
      return;
    }

    struct file_desc * ins = malloc(sizeof(struct file_desc));
    ins->fd = thread_current()->max_fd;
    thread_current()->max_fd++;
    ins->fileloc = temp;
    list_push_back(&thread_current()->files, &ins->threadelem);
    f->eax = thread_current()->max_fd - 1;
    sema_up(&filesys_sema);
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
    void * stack_value = safe_acc(arg1);
    if(stack_value == NULL)
      return; 

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

      sema_down(&filesys_sema);
      for(struct list_elem * index = list_begin(&thread_current()->files);
	  (index) != list_end(&thread_current()->files);
	  index = list_next(index))
      {
	 if(list_entry(index, struct file_desc, threadelem)->fd == *(int *) arg1)
           {
             f->eax = file_length(list_entry(index, struct file_desc, threadelem)->fileloc);
             sema_up(&filesys_sema);
             return;
           }
      }
    sema_up(&filesys_sema);
    return;
  }

  if(*((int *) f->esp) == SYS_TELL)
  {
      void * stack_value = safe_acc(arg1);
      if(stack_value == NULL)
        return; 

      stack_value = safe_acc((int *) arg1);
      if(stack_value == NULL)
      return; 

      sema_down(&filesys_sema);
      for(struct list_elem * index = list_begin(&thread_current()->files);
	  (index) != list_end(&thread_current()->files);
	  index = list_next(index))
      {
	 if(list_entry(index, struct file_desc, threadelem)->fd == *(int *) arg1)
           {
             f->eax = file_tell(list_entry(index, struct file_desc, threadelem)->fileloc);
             sema_up(&filesys_sema);
             return;
           }
      }
    sema_up(&filesys_sema);
    return;
  }

  if(*((int *) f->esp) == SYS_SEEK)
  {
      void * stack_value = safe_acc(arg1);
      if(stack_value == NULL)
        return; 

      sema_down(&filesys_sema);
      for(struct list_elem * index = list_begin(&thread_current()->files);
	  (index) != list_end(&thread_current()->files);
	  index = list_next(index))
      {
	 if(list_entry(index, struct file_desc, threadelem)->fd == *(int *) arg1)
           {
             file_seek(list_entry(index, struct file_desc, threadelem)->fileloc, *(unsigned *) arg2);
             sema_up(&filesys_sema);
             return;
           }
      }
    sema_up(&filesys_sema);
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


    sema_down(&filesys_sema);
    //if(exec==0)
    sema_init(get_execsema(), 0);

    exec=1;
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

    if(f->eax != -1)
    {
      struct child_desc * child = malloc(sizeof(struct child_desc));
      child->child_pid = temp;
      child->wait_num = 0;
      list_push_back(&thread_current()->children, &child->childelem);
    }

    sema_up(&filesys_sema);
    return;
  }

  if(*((int *) f->esp) == SYS_WAIT)
  {

      void * stack_value = safe_acc(arg1);
      if(stack_value == NULL)
        return; 

      int valid = 0;
      for(struct list_elem * index = list_begin(&thread_current()->children);
	  (index) != list_end(&thread_current()->children);
	  index = list_next(index))
      {
	 if(list_entry(index, struct child_desc, childelem)->child_pid == *(int *) arg1)
           {
             if(list_entry(index, struct child_desc, childelem)->wait_num >0)
             {
             valid = 0;
             }
             else
             {
              valid = 1;
              list_entry(index, struct child_desc, childelem)->wait_num++;
             }
             break;
           }
      }

    if(valid == 0)
    {
      f->eax = -1;
      return;
    }

    tid_t temp = process_wait(*(int *) arg1);
    f->eax = temp;
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
    stack_value = safe_acc((int *) arg1);
    if(stack_value == NULL)
    return; 
    void * stack_value = safe_acc(arg2);
    if(stack_value == NULL)
    return; 

    stack_value = safe_acc(*(char **) arg2);
    if(stack_value == NULL)
    return; 
    if(*(int *) arg1 == 1)
    {
      putbuf((*(char **) arg2), *(int *) arg3);
      f->eax = *(int *) arg3;
      return;
    }
    else
    {
      sema_down(&filesys_sema);
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
             f->eax = file_write(list_entry(index, struct file_desc, threadelem)->fileloc, *(char **) arg2, *(int *) arg3);
             sema_up(&filesys_sema); 
             return;
           }
      }
      f->eax = -1;
      sema_up(&filesys_sema);
      return;
    }
  }

  if(*((int *) f->esp) == SYS_READ)
  {
     stack_value = safe_acc(arg3);
     if(stack_value == NULL)
     return; 

     stack_value = safe_acc(arg2);
     if(stack_value == NULL)
     return; 

    stack_value = safe_acc(*(char **) arg2);
    if(stack_value == NULL)
      return; 

    stack_value = safe_acc((int *) arg1);
    if(stack_value == NULL)
    return; 

    if(*(int *) arg1 == 0)
    {
      void * stack_value = safe_acc(arg2);
      if(stack_value == NULL)
      return; 

      stack_value = safe_acc(*(char **) arg2);
      if(stack_value == NULL)
      return; 

      stack_value = safe_acc((int *) arg1);
      if(stack_value == NULL)
      return; 

      input_getc((*(char **) arg2), *(int *) arg3);
      f->eax = *(int *) arg3;
      return;
    }
    else
    {
      sema_down(&filesys_sema);
      for(struct list_elem * index = list_begin(&thread_current()->files);
	  (index) != list_end(&thread_current()->files);
	  index = list_next(index))
      {
	 if(list_entry(index, struct file_desc, threadelem)->fd == *(int *) arg1)
           {

             f->eax = file_read(list_entry(index, struct file_desc, threadelem)->fileloc,*(char **) arg2, *(int *) arg3);
             sema_up(&filesys_sema);
             return;
           }
      }
      f->eax = -1;
      sema_up(&filesys_sema);
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

      stack_value = safe_acc((int *) arg1);
      if(stack_value == NULL)
      return; 

      sema_down(&filesys_sema);
      for(struct list_elem * index = list_begin(&thread_current()->files);
	  (index) != list_end(&thread_current()->files);
	  index = list_next(index))
      {
	 if(list_entry(index, struct file_desc, threadelem)->fd == *(int *) arg1)
           {
             f->eax = file_close(list_entry(index, struct file_desc, threadelem)->fileloc);
struct file_desc * aa =  list_entry(index, struct file_desc, threadelem);
             list_remove(index);
free(aa);
             sema_up(&filesys_sema);
             return;
           }
      }
      sema_up(&filesys_sema);
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

