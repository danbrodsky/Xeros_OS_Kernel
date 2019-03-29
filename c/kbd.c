#include <xeroskernel.h>
#include <kbd.h>
#include <stdarg.h>

static  int     state; /* the state of the keyboard */

/* indicator for when to echo key input to screen */
static int echo = 0;

/* current character to trigger end-of-file (set to EOT char) */
static int eof = 4;

/* current open spot in keyboard buffer */
static int buff_idx = 0;

/* next position to read in keyboard buffer */
static int read_idx = 0;

/* indicator for how much data buffer contains*/
static int size = 0;

/* keyboard buffer */
static char kbdbuffer[MAX_BUFF_SIZE];

/*  Normal table to translate scan code  */
unsigned char   kbcode[] = { 0,
          27,  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',
         '0',  '-',  '=', '\b', '\t',  'q',  'w',  'e',  'r',  't',
         'y',  'u',  'i',  'o',  'p',  '[',  ']', '\n',    0,  'a',
         's',  'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';', '\'',
         '`',    0, '\\',  'z',  'x',  'c',  'v',  'b',  'n',  'm',
         ',',  '.',  '/',    0,    0,    0,  ' ' };

/* captialized ascii code table to tranlate scan code */
unsigned char   kbshift[] = { 0,
           0,  '!',  '@',  '#',  '$',  '%',  '^',  '&',  '*',  '(',
         ')',  '_',  '+', '\b', '\t',  'Q',  'W',  'E',  'R',  'T',
         'Y',  'U',  'I',  'O',  'P',  '{',  '}', '\n',    0,  'A',
         'S',  'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',  '"',
         '~',    0,  '|',  'Z',  'X',  'C',  'V',  'B',  'N',  'M',
         '<',  '>',  '?',    0,    0,    0,  ' ' };
/* extended ascii code table to translate scan code */
unsigned char   kbctl[] = { 0,
           0,    0,    0,    0,    0,    0,    0,    0,    0,    0,
           0,   31,    0, '\b', '\t',   17,   23,    5,   18,   20,
          25,   21,    9,   15,   16,   27,   29, '\n',    0,    1,
          19,    4,    6,    7,    8,   10,   11,   12,    0,    0,
           0,    0,   28,   26,   24,    3,   22,    2,   14,   13 };


static int extchar( unsigned char   code)
{
        state &= ~EXTENDED;
}

unsigned int kbtoa( unsigned char code )
{
  unsigned int    ch;
  
  if (state & EXTENDED)
    return extchar(code);
  if (code & KEY_UP) {
    switch (code & 0x7f) {
    case LSHIFT:
    case RSHIFT:
      state &= ~INSHIFT;
      break;
    case CAPSL:
      kprintf("Capslock off detected\n");
      state &= ~CAPSLOCK;
      break;
    case LCTL:
      state &= ~INCTL;
      break;
    case LMETA:
      state &= ~INMETA;
      break;
    }
    
    return NOCHAR;
  }
  
  
  /* check for special keys */
  switch (code) {
  case LSHIFT:
  case RSHIFT:
    state |= INSHIFT;
    kprintf("shift detected!\n");
    return NOCHAR;
  case CAPSL:
    state |= CAPSLOCK;
    kprintf("Capslock ON detected!\n");
    return NOCHAR;
  case LCTL:
    state |= INCTL;
    return NOCHAR;
  case LMETA:
    state |= INMETA;
    return NOCHAR;
  case EXTESC:
    state |= EXTENDED;
    return NOCHAR;
  }
  
  ch = NOCHAR;
  
  if (code < sizeof(kbcode)){
    if ( state & CAPSLOCK )
      ch = kbshift[code];
	  else
	    ch = kbcode[code];
  }
  if (state & INSHIFT) {
    if (code >= sizeof(kbshift))
      return NOCHAR;
    if ( state & CAPSLOCK )
      ch = kbcode[code];
    else
      ch = kbshift[code];
  }
  if (state & INCTL) {
    if (code >= sizeof(kbctl))
      return NOCHAR;
    ch = kbctl[code];
  }
  if (state & INMETA)
    ch += 0x80;
  return ch;
}

void kbd_inthandler(void) {
    // read new byte from keyboard port
    if (!( inb(0x64) & 0x1  )) {
        return;
    }
    unsigned char scan_byte = inb(0x60);
    unsigned char ascii_key = kbtoa(scan_byte);


    // make sure valid key was received and buffer isn't full
    if (!ascii_key || (buff_idx == read_idx && size)) {
        return;
    }

    // if eof key was pressed then clear keyboard buffer
    if (ascii_key == eof) {

        kbdbuffer[buff_idx] = ascii_key;
        buff_idx = (buff_idx + 1) % MAX_BUFF_SIZE;
        ++size;
        // disable keyboard hardware
        kbdclose();
        return;
    }

    // if echo is enabled, write the keyboard input to the screen
    if (echo) {
        kprintf("%c", ascii_key);
    }

    // write pressed key to buffer
    kbdbuffer[buff_idx] = ascii_key;
    buff_idx = (buff_idx + 1) % MAX_BUFF_SIZE;
    ++size;
}

void kbdopen(void) {
    // enable keyboard
    buff_idx = 0;
    read_idx = 0;
    size = 0;
    outb(0x64, 0xAE);
}
void kbdclose(void) {
    // disable keyboard
    outb(0x64, 0xAD);
}

int kbdwrite(void* buff, int bufflen) {
    // write on keyboard is not supported
    return -1;
}
int kbdread(char* buff, int bufflen) {
    kprintf("entering into keyboard read \n");
    int written = 0;
    // write any characters in keyboard buffer to application buffer
        while (size) {

            // stop reading bytes if we've reached bufflen limit
            if (written >= bufflen) {
                return written;
            }

            // EOF key pressed
            if (kbdbuffer[read_idx] == eof) {
                return 0;
            }

            *buff = kbdbuffer[read_idx];

            ++buff;
            read_idx = (read_idx + 1) % MAX_BUFF_SIZE;
            --size;
            ++written;

            // return key pressed, return call
            if (kbdbuffer[read_idx] == '\n') {
                return written;
            }
        }
    return written;

    // read 

    // buffer up to 4 characters
    

    // read into buffer until full or enter key

    // Ctrl-D detected, return eof

    // signal detected

    // enter key (\n) detected


}

int kbdioctl(unsigned long command, va_list args) {

    // Command 53: change EOF character
    if (command == CTL_EOF_CHANGE) {
        int new_eof = va_arg(args, int );
        eof = new_eof;
        
        return 0;
    }
    // Command 55: turn echoing off
    if (command == CTL_ECHO_OFF) {
        echo = 0;

        return 0;
    }
    // Command 56: turn echoing on
    if (command == CTL_ECHO_ON) {
        echo = 1;

        return 0;
    }

    // invalid command given, return error
    return -1;
}

void kbdinit(void) {

    kprintf("created keyboard in devtab[0]");
    devtab[0].dvopen = &kbdopen;
    devtab[0].dvclose = &kbdclose;
    devtab[0].dvwrite = &kbdwrite;
    devtab[0].dvread = &kbdread;
    devtab[0].dvioctl = &kbdioctl;

}

