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

#ifndef UTILS_H_
#define UTILS_H_

#ifndef MAX
#define MAX(a, b) ((a > b) ? a : b)
#endif

#include <algorithm>
#include <iostream>
#include <sstream>
#include <string>

#include <client/Objects.h>
#include <oram/Bucket.h>
#include <oram/OramInterface.h>

#include <cereal/archives/binary.hpp>

/**
 * @brief The template will provide us with a function equivalent to
 *        instanceof in Java.
 */
template <typename Base, typename T>
inline bool instanceof (const T*)
{
    return std::is_base_of<Base, T>::value;
}

template <typename Dst, typename Src>
inline Dst* transform(Src* data)
{
    return static_cast<Dst*>(static_cast<void*>(data));
}

OramInterface::Operation
deduct_operation(OramAccessOp op);

void copy(ODict::Node* const dst, const ODict::Node* const src);

int get_height(const ODict::Node* const node);

std::string
random_string(const int& length, std::string_view secret_key);

std::string
random_string(const int& length);

std::vector<std::string>
split(const std::string& input, const std::string& regex);

std::string
read_keycert(std::string_view file_path);

/**
 * @brief A secure PRP.
 */
std::vector<unsigned int> pseudo_random_permutation(const size_t& value_size, std::string_view secret_key);

std::pair<unsigned int, unsigned int> get_bits(const unsigned int& base, const unsigned int& number, const unsigned int& alpha);

std::string encrypt_message(std::string_view key, std::string_view message, const unsigned char* nonce);

std::string decrypt_message(std::string_view key, std::string_view ciphertext, const unsigned char* nonce, const size_t& raw_length);

template <typename Object>
std::string serialize(const Object& obj)
{
    std::ostringstream oss;
    {
        cereal::BinaryOutputArchive oa(oss);
        oa(obj);
        oss.flush();
    }
    return oss.str();
}

template <typename Object>
Object deserialize(const std::string& serial_str)
{
    Object obj;
    std::istringstream iss(serial_str);
    {
        cereal::BinaryInputArchive ia(iss);
        ia >> obj;
    }

    return obj;
}

#endif