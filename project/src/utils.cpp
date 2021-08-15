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

#include <utils.h>

#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <queue>
#include <regex>
#include <sodium.h>
#include <sstream>
#include <stdexcept>

unsigned int Range::Node::counter = 0;

void copy(ODict::Node* const dst, const ODict::Node* const src)
{
}

int get_height(const ODict::Node* const node)
{
    return node == nullptr ? 0 : MAX(node->left_height, node->right_height) + 1;
}

std::string random_string(const int& len)
{
    std::string tmp_s;
    std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    unsigned int random_value[len];
    randombytes_buf_deterministic(random_value, sizeof(random_value), (unsigned char*)std::to_string(randombytes_random()).c_str());

    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[random_value[i] % alphanum.size()];

    return tmp_s;
}

std::string random_string(const int& len, std::string_view secret_key)
{
    std::string tmp_s;
    std::string alphanum = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    unsigned int random_value[len];
    randombytes_buf_deterministic(random_value, sizeof(random_value), (unsigned char*)std::to_string(randombytes_random()).c_str());

    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i)
        tmp_s += alphanum[random_value[i] % alphanum.size()];

    return tmp_s;
}

OramInterface::Operation deduct_operation(OramAccessOp op)
{
    switch (op) {
    case ORAM_ACCESS_READ:
        return OramInterface::Operation::READ;
    case ORAM_ACCESS_WRITE:
        return OramInterface::Operation::WRITE;
    default:
        throw std::runtime_error("This operation is not supported!");
    }
}

std::vector<std::string> split(const std::string& input, const std::string& regex)
{
    // passing -1 as the submatch index parameter performs splitting
    std::regex re(regex);
    std::sregex_token_iterator
        first { input.begin(), input.end(), re, -1 },
        last;
    return { first, last };
}

/**
 * The randombytes_buf() function fills size bytes starting at buf with an unpredictable sequence of bytes.
 * The randombytes_buf_deterministic function stores size bytes into buf indistinguishable
 * from random bytes without knowing seed.
 * For a given seed, this function will always output the same sequence. size can be up to 2^38 (256 GB).
 */
std::vector<unsigned int>
pseudo_random_permutation(const size_t& value_size,
    std::string_view secret_key)
{
    /* Calculate the base digit number. log2 n */
    unsigned int base = std::ceil(log(value_size) / log(2));
    unsigned int interval = (unsigned int)(pow(2, base));
    unsigned int permutation[interval];
    std::vector<unsigned int> ans;
    for (unsigned int i = 0; i < interval; i++) {
        ans.push_back(i);
    }

    randombytes_buf_deterministic(
        permutation,
        sizeof(permutation),
        (unsigned char*)(secret_key.data()));

    for (unsigned int i = 0; i < interval - 1; i++) {
        // j := random integer such that i ≤ j < n
        unsigned int j = i + permutation[i] % (interval - i);
        std::swap(ans[i], ans[j]);
    }

    return ans;
}

std::pair<unsigned int, unsigned int>
get_bits(const unsigned int& base, const unsigned int& number, const unsigned int& alpha)
{
    unsigned int most = number >> (base - alpha);
    unsigned int rest = number & (unsigned int)(pow(2, base - alpha) - 1);
    return { most, rest };
}

std::string
read_keycert(std::string_view file_path)
{
    try {
        std::fstream file(file_path.data(), std::ios::in);

        std::stringstream ret;
        std::copy(std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>(),
            std::ostreambuf_iterator<char>(ret));

        return ret.str();
    } catch (const std::invalid_argument& e) {
        std::cout << e.what() << std::endl;
    }

    return "";
}

std::vector<unsigned int>
find_all(const std::vector<std::pair<std::string, unsigned int>>& memory, const std::string& value)
{
    std::vector<unsigned int> matches;
    std::vector<std::pair<std::string, unsigned int>>::const_iterator i = memory.begin();
    auto lambda = [value](const std::pair<std::string, unsigned int>& item) {
        return item.first == value;
    };
    while (true) {
        i = std::find_if(i, memory.end(), lambda);
        if (i == memory.end())
            break;
        matches.push_back((*i).second);
    }

    return matches;
}

Range::Node*
build_tree_t1(
    const int& lhs, const int& rhs,
    const std::vector<std::pair<std::string, unsigned int>>& memory)
{
    // lhs >= rhs means that we reach the leaf level.
    if (lhs >= rhs) {
        Range::Node* node = new Range::Node(lhs, rhs);
        const std::vector<unsigned int> matches = find_all(memory, std::to_string(lhs));
        node->documents.assign(matches.begin(), matches.end());
        return node;
    } else {
        const int mid = (lhs + rhs) >> 1;
        Range::Node* const left = build_tree_t1(lhs, mid, memory);
        Range::Node* const right = build_tree_t1(mid + 1, rhs, memory);
        Range::Node* node = new Range::Node(lhs, rhs);
        node->documents.assign(left->documents.begin(), right->documents.end());
        node->documents.insert(node->documents.end(), right->documents.begin(), right->documents.end());
        return node;
    }
}

// Level-traverse
Range::Node*
add_internal_nodes_for_tree_t1(Range::Node* const root)
{
    std::queue<Range::Node*> q;
    // Stores the nodes of each level.
    std::vector<std::vector<Range::Node*>> level_result;

    q.push(root);
    while (!q.empty()) {
        const size_t cur_size = q.size();
        const size_t level = level_result.size();
        for (unsigned int i = 0; i < cur_size; i++) {
            Range::Node* const front = q.front();
            q.pop();
            level_result[level].push_back(front);

            if (root->left != nullptr) {
                q.push(root->left);
            }
            if (root->right != nullptr) {
                q.push(root->right);
            }
        }
    }

    /*
       Add internal nodes to the tree.
       Only nodes with odd subscripts will be added with a parent node along with its neighbor.
     */
    for (auto nodes : level_result) {
        for (unsigned int i = 1; i < nodes.size() - 1; i += 2) {
            Range::Node* const left = nodes[i];
            Range::Node* const right = nodes[i + 1];

            Range::Node* parent = new Range::Node(left->range_cover.first, right->range_cover.second);
            parent->documents.assign(left->documents.begin(), left->documents.end());
            parent->documents.insert(parent->documents.end(), right->documents.begin(), right->documents.end());
            left->parent = right->parent = parent;
        }
    }

    return root;
}

Range::Node*
single_range_cover(Range::Node* const root, const int& lhs, const int& rhs)
{
    const int cur_lhs = root->range_cover.first;
    const int cur_rhs = root->range_cover.second;
    const int cur_mid = (cur_lhs + cur_rhs) >> 1;

    if (lhs > cur_mid) {
        return root->right;
    } else if (rhs <= cur_mid) {
        return root->left;
    } else {
        Range::Node* const left = single_range_cover(root->left, lhs, cur_mid);
        Range::Node* const right = single_range_cover(root->right, cur_mid + 1, rhs);

        if (left->parent != nullptr
            && right->parent != nullptr
            && left->parent->id == right->parent->id) {
            return left->parent;
        } else {
            return root;
        }
    }
}
/*
std::string encrypt_message(std::string_view key,
    std::string_view message,
    const unsigned char* nonce)
{
    unsigned int ciphertext_length = crypto_secretbox_MACBYTES + message.size();
    unsigned char ciphertext[ciphertext_length];
    crypto_secretbox_easy(
        ciphertext, (unsigned char*)message.data(), message.size(), nonce, (unsigned char*)key.data());

    return std::string((char*)(ciphertext), ciphertext_length);
}

std::string decrypt_message(
    std::string_view key,
    std::string_view ciphertext,
    const unsigned char* nonce,
    const size_t& raw_length)
{
        unsigned int ciphertext_length = crypto_secretbox_MACBYTES + message.size();
        unsigned char nonce[crypto_secretbox_NONCEBYTES];
        unsigned char plaintext[raw_length];
        crypto_secretbox_easy(
            plaintext, (unsigned char*)ciphertext.data(), ciphertext.size(), nonce, (unsigned char*)key.data());

        return std::string((char*)(plaintext), raw_length);
}
*/
