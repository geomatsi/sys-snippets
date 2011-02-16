#include <iostream>
#include <vector>

#include <pthread.h>

using namespace std;

void * tprc(void *);

int V1 = 0;
int V2 = 0;

int main(void){

    vector<pthread_t> pool;
    pthread_t tid;

    for(int i = 0; i < 5; i++){
        pthread_create(&tid, NULL, tprc, NULL);
        pool.push_back(tid);
    }

    for(int i = 0; i < pool.size(); i++){
        pthread_join(pool[i], NULL);
    }

    cout << "V1 = " << V1 << endl;
    cout << "V2 = " << V2 << endl;
}

void * tprc(void * data){
    
    cout << "----> thread " << pthread_self() << endl;
    
    for(int i = 0; i < 5; i++){
        V1 += 2;
        V2++;
        cout << "   >> thread " << pthread_self() << endl;
        V1 -= 2;
    }

    cout << "<---- thread " << pthread_self() << endl;

}

