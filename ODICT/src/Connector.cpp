#include <Connector.h>
#include <plog/Log.h>

#include <iostream>

SEAL::Connector::Connector(std::string_view connection_information)
    : connection(std::make_unique<pqxx::connection>(connection_information.data()))
    , transaction(std::make_unique<pqxx::work>(*connection.get()))
{
    PLOG(plog::info) << "Connection to PostgreSQL established with code " << connection.get()->is_open();
}

bool SEAL::Connector::insert_handler(std::string_view sql) const
{
    try {
        transaction.get()->exec0(sql.data());
        PLOG(plog::info) << "INSERT: " << sql;

        transaction.get()->commit();
        return true;
    } catch (const pqxx::sql_error& e) {
        PLOG(plog::error) << e.what();

        transaction.get()->commit();
        return false;
    }
}