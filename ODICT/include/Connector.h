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