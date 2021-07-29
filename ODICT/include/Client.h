#ifndef CLIENT_H_
#define CLIENT_H_

#include <memory>
#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include <OramAccessController.h>
#include <Objects.h>
#include <ClientCache.h>

/**
 * Some notes:
 * 1. ReadAndRemove function is ORAM_ACCESS(READ), which automatically evicts some blocks from the ORAM.
 * 2. ODS consists of three functions: start, access (sequential), and finalize;
 * 3. Client will maintain a local position map as well as a operation cache to do some local adjustments for the 
 *    ODS to get updated in advance; otherwise the server needs some information to update the nodes.
 */

/**
 * To-do list.
 * TODO 1. implement batch read (use cache); 2. create a cache pool.
 */

namespace SEAL
{
    /**
     * Client will have access to an oblivious data structure uses as a secret index on the server side.
     */
    class Client
    {
    private:
        size_t block_size;

        size_t odict_size;

        size_t block_number;

        int root_id;

        int root_pos;

        std::unique_ptr<OramAccessController> oramAccessController;

        std::vector<ODict::Node *> clientCache;

        int node_count;

        int read_count; // for padding.

        int write_count; // for padding.

        Cache<ODict::Node> *cache;

        /**
         * @brief Initialize the client cache.
         * 
         * @param max_size the maximum size of the cache.
         */
        void init_cache(const size_t &max_size);

        /**
         * For padding. ORAM does not allow access to address that does not exist.
         */
        void init_dummy_data();

        /**
         * @param op contains
         *                   > @param data the data to be read (dummy) or to be written.
         *                   > @param OramAccessOp enumeration. WRITE / READ.
         * 
         * @brief The ODS.Access calls generate ReadAndRemove operations to the underlying position-based ORAM,
         *        and hence the physical accesses observed by the server are oblivious.
         * @note  Not every ODS.Access call will generate a ReadAndRemove operation, @see cache.
         */
        void ODS_access(ODict::Operation &op);

        /**
         * @brief To avoid volumn pattern leakage, we pad every sequence of ODict accesses to a fixed length.
         */
        void pad_dummy_access();

        /**
         * @brief Tell the client to start ODS
         */
        void ODS_start();

        /**
         * @param ops a sequence of ODS accesses.
         * 
         * @brief This tells ODS what nodes to fetch and remove from the
         *        server, and what local updates to make to those nodes.
         */
        void ODS_access(std::vector<ODict::Operation> &ops);

        /**
         * @param pad_val the maximum number of access.
         * 
         * @brief All read / write operations are finished. Flush the cache back to the server.
         * 
         * @note pad_val = 1.44 * 3logN for oblivious map.
         */
        void ODS_finalize(const int &pad_val);

        /**
         * @brief Find the position of a node based on its id in some other nodes' childrenPos array.
         * 
         * @param id the id of the node
         * @return the position tag found in the client cache.
         */
        int find_pos_by_id(const int &id);

        /**
         * @brief Find the pos, read the node.
         * 
         * @param id the id of the desired node
         * @param ret the node to be written (as a buffer)
         */
        void cache_helper(const int &id, ODict::Node *const ret);

        /**
         * @brief Find a node in the AVL Tree using its key (i.e., the id of the node). It is a wrapper function.
         * 
         * @param key
         * @return the node (nullptr = not found)
         */
        ODict::Node *find(const int &key);

        /**
         * @brief Batch find to better utiltize the client cache.
         * 
         * @param keys
         * @return mapping between keys and nodes.
         */
        std::map<int, ODict::Node *> find(const std::vector<int> &keys);

        /**
         * @brief Actual function for find. Recursive function.
         */
        ODict::Node *find_priv(const int &key, const int &cur_root_id);

        /**
         * @brief Insert a node into the AVL Tree. It is a wrapper function.
         */
        ODict::Node *insert(ODict::Node *node);

        /**
         * @brief batch insertion.
         * 
         * @param nodes nodes to be inserted
         */
        void insert(const std::vector<ODict::Node *> &nodes);

        /**
         * @brief Actural function for insert.
         */
        ODict::Node *insert_priv(ODict::Node *node, const int &cur_root_id);

        /**
         * @brief Rebalance the AVL Tree because of the insertion.
         */
        ODict::Node *balance(const int &cur_root_id);

        /**
         * @brief RightRotate for LL case.
         */
        ODict::Node *right_rotate(const int &cur_root_id);

        /**
         * @brief LeftAndRightRotate for LR case.
         */
        ODict::Node *left_right_rotate(const int &cur_root_id);

        /**
         * @brief LeftRotate for RR case.
         */
        ODict::Node *left_rotate(const int &cur_root_id);

        /**
         * @brief RightAndLeftRotate for RL case.
         */
        ODict::Node *right_left_rotate(const int &cur_root_id);

        /**
         * @brief Creates test data.
         */
        std::vector<ODict::Node *> create_test_cases(const int &number);

    public:
        /**
         * @brief the construtor of the class Client.
         * 
         * @param bucket_size the size of each oram bucket.
         * @param block_number how many blocks should one bucket hold
         * @param block_size the length of one block. @note Should set to sizeof(ODict::node)
         * @param odict_size the approximate size of the obilivious data structure.
         * @param max_size the maximum size of the client cache.
         */
        Client(const int &bucket_size, const int &block_number, const int &block_size,
               const int &odict_size, const size_t &max_size);

        /**
         * @brief A test function.
         */
        const char *add_node(const int &number);
    };
}

#endif