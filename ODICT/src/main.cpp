#include <Client.h>

#include <cstring>
#include <cassert>
#include <random>
#include <iostream>

/**
 * Bug: We cannot evict the cache when it is full because evicting a node means to not only update pos_tag for it,
 *      but also update its parent's pos_tags.
 */ 
int main(int argc, const char **args)
{
    std::cout << "test case num: " << std::endl;
    int number;
    std::cin >> number;
    SEAL::Client * client = new SEAL::Client(256, 256, sizeof(ODict::Node), 1024, INT_MAX);

    std::cout << client->add_node(number) << std::endl;

    return 0;
}