#include <iostream>
#include <vector>
#include <string>

#include <pthread.h>

using namespace std;

//

void * tprc(void *);
void create_key(void);

//

pthread_once_t once = PTHREAD_ONCE_INIT;
pthread_key_t key;

typedef struct td{
    string key;
    pthread_t val;
    td() : key(""), val(0) { };
    td(string k, pthread_t v) : key(k), val(v) { };
}td;

vector<pthread_t> pool;
vector<td *> gc;
 
//

int main(void){

   pthread_t tid;

    for(int i = 0; i < 5; i++){
        pthread_create(&tid, NULL, tprc, NULL);
        pool.push_back(tid);
    }

    for(int i = 0; i < pool.size(); i++){
        pthread_join(pool[i], NULL);
    }

    for(int i = 0; i < gc.size(); i++){
        delete gc[i];
    }

}

void create_key(void){
    pthread_key_create(&key, NULL);
}

void * tprc(void * t){
    
    cout << "----> thread " << pthread_self() << endl;
    
    pthread_once(&once, create_key);
    td * data = (td*) pthread_getspecific(key);
    if( data == NULL){
        data = new td("id", pthread_self());
        pthread_setspecific(key, (void*)data);
        gc.push_back(data);
    }
    cout << "   >> data->str = " << data->key << endl;
    cout << "   >> data->val = " << data->val << endl;
    cout << "<---- thread " << pthread_self() << endl;

}

