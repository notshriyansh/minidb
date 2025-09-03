#pragma once
#include <array>
#include <cstdint>
#include <memory>
#include <string>

//  Aliases 
using u8 = uint8_t;
using u32 = uint32_t;

//  Constants
constexpr size_t COLUMN_USERNAME_SIZE = 32;
constexpr size_t COLUMN_EMAIL_SIZE = 255;

// ROw
struct Row {
    u32 id;
    std::array<char, COLUMN_USERNAME_SIZE + 1> username{};
    std::array<char, COLUMN_EMAIL_SIZE + 1> email{};
};

//  Statement 
enum class StatementType { INSERT, SELECT };

struct Statement {
    StatementType type;
    Row row_to_insert{};
};

//  Result enums
enum class ExecuteResult { SUCCESS, DUPLICATE_KEY };
enum class PrepareResult {
    SUCCESS,
    NEGATIVE_ID,
    STRING_TOO_LONG,
    SYNTAX_ERROR,
    UNRECOGNIZED_STATEMENT
};

// Forward declarations
class Table;
class Cursor;

// API 
Table* db_open(const std::string& filename);
void db_close(Table* table);

PrepareResult prepare_statement(const std::string& input, Statement& statement);
ExecuteResult execute_statement(const Statement& statement, Table* table);

std::unique_ptr<Cursor> table_start(Table* table);
void deserialize_row(u8* source, Row& destination);
