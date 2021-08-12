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

#include <client/Client.h>
#include <crypto/sm4.h>
#include <plog/Log.h>
#include <utils.h>

#include <sodium.h>
#include <sys/stat.h>

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <stdexcept>

ODict::Node*
SEAL::Client::read_from_oram(const int& id)
{
    std::string data = serialize<ODict::Node>(ODict::Node(id, -1));
    ODict::Operation* const op = new ODict::Operation(id, data, ORAM_ACCESS_READ);
    ODS_access(*op);
    ODict::Node* node = new ODict::Node();
    *node = deserialize<ODict::Node>(op->data);
    return node;
}

void SEAL::Client::write_to_oram(const ODict::Node* const node)
{
    std::string data = serialize<ODict::Node>(*node);
    ODict::Operation* const op = new ODict::Operation(node->id, data, ORAM_ACCESS_WRITE);
    ODS_access(*op);
}

// TODO: FIX RIGHT ROTATE.
void SEAL::Client::init_dummy_data()
{
    PLOG(plog::info) << "Initializing dummy data!";

    ODict::Node* test_root = new ODict::Node(0, -1);
    std::string data = serialize<ODict::Node>(*test_root);
    oramAccessController.get()->oblivious_access(ORAM_ACCESS_WRITE, 0, data);
}

void SEAL::Client::init_key(std::string_view password)
{
    try {
        PLOG(plog::info) << "Initializing secret key...";
        if (sodium_init() < 0) {
            throw std::runtime_error("Crypto Library cannot be initialized due to sodium initilization failure!\n");
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

        secret_key = std::string((char*)key, crypto_box_SEEDBYTES);
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

    return cache->find_pos_by_id(id);
}

void SEAL::Client::ODS_access(ODict::Operation& op)
{
    ODict::Node node = deserialize<ODict::Node>(op.data);

    const int id = op.id;

    switch (op.op) {
    case ORAM_ACCESS_READ: {
        PLOG(plog::info) << "Read node " << id
                         << " from Oblivious Data Structure";
        read_count += 1;
        ODict::Node ret = cache->get(id);
        if (ret.id != 0) {
            PLOG(plog::info) << "Found node in cache: " << ret.id;
            op.data = serialize<ODict::Node>(ret);
        } else {
            PLOG(plog::info) << "Node not found in cache! Fetch from ORAM";
            cache_helper(id, &node);
            node.old_tag = node.pos_tag;
            op.data = serialize<ODict::Node>(node);

            cache->put(id, node);
        }

        return;
    }

    case ORAM_ACCESS_WRITE: {
        PLOG(plog::info) << "Write node " << node.id
                         << " to the Oblivious Data Structure";
        write_count += 1;
        ODict::Node ret = cache->get(node.id);

        if (ret.id != 0) {
            PLOG(plog::info) << "Found node in cache: " << ret.id;
            cache->put(ret.id, node);
        } else {
            PLOG(plog::info) << "Node not found in cache! Fetch from ORAM";
            ODict::Node* tmp = new ODict::Node();
            cache_helper(id, tmp);

            cache->put(id, node);
        }

        return;
    }

    case ORAM_ACCESS_DELETE: {
        PLOG(plog::info) << "Delete node " << node.id
                         << " from Oblivious Data Structure";
        ODict::Node ret = cache->get(node.id);
        if (ret.id != 0) {
            PLOG(plog::info) << "Found node in cache: " << ret.id;
        } else {
            PLOG(plog::info) << "Node not found in cache! Fetch from ORAM";
            cache_helper(id, &node);
        }

        return;
    }

    case ORAM_ACCESS_INSERT: {
        PLOG(plog::info) << "Insert a new node " << node.id << " into cache";
        cache->put(id, node);
        return;
    }
    }
}

void SEAL::Client::cache_helper(const int& id, ODict::Node* const ret)
{
    int pos = find_pos_by_id(id);
    std::string buffer = serialize<ODict::Node>(ODict::Node(id, pos));
    oramAccessController.get()->oblivious_access_direct(ORAM_ACCESS_READ, buffer);
    *ret = deserialize<ODict::Node>(buffer);
    const std::string plaintext = decrypt_SM4_EBC(ret->data, secret_key);
    const std::string key = decrypt_SM4_EBC(ret->key, secret_key);
    ret->data = plaintext;
    ret->key = key;
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
        std::string data = "ok";
        oramAccessController.get()->oblivious_access(ORAM_ACCESS_READ, 0, data);
    }

    // Evict the cache
    while (!cache->empty()) {
        ODict::Node node = cache->get();
        // We store a node as a char array.
        const std::string ciphertext = encrypt_SM4_EBC(node.data, secret_key);
        const std::string encrypted_key = encrypt_SM4_EBC(node.key, secret_key);
        node.data = ciphertext;
        node.key = encrypted_key;

        std::string buffer = serialize<ODict::Node>(node);
        PLOG(plog::debug) << "evicting " << node.id << "with data " << node.data << " key = " << node.key;
        oramAccessController.get()->oblivious_access_direct(ORAM_ACCESS_WRITE,
            buffer);
        cache->pop();
    }
    PLOG(plog::debug) << "Eviction finished.";

    // Pad add to padVal.

    for (int i = pad_val - write_count; i <= pad_val; i++) {
        // dummy operation.
        ODict::Node* node = new ODict::Node(0, -1);
        std::string data = serialize<ODict::Node>(*node);
        oramAccessController.get()->oblivious_access(ORAM_ACCESS_WRITE, 0, data);
    }

    write_count = read_count = 0;
    PLOG(plog::debug) << "ODS_finalize finished.";
}

ODict::Node*
SEAL::Client::find(std::string_view key)
{
    ODS_start();
    ODict::Node* node = find_priv(key, root_id);
    ODS_finalize((int)(3 * 1.44 * log(node_count)));

    return node;
}

std::map<int, ODict::Node*>
SEAL::Client::find(
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

ODict::Node*
SEAL::Client::find_priv(std::string_view key, const int& root_id)
{
    ODict::Node* root = nullptr;
    if (root_id == 0) {
        return root;
    }

    root = read_from_oram(root_id);

    if (root->key == key) {
        return root;
    } else if (root->key < key) {
        return find_priv(key, root->right_id);
    } else {
        return find_priv(key, root->left_id);
    }
}

ODict::Node*
SEAL::Client::insert(ODict::Node* node)
{
    ODict::Node* res = insert_priv(node, root_id);

    if (this->root_id == 0) {
        this->root_id = res->id;
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
ODict::Node*
SEAL::Client::insert_priv(ODict::Node* node, const int& root_id)
{
    // If there is yet no root, assign node as the root node.
    if (root_id == 0) {
        std::string data = serialize<ODict::Node>(*node);
        ODict::Operation* const op = new ODict::Operation(
            node->id, data, ORAM_ACCESS_INSERT); // insert into cache.
        ODS_access(*op);

        return node;
    }

    // Read the current root node.
    ODict::Node* root = read_from_oram(root_id);

    if (root->key < node->key) {
        ODict::Node* cur = insert_priv(node, root->right_id);
        root->right_id = cur->id;
        root->right_height = get_height(cur);
    } else if (root->key > node->key) {
        ODict::Node* cur = insert_priv(node, root->left_id);
        root->left_id = cur->id;
        root->left_height = get_height(cur);
    }

    write_to_oram(root);

    return balance(root_id);
}

ODict::Node*
SEAL::Client::balance(const int& root_id)
{
    ODict::Node* root = read_from_oram(root_id);

    int left_height = root->left_height, right_height = root->right_height;

    // LL / LR
    if (left_height - right_height > 1) {
        ODict::Node* left = read_from_oram(root->left_id);
        // case 1: LL. Single Rotation. RightRotate.
        if (left->left_height >= left->right_height) {
            return right_rotate(root_id);
        }
        // case 2: LR. Double Rotation. LeftAndRightRotate.
        else {
            return left_right_rotate(root_id);
        }
    } else if (right_height - left_height > 1) {
        ODict::Node* right = read_from_oram(root->right_id);
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

ODict::Node*
SEAL::Client::right_rotate(const int& root_id)
{
    PLOG(plog::info) << "Doing right rotating";

    ODict::Node* root = read_from_oram(root_id);
    ODict::Node* left = read_from_oram(root->left_id);

    root->left_id = left->right_id;
    root->childrenPos[0].pos_tag = left->childrenPos[1].pos_tag;
    root->left_height = left->right_height;
    write_to_oram(root);

    left->right_height = get_height(root);
    left->right_id = root_id;
    left->childrenPos[1].pos_tag = root->pos_tag;
    write_to_oram(left);

    return left;
}

ODict::Node*
SEAL::Client::left_rotate(const int& root_id)
{
    PLOG(plog::info) << "Doing left rotating";

    ODict::Node* root = read_from_oram(root_id);
    ODict::Node* right = read_from_oram(root->right_id);

    root->right_id = right->left_id;
    root->childrenPos[1].pos_tag = right->childrenPos[0].pos_tag;
    root->right_height = right->left_height;
    write_to_oram(root);

    right->left_height = get_height(root);
    right->left_id = root_id;
    right->childrenPos[0].pos_tag = root->pos_tag;
    write_to_oram(right);

    return right;
}

ODict::Node*
SEAL::Client::left_right_rotate(const int& root_id)
{
    PLOG(plog::info) << "Doing left right rotating";

    ODict::Node* root = read_from_oram(root_id);
    ODict::Node* left = left_rotate(root->left_id);

    root->left_id = left->id;
    root->left_height = get_height(left);
    write_to_oram(root);

    return right_rotate(root_id);
}

ODict::Node*
SEAL::Client::right_left_rotate(const int& root_id)
{
    PLOG(plog::info) << "Doing right left rotating";

    ODict::Node* root = read_from_oram(root_id);
    ODict::Node* right = right_rotate(root->right_id);

    root->right_id = right->id;
    root->right_height = get_height(right);
    write_to_oram(root);

    return left_rotate(root_id);
}

ODict::Node*
SEAL::Client::remove_priv(std::string_view key,
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
        ODict::Node* left = read_from_oram(root->left_id);
        // case 1: LL. Single Rotation. RightRotate.
        if (left->left_height >= left->right_height) {
            return right_rotate(root_id);
        }
        // case 2: LR. Double Rotation. LeftAndRightRotate.
        else {
            return left_right_rotate(root_id);
        }
    } else if (right_height - left_height > 1) {
        ODict::Node* right = read_from_oram(root->right_id);
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

ODict::Node*
SEAL::Client::find_min(const int& cur_root_id)
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
const char*
SEAL::Client::add_node(const int& number)
{
    insert(create_test_cases(number));

    std::vector<std::string> keys;
    for (int i = 0; i < number; i++) {
        keys.push_back(std::to_string(i));
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
// TODO: Modify the function. It is incorrect now. -> Rewrite the parameters to [keyword, id] tuples, count.
/*
Assume that in each document, the keyword will not repeat, and then we can simply reuse the global count
table to pad the document; to filter out those not needed, we can generate the dummy id's above the valid count.
I.e., we generate id that exceeds the maximum one, which denotes such an id is invalid.
*/
void SEAL::Client::adj_padding(
    std::vector<std::pair<std::string, unsigned int>>& memory,
    std::map<std::string, unsigned int>& count)
{
    for (auto iter = count.begin(); iter != count.end(); iter++) {
        const unsigned int power = std::ceil((log((double)iter->second) / log((double)x)));
        const unsigned int size = (unsigned int)pow(x, power);

        // Pad the document
        for (unsigned int i = iter->second ; i <= size; i++) {
            memory.push_back(
                std::make_pair(iter->first, randombytes_uniform(0xfffffff0 - memory.size()) + memory.size()));
        }
        iter->second = size;
    }

    const size_t memory_size = memory.size();
    /* Pad the document to x * N */
    for (unsigned int i = memory_size; i <= x * memory_size; i++) {
        memory.push_back(std::make_pair(random_string(16, secret_key), randombytes_uniform(UINT_MAX)));
        count[memory.back().first]++;
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
            // Split the string by comma.
            std::vector<std::string> tokens = split(line, "(\\s*),(\\s*)");

            unsigned int id = std::stoul(tokens[0]);

            // build (keyword, id) tuples.
            for (unsigned int i = 1; i < tokens.size(); i++) {
                std::pair<std::string, unsigned int> tuple = std::make_pair(tokens[i], id);
                memory.push_back(tuple);
            }
        }

        std::map<std::string, unsigned int> count;
        for (unsigned int i = 0; i < memory.size(); i++) {
            count[memory[i].first]++;
        }

        // Pad the document set.
        adj_padding(memory, count);
        // Sort the keyword in lexicographical order.
        auto lambda_cmp =
            [](const std::pair<std::string, unsigned int>& lhs,
                const std::pair<std::string, unsigned int>& rhs) -> bool {
            return lhs.first < rhs.first;
        };
        std::sort(memory.begin(), memory.end(), lambda_cmp);

        this->memory_size = memory.size();
        std::map<std::string, unsigned int> first_occurrence;
        for (unsigned int i = 0; i < memory.size(); i++) {
            if (first_occurrence.count(memory[i].first) == 0) {
                first_occurrence[memory[i].first] = i;
            }
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
        /* Build the secret index. */
        ODict::Node* const node = new ODict::Node();
        const unsigned int iw = first_occurrence.at(memory[i].first);
        const unsigned int cntw = count.at(memory[i].first);
        const std::string data = std::to_string(iw).append('_' + std::to_string(cntw));
        node->id = node_count++;
        node->key = memory[i].first;
        node->data = data;
        nodes.push_back(node);

        /* Insert into the remote database. */
    }
    insert(nodes);

    adj_oram_init(memory);
    PLOG(plog::info) << "SUB ORAMS INITIALIZED.";
}

void SEAL::Client::adj_oram_init_helper(
    const std::vector<std::vector<unsigned int>>& sub_arrays)
{
    try {
        for (unsigned int i = 0; i < sub_arrays.size(); i++) {
            /* Initialize local oram access controllers */
            adj_oramAccessControllers.emplace_back(
                new OramAccessController(bucket_size, block_number, sizeof(unsigned int), i, false, stub_));

            for (unsigned int j = 0; j < sub_arrays[i].size(); j++) {
                std::string data = encrypt_SM4_EBC(std::to_string(sub_arrays[i][j]), secret_key);
                adj_oramAccessControllers[i].get()->oblivious_access(
                    OramAccessOp::ORAM_ACCESS_WRITE, j, data);
            }
        }
    } catch (const std::runtime_error& e) {
        PLOG(plog::error) << e.what();
    }
}

/**
 * TODO: Before calling adj_oram_init, the client should first do INSERT INTO. 
 */
void SEAL::Client::adj_oram_init(
    const std::vector<std::pair<std::string, unsigned int>>& memory)
{
    size_t mu = pow(2, alpha);
    size_t base = std::ceil((log(memory.size()) / log(2)));
    size_t array_size = std::ceil(pow(2, base) / mu);

    std::vector<std::vector<unsigned int>> sub_arrays(
        mu, std::vector<unsigned int>(array_size, randombytes_uniform(UINT_MAX)));

    std::vector<unsigned int> prp = pseudo_random_permutation(memory_size, secret_key);

    for (unsigned int i = 0; i < memory.size(); i++) {
        unsigned int value = prp[i];
        std::pair<unsigned int, unsigned int> bits = get_bits(base, value, alpha);

        PLOG(plog::info) << "ORAM BLOCK " << bits.first << ", INDEX " << bits.second << ": " << memory[i].second;
        sub_arrays[bits.first][bits.second] = memory[i].second;
    }

    std::cout << "OK" << std::endl;
    adj_oram_init_helper(sub_arrays);
}

std::vector<std::string>
SEAL::Client::search(std::string_view keyword)
{
    auto begin = std::chrono::high_resolution_clock::now();

    const ODict::Node* const node = find(keyword);
    const std::string index_value = node->data;
    const std::vector<std::string> res = split(index_value, "_");
    const unsigned int iw = std::stoul(res[0]);
    const unsigned int countw = std::stoul(res[1]);

    PLOG(plog::debug) << "In search: " << iw << ", " << countw << std::endl;
    const size_t base = std::ceil((log(memory_size) / log(2)));

    std::vector<std::string> ans;

    const std::vector<unsigned int> prp = pseudo_random_permutation(memory_size, secret_key);
    for (unsigned int i = iw; i <= iw + countw; i++) {
        const unsigned int value = prp[i];
        const std::pair<unsigned int, unsigned int> bits = get_bits(base, value, alpha);

        std::string res;
        adj_oramAccessControllers[bits.first].get()->oblivious_access(
            OramAccessOp::ORAM_ACCESS_READ, bits.second, res);
        res = decrypt_SM4_EBC(res, secret_key);
        /* Filter out dummy records. */
        if (std::stoul(res) < memory_size) {
            ans.push_back(res);
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - begin;
    PLOG(plog::info) << "Search finished, time consumed: " << elapsed.count() << " s";

    return ans;
}

std::vector<ODict::Node*>
SEAL::Client::create_test_cases(const int& number)
{
    std::vector<ODict::Node*> vec;

    for (int i = 0; i < number; i++) {
        const std::string information = random_string(16, secret_key);
        ODict::Node* const test_root = new ODict::Node();
        test_root->id = node_count++;
        test_root->key = std::to_string(i);
        test_root->data = information;
        vec.push_back(test_root);
    }

    return vec;
}

OramAccessController*
SEAL::Client::get_oram_controller()
{
    return oramAccessController.get();
}

SEAL::Client::Client(const int& bucket_size, const int& block_number,
    const int& block_size, const int& odict_size,
    const size_t& max_size, const unsigned int& alpha,
    const unsigned int& x, std::string_view password,
    Seal::Stub* stub_)
    : bucket_size(bucket_size)
    , block_number(block_number)
    , block_size(block_size)
    , odict_size(odict_size)
    , root_id(0)
    , root_pos(-1)
    , stub_(stub_)
    , oramAccessController(std::make_unique<OramAccessController>(bucket_size, block_number, block_size, -1, true, stub_))
    , node_count(1)
    , read_count(0)
    , write_count(0)
    , cache(new Cache<ODict::Node>(max_size, oramAccessController.get()))
    , alpha(alpha)
    , x(x)
{
    init_key(password);
    PLOG(plog::info) << "Client initialized\n";
}

void SEAL::Client::test_adj(std::string_view file_path)
{
    adj_data_in(file_path);

    //PLOG(plog::info) << "trying to fetch keyword beautiful: "
    //                 << find("beautiful"s)->data;
}

void SEAL::Client::set_stub(const std::unique_ptr<Seal::Stub>& stub)
{
    stub_ = stub.get();
    oramAccessController.get()->set_stub(stub_);
}