#include <csignal>

#include <string>
#include <iostream>

using namespace std;


void sig_handler(int signo){
    cout << ">>> Got signal " << signo << endl;
    cout << ">>> Sleep 3 sec inside signal handler" << endl;
    sleep(3);
}

void sig_alarm(int signo){
    cout << " ALARM " << endl;
    exit(0);
}

int main(void){
        
    struct sigaction act, alrm;
    
    sigfillset( &(act.sa_mask) );
    act.sa_flags = 0;
    act.sa_handler = &sig_handler;
    sigaction(SIGINT, &act, NULL);

    sigfillset( &(alrm.sa_mask) );
    alrm.sa_flags = 0;
    alrm.sa_handler = &sig_alarm;
    sigaction(SIGALRM, &alrm, NULL);

    alarm(10);

    while( true ){ 
        cout << " Waiting for something" << endl;
        sleep(2);
    }


}
