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

bool SEAL::Connector::create_table_handler(
    std::string_view table,
    const std::vector<std::string>& column_names) const
{
    const std::string drop_table_command = ((std::string) "DROP TABLE IF EXISTS ").append(table);
    transaction.get()->exec0(drop_table_command);
    transaction.get()->commit();
    std::stringstream create_table_command("CREATE TABLE IF NOT EXISTS ");
    create_table_command << table << "(";

    for (unsigned int i = 0; i < column_names.size(); i++) {
        create_table_command << column_names[i] << " VARCHAR(128)";
        if (i == column_names.size() - 1) {
            create_table_command << ", ";
        } else {
            create_table_command << ")";
        }
    }

    transaction.get()->exec0(create_table_command.str());
    transaction.get()->commit();
    return true;
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

bool SEAL::Connector::insert_handler(
    std::string_view table,
    const std::vector<std::string>& values) const
{
    // Build insert statement by stringstream for convenience.
    std::stringstream insert_command("INSERT INTO ");
    insert_command << table << " VALUES(";
    for (unsigned int i = 0; i < values.size(); i++) {
        insert_command << "\'" << values[i - 1] << "\'";
        if (i == values.size() - 1) {
            insert_command << ")";
        } else {
            insert_command << ", ";
        }
    }

    transaction.get()->exec0(insert_command.str());
    transaction.get()->commit();
    return true;
}

pqxx::result
SEAL::Connector::select_handler(
    std::string_view table,
    std::string_view where,
    const std::vector<std::string>& columns) const
{
    std::stringstream select_command("SELECT ");
    for (unsigned int i = 0; i < columns.size(); i++) {
        select_command << columns[i];
        if (i == columns.size() - 1) {
            select_command << columns[i];
        } else {
            select_command << columns[i] << ", ";
        }
    }
    select_command << " FROM " << table << "WHERE document_id = " << where;

    return transaction.get()->exec(select_command.str());
}