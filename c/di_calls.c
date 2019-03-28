#include <xeroskernel.h>

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

    while (p->fdt != NULL){
        ++slot;
    }

    if ( slot >= MAX_DEV ) {
        // no open slots in fdt, return error
        return SYSERR;
    }

    // call into the lower half of device driver
    ( curr_dev->dvopen )();

    // store device pointer in process fdt
    p->fdt[slot] = curr_dev;

    return OK;
}


int di_close(int fd, pcb* p) {

    device* curr_dev = &devtab[fd];

    if ( curr_dev->dvclose == NULL || fd >= MAX_DEV ) {
        // device close call does not exist or no fd at this index in proc, return error
        return SYSERR;
    }

    (curr_dev->dvclose)();
    p->fdt[fd] = NULL;
    return 0;
}


int di_write(int fd, void* buff, int bufflen, pcb* p) {

    device* curr_dev = &devtab[fd];

    if ( curr_dev->dvwrite == NULL || fd >= MAX_DEV ) {
        // device close call does not exist or no fd at this index in proc, return error
        return SYSERR;
    }

    // call lower half function for write, and return the size written received
    return (curr_dev->dvwrite)(void* buff, int bufflen);
}

int di_read(int fd, void* buff, int bufflen, pcb* p) {

    device* curr_dev = &devtab[fd];

    if ( curr_dev->dvread == NULL || fd >= MAX_DEV ) {
        // device close call does not exist or no fd at this index in proc, return error
        return SYSERR;
    }

    // call lower half function for read, and return the amount read received
    return (curr_dev->dvwrite)(void* buff, int bufflen);
}

int di_ioctl(int fd, unsigned long command, ..., pcb* p) {

    device* curr_dev = &devtab[fd];

    if ( curr_dev->dvioctl == NULL || fd >= MAX_DEV ) {
        // device close call does not exist or no fd at this index in proc, return error
        return SYSERR;
    }

    // call lower half function for read, and return the amount read received
    return (curr_dev->dvioctl)(unsigned long command, ...);
}

