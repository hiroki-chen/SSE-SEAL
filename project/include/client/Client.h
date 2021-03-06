/*
 Copyright (c) 2021 Haobin Chen

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ClientCache.h"
#include "Connector.h"
#include "Objects.h"
#include "OramAccessController.h"
#include <proto/seal.grpc.pb.h>
#include <proto/seal.pb.h>

/**
 * Some notes:
 * 1. ReadAndRemove function is ORAM_ACCESS(READ), which automatically evicts some blocks from the ORAM.
 * 2. ODS consists of three functions: start, access (sequential), and finalize;
 * 3. Client will maintain a local position map as well as a operation cache to do some local adjustments for the 
 *    ODS to get updated in advance; otherwise the server needs some information to update the nodes.
 */

/**
 * To-do list.
 */

namespace SEAL {
/**
* Client will have access to an oblivious data structure uses as a secret index on the server side.
*/
class Client {
private:
    /* ================= Oblivious Data Structure ======================*/
    const size_t bucket_size;

    const size_t block_number;

    const size_t block_size;

    const size_t odict_size;

    int root_id;

    int root_pos;

    Seal::Stub* stub_;

    std::unique_ptr<OramAccessController> oramAccessController;

    std::vector<ODict::Node*> clientCache;

    int node_count;

    int read_count; // for padding.

    int write_count; // for padding.

    Cache<ODict::Node>* cache;

    /*===================== Adjustable Oram Block For SEAL Model =====================*/
    const unsigned int alpha; // leakage-alpha

    const unsigned int x; // for padding

    std::string secret_key; // for pseudo-random permutation

    std::vector<std::unique_ptr<OramAccessController>> adj_oramAccessControllers;

    std::map<std::string, std::vector<std::unique_ptr<OramAccessController>>> adj_oramAccessControllers_range;

    size_t memory_size;

    /*==================== Nodes used for range-query ====================*/
    /**
     * @brief Mapps from table.column to the search tree T1.
     * 
     */
    std::map<std::string, Range::Node*> root_t1;

    std::map<std::string, size_t> kwd_size;

    /*==================== Functions =====================*/
    /**
     * @brief Initialize the client cache.
     * 
     * @param max_size the maximum size of the cache.
     */
    void init_cache(const size_t& max_size);

    /**
     * @brief Init the secret key
     */
    void init_key(std::string_view password);

    /**
     * @param op contains
     *                   > @param data the data to be read (dummy) or to be written.
     *                   > @param OramAccessOp enumeration. WRITE / READ.
     * 
     * @brief The ODS.Access calls generate ReadAndRemove operations to the underlying position-based ORAM,
     *        and hence the physical accesses observed by the server are oblivious.
     * @note  Not every ODS.Access call will generate a ReadAndRemove operation, @see cache.
     */
    void ODS_access(ODict::Operation& op);

    /**
     * @brief To avoid volumn pattern leakage, we pad every sequence of ODict accesses to a fixed length.
     */
    void pad_dummy_access(void);

    /**
     * @brief Tell the client to start ODS
     */
    void ODS_start(void);

    /**
     * @param ops a sequence of ODS accesses.
     * 
     * @brief This tells ODS what nodes to fetch and remove from the
     *        server, and what local updates to make to those nodes.
     */
    void ODS_access(std::vector<ODict::Operation>& ops);

    /**
     * @param pad_val the maximum number of access.
     * 
     * @brief All read / write operations are finished. Flush the cache back to the server.
     * 
     * @note pad_val = 1.44 * 3logN for oblivious map.
     */
    void ODS_finalize(const int& pad_val);

    /**
     * @brief Find the position of a node based on its id in some other nodes' childrenPos array.
     * 
     * @param id the id of the node
     * @return the position tag found in the client cache.
     */
    int find_pos_by_id(const int& id);

    /**
     * @brief Find the pos, read the node.
     * 
     * @param id the id of the desired node
     * @param ret the node to be written (as a buffer)
     */
    void cache_helper(const int& id, ODict::Node* const ret);

    /**
     * @brief Find a node in the AVL Tree using its key (i.e., the id of the node). It is a wrapper function.
     * 
     * @param key
     * @return the node (nullptr = not found)
     */
    ODict::Node*
    find(std::string_view key);

    /**
     * @brief Batch find to better utiltize the client cache.
     * 
     * @param keys
     * @return mapping between keys and nodes.
     */
    std::map<int, ODict::Node*>
    find(const std::vector<std::string>& keys);

    /**
     * @brief Actual function for find. Recursive function.
     */
    ODict::Node*
    find_priv(std::string_view key, const int& cur_root_id);

    /**
     * @brief Find the minimum node to be the root for deletion.
     */
    ODict::Node*
    find_min(const int& cur_root_id);

    /**
     * @brief Insert a node into the AVL Tree. It is a wrapper function.
     */
    ODict::Node*
    insert(ODict::Node* node);

    /**
     * @brief batch insertion.
     *
     * @param nodes nodes to be inserted
     */
    void insert(const std::vector<ODict::Node*>& nodes);

    /**
     * @brief Actural function for insert.
     */
    ODict::Node*
    insert_priv(ODict::Node* node, const int& cur_root_id);

    /**
     * @brief Batch remove the nodes from the AVL Tree.
     * 
     * @param keys
     */
    void remove(const std::vector<std::string>& keys);

    /**
     * @brief Remove a node from the AVL Tree.
     * 
     * @param key 
     */
    void remove(std::string_view key);

    /**
     * @brief Actual remove function.
     * 
     * @param key
     * @param cur_root_id the current root id.
     */
    ODict::Node*
    remove_priv(std::string_view key, const int& cur_root_id);

    /**
     * @brief Rebalance the AVL Tree because of the insertion.
     */
    ODict::Node*
    balance(const int& cur_root_id);

    /**
     * @brief RightRotate for LL case.
     */
    ODict::Node*
    right_rotate(const int& cur_root_id);

    /**
     * @brief LeftAndRightRotate for LR case.
     */
    ODict::Node*
    left_right_rotate(const int& cur_root_id);

    /**
     * @brief LeftRotate for RR case.
     */
    ODict::Node*
    left_rotate(const int& cur_root_id);

    /**
     * @brief RightAndLeftRotate for RL case.
     */
    ODict::Node*
    right_left_rotate(const int& cur_root_id);

    /**
     * @brief Creates test data.
     */
    std::vector<ODict::Node*>
    create_test_cases(const int& number);

    /**
     * @brief Adjustable Padding for any document to the nearest power of x.
     * 
     * @param memory the dataset to be padded.
     * @param count the count of the keyword. (Assume that keywords do not repeat in one document)
     */
    void adj_padding(
        std::vector<std::pair<std::string, SEAL::Document>>& memory,
        std::map<std::string, unsigned int>& count);

    /**
     * @brief Load the dataset from the file, and also analyze the document input.
     * 
     * @param file_path the path of the input file. Fomrat: [id]: [keyword1, keyword2, ...].
     */
    void adj_data_in(std::string_view file_path);

    /**
     * @brief Initialize the oram block by block.
     * 
     * @param memory
     * @param map_key
     */
    void adj_oram_init(
        const std::vector<std::pair<std::string, SEAL::Document>>& memory,
        const std::string& map_key);

    /**
     * @brief Initialize each oram controller.
     * 
     * @param sub_array plain memory blocks
     * @param map_key
     */
    void adj_oram_init_helper(
        const std::vector<std::vector<SEAL::Document>>& sub_arrays,
        const std::string& map_key);

    /**
     * @brief Initializes each oram controller for range-query.
     * 
     * @param sub_arrays 
     */
    void adj_oram_init_helper_range(
        const std::vector<std::vector<SEAL::Document>>& sub_arrays,
        const std::string& map_key);

    /**
     * @brief Build the secret index on input documents.
     * 
     * @param memory the plain memory denoted as (keyword, id) pairs in lexicographical order.
     * @param first_occurrence stores the position where a keyword w first appears in memory.
     * @param count the count table.
     */
    void adj_insert(const std::vector<std::pair<std::string, SEAL::Document>>& memory,
        const std::map<std::string, unsigned int>& first_occurrence,
        const std::map<std::string, unsigned int>& count,
        const std::string& map_key);

    /**
     * @brief The function builds LOGRITHMIC-SRC-i TREE T2 on the server side by treating subscript as the keywords.
     * 
     * @param sorted_documents Documents sorted on the attribute.
     * @param map_key To initialize oram storage
     */
    void adj_insert_range(
        const std::vector<SEAL::Document>& sorted_documents,
        const std::string& map_key);

    //====================Util=====================//
    /**
     * @brief Read the node from the oram. No need to manipulate the oblivious data structure anymore.
     * 
     * @param id
     */
    ODict::Node*
    read_from_oram(const int& id);

    /**
     * @brief Write the node to the oram
     * 
     * @param node
     */
    void write_to_oram(const ODict::Node* const node);

public:
    /**
     * @brief the construtor of the class Client.
     * 
     * @param bucket_size the size of each oram bucket.
     * @param block_number how many blocks should one bucket hold
     * @param block_size the length of one block. @note Should set to sizeof(ODict::node)
     * @param odict_size the approximate size of the obilivious data structure.
     * @param max_size the maximum size of the client cache.
     * @param password the password for encryption / decryption
     * @param connection_info the connection information to the relational database.
     * @param stub_ grpc
     */
    Client(const int& bucket_size, const int& block_number,
        const int& block_size, const int& odict_size,
        const size_t& max_size, const unsigned int& alpha,
        const unsigned int& x, std::string_view password,
        Seal::Stub* stub_);

    /**
     * @brief A test function.
     */
    const char*
    add_node(const int& number);

    /**
     * For padding. ORAM does not allow access to address that does not exist.
     */
    void init_dummy_data(void);

    /**
     * @brief Test the functionality of SEAL.
     * 
     * @param file_path the input dataset
     */
    void test_adj(std::string_view file_path);

    /**
     * @brief Get the odict controller.
     * 
     * @return OramAccessController
     */
    OramAccessController*
    get_oram_controller();

    void set_stub(const std::unique_ptr<Seal::Stub>& stub);

    /**
     * @brief Search the document id by a keyword.
     * 
     * @param keyword
     * @return the documents
     */
    std::vector<SEAL::Document>
    search(std::string_view keyword);

    /**
     * @brief Search the document whose keywords are in the given range.
     * 
     * @param map_key Used to search for the root node.
     * @param lower The lower bound of the range.
     * @param upper The upper bound of the range.
     * @return std::vector<SEAL::Document> 
     */
    std::vector<SEAL::Document>
    search_range(std::string_view map_key, std::string_view lower, std::string_view upper);

    /**
     * @brief A helper that transforms range query to series of point query.
     * 
     * @param doc_subscripts
     * @param map_key
     * @return std::vector<SEAL::Document> 
     */
    std::vector<SEAL::Document>
    search_range_helper(const std::vector<unsigned int>& doc_subscripts,std::string_view map_key);

    /**
     * @brief Get the t1 root object
     * 
     * @param map_key 
     * @return Range::Node* 
     */
    Range::Node*
    get_t1_root(const std::string& map_key);
};
}

#endif