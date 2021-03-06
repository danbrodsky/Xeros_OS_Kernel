#include <xeroskernel.h>
#include <xeroslib.h>
#include <stdarg.h>

/* open a device */
int di_open(int device_no, pcb* p) {
    // find if device exists
    device* curr_dev = &devtab[device_no];

    if ( curr_dev->dvopen == NULL ) {
        // device open call does not exist, return error
        return SYSERR;
    }

    // find open spot in process FDT
    int slot = 0;

    while (p->fdt[slot] != NULL){
        ++slot;
    }

    if ( slot >= MAX_PROC_DEV ) {
        // no open slots in fdt, return error
        return SYSERR;
    }

    // call into the lower half of device driver
    ( curr_dev->dvopen )();

    // store device pointer in process fdt
    p->fdt[slot] = curr_dev;

    return device_no;
}


int di_close(int fd, pcb* p) {

    device* curr_dev = p->fdt[fd];

    if ( curr_dev->dvclose == NULL || fd >= MAX_DEV ) {
        // device close call does not exist or no fd at this index in proc, return error
        return SYSERR;
    }

    (curr_dev->dvclose)();
    p->fdt[fd] = NULL;
    return 0;
}


int di_write(int fd, void* buff, int bufflen, pcb* p) {

    device* curr_dev = p->fdt[fd];

    if ( curr_dev->dvwrite == NULL || fd >= MAX_DEV ) {
        // device write call does not exist or no fd at this index in proc, return error
        return SYSERR;
    }

    // call lower half function for write, and return the size written received
    return (curr_dev->dvwrite)( buff, bufflen);
}

int di_read(int fd, void* buff, int bufflen, pcb* p) {

    device* curr_dev = p->fdt[fd];

    if ( curr_dev->dvread == NULL || fd >= MAX_DEV ) {
        // device read call does not exist or no fd at this index in proc, return error
        return SYSERR;
    }

    // call lower half function for read, and return the amount read received
    return (curr_dev->dvread)( buff, bufflen, p);
}

int di_ioctl(int fd, unsigned long command, va_list args,  pcb* p) {

    device* curr_dev = p->fdt[fd];

    if ( curr_dev->dvioctl == NULL || fd >= MAX_DEV ) {
        // device control call does not exist or no fd at this index in proc, return error
        return SYSERR;
    }

    // call lower half function for ioctl, and returns whether it succeeded
    return (curr_dev->dvioctl)(command, args);
}

