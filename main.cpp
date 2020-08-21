#include "cpu.h"
#include <iostream>
using namespace std;
int main()
{
    CPU i8080("invaders.rom");
    i8080.run();
    return 0;
}
