//
// Created by Administrator on 2020/3/12.
//

#ifndef MEMLEAK_MEMRECORD_H
#define MEMLEAK_MEMRECORD_H

#include <iostream>
#include <set>

#include <new>          //for placement new
#include <cstddef>      //for ptrdiff_t, size_t
#include <cstdlib>      //for exit()
#include <climits>      //for UINT_MAX
#include <iostream>     //for cerr
#include <mutex>        //for opt MRList.add();
#include <atomic>

#ifndef WIN32
    #include <dlfcn.h>
#else
    #include <windows.h>
#endif

using namespace std;

typedef void* (*_malloc_fun_t)(size_t);
typedef void (*_free_fun_t)(void *);
static _malloc_fun_t sys_malloc=nullptr;
static _free_fun_t sys_free=nullptr;

static void init_malloc_fun_ptr(){
    if(sys_malloc==nullptr){
#ifndef WIN32
        sys_malloc=reinterpret_cast<_malloc_fun_t>(dlsym(RTLD_NEXT,"malloc"));
//        std::cout<<std::hex<<sys_malloc<<std::endl;
#else
        HMODULE hmodule=LoadLibrary("msvcrt.dll");

        if(hmodule){
            sys_malloc=reinterpret_cast<_malloc_fun_t>(GetProcAddress(hmodule,"malloc"));
//            std::cout<<std::hex<<sys_malloc<<std::endl;
        }else{
            sys_malloc=nullptr;
        }

#endif
    }
    if(sys_malloc==nullptr){
        std::cout<<"sys_malloc config failed."<<std::endl;
        exit(2);
    }
}
static void init_free_fun_ptr(){
    if(sys_free==nullptr){
#ifndef WIN32
        sys_free=reinterpret_cast<_free_fun_t>(dlsym(RTLD_NEXT,"free"));
//        std::cout<<std::hex<<sys_free<<std::endl;
#else
        HMODULE hmodule=LoadLibrary("msvcrt.dll");
        if(hmodule){
            sys_free=reinterpret_cast<_free_fun_t>(GetProcAddress(hmodule,"free"));
//            std::cout<<std::hex<<sys_free<<std::endl;
        }else{
            sys_free=nullptr;
        }
#endif
    }
    if(sys_free==nullptr){
        std::cout<<"sys_free config failed."<<std::endl;
        exit(2);
    }
}

namespace oyas{

    struct MRListNode{
        MRListNode *pre,*next;
        void *val;
        void init(void *p=nullptr){
            pre=nullptr;
            next=nullptr;
            val=p;
        }
    };

    class MemRecord{
        friend void* malloc(size_t);
        friend void free(void*);
    public:
        MemRecord(){
            init_malloc_fun_ptr();
            init_free_fun_ptr();
            mrHead=(MRListNode*)sys_malloc(sizeof(MRListNode));
            mrHead->init();
            m=0;
            f=0;
        }
        ~MemRecord(){
            if(mrHead->next!=nullptr){
                MRListNode *node=mrHead->next;
                int num=0;
                while(node){
                    std::cout<<"MEM leak at address "<<"#"<<++num<<": "<<node->val<<std::endl;
                    sys_free(node->val);
                    node=node->next;
                }
            }
            sys_free(mrHead);
            std::cout<<"called malloc: "<<m<<" times, called free: "<<f<<" times."<<std::endl;
        }
        void* add_node(void* p){
            MRListNode *node=(MRListNode*)sys_malloc(sizeof(MRListNode));
            m++;
            node->init(p);
            std::unique_lock<std::mutex>(mtx1);  // scope *1-*2
            node->next=mrHead->next;            // *1
            mrHead->next=node;                  // *2
            node->pre=mrHead;
            if(node->next!=nullptr){
                node->next->pre=node;
            }
            return p;
        }
        void remove_node(void* p){
            std::unique_lock<std::mutex>(mtx2);
            f++;
            MRListNode *node=mrHead->next;
            while(node){
                if(node->val==p){
                    node->pre->next=node->next;
                    if(node->next!=nullptr){
                        node->next->pre=node->pre;
                    }
                    sys_free(node);
                    return;
                }
                node=node->next;
            }
            std::cout<<"WARNING: "<<p<<" is not a ptr from malloc."<<std::endl;
        }
        void* _malloc(size_t size){
            return add_node(sys_malloc(size));
        }
        void _free(void *p){
            remove_node(p);
            sys_free(p);
            return;
        }
    private:
        MRListNode *mrHead;
        std::mutex mtx1,mtx2;
        std::atomic<int> m,f;
    };

}

oyas::MemRecord _MR_;

void* malloc(size_t size){
//    init_malloc_fun_ptr(); //move in MemRecord.ctor
/*  move in MemRecord._malloc;
    if(sys_malloc){
        return sys_malloc(size);
    }else{
    std::cout<<"sys_malloc conf failed."<<std::endl;
    exit(2);
    return nullptr;
    }
*/
    std::cout<<"oyas.malloc"<<std::endl;
    return _MR_._malloc(size);
}
void free(void *p){
//    init_free_fun_ptr();  //move in MemRecord.ctor
/*  move in MemRecord._free
    if(sys_free){
        sys_free(p);
        return;
    }else{
    std::cout<<"sys_free"<<std::endl;
        exit(2);
    }
*/
    std::cout<<"oyas.free"<<std::endl;
    _MR_._free(p);
    return;
}


#endif //MEMLEAK_MEMRECORD_H
