/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>
#include <kbd.h>

extern char	*maxaddr;
void *handler_sig8(void *cntx);
void *handler_sig30(void *cntx);
void *handler_sig0(void *cntx);
void test_process();

void a(void);
void t(void);
void alarm_proc(void);
void alarm(void);

static int alarm_wait_time = 100;
static int shell_id;
static int xit = FALSE;


void test_syssighandler(){
    int sig8 = 8;
    int sig30 = 30;

    void *handler = &handler_sig8;
    int ret_val = syssighandler(-1 , &handler_sig8, &handler);
    if(ret_val != SIGHANDLER_INVALID_SIGNUM){
        kprintf("test SIGHANDLER_INVALID_SIGNUM 1 fail\n");
        while(1);
    }

    ret_val = syssighandler(MAX_SIG + 1 , &handler_sig8, &handler);
    if(ret_val != SIGHANDLER_INVALID_SIGNUM){
        kprintf("test SIGHANDLER_INVALID_SIGNUM 2 fail\n");
        while(1);
    }


    ret_val = syssighandler(sig8, maxaddr + 1, &handler);
    if(ret_val != SIGHANDLER_INVALID_NEW_HANDLER){
        kprintf("test SIGHANDLER_INVALID_NEW_HANDLER fail\n");
        while(1);
    }

    ret_val = syssighandler(sig8, &handler_sig8, maxaddr +1);
    if(ret_val != SIGHANDLER_INVALID_OLD_HANDLER){
        kprintf("test SIGHANDLER_INVALID_OLD_HANDLER fail\n");
        while(1);
    }

    ret_val = syssighandler(sig8, &handler_sig8, &handler);
    if(ret_val != SIGHANDLER_SIGNAL_INSTALL_SUCCESS){
        kprintf("test SIGHANDLER_SIGNAL_INSTALL_SUCCESS fail\n");
        while(1);
    }

    kprintf("all test_syssighandler test cases passed...\n");
    while(1);
}

void test_syskill1(){
    int sig8 = 8;
    int sig30 = 30;
    int pid1 = 1;
    void *handler;
    int ret_val = syssighandler(sig8, &handler_sig8, &handler);
    if(ret_val != SIGHANDLER_SIGNAL_INSTALL_SUCCESS){
        kprintf("test SIGHANDLER_SIGNAL_INSTALL_SUCCESS fail\n");
        while(1);
    }

    ret_val = syssighandler(sig30, &handler_sig30, &handler);
    if(ret_val != SIGHANDLER_SIGNAL_INSTALL_SUCCESS){
        kprintf("test SIGHANDLER_SIGNAL_INSTALL_SUCCESS fail\n");
        while(1);
    }

    ret_val = syskill(4, sig8);
    if(ret_val != SIGKILL_INVALID_PID){
        kprintf("test SIGKILL_INVALID_PID1 fail\n");
        while(1);
    }

    ret_val = syskill(pid1, -1);
    if(ret_val != SIGKILL_INVALID_SIGNUM){
        kprintf("test SIGKILL_INVALID_SIGNUM1 fail\n");
        while(1);
    }

    ret_val = syskill(pid1, MAX_SIG);
    if(ret_val != SIGKILL_INVALID_SIGNUM){
        kprintf("test SIGKILL_INVALID_SIGNUM2 fail\n");
        while(1);
    }

    ret_val = syskill(pid1, sig8);
    if(ret_val != SIGKILL_SUCCESS){
        kprintf("test SIGKILL_SUCCESS fail1\n");
        while(1);
    }

    ret_val = syskill(pid1, sig30);
    if(ret_val != SIGKILL_SUCCESS){
        kprintf("test SIGKILL_SUCCESS fail2\n");
        while(1);
    }

    kprintf("all test_syskill1 test cases passed...\n");
    while(1);
}

void test_signal_priority(){
    int sig8  = 8;
    int sig30 = 30;
    int sig0  = 0;

    int pid1 = 1;
    int pid2 =  syscreate( &test_process, 4096 );
    syssleep(500);

    void *handler;
    int ret_val = syskill(pid2, sig8);
    if(ret_val != SIGKILL_SUCCESS){
        kprintf("test SIGKILL_SUCCESS fail1\n");
        while(1);
    }


    ret_val = syskill(pid2, sig0);
    if(ret_val != SIGKILL_SUCCESS){
        kprintf("test SIGKILL_SUCCESS fail2\n");
        while(1);
    }

    ret_val = syskill(pid2, sig30);
    if(ret_val != SIGKILL_SUCCESS){
        kprintf("test SIGKILL_SUCCESS fail3\n");
        while(1);
    }

    kprintf("all test_signal_priority test cases passed...\n");
    while(1){
        sysyield();
    }
}

void test_process(){
    int sig8  = 8;
    int sig30 = 30;
    int sig0  = 0;

    void *handler;
    int ret_val = syssighandler(sig8, &handler_sig8, &handler);
    if(ret_val != SIGHANDLER_SIGNAL_INSTALL_SUCCESS){
        kprintf("test SIGHANDLER_SIGNAL_INSTALL_SUCCESS1 fail\n");
        while(1);
    }

    ret_val = syssighandler(sig30, &handler_sig30, &handler);
    if(ret_val != SIGHANDLER_SIGNAL_INSTALL_SUCCESS){
        kprintf("test SIGHANDLER_SIGNAL_INSTALL_SUCCESS2 fail\n");
        while(1);
    }

    ret_val = syssighandler(sig0, &handler_sig0, &handler);
    if(ret_val != SIGHANDLER_SIGNAL_INSTALL_SUCCESS){
        kprintf("test SIGHANDLER_SIGNAL_INSTALL_SUCCESS3 fail\n");
        while(1);
    }

    ret_val = syswait(1);
    if(ret_val != SIGNAL_INTERRUPT){
        kprintf("test SIGNAL_INTERRUPT fail\n");
        while(1);
    }
    while(1);
}

void *handler_sig8(void *cntx){
    kprintf("signal 8 executed\n");
}

void *handler_sig30(void *cntx){
    kprintf("signal 30 executed\n");
}

void *handler_sig0(void *cntx){
    kprintf("signal 0 executed\n");
}

void a(void) {
    sysputs("hi");
}

void t(void) {
    while (1) {
        sysputs("\nT\n");
        syssleep(10000);
    }
}

void alarm(void){
    sysputs("ALARM ALARM ALARM\n");
    void *handler;
    syssighandler(18, NULL, &handler);
}

void alarm_proc(void){
    syssleep(alarm_wait_time);
    syskill(shell_id, 18);
}

void shell(void) {

    int sig8  = 8;
    int sig30 = 30;
    int sig0  = 0;

    void *handler;
    int ret_val = syssighandler(sig8, &handler_sig8, &handler);
    if(ret_val != SIGHANDLER_SIGNAL_INSTALL_SUCCESS){
        kprintf("test SIGHANDLER_SIGNAL_INSTALL_SUCCESS1 fail\n");
        while(1);
    }

    ret_val = syssighandler(sig30, &handler_sig30, &handler);
    if(ret_val != SIGHANDLER_SIGNAL_INSTALL_SUCCESS){
        kprintf("test SIGHANDLER_SIGNAL_INSTALL_SUCCESS2 fail\n");
        while(1);
    }

    ret_val = syssighandler(sig0, &handler_sig0, &handler);
    if(ret_val != SIGHANDLER_SIGNAL_INSTALL_SUCCESS){
        kprintf("test SIGHANDLER_SIGNAL_INSTALL_SUCCESS3 fail\n");
        while(1);
    }

    // array for remembering process ids
    int proc_ids[MAX_PROC];

    // open keyboard
    int fd = sysopen(0);
    sysioctl(fd, CTL_ECHO_ON);
    while(1) {
        unsigned char output[20];


        sysputs("> ");

        unsigned char command[20] = "";
        int written = sysread(fd, &command, 20);

        int background = 0;

        // check if & at end
        if (*(command+written-2) == '&') {
            sysputs("proc in background\n");
            background = 1;
        }

        // eof key pressed
        if (written == 0) {
            sysclose(fd);
            break;
        }
        else if (!strcmp(command, "ps\n")) {
            // list active processes
            sysputs("PID        Status        Duration\n");
            processStatuses psTab;
            int procs = sysgetcputimes(&psTab);

            for(int j = 0; j <= procs; j++) {
                sprintf(output, "%2d    %10d    %10d\n", psTab.pid[j], ps_states[psTab.status[j]], psTab.cpuTime[j]);
                sysputs(output);
            }
        }
        // exit shell
        else if (!strcmp(command, "ex\n") || !strcmp(command, "ex&\n")) {
            sysclose(fd);
            xit = TRUE;
            break;
        }

        // kill process
        else if (!strncmp(command, "k ", 2)) {
            // call syskill on the target process
            int result = syskill( atoi(command + 1), MAX_SIG-1);
            if (result != 0)
                sysputs("No such process\n");
        }
        else if (!strncmp(command, "k& ", 3)) {
            // call syskill on the target process
            int result = syskill( atoi(command + 2), MAX_SIG-1);
            if (result != 0)
                sysputs("No such process\n");
        }
        else if (!strncmp(command, "a ", 2)) {
            int pid = syscreate(&alarm_proc, PROC_STACK);
	    shell_id = sysgetpid();
            alarm_wait_time = atoi(command + 1);
            syssighandler(18, &alarm, &handler);
            syswait(pid);
        }
        else if (!strncmp(command, "a& ", 3)){
            int pid = syscreate(&alarm_proc, PROC_STACK);
	    shell_id = sysgetpid();
            alarm_wait_time = atoi(command + 2);
            syssighandler(18, &alarm, &handler);
        }
        else if (!strcmp(command, "t\n") || !strcmp(command, "t&\n")) {
            int pid = syscreate(&t, PROC_STACK);
            if (!background) {
                syswait(pid);
            }
        }
    }
}


void root(void) {
    unsigned char username[20] = "";
    unsigned char password[20] = "";
    char* correct_username = "cs415\n"; 
    char* correct_password = "EveryonegetsanA\n";
    while(1){
        if (!strcmp(correct_username, username) && !strcmp(correct_password, password)) {
            // create shell, wait for exit
            int shell_pid = syscreate(&shell, PROC_STACK );
            syswait(shell_pid);
            if(!xit){
                continue;
            }
            xit = FALSE;
        }
        sysputs("Welcome to Xeros - a not so experimental OS\n");
        unsigned char output[20];
        /* char* correct_username = "cs415\n"; */
        /* char* correct_password = "EveryonegetsanA\n"; */

               //char* correct_username = "a\n";
        //char* correct_password = "a\n";
        username[20] = "";
        password[20] = ""; 

        // open keyboard
        int fd = sysopen(0);

        // turn on keyboard echo
        sysioctl(fd, CTL_ECHO_ON, 0);

        sysputs("Username:");
        sysread(fd, &username, 20);

        /* sprintf(output, "buffer contents: %s\n", &username); */
        /* sysputs(output); */

        // turn off keyboard echo
        sysioctl(fd, CTL_ECHO_OFF, 0);

        sysputs("\nPassword:");
        sysread(fd, &password, 20);

        // close keyboard
        fd = sysclose(0);

        sprintf(output, "%s", username);
        sysputs(output);

        sprintf(output, "%s", password);
        sysputs(output);
    }
}

