#include <iostream>
#include <string>

typedef struct
{
    char frame[4];   
    int len;

}EMBADEDSTRUCT;


typedef struct {
    // EMBADEDSTRUCT frame;
    int a;
    int b;
    // std::string c;
    short d;
    char e;
    char f[60];
}TESTSTRUCT;
int main(int argc, char * argv[]) {
    auto a1 = TESTSTRUCT{};
    auto a2 = TESTSTRUCT{123};
    auto a3 = TESTSTRUCT{123, 121};
    TESTSTRUCT a4;
    // auto a5 = TESTSTRUCT{123, 121, "sstring", 1, 'e', "fffff"};

    // a5.print();
    return 0;
}