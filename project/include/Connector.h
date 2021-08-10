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

#ifndef _CONNECTOR_H
#define _CONNECTOR_H

#define PSQL_CONNECTION_INFORMATION "dbname = testdb "      \
                                    "user = chenhaobin "    \
                                    "password = 52chb1314 " \
                                    "host = 127.0.0.1 "     \
                                    "port = 5432"

#include <memory>
#include <string_view>

/**
 * For openGauss Database Connection.
 */
#include <pqxx/pqxx>

namespace SEAL {
class Connector {
private:
    // The main connection
    const std::unique_ptr<pqxx::connection> connection;

    // For transactional operations
    const std::unique_ptr<pqxx::work> transaction;

public:
    Connector() = delete;

    Connector(std::string_view connection_information);

    bool update_handler(std::string_view sql) const;

    bool delete_handler(std::string_view sql) const;

    bool insert_handler(std::string_view sql) const;

    std::vector<std::string> select_handler(const std::string& sql, const std::string& column) const;
};
}

#endif