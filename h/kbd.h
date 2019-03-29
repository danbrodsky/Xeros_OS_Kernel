#include <xeroskernel.h>
#include <stdarg.h>

#define KEY_UP   0x80            /* If this bit is on then it is a key   */
                                 /* up event instead of a key down event */

/* Control code */
#define LSHIFT  0x2a
#define RSHIFT  0x36
#define LMETA   0x38

#define LCTL    0x1d
#define CAPSL   0x3a


/* scan state flags */
#define INCTL           0x01    /* control key is down          */
#define INSHIFT         0x02    /* shift key is down            */
#define CAPSLOCK        0x04    /* caps lock mode               */
#define INMETA          0x08    /* meta (alt) key is down       */
#define EXTENDED        0x10    /* in extended character mode   */

#define EXTESC          0xe0    /* extended character escape    */
#define NOCHAR  256

#define MAX_BUFF_SIZE 4

#define CTL_EOF_CHANGE 53
#define CTL_ECHO_OFF   55
#define CTL_ECHO_ON    56

void kbdopen(void);
void kbdclose(void);

int kbdwrite(void* buff, int bufflen);
int kbdread(char* buff, int bufflen);

void kbdinit(void);

int kbdioctl(unsigned long command, va_list args);

unsigned int kbtoa(unsigned char);

void kbd_inthandler(void);
