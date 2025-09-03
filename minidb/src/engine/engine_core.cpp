// src/engine/engine_core.cpp
#include "engine_core.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>



Table* db_open(const std::string& filename) {
    return new Table(filename);
}

void db_close(Table* table) {
    delete table;
}

PrepareResult prepare_statement(const std::string& input, Statement& statement) {
    return ::prepare_statement(input, statement); // call your existing fn
}

ExecuteResult execute_statement(const Statement& statement, Table* table) {
    switch (statement.type) {
        case StatementType::INSERT:
            return execute_insert(statement, table);
        case StatementType::SELECT:
            return execute_select(statement, table);
    }
    return ExecuteResult::SUCCESS; // fallback
}
