/*************************************************************************
    > File Name: vec.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Wed 18 Nov 2015 05:34:07 PM CST
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include <iostream>
#include <vector>
#include <cstddef>

#include <new>

using namespace std;

template <class T>
inline T* allocate(ptrdiff_t size, T*) {
    set_new_handler(0);
    T* tmp = (T*)(::operator new((size_t)(size * sizeof(T))));
    if (tmp == 0) {
        cerr << "out of memory" << endl; 
        exit(1);
    }
    return tmp;
}

template <class T>
inline void deallocate(T* buffer) {
    ::operator delete(buffer);
}

class Obj
{
    static int i;
    int idx;
    bool copy;
    public:
        Obj() : copy(false) {
            idx = i ++;
            std::cout << "constructor:" << idx << std::endl;
        }
        ~Obj() {
            std::cout << "deconstructor:" << idx << std::endl;
        }

        Obj(const Obj& rhs) : copy(true) , idx(rhs.idx) {
            std::cout << "copy constructor:" << idx << std::endl;
        }

        void show() {
            std::cout << "show:" << idx << std::endl;
        }
};

int Obj::i = 0;

int main(int argc, char * argv[])
{
    std::vector<Obj> v;
    std::cout << sizeof(v) << std::endl;
    v.resize(4);
    std::cout << sizeof(v) << std::endl;

    for (int i = 0; i < v.capacity(); i++)
        v[i].show();

    std::cout << v.size() << ", " << v.capacity() << std::endl;
    for (int i = 0; i < 5; i++)
        v.push_back(Obj());

    std::cout << v.size() << ", " << v.capacity() << std::endl;
    v.clear();
    std::cout << v.size() << ", " << v.capacity() << std::endl;

    for (int i = 0; i < v.capacity(); i++)
        v[i].show();

    std::cout << "resize" << std::endl;
    std::vector<Obj> * t = new std::vector<Obj>();
    t->swap(v);
    delete t;
    std::cout << v.size() << ", " << v.capacity() << std::endl;

    std::cout << "------------" << std::endl;
    Obj * ptr = (Obj *)malloc(sizeof(Obj) * 4);
    std::cout << "------------" << std::endl;

    Obj obj;
    for (int i = 0; i < 4; i++)
        *(ptr + i) = obj;

    std::cout << "------------" << std::endl;
    for (int i = 0; i < 4; i++)
        (ptr + i)->show();

    std::cout << "------------" << std::endl;
    free((void *)ptr);

    std::cout << "------------" << std::endl;
    Obj * aptr = ::allocate(1, (Obj *)0);
    new ((void *)aptr) Obj();
    aptr->show();
    aptr->~Obj();

    ::deallocate(aptr);
}

