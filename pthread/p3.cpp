#include <iostream>
#include <map>

#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <sched.h>

using namespace std;

void * worker(void *);

typedef map<pthread_t, unsigned int>::iterator pool_iter_t;
map<pthread_t, unsigned int> pool;

pthread_barrier_t start;
pthread_barrier_t stop;
pthread_mutex_t mutex;

const int EOK = 0;
const int NUM = 6;

int main(void){

    pthread_t tid;

    if( EOK != pthread_barrier_init(&start, NULL, NUM + 1) ){
        perror("init condition");
        exit(1);
    }

    if( EOK != pthread_barrier_init(&stop, NULL, NUM) ){
        perror("init condition");
        exit(1);
    }

    if( EOK != pthread_mutex_init(&mutex, NULL) ){
        perror("init mutex");
        exit(1);
    }

    for(unsigned int i = 0; i < NUM; i++){
        pthread_create(&tid, NULL, worker, NULL);
        pool.insert(make_pair(tid, i));
    }
    
    cout << "MAIN: all threads are created. Sleep 5 sec..." << endl;
    sleep(5);
    cout << "MAIN: release barrier in 1 sec..." << endl;
    sleep(1);
    pthread_barrier_wait(&start); 

    for(pool_iter_t it = pool.begin(); it != pool.end(); it++){
        pthread_join(it->first, NULL);
    }

}

void * worker(void * data){
    
    pthread_mutex_lock(&mutex);

    unsigned int id = 0;
    pool_iter_t it = pool.find(pthread_self());
    
    if( it != pool.end() ){
        id = it->second;
    }else{
        cerr << "ERROR: can't get index of thread " << pthread_self() << endl;
    }
    
    pthread_mutex_unlock(&mutex);

    cout << "----> thread " << id << endl;
    cout << "      thread " << id << " wait start" << endl;
    
    pthread_barrier_wait(&start);

    cout << "      thread " << id << " released" << endl;

    // work to do
    
    cout << "      thread " << id << " active" << endl;
    
    for(int i = 0; i < 3; i++){ 
        sched_yield();        
    }

    cout << "      thread " << id << " waits on stop barrier" << endl;
    pthread_barrier_wait(&stop);

    cout << "<---- thread " << id << endl;
    return NULL;
}

