#ifndef SEAL_SERVICE_H
#define SEAL_SERVICE_H

#include <grpc++/server.h>
#include <grpc++/server_context.h>

#include <Connector.h>
#include <oram/OramStruct.h>
#include <proto/seal.grpc.pb.h>
#include <proto/seal.pb.h>
#include <oram/Bucket.h>

#include <memory>
#include <vector>

class SealService : public Seal::Service {
private:
    std::unique_ptr<SEAL::Connector> connector;

    /**
     * @brief The storage array provided by the server.
     */ 
    std::vector<Bucket> odict_storage;


    /**
     * @brief The ORAM block (sub-divided) array provided by the server.
     * 
     * @note This is a two-dimensional array. [oram_id][bucket_index]
     */ 
    std::vector<std::vector<Bucket>> oram_storage;

public:
    SealService();

    virtual ~SealService();

    grpc::Status setup(grpc::ServerContext* context, const SetupMessage* request, google::protobuf::Empty* e) override;

    grpc::Status set_capacity(grpc::ServerContext* context, const BucketSetMessage* message, google::protobuf::Empty* e) override;

    grpc::Status read_bucket(grpc::ServerContext* context, const BucketReadMessage* message, BucketReadResponse* reponse) override;

    grpc::Status write_bucket(grpc::ServerContext* context, const BucketWriteMessage* message, google::protobuf::Empty* e) override;
};

#endif