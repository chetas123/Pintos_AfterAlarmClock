#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H
#define PID_ERROR ((pid_t) -1)
#include <list.h>

void syscall_init (void);

/* Added for project-2 */
typedef int (*handler) (int, int, int);
typedef int pid_t;

struct fd
{
  int fdid;
  struct file *file;
//  struct list_elem fd_elem;
  struct list_elem thread_elem;
};

static int sys_write (int fdid, const void *buffer, unsigned size);
static int sys_halt (void);
int sys_exit (int status);
static pid_t sys_exec (const char *file);
static int sys_wait (pid_t pid);
static int sys_create (const char *file, unsigned initial_size);
static int sys_remove (const char *file);
static int sys_open (const char *file);
static int sys_filesize (int fdid);
static int sys_read (int fdid, void *buffer, unsigned size);
static int sys_seek (int fdid, unsigned position);
static int sys_tell (int fdid);
static int sys_close (int fdid);

static int alloc_fdid (void);
static struct fd *get_fd_by_fdid (int fdid);

#endif /* userprog/syscall.h */
