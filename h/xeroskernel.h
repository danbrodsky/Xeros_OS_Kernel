/* xeroskernel.h - disable, enable, halt, restore, isodd, min, max */

#ifndef XEROSKERNEL_H
#define XEROSKERNEL_H

/* Symbolic constants used throughout Xinu */

typedef	char    Bool;        /* Boolean type                  */
typedef unsigned int size_t; /* Something that can hold the value of
                              * theoretical maximum number of bytes 
                              * addressable in this architecture.
                              */
#define	FALSE   0       /* Boolean constants             */
#define	TRUE    1
#define	EMPTY   (-1)    /* an illegal gpq                */
#define	NULL    0       /* Null pointer for linked lists */
#define	NULLCH '\0'     /* The null character            */

#define CREATE_FAILURE -1  /* Process creation failed     */



/* Universal return constants */

#define	OK            1         /* system call ok               */
#define	SYSERR       -1         /* system call failed           */
#define	EOF          -2         /* End-of-file (usu. from read)	*/
#define	TIMEOUT      -3         /* time out  (usu. recvtim)     */
#define	INTRMSG      -4         /* keyboard "intr" key pressed	*/
                                /*  (usu. defined as ^B)        */
#define	BLOCKERR     -5         /* non-blocking op would block  */

/* Functions defined by startup code */


void           bzero(void *base, int cnt);
void           bcopy(const void *src, void *dest, unsigned int n);
void           disable(void);
unsigned short getCS(void);
unsigned char  inb(unsigned int);
void           init8259(void);
int            kprintf(char * fmt, ...);
void           lidt(void);
void           outb(unsigned int, unsigned char);


/* Some constants involved with process creation and managment */
 
   /* Maximum number of processes */      
#define MAX_PROC        64           
   /* Kernel trap number          */
#define KERNEL_INT      80
   /* Interrupt number for the timer */
#define TIMER_INT      (TIMER_IRQ + 32)
   /* Minimum size of a stack when a process is created */
#define PROC_STACK      (4096 * 4)    
              
   /* Number of milliseconds in a tick */
#define MILLISECONDS_TICK 10        


/* Constants to track states that a process is in */
#define STATE_STOPPED   0
#define STATE_READY     1
#define STATE_BLOCKED   2
#define STATE_WAITING   3

#define STATE_SLEEP     22
#define STATE_RUNNING   23

/* System call identifiers */
#define SYS_STOP        10
#define SYS_YIELD       11
#define SYS_CREATE      22
#define SYS_TIMER       33
#define SYS_GETPID      144
#define SYS_PUTS        155
#define SYS_SLEEP       166
#define SYS_KILL        177
#define SYS_CPUTIMES    178
#define SYS_SIGHANDLER  179
#define SYS_SIGRETURN   180
#define SYS_SIGKILL     181
#define SYS_SIGWAIT     182
#define SYS_OPEN        183
#define SYS_CLOSE       184 
#define SYS_WRITE       185 
#define SYS_READ        186 
#define SYS_IOCTL       187 



#define MAX_SIG 32

// syssighandler return codes
#define SIGHANDLER_INVALID_SIGNUM          -1
#define SIGHANDLER_INVALID_NEW_HANDLER     -2
#define SIGHANDLER_INVALID_OLD_HANDLER     -3
#define SIGHANDLER_SIGNAL_INSTALL_SUCCESS   0

// sigkill return codes
#define SIGKILL_INVALID_PID     -514
#define SIGKILL_INVALID_SIGNUM  -583
#define SIGKILL_SUCCESS          0

// syswait return code
#define SYSWAIT_INVALID_PID     -1
#define SYSWAIT_SUCCESS          0

// signal interrupt code
#define SIGNAL_INTERRUPT        -666

/* Structure to track the information associated with a single process */

typedef struct struct_pcb pcb;
struct struct_pcb {
  void        *esp;    /* Pointer to top of saved stack           */
  pcb         *next;   /* Next process in the list, if applicable */
  pcb         *prev;   /* Previous proccess in list, if applicable*/
  int          state;  /* State the process is in, see above      */
  unsigned int pid;    /* The process's ID                        */
  int          ret;    /* Return value of system call             */
                       /* if process interrupted because of system*/
                       /* call                                    */
  long         args;   
  unsigned int otherpid;

  void        *signal_table[MAX_SIG];
  int         pending_signals[MAX_SIG];
  int         signal_mask[MAX_SIG];

  void        *buffer;
  int          bufferlen;
  int          sleepdiff;
  long         cpuTime;  /* CPU time consumed                     */
};


typedef struct struct_ps processStatuses;
struct struct_ps {
  int  entries;            // Last entry used in the table
  int  pid[MAX_PROC];      // The process ID
  int  status[MAX_PROC];   // The process status
  long  cpuTime[MAX_PROC]; // CPU time used in milliseconds
};


/* The actual space is set aside in create.c */
extern pcb     proctab[MAX_PROC];

#pragma pack(1)

/* What the set of pushed registers looks like on the stack */
typedef struct context_frame {
  unsigned long        edi;
  unsigned long        esi;
  unsigned long        ebp;
  unsigned long        esp;
  unsigned long        ebx;
  unsigned long        edx;
  unsigned long        ecx;
  unsigned long        eax;
  unsigned long        iret_eip;
  unsigned long        iret_cs;
  unsigned long        eflags;
  unsigned long        stackSlots[];
} context_frame;

/* Device section */


/* maximum number of devices */
#define MAX_DEV 32

/* struct for device information */
typedef struct struct_device device;

struct struct_device {
    
};

/* table for storing device information */
device devtab[MAX_DEV];


/* Memory mangement system functions, it is OK for user level   */
/* processes to call these.                                     */

int      kfree(void *ptr);
void     kmeminit( void );
void     *kmalloc( size_t );


/* A typedef for the signature of the function passed to syscreate */
typedef void    (*funcptr)(void);


/* Internal functions for the kernel, applications must never  */
/* call these.                                                 */
void     dispatch( void );
void     dispatchinit( void );
void     ready( pcb *p );
pcb      *next( void );
void     contextinit( void );
int      contextswitch( pcb *p );
int      create( funcptr fp, size_t stack );
void     set_evec(unsigned int xnum, unsigned long handler);
void     printCF (void * stack);  /* print the call frame */
int      syscall(int call, ...);  /* Used in the system call stub */
void     sleep(pcb *, unsigned int);
void     tick( void );
void removeFromSleep(pcb * p);



/* Function prototypes for system calls as called by the application */
int          syscreate( funcptr fp, size_t stack );
void         sysyield( void );
void         sysstop( void );
unsigned int sysgetpid( void );
unsigned int syssleep(unsigned int);
void     sysputs(char *str);


int sysgetcputimes(processStatuses *ps);
int syssighandler(int signal, void (*newhandler)(void *), void (** oldHandler)(void *));
void syssigreturn(void *old_sp);
int syskill(int PID, int signalNumber);
int syswait(int PID);


void sigtramp(void (*handler)(void *), void *cntx);
void signal(pcb *p);
void print_signal_status(pcb *p);


/* The initial process that the system creates and schedules */
void     root( void );
void     test_syssighandler(void);
void     test_syskill1(void);
void     test_signal_priority(void);

/* additional syscalls */
extern int sysopen(int device_no);
extern int sysclose(int fd);
extern int syswrite(int fd, void *buff, int bufflen);
extern int sysread(int fd, void *buff, int bufflen);
extern int sysioctl(int fd, unsigned long command, ...);

/* Device independent calls */
extern int di_open(int device_no, pcb* p);
extern int di_close(int fd, pcb* p);
extern int di_write(int fd, void* buff, int bufflen, pcb* p);
extern int di_read(int fd, void* buff, int bufflen, pcb* p);
extern int di_ioctl(int fd, unsigned long command, ..., pcb* p);

void           set_evec(unsigned int xnum, unsigned long handler);


/* Anything you add must be between the #define and this comment */
#endif

