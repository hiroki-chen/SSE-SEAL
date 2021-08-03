#include <Client.h>
#include <plog/Log.h>
#include <sodium.h>
#include <sys/stat.h>
#include <utils.h>

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>

// TODO: FIX RIGHT ROTATE.
void SEAL::Client::init_dummy_data()
{
    PLOG(plog::info) << "Initializing dummy data!";

    ODict::Node* test_root = new ODict::Node(0, -1);
    unsigned char* data = transform<unsigned char, ODict::Node>(test_root);
    oramAccessController.get()->oblivious_access(ORAM_ACCESS_WRITE, 0, data);
}

void SEAL::Client::init_key(std::string_view password)
{
    try {
        PLOG(plog::info) << "Initializing secret key...";
        if (sodium_init() < 0) {
            throw std::runtime_error("Crypto Library cannot be initialized!\n");
        }

        unsigned char key[crypto_box_SEEDBYTES];
        unsigned char salt[crypto_pwhash_SALTBYTES];
        randombytes_buf(salt, sizeof(salt));
        if (crypto_pwhash(key, sizeof key, password.data(), strlen(password.data()),
                salt, crypto_pwhash_OPSLIMIT_INTERACTIVE,
                crypto_pwhash_MEMLIMIT_INTERACTIVE,
                crypto_pwhash_ALG_DEFAULT)
            != 0) {
            throw std::runtime_error("Out of memory!");
        }

        secret_key = (char*)key;
        PLOG(plog::info) << "Key sampled: " << secret_key;
    } catch (const std::runtime_error& e) {
        PLOG(plog::error) << e.what();
        std::cout << e.what();
    }
}

int SEAL::Client::find_pos_by_id(const int& id)
{
    PLOG(plog::info) << "Finding position_tag for " << id;

    if (id == root_id) {
        return root_pos;
    }

    for (auto node : clientCache) {
        if (node->left_id == id) {
            return node->childrenPos[0].pos_tag;
        } else if (node->right_id == id) {
            return node->childrenPos[1].pos_tag;
        }
    }

    // Find from the oram.
    unsigned char* buffer = new unsigned char[block_size];
    oramAccessController.get()->oblivious_access(ORAM_ACCESS_READ, id, buffer);
    return transform<ODict::Node, unsigned char>(buffer)->pos_tag;
}

void SEAL::Client::ODS_access(ODict::Operation& op)
{
    ODict::Node* node = transform<ODict::Node, char>(op.data);

    const int id = op.id;

    switch (op.op) {
    case ORAM_ACCESS_READ: {
        PLOG(plog::info) << "Read node " << id
                         << " from Oblivious Data Structure";
        read_count += 1;
        ODict::Node* ret = cache->get(id);
        if (ret != nullptr) {
            PLOG(plog::info) << "Found node in cache: " << ret->id;
            memcpy(node, ret, block_size);
        } else {
            PLOG(plog::info) << "Node not found in cache! Fetch from ORAM";
            cache_helper(id, node);
            node->old_tag = node->pos_tag;

            ODict::Node* evicted = cache->put(id, node);
            if (evicted != nullptr) {
                evicted->pos_tag = oramAccessController.get()->random_new_pos();
                unsigned char* buffer = new unsigned char[block_size];
                memcpy(buffer, evicted, block_size);
                oramAccessController.get()->oblivious_access_direct(ORAM_ACCESS_WRITE,
                    buffer);
            }
        }

        return;
    }

    case ORAM_ACCESS_WRITE: {
        PLOG(plog::info) << "Write node " << node->id
                         << " to the Oblivious Data Structure";
        write_count += 1;
        ODict::Node* ret = cache->get(node->id);

        if (ret != nullptr) {
            PLOG(plog::info) << "Found node in cache: " << ret->id;
            memcpy(ret, node, block_size);
        } else {
            PLOG(plog::info) << "Node not found in cache! Fetch from ORAM";
            ODict::Node* tmp = new ODict::Node();
            cache_helper(id, tmp);
            memcpy(tmp, node, block_size);

            ODict::Node* evicted = cache->put(id, node);
            if (evicted != nullptr) {
                evicted->pos_tag = oramAccessController.get()->random_new_pos();
                unsigned char* buffer = new unsigned char[block_size];
                memcpy(buffer, evicted, block_size);
                oramAccessController.get()->oblivious_access_direct(ORAM_ACCESS_WRITE,
                    buffer);
            }
        }

        return;
    }

    case ORAM_ACCESS_DELETE: {
        PLOG(plog::info) << "Delete node " << node->id
                         << " from Oblivious Data Structure";
        ODict::Node* ret = cache->get(node->id);
        if (ret != nullptr) {
            PLOG(plog::info) << "Found node in cache: " << ret->id;
        } else {
            PLOG(plog::info) << "Node not found in cache! Fetch from ORAM";
            cache_helper(id, node);
        }

        return;
    }

    case ORAM_ACCESS_INSERT: {
        PLOG(plog::info) << "Insert a new node " << node->id << " into cache";

        ODict::Node* evicted = cache->put(id, node);
        if (evicted != nullptr) {
            PLOG(plog::debug) << "evicted: insert";
            PLOG(plog::debug) << evicted->data;
            evicted->pos_tag = oramAccessController.get()->random_new_pos();
            unsigned char* buffer = new unsigned char[block_size];
            memcpy(buffer, evicted, block_size);
            oramAccessController.get()->oblivious_access_direct(ORAM_ACCESS_WRITE,
                buffer);
        }

        return;
    }
    }
}

void SEAL::Client::cache_helper(const int& id, ODict::Node* const ret)
{
    int pos = find_pos_by_id(id);
    unsigned char* buffer = new unsigned char[block_size];
    memcpy(buffer, new ODict::Node(id, pos), block_size);
    oramAccessController.get()->oblivious_access_direct(ORAM_ACCESS_READ, buffer);
    memcpy(ret, buffer, block_size);
}

void SEAL::Client::ODS_start() { cache->clear(); }

void SEAL::Client::ODS_access(std::vector<ODict::Operation>& ops)
{
    for (auto op : ops) {
        ODS_access(op);
    }
}

void SEAL::Client::ODS_finalize(const int& pad_val)
{
    // Update rootPos based on rootId / generate new position tags.
    root_pos = cache->update_pos(root_id);

    // Pad the operation_cache.

    for (int i = pad_val - read_count; i <= pad_val; i++) {
        // dummy operation.
        unsigned char* data = new unsigned char[block_size];
        oramAccessController.get()->oblivious_access(ORAM_ACCESS_READ, 0, data);
    }

    // Evict the cache
    while (!cache->empty()) {
        ODict::Node* node = cache->get();
        PLOG(plog::debug) << "evicting " << node->id;
        // We store a node as a char array.
        unsigned char* buffer = new unsigned char[block_size];
        memcpy(buffer, node, block_size);
        oramAccessController.get()->oblivious_access_direct(ORAM_ACCESS_WRITE,
            buffer);
        cache->pop();
    }

    // Pad add to padVal.
    for (int i = pad_val - write_count; i <= pad_val; i++) {
        // dummy operation.
        unsigned char* data = new unsigned char[block_size];
        oramAccessController.get()->oblivious_access(ORAM_ACCESS_WRITE, 0, data);
    }

    write_count = read_count = 0;
}

ODict::Node* SEAL::Client::find(std::string_view key)
{
    ODS_start();
    ODict::Node* node = find_priv(key, root_id);
    ODS_finalize((int)(3 * 1.44 * log(node_count)));

    return node;
}

std::map<int, ODict::Node*> SEAL::Client::find(
    const std::vector<std::string>& keys)
{
    ODS_start();

    std::map<int, ODict::Node*> ans;
    for (unsigned int i = 0; i < keys.size(); i++) {
        ODict::Node* node = find_priv(keys[i], root_id);
        ans[stoi(keys[i])] = node;
    }

    ODS_finalize(int(3 * 1.44 * log(node_count)));

    return ans;
}

ODict::Node* SEAL::Client::find_priv(std::string_view key, const int& root_id)
{
    ODict::Node* root = nullptr;
    if (root_id == 0) {
        return root;
    }

    char* data = new char[block_size];
    ODict::Operation* const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);

    root = transform<ODict::Node, char>(data);

    if (root->key == key) {
        return root;
    } else if (root->key < key) {
        return find_priv(key, root->right_id);
    } else {
        return find_priv(key, root->left_id);
    }
}

ODict::Node* SEAL::Client::insert(ODict::Node* node)
{
    ODict::Node* res = insert_priv(node, root_id);

    if (this->root_id == 0) {
        this->root_id = node->id;
    }

    return res;
}

void SEAL::Client::insert(const std::vector<ODict::Node*>& nodes)
{
    ODS_start();

    for (auto node : nodes) {
        PLOG(plog::info) << "Inserting " << node->id;
        this->root_id = insert(node)->id;
    }

    ODS_finalize((int)1.44 * 3 * log(node_count));
}

// we suppose that node->id is pre-set.
ODict::Node* SEAL::Client::insert_priv(ODict::Node* node, const int& root_id)
{
    // If there is yet no root, assign node as the root node.
    if (root_id == 0) {
        ODict::Node* ret = new ODict::Node();
        memcpy(ret, node, block_size);
        char* data = transform<char, ODict::Node>(ret);
        ODict::Operation* const op = new ODict::Operation(
            node->id, data, ORAM_ACCESS_INSERT); // insert into cache.
        ODS_access(*op);

        return ret;
    }

    // Read the current root node.
    char* data = new char[block_size];
    ODict::Operation* const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node* root = transform<ODict::Node, char>(data);

    PLOG(plog::info) << "Reading " << root->id;

    if (root->key < node->key) {
        ODict::Node* cur = insert_priv(node, root->right_id);
        root->right_id = cur->id;
        root->right_height = get_height(cur);

        char* buffer = transform<char, ODict::Node>(root);
        ODict::Operation* const op2 = new ODict::Operation(root_id, buffer, ORAM_ACCESS_WRITE);
        ODS_access(*op2);
    } else if (root->key > node->key) {
        ODict::Node* cur = insert_priv(node, root->left_id);
        root->left_id = cur->id;
        root->left_height = get_height(cur);

        char* buffer = transform<char, ODict::Node>(root);
        ODict::Operation* const op2 = new ODict::Operation(root_id, buffer, ORAM_ACCESS_WRITE);
        ODS_access(*op2);
    }

    return balance(root_id);
}

ODict::Node* SEAL::Client::balance(const int& root_id)
{
    char* data = new char[block_size];
    ODict::Operation* const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node* root = static_cast<ODict::Node*>(static_cast<void*>(op->data));

    int left_height = root->left_height, right_height = root->right_height;

    // LL / LR
    if (left_height - right_height > 1) {
        op->id = root->left_id;
        ODS_access(*op);
        ODict::Node* left = transform<ODict::Node, char>(op->data);
        // case 1: LL. Single Rotation. RightRotate.
        if (left->left_height >= left->right_height) {
            return right_rotate(root_id);
        }
        // case 2: LR. Double Rotation. LeftAndRightRotate.
        else {
            return left_right_rotate(root_id);
        }
    } else if (right_height - left_height > 1) {
        op->id = root->right_id;
        ODS_access(*op);
        ODict::Node* right = transform<ODict::Node, char>(op->data);
        // case 3: RR. Single Rotation. LeftRotate
        if (right->right_height >= right->left_height) {
            return left_rotate(root_id);
        }
        // case 4: RL. Double Rotation. RightAndLeftRotate.
        else {
            return right_left_rotate(root_id);
        }
    }

    return root;
}

ODict::Node* SEAL::Client::right_rotate(const int& root_id)
{
    PLOG(plog::info) << "Doing right rotating";

    char* data = new char[block_size];
    ODict::Operation* const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node* root = new ODict::Node();
    memcpy(root, op->data, block_size);

    op->id = root->left_id;
    ODS_access(*op);
    ODict::Node* left = new ODict::Node();
    memcpy(left, op->data,
        block_size); // Deep Copy! Or you will lose information.

    root->left_id = left->right_id;
    root->childrenPos[0].pos_tag = left->childrenPos[1].pos_tag;
    root->left_height = left->right_height;
    char* data2 = transform<char, ODict::Node>(root);
    ODict::Operation* const op2 = new ODict::Operation(root->id, data2, ORAM_ACCESS_WRITE);
    ODS_access(*op2);

    left->right_height = get_height(root);
    left->right_id = root_id;
    left->childrenPos[1].pos_tag = root->pos_tag;
    char* data3 = transform<char, ODict::Node>(left);
    ODict::Operation* const op3 = new ODict::Operation(left->id, data3, ORAM_ACCESS_WRITE);
    ODS_access(*op3);

    return left;
}

ODict::Node* SEAL::Client::left_rotate(const int& root_id)
{
    PLOG(plog::info) << "Doing left rotating";

    char* data = new char[block_size];
    ODict::Operation* const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node* root = new ODict::Node();
    memcpy(root, op->data, block_size);

    op->id = root->right_id;
    ODS_access(*op);
    ODict::Node* right = new ODict::Node();
    memcpy(right, op->data,
        block_size); // Deep Copy! Or you will lose information.

    root->right_id = right->left_id;
    root->childrenPos[1].pos_tag = right->childrenPos[0].pos_tag;
    root->right_height = right->left_height;
    char* data2 = transform<char, ODict::Node>(root);
    ODict::Operation* const op2 = new ODict::Operation(root->id, data2, ORAM_ACCESS_WRITE);
    ODS_access(*op2);

    right->left_height = get_height(root);
    right->left_id = root_id;
    right->childrenPos[0].pos_tag = root->pos_tag;
    char* data3 = transform<char, ODict::Node>(right);
    ODict::Operation* const op3 = new ODict::Operation(right->id, data3, ORAM_ACCESS_WRITE);
    ODS_access(*op3);

    return right;
}

ODict::Node* SEAL::Client::left_right_rotate(const int& root_id)
{
    PLOG(plog::info) << "Doing left right rotating";

    char* data = new char[block_size];
    ODict::Operation* const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node* root = new ODict::Node();
    memcpy(root, op->data, block_size);

    ODict::Node* left = left_rotate(root->left_id);
    root->left_id = left->id;
    root->left_height = get_height(left);
    char* data2 = new char[block_size];
    memcpy(data2, root, block_size);
    ODict::Operation* const op2 = new ODict::Operation(root->id, data2, ORAM_ACCESS_WRITE);
    ODS_access(*op2);

    return right_rotate(root_id);
}

ODict::Node* SEAL::Client::right_left_rotate(const int& root_id)
{
    PLOG(plog::info) << "Doing right left rotating";

    char* data = new char[block_size];
    ODict::Operation* const op = new ODict::Operation(root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node* root = new ODict::Node();
    memcpy(root, op->data, block_size);

    ODict::Node* right = right_rotate(root->right_id);
    root->right_id = right->id;
    root->right_height = get_height(right);
    char* data2 = new char[block_size];
    memcpy(data2, root, block_size);
    ODict::Operation* const op2 = new ODict::Operation(root->id, data2, ORAM_ACCESS_WRITE);
    ODS_access(*op2);

    return left_rotate(root_id);
}

ODict::Node* SEAL::Client::remove_priv(std::string_view key,
    const int& cur_root_id)
{
    if (cur_root_id == 0) {
        return nullptr;
    }

    // Read the current root node.
    char* data = new char[block_size];
    ODict::Operation* const op = new ODict::Operation(cur_root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node* root = transform<ODict::Node, char>(data);

    if (root->key > key) {
        ODict::Node* left = remove_priv(key, root->left_id);
        root->left_id = left == nullptr ? 0 : left->id;
        root->left_height = get_height(left);

        char* buffer = transform<char, ODict::Node>(root);
        ODict::Operation* const op2 = new ODict::Operation(cur_root_id, buffer, ORAM_ACCESS_WRITE);
        ODS_access(*op2);
    } else if (root->key < key) {
        ODict::Node* right = remove_priv(key, root->right_id);
        root->right_id = right == nullptr ? 0 : right->id;
        root->right_height = right->right_height;

        char* buffer = transform<char, ODict::Node>(root);
        ODict::Operation* const op2 = new ODict::Operation(cur_root_id, buffer, ORAM_ACCESS_WRITE);
        ODS_access(*op2);
    } else {
        // case 1: a leaf node.
        if (root->left_id == 0 && root->right_id == 0) {
            char* data = new char[block_size];
            ODict::Operation* const op3 = new ODict::Operation(cur_root_id, data, ORAM_ACCESS_DELETE);
            ODS_access(*op3);
        }
        // case 2: root has one child.
        else if (root->left_id != 0 && root->right_id == 0) {
            char* data = new char[block_size];
            ODict::Operation* const op3 = new ODict::Operation(root->left_id, data, ORAM_ACCESS_READ);
            ODS_access(*op3);

            ODict::Node* left = transform<ODict::Node, char>(data);
            memcpy(root, left, block_size);
            root->id = cur_root_id;

            char* data2 = new char[block_size];
            ODict::Operation* const op4 = new ODict::Operation(cur_root_id, data2, ORAM_ACCESS_WRITE);
            ODS_access(*op4);

            op3->op = ORAM_ACCESS_DELETE;
            ODS_access(*op3);
        } else if (root->right_id != 0 && root->left_id == 0) {
            char* data = new char[block_size];
            ODict::Operation* const op3 = new ODict::Operation(root->right_id, data, ORAM_ACCESS_READ);
            ODS_access(*op3);

            ODict::Node* right = transform<ODict::Node, char>(data);
            memcpy(root, right, block_size);
            root->id = cur_root_id;

            char* data2 = new char[block_size];
            ODict::Operation* const op4 = new ODict::Operation(cur_root_id, data2, ORAM_ACCESS_WRITE);
            ODS_access(*op4);

            op3->op = ORAM_ACCESS_DELETE;
            ODS_access(*op3);
        }
        // case 3: root has two children.
        else {
            ODict::Node* min = find_min(root->right_id);
            memcpy(root, min, block_size);
            root->id = cur_root_id;
            ODict::Node* right = remove_priv(root->key, root->right_id);
            root->right_id = right->id;
            root->right_height = right->right_height;

            char* data = new char[block_size];
            ODict::Operation* const op = new ODict::Operation(cur_root_id, data, ORAM_ACCESS_WRITE);
            ODS_access(*op);
        }
    }

    int left_height = root->left_height, right_height = root->right_height;

    // LL / LR
    if (left_height - right_height > 1) {
        op->id = root->left_id;
        ODS_access(*op);
        ODict::Node* left = transform<ODict::Node, char>(op->data);
        // case 1: LL. Single Rotation. RightRotate.
        if (left->left_height >= left->right_height) {
            return right_rotate(root_id);
        }
        // case 2: LR. Double Rotation. LeftAndRightRotate.
        else {
            return left_right_rotate(root_id);
        }
    } else if (right_height - left_height > 1) {
        op->id = root->right_id;
        ODS_access(*op);
        ODict::Node* right = transform<ODict::Node, char>(op->data);
        // case 3: RR. Single Rotation. LeftRotate
        if (right->right_height >= right->left_height) {
            return left_rotate(root_id);
        }
        // case 4: RL. Double Rotation. RightAndLeftRotate.
        else {
            return right_left_rotate(root_id);
        }
    }

    return root;
}

ODict::Node* SEAL::Client::find_min(const int& cur_root_id)
{
    if (cur_root_id == 0) {
        return nullptr;
    }

    char* data = new char[block_size];
    ODict::Operation* const op = new ODict::Operation(cur_root_id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node* root = transform<ODict::Node, char>(data);

    return root->left_id == 0 ? root : find_min(root->left_id);
}

void SEAL::Client::remove(std::string_view key) { remove_priv(key, root_id); }

void SEAL::Client::remove(const std::vector<std::string>& keys)
{
    ODS_start();

    for (unsigned int i = 0; i < keys.size(); i++) {
        remove(keys[i]);
    }

    ODS_finalize((int)3 * 1.44 * log(node_count));
}

/**
 * @brief A test function, you can add whatever you like.
 */
const char* SEAL::Client::add_node(const int& number)
{
    insert(create_test_cases(number));

    std::vector<std::string> keys;
    for (int i = 0; i < number; i++) {
        keys.push_back(std::to_string(i % 2 == 0 ? i : number - i));
    }

    auto begin = std::chrono::high_resolution_clock::now();
    auto ans = find(keys);
    auto end = std::chrono::high_resolution_clock::now();

    std::chrono::duration<double> elapsed = end - begin;
    PLOG(plog::info) << "Read finished, time elapsed: " << elapsed.count() << "s";
    std::cout << "Read finished, time elapsed: " << elapsed.count() << "s"
              << std::endl;

    for (auto iter = ans.begin(); iter != ans.end(); iter++) {
        ODict::Node* res = iter->second;
        if (res != nullptr) {
            PLOG(plog::info) << "Read " << res->key << ": " << res->data << ", "
                             << res->left_id << ", " << res->right_id;
        } else {
            PLOG(plog::info) << "Not found";
        }
    }

    return "ok";
}

void SEAL::Client::adj_padding(std::vector<std::string>& document)
{
    unsigned int power = std::ceil((log((double)document.size()) / log((double)x)));
    unsigned int size = (unsigned int)pow(x, power);

    // Pad the document
    for (unsigned int i = document.size(); i <= size; i++) {
        document.push_back(random_string(16, secret_key));
    }
}

void SEAL::Client::adj_data_in(std::string_view file_path)
{
    try {
        if (stat(file_path.data(), NULL) == 0) {
            throw std::invalid_argument(
                "Not an invalid file on disk or not a valid relative file path!\n");
        }
        std::ifstream file(file_path.data(), std::ios::in);

        std::vector<std::pair<std::string, unsigned int>> memory;
        // Read the file by lines.
        while (!file.eof()) {
            std::string line;
            std::getline(file, line);
            // Split the string by coma.
            std::vector<std::string> tokens = split(line, "(\\s*),(\\s*)");
            adj_padding(tokens);

            unsigned int id = std::stoul(tokens[0]);

            // build (keyword, id) tuples.
            for (unsigned int i = 1; i < tokens.size(); i++) {
                std::pair<std::string, unsigned int> tuple = std::make_pair(tokens[i], id);
                memory.push_back(tuple);
            }
        }

        size_t memory_size = memory.size();
        for (unsigned int i = memory_size; i <= x * memory_size; i++) {
            memory.push_back(std::make_pair(random_string(16, secret_key), UINT_MAX));
        }

        // Sort the keyword in lexicographical order.
        auto lambda_cmp =
            [](const std::pair<std::string, unsigned int>& lhs,
                const std::pair<std::string, unsigned int>& rhs) -> bool {
            return lhs.first < rhs.first;
        };
        std::sort(memory.begin(), memory.end(), lambda_cmp);

        std::map<std::string, unsigned int> first_occurrence;
        std::map<std::string, unsigned int> count;
        for (unsigned int i = 0; i < memory.size(); i++) {
            if (first_occurrence.count(memory[i].first) == 0) {
                first_occurrence[memory[i].first] = i;
            }
            count[memory[i].first]++;
        }

        adj_insert(memory, first_occurrence, count);

        file.close();
    } catch (const std::invalid_argument& e) {
        PLOG(plog::error) << "The file " << file_path
                          << " is not found on the disk";
        std::cerr << e.what();
    }
}

void SEAL::Client::adj_insert(
    const std::vector<std::pair<std::string, unsigned int>>& memory,
    const std::map<std::string, unsigned int>& first_occurrence,
    const std::map<std::string, unsigned int>& count)
{
    std::vector<ODict::Node*> nodes;
    for (unsigned int i = 0; i < memory.size(); i++) {
        std::cout << memory[i].first << ", " << memory[i].second << std::endl;
        ODict::Node* const node = new ODict::Node();
        const unsigned int iw = first_occurrence.at(memory[i].first);
        const unsigned int cntw = count.at(memory[i].first);
        const std::string data = std::to_string(iw).append('_' + std::to_string(cntw));
        node->id = node_count++;
        node->key = memory[i].first;
        node->data = new char[data.size() + 1];
        strcpy(node->data, data.c_str());
        nodes.push_back(node);
    }
    insert(nodes);

    adj_oram_init(memory);
    PLOG(plog::info) << "SUB ORAMS INITIALIZED.";
}

void SEAL::Client::adj_oram_controller_init(
    const unsigned int& mu,
    const std::vector<std::vector<unsigned int>>& sub_arrays)
{
    for (unsigned int i = 0; i < mu; i++) {
        adj_oramAccessControllers.push_back(
                std::make_unique<OramAccessController>(bucket_size, block_number, sizeof(unsigned int)));
    }
    PLOG(plog::info) << "Oram controllers all warmed up.";

    for (unsigned int i = 0; i < sub_arrays.size(); i++) {
        for (unsigned int j = 0; j < sub_arrays[i].size(); j++) {
            adj_oramAccessControllers[i].get()->oblivious_access(
                ORAM_ACCESS_WRITE, j, (unsigned char*)(std::to_string(sub_arrays[i][j]).c_str()));
        }
    }
}

void SEAL::Client::adj_oram_init(
    const std::vector<std::pair<std::string, unsigned int>>& memory)
{
    size_t memory_size = memory.size();
    //size_t mu = pow(2, alpha);
    //size_t array_size = std::ceil(memory_size / mu);

    //std::vector<std::vector<unsigned int>> sub_arrays(mu, std::vector<unsigned int>(array_size, randombytes_uniform(UINT_MAX)));

    std::map<unsigned int, unsigned int> prp = pseudo_random_permutation(memory_size, secret_key);
    std::cout << "memory size: " << memory_size << std::endl;
    unsigned int base = (unsigned int)std::ceil((log(memory_size) / log(2)));
    std::cout << "base: " << base << std::endl;

    for (unsigned int i = 0; i < memory.size(); i++) {
        unsigned int value = prp[i];
        std::cout << value << std::endl;
        std::pair<unsigned int, unsigned int> bits = get_bits(base, value, alpha);

        //sub_arrays[bits.first][bits.second] = memory[i].second;
        PLOG(plog::info) << "ORAM BLOCK " << bits.first << ", INDEX " << bits.second << ": " << memory[i].second;
    }

    //adj_oram_controller_init(mu, sub_arrays);
}

std::vector<ODict::Node*> SEAL::Client::create_test_cases(const int& number)
{
    std::vector<ODict::Node*> vec;

    for (int i = 0; i < number; i++) {
        const std::string information = random_string(16, secret_key);
        ODict::Node* const test_root = new ODict::Node();
        test_root->id = node_count++;
        test_root->key = std::to_string(i % 2 == 0 ? test_root->id : number - test_root->id);
        test_root->data = new char[information.size() + 1];
        strcpy(test_root->data, information.c_str());
        vec.push_back(test_root);
    }

    return vec;
}

SEAL::Client::Client(const int& bucket_size, const int& block_number,
    const int& block_size, const int& odict_size,
    const size_t& max_size, const unsigned int& alpha,
    const unsigned int& x, std::string_view password,
    std::string_view connection_info)
    : bucket_size(bucket_size)
    , block_number(block_number)
    , block_size(block_size)
    , odict_size(odict_size)
    , root_id(0)
    , root_pos(-1)
    , oramAccessController(std::make_unique<OramAccessController>(bucket_size, block_number, block_size))
    , node_count(1)
    , read_count(0)
    , write_count(0)
    , cache(new Cache<ODict::Node>(max_size, oramAccessController.get()))
    , alpha(alpha)
    , x(x)
    , connector(std::make_unique<SEAL::Connector>(connection_info))
{
    init_key(password);
    init_dummy_data();
    PLOG(plog::info) << "Client initialized\n";
}

void SEAL::Client::test_adj(std::string_view file_path)
{
    adj_data_in(file_path);

    PLOG(plog::info) << "trying to fetch keyword beautiful: "
                     << find("beautiful"s)->data;
}

void SEAL::Client::test_sql(std::string_view sql)
{
    connector.get()->insert_handler(sql);
}