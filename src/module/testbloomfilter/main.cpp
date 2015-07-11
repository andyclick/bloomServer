#include "BloomFilterManager.h"
#include <iostream>

using namespace octopus::b_server;

int main(int argc, char** argv)
{
    BloomFilterManager testBloom;
    testBloom.initialize();
    std::cout << "Mark." << std::endl;
    return 0;
}
