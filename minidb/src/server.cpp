#include "engine/engine_api.h"
#include "engine/engine_core.h"

#include <httplib.h>         
#include <nlohmann/json.hpp>  
#include <iostream>

using json = nlohmann::json;

int main() {
    Engine engine("mydb.db"); 

    httplib::Server svr;

    // POST /query endpoint
    svr.Post("/query", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string sql = body["query"];

            Statement statement;
            PrepareResult prep = engine.prepare(sql, statement);

            if (prep != PrepareResult::SUCCESS) {
                res.status = 400;
                res.set_content(R"({"error":"Bad query"})", "application/json");
                return;
            }

            ExecuteResult exec = engine.execute(statement);

            if (statement.type == StatementType::SELECT) {
                
                json result = json::array();
                u8* page = engine.get_table()->pager.get_page(engine.get_table()->root_page_num);
                u32 num_cells = *reinterpret_cast<u32*>(page + 2);

                for (u32 i = 0; i < num_cells; i++) {
                    Row row;
                    u8* cell = page + 6 + i * (sizeof(u32) + COLUMN_USERNAME_SIZE + 1 + COLUMN_EMAIL_SIZE + 1);
                    deserialize_row(cell, row);

                    result.push_back({
                        {"id", row.id},
                        {"username", row.username.data()},
                        {"email", row.email.data()}
                    });
                }

                res.set_content(result.dump(), "application/json");
            } else {
                res.set_content(R"({"status":"success"})", "application/json");
            }

        } catch (std::exception& e) {
            res.status = 500;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    std::cout << "Server running at http://localhost:8080\n";
    svr.listen("0.0.0.0", 8080);

    return 0;
}
