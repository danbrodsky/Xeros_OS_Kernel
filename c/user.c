/* user.c : User processes
 */

#include <xeroskernel.h>
#include <xeroslib.h>

extern char	*maxaddr;
void *handler_sig8(void *cntx);
void *handler_sig30(void *cntx);
void *handler_sig0(void *cntx);
void test_process();

/*
void busy( void ) {
  int myPid;
  char buff[100];
  int i;
  int count = 0;

  myPid = sysgetpid();
  
  for (i = 0; i < 10; i++) {
    sprintf(buff, "My pid is %d\n", myPid);
    sysputs(buff);
    if (myPid == 2 && count == 1) syskill(3);
    count++;
    sysyield();
  }
}



void sleep1( void ) {
  int myPid;
  char buff[100];

  myPid = sysgetpid();
  sprintf(buff, "Sleeping 1000 is %d\n", myPid);
  sysputs(buff);
  syssleep(1000);
  sprintf(buff, "Awoke 1000 from my nap %d\n", myPid);
  sysputs(buff);
}



void sleep2( void ) {
  int myPid;
  char buff[100];

  myPid = sysgetpid();
  sprintf(buff, "Sleeping 2000 pid is %d\n", myPid);
  sysputs(buff);
  syssleep(2000);
  sprintf(buff, "Awoke 2000 from my nap %d\n", myPid);
  sysputs(buff);
}



void sleep3( void ) {
  int myPid;
  char buff[100];

  myPid = sysgetpid();
  sprintf(buff, "Sleeping 3000 pid is %d\n", myPid);
  sysputs(buff);
  syssleep(3000);
  sprintf(buff, "Awoke 3000 from my nap %d\n", myPid);
  sysputs(buff);
}

 */







/*
void producer( void ) {

    int         i;
    char        buff[100];


    // Sping to get some cpu time
    for(i = 0; i < 100000; i++);

    syssleep(3000);
    for( i = 0; i < 20; i++ ) {
      
      sprintf(buff, "Producer %x and in hex %x %d\n", i+1, i, i+1);
      sysputs(buff);
      syssleep(1500);

    }
    for (i = 0; i < 15; i++) {
      sysputs("P");
      syssleep(1500);
    }
    sprintf(buff, "Producer finished\n");
    sysputs( buff );
    sysstop();
}

void consumer( void ) {

    int         i;
    char        buff[100];

    for(i = 0; i < 50000; i++);
    syssleep(3000);
    for( i = 0; i < 10; i++ ) {
      sprintf(buff, "Consumer %d\n", i);
      sysputs( buff );
      syssleep(1500);
      sysyield();
    }

    for (i = 0; i < 40; i++) {
      sysputs("C");
      syssleep(700);
    }

    sprintf(buff, "Consumer finished\n");
    sysputs( buff );
    sysstop();
}

void     root( void ) {

    char  buff[100];
    int pids[5];
    int proc_pid, con_pid;
    int i;

    sysputs("Root has been called\n");


    // Test for ready queue removal. 
   
    proc_pid = syscreate(&busy, 1024);
    con_pid = syscreate(&busy, 1024);
    sysyield();
    syskill(proc_pid);
    sysyield();
    syskill(con_pid);

    
    for(i = 0; i < 5; i++) {
      pids[i] = syscreate(&busy, 1024);
    }

    sysyield();
    
    syskill(pids[3]);
    sysyield();
    syskill(pids[2]);
    syskill(pids[4]);
    sysyield();
    syskill(pids[0]);
    sysyield();
    syskill(pids[1]);
    sysyield();

    syssleep(8000);;



    kprintf("***********Sleeping no kills *****\n");
    // Now test for sleeping processes
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);

    sysyield();
    syssleep(8000);;



    kprintf("***********Sleeping kill 2000 *****\n");
    // Now test for removing middle sleeping processes
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);

    syssleep(110);
    syskill(pids[1]);
    syssleep(8000);;

    kprintf("***********Sleeping kill last 3000 *****\n");
    // Now test for removing last sleeping processes
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);

    sysyield();
    syskill(pids[2]);
    syssleep(8000);;

    kprintf("***********Sleeping kill first process 1000*****\n");
    // Now test for first sleeping processes
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);

    syssleep(100);
    syskill(pids[0]);
    syssleep(8000);;

    // Now test for 1 process


    kprintf("***********One sleeping process, killed***\n");
    pids[0] = syscreate(&sleep2, 1024);

    sysyield();
    syskill(pids[0]);
    syssleep(8000);;

    kprintf("***********One sleeping process, not killed***\n");
    pids[0] = syscreate(&sleep2, 1024);

    sysyield();
    syssleep(8000);;



    kprintf("***********Three sleeping processes***\n");    // 
    pids[0] = syscreate(&sleep1, 1024);
    pids[1] = syscreate(&sleep2, 1024);
    pids[2] = syscreate(&sleep3, 1024);


    // Producer and consumer started too
    proc_pid = syscreate( &producer, 4096 );
    con_pid = syscreate( &consumer, 4096 );
    sprintf(buff, "Proc pid = %d Con pid = %d\n", proc_pid, con_pid);
    sysputs( buff );


    processStatuses psTab;
    int procs;
    



    syssleep(500);
    procs = sysgetcputimes(&psTab);

    for(int j = 0; j <= procs; j++) {
      sprintf(buff, "%4d    %4d    %10d\n", psTab.pid[j], psTab.status[j], 
	      psTab.cpuTime[j]);
      kprintf(buff);
    }


    syssleep(10000);
    procs = sysgetcputimes(&psTab);

    for(int j = 0; j <= procs; j++) {
      sprintf(buff, "%4d    %4d    %10d\n", psTab.pid[j], psTab.status[j], 
	      psTab.cpuTime[j]);
      kprintf(buff);
    }

    sprintf(buff, "Root finished\n");
    sysputs( buff );
    sysstop();
    
    for( ;; ) {
     sysyield();
    }
    
}

 */


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


