#include <Client.h>

#include <cstring>
#include <cassert>
#include <random>
#include <iostream>

int main(int argc, const char **args)
{
    std::cout << "test case num: " << std::endl;
    int number;
    std::cin >> number;
    SEAL::Client * client = new SEAL::Client(256, 256, sizeof(ODict::Node), 1024, INT_MAX);

    std::cout << client->add_node(number) << std::endl;

    return 0;
}