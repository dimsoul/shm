#include <iostream>
#include <string>

#include "../include/shm.hpp"

using namespace std;

constexpr int ProjectName = 4;

struct A 
{
    int a;
    int b;
};

int main () 
{
    try {
        auto pa = shm<A,false>("./", 0666, ProjectName, 6);
        pa.unmapShm();
    } catch (const std::exception& e) {
        cout << e.what() << endl;
    }
    return 0;
}