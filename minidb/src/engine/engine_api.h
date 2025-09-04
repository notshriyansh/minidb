#pragma once
#include "engine_core.h"
#include <string>

class Engine {
public:
    explicit Engine(const std::string& filename);
    ~Engine();

    PrepareResult prepare(const std::string& input, Statement& statement);
    ExecuteResult execute(const Statement& statement);

    Table* get_table();

private:
    Table* table;
};
