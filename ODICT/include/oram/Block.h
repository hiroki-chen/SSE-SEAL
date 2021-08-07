#ifndef PORAM_BLOCK_H
#define PORAM_BLOCK_H

#include <algorithm>
#include <vector>

#include <cereal/access.hpp>
#include <cereal/archives/binary.hpp>
#include <cereal/types/string.hpp>

#define BLOCK_SIZE 2

class Block {
    /**
     * Intrusive serialize helper. The interface is the same as what is provided by the boost libarary.
     */
    friend class cereal::access;
    template <class Archive>
    void serialize(Archive& ar)
    {
        ar(leaf_id, index, data);
    }

public:
    /**
     * @brief To which leaf it belongs.
     */ 
    int leaf_id;

    /**
     * @brief The address (plain, without oram)
     */ 
    int index;

    /**
     * @brief Variable data.
     * @note To store class object as a string, use the serialize helper provided by the util header.
     */ 
    std::string data;

    Block();

    Block(const Block& block);

    Block(const int& leaf_id, const int& index, const std::string& data);

    void printBlock();

    virtual ~Block();
};

#endif //PORAM_BLOCK_H
