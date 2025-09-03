// src/engine/engine_core.cpp
#include "engine_core.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <vector>


#include <cassert>

// ----- Constants -----
constexpr size_t PAGE_SIZE = 4096;
constexpr size_t ROW_SIZE = sizeof(u32) + (COLUMN_USERNAME_SIZE + 1) + (COLUMN_EMAIL_SIZE + 1);
constexpr size_t TABLE_MAX_PAGES = 100;

// B-tree node constants
constexpr u32 NODE_INTERNAL = 1;
constexpr u32 NODE_LEAF = 2;
constexpr u32 INVALID_PAGE_NUM = UINT32_MAX;

// ----- Row serialization -----
void serialize_row(const Row& source, u8* destination) {
    memcpy(destination, &source.id, sizeof(u32));
    memcpy(destination + sizeof(u32), source.username.data(), COLUMN_USERNAME_SIZE + 1);
    memcpy(destination + sizeof(u32) + COLUMN_USERNAME_SIZE + 1, source.email.data(), COLUMN_EMAIL_SIZE + 1);
}

void deserialize_row(u8* source, Row& destination) {
    memcpy(&destination.id, source, sizeof(u32));
    memcpy(destination.username.data(), source + sizeof(u32), COLUMN_USERNAME_SIZE + 1);
    memcpy(destination.email.data(), source + sizeof(u32) + COLUMN_USERNAME_SIZE + 1, COLUMN_EMAIL_SIZE + 1);
}

// ----- Pager -----
class Pager {
public:
    Pager(const std::string& filename) : file(filename, std::ios::in | std::ios::out | std::ios::binary) {
        if (!file.is_open()) {
            // If file doesn't exist, create it
            file.open(filename, std::ios::out | std::ios::binary);
            file.close();
            file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
        }
        file.seekg(0, std::ios::end);
        file_length = file.tellg();
        num_pages = file_length / PAGE_SIZE;
        if (file_length % PAGE_SIZE != 0) {
            throw std::runtime_error("Corrupt database file.");
        }
    }

    ~Pager() {
        flush();
        for (auto page : pages) {
            delete[] page;
        }
    }

    u8* get_page(u32 page_num) {
        if (page_num >= TABLE_MAX_PAGES) throw std::runtime_error("Page number out of bounds");

        if (!pages[page_num]) {
            // Allocate memory and load from file if exists
            pages[page_num] = new u8[PAGE_SIZE]();
            if (page_num < num_pages) {
                file.seekg(page_num * PAGE_SIZE, std::ios::beg);
                file.read(reinterpret_cast<char*>(pages[page_num]), PAGE_SIZE);
            }
        }
        return pages[page_num];
    }

    void flush() {
        for (u32 i = 0; i < num_pages; i++) {
            if (pages[i]) {
                file.seekp(i * PAGE_SIZE, std::ios::beg);
                file.write(reinterpret_cast<char*>(pages[i]), PAGE_SIZE);
            }
        }
        file.flush();
    }

    u32 num_pages = 0;

private:
    std::fstream file;
    u32 file_length;
    std::array<u8*, TABLE_MAX_PAGES> pages{};
};

// ----- Cursor -----
class Cursor {
public:
    Cursor(Table* table, u32 page_num, u32 cell_num, bool end_of_table = false)
        : table(table), page_num(page_num), cell_num(cell_num), end_of_table(end_of_table) {}

    Table* table;
    u32 page_num;
    u32 cell_num;
    bool end_of_table;
};

// ----- Table -----
class Table {
public:
    explicit Table(const std::string& filename) : pager(filename) {
        if (pager.num_pages == 0) {
            // Initialize root node
            u8* root = pager.get_page(0);
            initialize_leaf_node(root);
            set_node_root(root, true);
            pager.num_pages = 1;
        }
        root_page_num = 0;
    }

    Pager pager;
    u32 root_page_num;
};

// ----- Node helper functions -----
inline u8 get_node_type(const u8* node) {
    return node[0];
}

inline void set_node_type(u8* node, u8 type) {
    node[0] = type;
}

inline bool is_node_root(const u8* node) {
    return node[1];
}

inline void set_node_root(u8* node, bool is_root) {
    node[1] = is_root;
}

void initialize_leaf_node(u8* node) {
    set_node_type(node, NODE_LEAF);
    set_node_root(node, false);
    *reinterpret_cast<u32*>(node + 2) = 0; // num_cells
}

// ----- Statement preparation -----
PrepareResult prepare_statement(const std::string& input, Statement& statement) {
    if (input.rfind("insert", 0) == 0) {
        statement.type = StatementType::INSERT;
        std::istringstream ss(input);
        std::string keyword;
        ss >> keyword >> statement.row_to_insert.id >> statement.row_to_insert.username.data() >> statement.row_to_insert.email.data();
        if (ss.fail()) return PrepareResult::SYNTAX_ERROR;
        if (statement.row_to_insert.id < 0) return PrepareResult::NEGATIVE_ID;
        if (strlen(statement.row_to_insert.username.data()) > COLUMN_USERNAME_SIZE) return PrepareResult::STRING_TOO_LONG;
        if (strlen(statement.row_to_insert.email.data()) > COLUMN_EMAIL_SIZE) return PrepareResult::STRING_TOO_LONG;
        return PrepareResult::SUCCESS;
    }
    if (input == "select") {
        statement.type = StatementType::SELECT;
        return PrepareResult::SUCCESS;
    }
    return PrepareResult::UNRECOGNIZED_STATEMENT;
}

// ----- Execution -----
ExecuteResult execute_insert(const Statement& statement, Table* table) {
    // Simplified insert logic
    u8* page = table->pager.get_page(table->root_page_num);
    u32 num_cells = *reinterpret_cast<u32*>(page + 2);

    if (num_cells >= (PAGE_SIZE / ROW_SIZE)) {
        return ExecuteResult::DUPLICATE_KEY; // Table full
    }

    u8* cell = page + 6 + num_cells * ROW_SIZE;
    serialize_row(statement.row_to_insert, cell);
    (*reinterpret_cast<u32*>(page + 2))++;

    return ExecuteResult::SUCCESS;
}

ExecuteResult execute_select(const Statement&, Table* table) {
    u8* page = table->pager.get_page(table->root_page_num);
    u32 num_cells = *reinterpret_cast<u32*>(page + 2);

    for (u32 i = 0; i < num_cells; i++) {
        Row row;
        u8* cell = page + 6 + i * ROW_SIZE;
        deserialize_row(cell, row);
        std::cout << "(" << row.id << ", " << row.username.data() << ", " << row.email.data() << ")\n";
    }

    return ExecuteResult::SUCCESS;
}


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

