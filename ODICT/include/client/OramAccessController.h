#ifndef ORAM_ACCESS_CONTROLLER_H_
#define ORAM_ACCESS_CONTROLLER_H_

#include <oram/OramInterface.h>
#include <oram/PathORAM.h>
#include <oram/RandForOramInterface.h>
#include <oram/UntrustedStorageInterface.h>
#include <proto/seal.grpc.pb.h>
#include <proto/seal.pb.h>

#include <grpc/grpc.h>

class OramAccessController {
private:
    UntrustedStorageInterface* storage;

    RandForOramInterface* random;

    OramInterface* oram;

    /**
     * @note This variable depends on the role that this current OramAccessController plays.
     */
    int block_size;

public:
    /**
     * @brief Get the random engine to initialize the random engine on the remote server side. 
     * (For oblivious dictionary data structure)
     * 
     * @return The random engine
     */
    RandForOramInterface* get_random_engine();

    /**
     * @deprecated
     * @brief The normal way to access the PathORAM.
     * 
     * @param op the operation: READ / WRITE. INSERT / DELETE are not used in this interface.
     * @param address the physical address.
     * @param data the data to be read / written.
     */
    void oblivious_access(OramAccessOp op, const int& address, unsigned char* data);

    /**
     * @brief The normal way to access the PathORAM.
     * 
     * @param op the operation: READ / WRITE. INSERT / DELETE are not used in this interface.
     * @param address the physical address.
     * @param data the data to be read / written.
     * @param oram_id the id of the oram block to be accessed.
     * @param stub_ interface to the remote server.
     */
    void oblivious_access(OramAccessOp op, const int& address, unsigned char* data, const int& oram_id, Seal::Stub* stub_);

    /**
     * @deprecated
     * @brief The special way to access the PathORAM. For oblviious data structures.
     * 
     * @param op the operation: READ/ WRITE. INSERT / DELETE are not used in this interface either.
     * @param data the data to be read / written. @note It must contain pos_tag.
     */
    void oblivious_access_direct(OramAccessOp op, unsigned char* data);

    /**
     * @brief The special way to access the PathORAM. For oblviious data structures.
     * 
     * @param op the operation: READ/ WRITE. INSERT / DELETE are not used in this interface either.
     * @param data the data to be read / written. @note It must contain pos_tag.
     * @param stub_ interface to the remote server.
     */
    void oblivious_access_direct(OramAccessOp op, unsigned char* data, Seal::Stub* stub_);

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