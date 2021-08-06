#ifndef SEAL_SERVICE_H
#define SEAL_SERVICE_H

#include <grpc++/server.h>
#include <grpc++/server_context.h>

#include <Connector.h>
#include <oram/OramStruct.h>
#include <proto/seal.grpc.pb.h>
#include <proto/seal.pb.h>

#include <memory>
#include <vector>

class SealService : public Seal::Service {
private:
    std::unique_ptr<SEAL::Connector> connector;

    /**
     * It is provided for Oblivious Dictionary.
     */
    std::unique_ptr<OramStruct> oram_for_odict;

    /**
     * It is provided for seperate oram blocks.
     */
    std::vector<std::unique_ptr<OramStruct>> oram_blocks;

    size_t block_size;

    size_t oram_block_size;

    size_t block_number;
    
    size_t bucket_size;

    /**
     * TODO: Construct several ORAM Blocks.
     */
public:
    SealService();

    virtual ~SealService();

    grpc::Status setup(grpc::ServerContext* context, const SetupMessage* request, google::protobuf::Empty* e) override;

    grpc::Status oram_access(grpc::ServerContext* context, const OramAccessMessage* message, OramAccessResponse* response) override;

    grpc::Status oram_init(grpc::ServerContext* context, const OramInitMessage* message, google::protobuf::Empty* reponse) override;
    // grpc::Status search(grpc::ServerContext* context, const SearchMessage* request, SearchResponse* response);
};

#endif