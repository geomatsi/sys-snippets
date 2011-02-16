#include <string>
#include <iostream>

#include <errno.h>
#include <signal.h>

using namespace std;

int main(void){
    cout << "SIGNO" << endl;

    for(int i = 1; i <= SIGRTMAX; i++){
        
        if( i % 8 == 1 ){
            cout << endl << i << ":";
        }

        int res = sigaction(i, NULL, NULL);
        cout << "\t" << ( (res != 0 && errno == EINVAL) ? '-' : '+' );
    }

    cout << endl;    
    return 0;
}

