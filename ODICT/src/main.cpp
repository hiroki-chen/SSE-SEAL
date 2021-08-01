#include <Client.h>
#include <utils.h>

#include <cassert>
#include <cstring>
#include <iostream>
#include <random>

/**
 * Bug: We cannot evict the cache when it is full because evicting a node means to not only update pos_tag for it,
 *      but also update its parent's pos_tags.
 */
int main(int argc, const char** args)
{
    std::cout << "test case num: " << std::endl;
    int number;
    std::cin >> number;
    // Every time the client will sample a new key from the password.
    SEAL::Client* client = new SEAL::Client(256, 256, sizeof(ODict::Node), 1024, INT_MAX, 5, 4, "123456789");

    client->test_adj("./input/test.txt");

    return 0;
}