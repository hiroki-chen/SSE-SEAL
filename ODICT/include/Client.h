#ifndef CLIENT_H_
#define CLIENT_H_

#include <algorithm>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <ClientCache.h>
#include <Connector.h>
#include <Objects.h>
#include <OramAccessController.h>

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
    const size_t block_size;

    const size_t odict_size;

    const size_t block_number;

    int root_id;

    int root_pos;

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

    /*==================== Connection to Relational Database (PostgreSQL based openGauss) =====================*/
    std::unique_ptr<SEAL::Connector> connector;

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
     * For padding. ORAM does not allow access to address that does not exist.
     */
    void init_dummy_data(void);

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
    ODict::Node* find(std::string_view key);

    /**
     * @brief Batch find to better utiltize the client cache.
     * 
     * @param keys
     * @return mapping between keys and nodes.
     */
    std::map<int, ODict::Node*> find(const std::vector<std::string>& keys);

    /**
     * @brief Actual function for find. Recursive function.
     */
    ODict::Node* find_priv(std::string_view key, const int& cur_root_id);

    /**
     * @brief Find the minimum node to be the root for deletion.
     */
    ODict::Node* find_min(const int& cur_root_id);

    /**
     * @brief Insert a node into the AVL Tree. It is a wrapper function.
     */
    ODict::Node* insert(ODict::Node* node);

    /**
     * @brief batch insertion.
     *
     * @param nodes nodes to be inserted
     */
    void insert(const std::vector<ODict::Node*>& nodes);

    /**
     * @brief Actural function for insert.
     */
    ODict::Node* insert_priv(ODict::Node* node, const int& cur_root_id);

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
    ODict::Node* remove_priv(std::string_view key, const int& cur_root_id);

    /**
     * @brief Rebalance the AVL Tree because of the insertion.
     */
    ODict::Node* balance(const int& cur_root_id);

    /**
     * @brief RightRotate for LL case.
     */
    ODict::Node* right_rotate(const int& cur_root_id);

    /**
     * @brief LeftAndRightRotate for LR case.
     */
    ODict::Node* left_right_rotate(const int& cur_root_id);

    /**
     * @brief LeftRotate for RR case.
     */
    ODict::Node* left_rotate(const int& cur_root_id);

    /**
     * @brief RightAndLeftRotate for RL case.
     */
    ODict::Node* right_left_rotate(const int& cur_root_id);

    /**
     * @brief Creates test data.
     */
    std::vector<ODict::Node*> create_test_cases(const int& number);

    /**
     * @brief Adjustable Padding for any document to the nearest power of x.
     * 
     * @param document a set of keywords
     * @param x the given parameter as base number
     */
    void adj_padding(std::vector<std::string>& document);

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
     */
    void adj_oram_init(const std::vector<std::pair<std::string, unsigned int>>& memory);

    /**
     * @brief Build the secret index on input documents.
     * 
     * @param memory the plain memory denoted as (keyword, id) pairs in lexicographical order.
     * @param first_occurrence stores the position where a keyword w first appears in memory.
     * @param count the count table.
     */
    void adj_insert(const std::vector<std::pair<std::string, unsigned int>>& memory,
        const std::map<std::string, unsigned int>& first_occurrence,
        const std::map<std::string, unsigned int>& count);

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
     */
    Client(const int& bucket_size, const int& block_number,
        const int& block_size, const int& odict_size,
        const size_t& max_size, const unsigned int& alpha,
        const unsigned int& x, std::string_view password,
        std::string_view connection_info);

    /**
     * @brief A test function.
     */
    const char* add_node(const int& number);

    /**
     * @brief Test the functionality of SEAL.
     * 
     * @param file_path the input dataset
     */
    void test_adj(std::string_view file_path);

    /**
     * @brief Test the database connection.
     * 
     * @param sql
     */
    void test_sql(std::string_view sql);
};
}

#endif