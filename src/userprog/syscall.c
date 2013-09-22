#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "devices/shutdown.h"
#include "threads/thread.h"
#include "threads/synch.h"

static void syscall_handler (struct intr_frame *);
static handler syscall_vec[30];
static struct lock _lock;
//static struct list all_files;

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");

  syscall_vec[SYS_HALT] = (handler)sys_halt;
  syscall_vec[SYS_EXIT] = (handler)sys_exit;
  syscall_vec[SYS_EXEC] = (handler)sys_exec;
  syscall_vec[SYS_WAIT] = (handler)sys_wait;
  syscall_vec[SYS_CREATE] = (handler)sys_create;
  syscall_vec[SYS_REMOVE] = (handler)sys_remove;
  syscall_vec[SYS_OPEN] = (handler)sys_open;
  syscall_vec[SYS_FILESIZE] = (handler)sys_filesize;
  syscall_vec[SYS_READ] = (handler)sys_read;
  syscall_vec[SYS_WRITE] = (handler)sys_write;
  syscall_vec[SYS_SEEK] = (handler)sys_seek;
  syscall_vec[SYS_TELL] = (handler)sys_tell;
  syscall_vec[SYS_CLOSE] = (handler)sys_close;

  lock_init (&_lock);
//  list_init (&all_files);
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  /*printf ("system call!\n");
  thread_exit (); */

  handler h;
  int *sp, ret;

  sp = f->esp;
  if (!is_user_vaddr (sp) || !is_user_vaddr (sp + 1) || !is_user_vaddr (sp + 2) || !is_user_vaddr (sp + 3))
    sys_exit (-1);

  h = syscall_vec[*sp];
  ret = h (*(sp + 1), *(sp + 2), *(sp + 3));

  f->eax = ret;
  return;
}

static int 
sys_write (int fdid, const void *buffer, unsigned size)
{
  int ret = -1;

  lock_acquire (&_lock);
  if (fdid == STDOUT_FILENO)
  {
    putbuf (buffer, size);
    ret = size;
  }

  lock_release (&_lock);
  return ret;
}

static int
sys_halt (void)
{
  shutdown_power_off ();
}

int 
sys_exit (int status)
{
  struct thread *t;
  struct list_elem *elem;

  t = thread_current ();
  t->ret_status = status;
  while (!list_empty (&t->files))		/*Close all files opened by current thread.*/
  {
    elem = list_begin (&t->files);
    sys_close (list_entry (elem, struct fd, thread_elem)->fdid);
  }
  thread_exit ();
  
  return status;
}

static pid_t 
sys_exec (const char *file)
{
  return;
}

static int 
sys_wait (pid_t pid)
{
  int ret_status;
  ret_status = process_wait (pid);

  return ret_status;
}

static int 
sys_create (const char *file, unsigned initial_size)
{
  if (!file)
    return sys_exit (-1);

  return filesys_create (file, initial_size);
}

static int 
sys_remove (const char *file)
{
  if (!file)
    return false;
  if (!is_user_vaddr (file))
    return sys_exit (-1);

  return filesys_remove (file);
}

static int 
sys_open (const char *file)
{
  int ret = -1;
  struct file *f;
  struct fd *newfd;

  if (!file)  						/*NULL pointer*/
    return ret;
  if (!is_user_vaddr (file))				/*Bad pointer*/
    sys_exit (ret);
  
  f = filesys_open (file);
  if (!f)						/*Bad filename or file doesn't exist*/
    return ret;
  else
  {
    newfd = (struct fd *) malloc (sizeof (struct fd));
    if (!newfd)						/*memory allocation failed.*/
    {
      file_close (f);
      return ret;
    }
    else
    {
      newfd->fdid = alloc_fdid ();
      newfd->file = f;
//      list_push_back (&all_files, &newfd->fd_elem);
      list_push_back (&thread_current ()->files, &newfd->thread_elem);
      ret = newfd->fdid;
      return ret;
    }
  }
}

static int
alloc_fdid (void)
{
  static int fdid = 2;
  return fdid++;
}

static int 
sys_filesize (int fdid)
{
  return;
}

static int 
sys_read (int fdid, void *buffer, unsigned size)
{
  return;
}

static int 
sys_seek (int fdid, unsigned position)
{
  return;
}

static int 
sys_tell (int fdid)
{
  return;
}

static int 
sys_close (int fdid)
{
  struct fd *fd;

  fd = get_fd_by_fdid (fdid);
  if (!fd)
    return -1;
  else
  {
    list_remove (&fd->thread_elem);
    file_close (fd->file);
    return 0;
  }
}

static struct fd *
get_fd_by_fdid (int fdid)
{
  struct fd *temp_fd;
  struct list_elem *elem;
  struct thread *cur;

  cur = thread_current ();
  for (elem = list_begin (&cur->files); elem != list_end (&cur->files); 
		  elem = list_next (&elem))
  {
    temp_fd = list_entry (elem, struct fd, thread_elem);
    if (temp_fd->fdid == fdid)
      return temp_fd;
  }

  return NULL;
}
