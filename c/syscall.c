/* syscall.c : syscalls
 */

#include <xeroskernel.h>
#include <stdarg.h>


int syscall( int req, ... ) {
/**********************************/

    va_list     ap;
    int         rc;

    va_start( ap, req );

    __asm __volatile( " \
        movl %1, %%eax \n\
        movl %2, %%edx \n\
        int  %3 \n\
        movl %%eax, %0 \n\
        "
        : "=g" (rc)
        : "g" (req), "g" (ap), "i" (KERNEL_INT)
        : "%eax" 
    );
 
    va_end( ap );

    return( rc );
}

int syscreate( funcptr fp, size_t stack ) {
/*********************************************/

    return( syscall( SYS_CREATE, fp, stack ) );
}

void sysyield( void ) {
/***************************/
  syscall( SYS_YIELD );
}

 void sysstop( void ) {
/**************************/
   syscall( SYS_STOP );
}

unsigned int sysgetpid( void ) {
/****************************/

    return( syscall( SYS_GETPID ) );
}

void sysputs( char *str ) {
/********************************/

    syscall( SYS_PUTS, str );
}

unsigned int syssleep( unsigned int t ) {
/*****************************/

    return syscall( SYS_SLEEP, t );
}

/*
int syskill(int pid) {
  return syscall(SYS_KILL, pid);
}
 */

int sysgetcputimes(processStatuses *ps) {
  return syscall(SYS_CPUTIMES, ps);
}

int syssighandler(int signal, void (*newhandler)(void *), void (** oldHandler)(void *)){
    return syscall(SYS_SIGHANDLER, signal, newhandler, oldHandler);
}

void syssigreturn(void *old_sp){
    syscall(SYS_SIGRETURN, old_sp);
}

int syskill(int PID, int signalNumber){
    syscall(SYS_SIGKILL, PID, signalNumber);
}

int syswait(int PID){
    syscall(SYS_SIGWAIT, PID);
}

int sysopen(int device_no) {
    syscall(SYS_OPEN, device_no);
}

int sysclose(int fd){
    syscall(SYS_CLOSE, fd);
}

int syswrite(int fd, void *buff, int bufflen) {
    syscall(SYS_WRITE, fd, buff, bufflen);
}

int sysread(int fd, void *buff, int bufflen) {
    syscall(SYS_READ, fd, buff, bufflen);
}

int sysioctl(int fd, unsigned long command, ...) {
    va_list ap;
    va_start( ap, command );

    syscall(SYS_IOCTL, fd, command, ap);

    va_end(ap);
}
