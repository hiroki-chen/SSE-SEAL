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