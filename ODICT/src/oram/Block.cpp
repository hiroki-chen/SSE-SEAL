#include <oram/Block.h>

#include <iostream>
#include <sstream>
#include <string>

using namespace std;

Block::Block()
{ // dummy index
    this->leaf_id = -1;
    this->index = -1;
}

Block::Block(int leaf_id, int index, int data[])
    : leaf_id(leaf_id)
    , index(index)
{
}

Block::~Block()
{
    // dtor
}

void Block::printBlock()
{
    string data_holder = "";
    for (unsigned int i = 0; i < BLOCK_SIZE; i++) {
        data_holder += to_string(this->data[i]);
        data_holder += " ";
    }
    cout << "index: " << to_string(this->index)
         << " leaf id: " << to_string(this->leaf_id) << " data: " << data_holder
         << endl;
}