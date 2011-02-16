#include <iostream>
#include <vector>
#include <string>
#include <map>

#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>

using namespace std;

void create_key(void);
void exit_msg(void *);
void * worker(void *);
void handler(int, siginfo_t *, void *);

//

pthread_once_t once = PTHREAD_ONCE_INIT;
pthread_key_t key;

const int EOK = 0;

//

typedef struct td{
    unsigned int id;
    pthread_t tid;
    td() : id(0), tid(0) { };
    td(unsigned int i, pthread_t t) : id(i), tid(t) { };
}td;

//

typedef map<pthread_t, unsigned int>::iterator pool_iter_t;
map<pthread_t, unsigned int> pool;
vector<td*> gc;

//

int main(void){

    sigset_t sig;
    pthread_t tid;
    struct sigaction act;

    // init handler
    act.sa_sigaction = &handler;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);

    // block USR signals in main thread
    sigemptyset(&sig);
    sigaddset(&sig, SIGUSR1);
    sigaddset(&sig, SIGUSR2);
    sigprocmask(SIG_BLOCK, &sig, NULL);

    // create threads
    pthread_create(&tid, NULL, worker, (void *) SIGUSR1);
    pool.insert(make_pair(tid, 1));
    
    pthread_create(&tid, NULL, worker, (void *) SIGUSR2);
    pool.insert(make_pair(tid, 2));
    
    cout << "MAIN: all threads are created. Sleep 1 sec..." << endl;
    sleep(1);
    
    // send signals to threads
    for(int i = 0; i < 3; i++){
        cout << "MAIN: send signals to threads..." << endl;
        kill(getpid(), SIGUSR1);
        kill(getpid(), SIGUSR2);
        cout << "MAIN: sleep 1 sec..." << endl;
        sleep(1);
    }

    // wait for all the threads

    for(pool_iter_t it = pool.begin(); it != pool.end(); it++){
        cout << "MAIN: cancel thread " << it->first << endl;
        pthread_cancel(it->first);
    }

    // clean thread-specific data storage
    for(int i = 0; i < gc.size(); i++){
        delete gc[i];
    }

    cout << "MAIN: wait 2 sec for worker threads to be cancelled..." << endl;
    sleep(2);
    cout << "MAIN: bye..." << endl;
    return 0;
}

//

void exit_msg(void * p){
    cout << ">>> tread " << int(p) << " exits..." << endl;
}

void create_key(void){
    pthread_key_create(&key, NULL);
}

void * worker(void * s){
    
    sigset_t sig;
    int signo = int(s);
    
    // set cancel actions
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

    // init thread-specific data
    unsigned int id = 0;
    pool_iter_t it = pool.find(pthread_self());
    
    if( it != pool.end() ){
        id = it->second;
    }else{
        cerr << "ERROR: can't get index of thread " << pthread_self() << endl;
    }

    pthread_once(&once, create_key);
    td * data = (td*) pthread_getspecific(key);
    if( data == NULL){
        data = new td(id, pthread_self());
        pthread_setspecific(key, (void*)data);
        gc.push_back(data);
    }

    // set signal mask
    sigfillset(&sig);
    sigprocmask(SIG_BLOCK, &sig, NULL);

    sigemptyset(&sig);
    sigaddset(&sig, signo);
    sigprocmask(SIG_UNBLOCK, &sig, NULL);
    
    // 

    cout << ">>> thread " << id << " waits for signal " << signo << endl;
    
    pthread_cleanup_push(&exit_msg, (void *)id);
    
    while( true ){
        sleep(1);
        pthread_testcancel();
    }
    
    pthread_cleanup_pop(0);
    
    return NULL;
}

void handler(int s, siginfo_t * t, void * p){

    int id = 0;
    pthread_t tid = 0;

    td * data = (td*) pthread_getspecific(key);
    
    if( data != NULL){
        id = data->id;
        tid = data->tid;
    }

    cout << "thread " << id << "(" << tid << ")" << " got signal " << s << endl;
}
