#ifndef ORAM_ACCESS_CONTROLLER_H_
#define ORAM_ACCESS_CONTROLLER_H_

#include <oram/OramInterface.h>
#include <oram/PathORAM.h>
#include <oram/RandForOramInterface.h>
#include <oram/UntrustedStorageInterface.h>

class OramAccessController {
private:
    UntrustedStorageInterface* storage;

    RandForOramInterface* random;

    OramInterface* oram;

    int block_size;

public:
    /**
     * @deprecated
     * @brief The normal way to access the PathORAM.
     * 
     * @param op the operation: READ / WRITE. INSERT / DELETE are not used in this interface.
     * @param address the physical address.
     * @param data the data to be read / written.
     */
    void
    oblivious_access(OramAccessOp op, const int& address, unsigned char* data);

    /**
     * @deprecated
     * @brief The special way to access the PathORAM. For oblviious data structures.
     * 
     * @param op the operation: READ/ WRITE. INSERT / DELETE are not used in this interface either.
     * @param data the data to be read / written. @note It must contain pos_tag.
     */
    void oblivious_access_direct(OramAccessOp op, unsigned char* data);

    /**
     * @brief Sample a new position in advance for oblivious data sturctures.
     * 
     * @return A random new position for a leaf node.
     */
    int random_new_pos();

    /**
     * @brief The constructor of the class.
     * 
     * @param bucket_size
     * @param block_number
     * @param block_size sizeof(the data you want to manipulate)
     */
    OramAccessController(const int& bucket_size, const int& block_number, const int& block_size);
};

#endif