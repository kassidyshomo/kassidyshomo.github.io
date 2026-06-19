#include <iostream>
#include <string>
#include <tuple>
#include <vector>

#include "sqlite3.h"

// Represents a single user record returned from the database
typedef std::tuple<std::string, std::string, std::string> user_record;

/*
SQLite callback function used to process query results.

Original artifact behavior:
- Earlier versions directly printed results without flexible handling
- Limited ability to reuse query results in program logic

Enhancement improvements:
- Supports both console output and vector storage modes
- Prevents null pointer crashes when reading SQLite results
- Enables reusable query result handling for multiple workflows
*/
static int callback(void* possible_vector, int argc, char** argv, char** azColName)
{
    if (possible_vector == NULL)
    {
        // Direct output mode: print query results to console
        for (int i = 0; i < argc; i++)
        {
            std::cout << azColName[i] << " = "
                << (argv[i] ? argv[i] : "NULL")
                << std::endl;
        }
        std::cout << std::endl;
    }
    else
    {
        // Vector storage mode: save results for later processing
        std::vector<user_record>* rows =
            static_cast<std::vector<user_record>*>(possible_vector);

        rows->push_back(std::make_tuple(
            argv[0] ? argv[0] : "",
            argv[1] ? argv[1] : "",
            argv[2] ? argv[2] : ""
        ));
    }

    return 0;
}

/*
Initializes the in-memory USERS database and inserts sample data.

Original artifact behavior:
- Database setup and initialization were handled in a single simple execution flow
- Limited separation between schema creation and data insertion

Enhancement improvements:
- Separates schema creation and data insertion steps
- Adds structured error handling using SQLite return codes
- Improves clarity of database initialization process
*/
bool initialize_database(sqlite3* db)
{
    char* error_message = NULL;

    // Create USERS table schema
    std::string sql =
        "CREATE TABLE USERS("
        "ID INT PRIMARY KEY NOT NULL,"
        "NAME TEXT NOT NULL,"
        "PASSWORD TEXT NOT NULL);";

    int result = sqlite3_exec(db, sql.c_str(), callback, NULL, &error_message);

    // Validate table creation success
    if (result != SQLITE_OK)
    {
        std::cout << "Failed to create USERS table. ERROR = "
            << error_message << std::endl;
        sqlite3_free(error_message);
        return false;
    }

    // Insert sample user data
    sql =
        "INSERT INTO USERS (ID, NAME, PASSWORD) VALUES (1, 'Kassidy', 'Shomo');"
        "INSERT INTO USERS (ID, NAME, PASSWORD) VALUES (2, 'Vincent', 'Tevnan');"
        "INSERT INTO USERS (ID, NAME, PASSWORD) VALUES (3, 'Karissa', 'Shomo');"
        "INSERT INTO USERS (ID, NAME, PASSWORD) VALUES (4, 'Lori', 'Silbaugh');";

    result = sqlite3_exec(db, sql.c_str(), callback, NULL, &error_message);

    // Validate insert success
    if (result != SQLITE_OK)
    {
        std::cout << "Data failed to insert to USERS table. ERROR = "
            << error_message << std::endl;
        sqlite3_free(error_message);
        return false;
    }

    return true;
}

/*
Executes a parameterized SQL query using prepared statements.

Original artifact behavior:
- SQL queries were vulnerable to injection through string concatenation
- No separation between SQL structure and user input
- No input binding or parameterization

Enhancement improvements:
- Uses SQLite prepared statements instead of raw SQL execution
- Binds user input parameters safely to prevent SQL injection
- Separates query structure from input values
- Improves safety and consistency of database interactions
*/
bool execute_safe_query(
    sqlite3* db,
    const std::string& query,
    const std::vector<std::string>& parameters,
    std::vector<user_record>& records)
{
    // Clear previous query results
    records.clear();

    sqlite3_stmt* statement = nullptr;

    // Prepare SQL statement before execution
    int result = sqlite3_prepare_v2(
        db,
        query.c_str(),
        -1,
        &statement,
        nullptr
    );

    if (result != SQLITE_OK)
    {
        std::cout << "Query preparation failed. ERROR = "
            << sqlite3_errmsg(db) << std::endl;
        return false;
    }

    // Bind parameters to prevent SQL injection
    for (size_t i = 0; i < parameters.size(); ++i)
    {
        sqlite3_bind_text(
            statement,
            static_cast<int>(i + 1),
            parameters[i].c_str(),
            -1,
            SQLITE_TRANSIENT
        );
    }

    // Execute query and process results row by row
    while ((result = sqlite3_step(statement)) == SQLITE_ROW)
    {
        const unsigned char* id_txt =
            sqlite3_column_text(statement, 0);
        const unsigned char* name_txt =
            sqlite3_column_text(statement, 1);
        const unsigned char* pass_txt =
            sqlite3_column_text(statement, 2);

        records.emplace_back(
            id_txt ? reinterpret_cast<const char*>(id_txt) : "",
            name_txt ? reinterpret_cast<const char*>(name_txt) : "",
            pass_txt ? reinterpret_cast<const char*>(pass_txt) : ""
        );
    }

    // Check for execution errors after stepping through results
    if (result != SQLITE_DONE)
    {
        std::cout << "Query execution failed. ERROR = "
            << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(statement);
        return false;
    }

    // Free prepared statement resources
    sqlite3_finalize(statement);

    return true;
}

/*
Retrieves a user by name using a parameterized query.

Enhancement focus:
- Eliminates SQL injection risk by using parameter binding
- Encapsulates query logic into reusable function
*/
bool get_user_by_name(
    sqlite3* db,
    const std::string& name,
    std::vector<user_record>& records)
{
    return execute_safe_query(
        db,
        "SELECT ID, NAME, PASSWORD FROM USERS WHERE NAME = ?;",
        { name },
        records
    );
}

/*
Displays query results in a readable format.

Enhancement improvement:
- Separates presentation logic from database execution logic
*/
void dump_results(
    const std::string& sql,
    const std::vector<user_record>& records)
{
    std::cout << "\nSQL: " << sql
        << " ==> " << records.size()
        << " records found.\n";

    for (const auto& record : records)
    {
        std::cout << "User: " << std::get<1>(record)
            << " [UID=" << std::get<0>(record)
            << " PWD=" << std::get<2>(record)
            << "]\n";
    }
}

/*
Runs database queries including safe queries and injection test cases.

Enhancement improvements:
- Demonstrates use of prepared statements instead of unsafe SQL concatenation
- Includes test cases that simulate SQL injection attempts
- Validates that unsafe inputs do not affect query structure
*/
void run_queries(sqlite3* db)
{
    std::vector<user_record> records;

    // Query all users in database
    std::string query_all =
        "SELECT ID, NAME, PASSWORD FROM USERS;";

    if (!execute_safe_query(db, query_all, {}, records))
        return;

    dump_results(query_all, records);

    // Normal lookup test
    if (!get_user_by_name(db, "Fred", records))
        return;

    dump_results(
        "SELECT ID, NAME, PASSWORD FROM USERS WHERE NAME='Fred' (display only)",
        records
    );

    // Test inputs designed to simulate SQL injection attempts
    std::vector<std::string> test_inputs = {
        "Fred",
        "' OR 1=1 --",
        "' OR 'hi'='hi",
        "' OR 'hack'='hack",
        "'; DROP TABLE USERS; --"
    };

    // Execute each test input safely using parameterized queries
    for (const auto& input : test_inputs)
    {
        if (!get_user_by_name(db, input, records))
            continue;

        std::cout << "\nTesting Input: " << input << std::endl;

        dump_results(
            "SELECT ID, NAME, PASSWORD FROM USERS WHERE NAME = ?",
            records
        );
    }
}

int main()
{
    std::cout << "SQL Injection Example\n";

    sqlite3* db = nullptr;

    // Open in-memory database connection
    int result = sqlite3_open(":memory:", &db);

    if (result != SQLITE_OK)
    {
        std::cout << "Failed to connect to database. ERROR="
            << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    std::cout << "Connected to the database.\n";

    // Initialize database schema and sample data
    if (!initialize_database(db))
    {
        std::cout << "Database Initialization Failed.\n";
        sqlite3_close(db);
        return -1;
    }

    // Execute query demonstrations
    run_queries(db);

    // Close database connection
    sqlite3_close(db);

    return 0;
}