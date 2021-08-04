#ifndef PORAM_BLOCK_H
#define PORAM_BLOCK_H

#include <algorithm>

using namespace std;

class Block {
public:
    static const int BLOCK_SIZE = 2;
    int leaf_id;
    int index;
    int* data;

    Block();
    Block(int leaf_id, int index, int data[]);
    void printBlock();
    virtual ~Block();
};

#endif //PORAM_BLOCK_H
