#include <sqlite3.h>
#include <iostream>
#include <string>

int main()
{
  sqlite3 *db;
  char *errMsg = nullptr;

  // 1. 打开内存数据库
  if (sqlite3_open(":memory:", &db))
  {
    std::cerr << "Cannot open database\n";
    return 1;
  }

  // 2. 允许加载扩展
  sqlite3_enable_load_extension(db, 1);

  // 3. 加载 graph 扩展
  if (sqlite3_load_extension(db, "./libgraph", 0, &errMsg) != SQLITE_OK)
  {
    std::cerr << "Load extension failed: " << errMsg << "\n";
    sqlite3_free(errMsg);
    return 1;
  }

  // 4. 创建 graph 虚拟表
  if (sqlite3_exec(db, "CREATE VIRTUAL TABLE graph USING graph();", 0, 0, &errMsg) != SQLITE_OK)
  {
    std::cerr << errMsg << "\n";
    sqlite3_free(errMsg);
  }

  // 5. 插入节点（Cypher）
  sqlite3_exec(db,
               "SELECT cypher_execute('CREATE (p:Person {name: \"Alice\", age: 30})');",
               0, 0, &errMsg);

  sqlite3_exec(db,
               "SELECT cypher_execute('CREATE (p:Person {name: \"Bob\", age: 25})');",
               0, 0, &errMsg);

  // 6. 插入关系（注意转义）
  sqlite3_exec(db,
               "SELECT cypher_execute(\"CREATE (a:Person {name: \\\"Alice\\\"})-[:KNOWS]->(b:Person {name: \\\"Bob\\\"})\");",
               0, 0, &errMsg);

  // 7. 查询
  sqlite3_stmt *stmt;
  const char *sql =
      "SELECT cypher_execute('MATCH (p:Person) WHERE p.age > 25 RETURN p');";

  if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK)
  {
    std::cerr << "Prepare failed\n";
    return 1;
  }

  // 8. 读取结果（JSON字符串）
  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    const unsigned char *result = sqlite3_column_text(stmt, 0);
    std::cout << "Query Result :\n";
    std::cout << result << std::endl;
  }

  sqlite3_finalize(stmt);

  sqlite3_close(db);

  return 0;
}