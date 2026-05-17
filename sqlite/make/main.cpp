#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <string>

#define DB_NAME "json_benchmark.db"
#define DATA_SIZE 10000

//  修改SELECT语句：新增type、device_id、room、status、timestamp字段（从JSON中提取）
//  贴合json_table的data字段结构，展示更全面的设备信息
const std::string SQL_QUERY_TEMP =
    "SELECT id, type, temperature, "
    "json_extract(data, '$.device_id') AS device_id, "
    "json_extract(data, '$.room') AS room, "
    "json_extract(data, '$.status') AS status "
    "FROM json_table WHERE temperature > 10 LIMIT 5;";

const std::string SQL_QUERY_SENSOR =
    "SELECT id, type, temperature, "
    "json_extract(data, '$.device_id') AS device_id, "
    "json_extract(data, '$.room') AS room, "
    "json_extract(data, '$.status') AS status, "
    "json_extract(data, '$.timestamp') AS timestamp "
    "FROM json_table WHERE type = 'sensor' LIMIT 5;";

const std::string SQL_UPDATE_STATUS =
    "UPDATE json_table "
    "SET data = json_set(data, '$.status', 'off') "
    "WHERE type = 'sensor';";

const std::string SQL_REMOVE_HUMIDITY =
    "UPDATE json_table "
    "SET data = json_remove(data, '$.humidity') "
    "WHERE temperature > 25;";

const std::string SQL_DELETE_CAMERA =
    "DELETE FROM json_table WHERE type = 'camera';";

std::string random_json()
{
    static std::mt19937 gen(42);

    std::vector<std::string> types = {"sensor", "light", "camera"};
    std::vector<std::string> rooms = {"living_room", "bedroom", "kitchen"};
    std::vector<std::string> status = {"on", "off"};

    std::uniform_int_distribution<> t(0, 2);
    std::uniform_int_distribution<> r(0, 2);
    std::uniform_int_distribution<> s(0, 1);
    std::uniform_real_distribution<float> temp(18, 30);
    std::uniform_real_distribution<float> hum(30, 80);

    std::string json = "{";
    json += "\"device_id\":\"dev_" + std::to_string(gen() % 10000) + "\",";
    json += "\"type\":\"" + types[t(gen)] + "\",";
    json += "\"room\":\"" + rooms[r(gen)] + "\",";
    json += "\"status\":\"" + status[s(gen)] + "\",";
    json += "\"temperature\":" + std::to_string(temp(gen)) + ",";
    json += "\"humidity\":" + std::to_string(hum(gen)) + ",";
    json += "\"timestamp\":" + std::to_string(1700000000 + gen() % 100000);
    json += "}";

    return json;
}

//  优化打印逻辑：适配新增查询字段，格式化输出，处理空指针，展示更清晰
void run_query_and_print(sqlite3 *db, const std::string &sql)
{
    sqlite3_stmt *stmt;
    //  增加错误判断，避免程序异常
    int ret = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        std::cerr << "查询语句准备失败：" << sqlite3_errmsg(db) << std::endl;
        return;
    }

    std::cout << "执行SQL: " << sql << "\n";
    std::cout << "查询结果展示：" << std::endl;
    //  打印表头，对应查询字段，保持对齐
    std::cout << "id\t类型\t温度\t\t设备ID\t\t房间\t\t状态\t\t时间戳" << std::endl;
    std::cout << "-------------------------------------------------------------------------" << std::endl;

    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        //  提取所有查询字段，按SELECT语句顺序对应，处理空指针避免异常
        int id = sqlite3_column_int(stmt, 0);
        const char* type = (const char*)sqlite3_column_text(stmt, 1);
        double temperature = sqlite3_column_double(stmt, 2);
        const char* device_id = (const char*)sqlite3_column_text(stmt, 3);
        const char* room = (const char*)sqlite3_column_text(stmt, 4);
        const char* status = (const char*)sqlite3_column_text(stmt, 5);
        const char* timestamp = (const char*)sqlite3_column_text(stmt, 6); // 仅SQL_QUERY_SENSOR有该字段

        //  字段为空时替换为"NULL"，避免输出乱码
        std::string type_str = type ? type : "NULL";
        std::string device_id_str = device_id ? device_id : "NULL";
        std::string room_str = room ? room : "NULL";
        std::string status_str = status ? status : "NULL";
        std::string timestamp_str = timestamp ? timestamp : "NULL";

        //  格式化输出，控制温度保留2位小数，字段对齐，便于查看
        printf("%d\t%s\t%.2f\t\t%s\t%s\t%s\t%s\n",
               id, type_str.c_str(), temperature,
               device_id_str.c_str(), room_str.c_str(), status_str.c_str(), timestamp_str.c_str());
    }

    std::cout << "----------------------\n";
    sqlite3_finalize(stmt);
}

int main()
{
    sqlite3 *db;
    int ret = sqlite3_open(DB_NAME, &db);
    if (ret != SQLITE_OK)
    {
        std::cerr << "数据库打开失败：" << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    char *errMsg = nullptr;

    sqlite3_exec(db, "DROP TABLE IF EXISTS json_table;", 0, 0, 0);

    sqlite3_exec(db,
                 "CREATE TABLE json_table ("
                 "id INTEGER PRIMARY KEY, "
                 "data TEXT, "
                 "temperature REAL GENERATED ALWAYS AS (json_extract(data, '$.temperature')) VIRTUAL, "
                 "type TEXT GENERATED ALWAYS AS (json_extract(data, '$.type')) VIRTUAL"
                 ");",
                 0, 0, &errMsg);
    if (errMsg)
    {
        std::cerr << "表创建失败：" << errMsg << std::endl;
        sqlite3_free(errMsg);
        errMsg = nullptr;
    }

    sqlite3_exec(db,
                 "CREATE INDEX idx_temp ON json_table(temperature);",
                 0, 0, &errMsg);
    if (errMsg)
    {
        std::cerr << "索引idx_temp创建失败：" << errMsg << std::endl;
        sqlite3_free(errMsg);
        errMsg = nullptr;
    }

    sqlite3_exec(db,
                 "CREATE INDEX idx_type ON json_table(type);",
                 0, 0, &errMsg);
    if (errMsg)
    {
        std::cerr << "索引idx_type创建失败：" << errMsg << std::endl;
        sqlite3_free(errMsg);
        errMsg = nullptr;
    }

    sqlite3_stmt *stmt;
    ret = sqlite3_prepare_v2(db,
                           "INSERT INTO json_table (data) VALUES (?);",
                           -1, &stmt, NULL);
    if (ret != SQLITE_OK)
    {
        std::cerr << "插入语句准备失败：" << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return -1;
    }

    sqlite3_exec(db, "BEGIN;", 0, 0, 0);

    for (int i = 0; i < DATA_SIZE; i++)
    {
        std::string json = random_json();
        sqlite3_bind_text(stmt, 1, json.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }

    sqlite3_exec(db, "COMMIT;", 0, 0, 0);

    sqlite3_finalize(stmt);

    std::cout << "\n===== 更新前查询结果 =====\n";
    run_query_and_print(db, SQL_QUERY_TEMP);

    sqlite3_exec(db, SQL_UPDATE_STATUS.c_str(), 0, 0, &errMsg);
    if (errMsg)
    {
        std::cerr << "状态更新失败：" << errMsg << std::endl;
        sqlite3_free(errMsg);
        errMsg = nullptr;
    }

    std::cout << "\n===== 更新后查询结果 =====\n";
    run_query_and_print(db, SQL_QUERY_SENSOR);

    sqlite3_exec(db, SQL_REMOVE_HUMIDITY.c_str(), 0, 0, &errMsg);
    if (errMsg)
    {
        std::cerr << "湿度字段删除失败：" << errMsg << std::endl;
        sqlite3_free(errMsg);
        errMsg = nullptr;
    }

    sqlite3_exec(db, SQL_DELETE_CAMERA.c_str(), 0, 0, &errMsg);
    if (errMsg)
    {
        std::cerr << "相机类型记录删除失败：" << errMsg << std::endl;
        sqlite3_free(errMsg);
        errMsg = nullptr;
    }

    std::cout << "\n===== 删除后查询结果 =====\n";
    run_query_and_print(db, SQL_QUERY_TEMP);

    sqlite3_close(db);

    return 0;
}
