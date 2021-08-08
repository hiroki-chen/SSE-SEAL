#ifndef PORAM_SERVERSTORAGE_H
#define PORAM_SERVERSTORAGE_H

#include <cmath>

#include "OramInterface.h"
#include "RandForOramInterface.h"
#include "UntrustedStorageInterface.h"

#include <proto/seal.grpc.pb.h>
#include <proto/seal.pb.h>

/**
 * This design is shit, bullshit.
 * 
 * We need to reconstruct the code completely because server storage is constructed on the server side,
 * rather than the client side.
 * 
 * TODO: 1. Provide Intefaces to connect to the server.
 * TODO: 2. Remove oramPathEivction class from the server.
 * TODO: 3. Totally rewrite the ORAM access interface provided by the server.
 */

/**
 * The ServerStorage class provided interfaces to communicate with the server.
 * 
 * @note ServerStorage 
 */
class ServerStorage : public UntrustedStorageInterface {
private:
    Seal::Stub* const stub_;

    const unsigned int oram_id;

    const bool is_odict;

public:

    /**
     * @brief The constructor for the ServerStorage class.
     * 
     * @param oram_id The id of the oram structure to which it belongs.
     * @param is_odict Oblivioud data structure is a little bit different.
     * @param stub_ Connection to the server.
     */
    ServerStorage(const unsigned int& oram_id, const bool& is_odict, Seal::Stub * stub_);

    void setCapacity(int totalNumOfBuckets);

    Bucket ReadBucket(int position);

    void WriteBucket(int position, const Bucket& bucket_to_write);

private:
    int capacity;
};

#endif //PORAM_ORAMREADPATHEVICTION_H
