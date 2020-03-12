#include <iostream>
#include <vector>
#include "src/MemRecord.h"
#include <thread>
using namespace std;
volatile bool flag=false;
void thfunn(){
    while(!flag){
        ;
    }
    vector<double*> vc;
    for(int i=0;i<100;i++){
        vc.push_back(new double(i*0.53));
    }
    for(auto e:vc){
        delete e;
    }
    int *p=new int(0);
    return;
}

int main() {
//    std::cout << "Hello, World!" << std::endl;
    vector<thread> vc;

    for(int i=0;i<15;i++){
        vc.push_back(std::thread(thfunn));
    }
    flag=true;
    for(auto &e:vc){
        e.join();
    }
    return 0;
}
