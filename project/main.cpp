#include <stdio.h>
#include <string>
#include <iostream>
#include "math_lib.h"
#include <nlohmann/json.hpp>
#include "logger.h"
#include <libpq-fe.h>
#include <optional>
#include <unordered_map>
#include <memory>

namespace calculator
{

struct Task
{
    int first_num{0};
    int second_num{0};
    char operation{'\0'};
    int result{0};
    int status{0};
    bool show_help{false};
};

struct PGconnDeleter 
{
    void operator()(PGconn* conn) const 
    {
        if (conn) PQfinish(conn);
    }
};

struct PGresultDeleter 
{
    void operator()(PGresult* res) const 
    {
        if (res) PQclear(res);
    }
};

using PGresultPtr = std::unique_ptr<PGresult, PGresultDeleter>;

class DataBase 
{
public:
    struct Config 
    {
        std::string host{"localhost"};
        std::string port{"5432"};
        std::string username{"calculator_user"}; 
        std::string password{"iwillwin"};
        std::string dbname{"calculator"};
    };

    DataBase() : connection_(nullptr) {}

    ~DataBase() { disconnect(); }

    void connect() 
    {
        std::string connStr = 
            "host=" + config_.host +
            " port=" + config_.port +
            " user=" + config_.username +
            " password=" + config_.password +
            " dbname=" + config_.dbname;

        connection_.reset(PQconnectdb(connStr.c_str()));

        if (PQstatus(connection_.get()) != CONNECTION_OK) 
        {
            std::string err = PQerrorMessage(connection_.get());
            connection_.reset();
            throw std::runtime_error("DB connection failed: " + err);
        }
    }

    void disconnect() 
    {
        connection_.reset(); 
    }

    std::optional<Task> getRecord(const Task& task) 
    {
        std::string key = makeKey(task);
        auto it = cache_.find(key);
        if (it != cache_.end())
            return it->second;
        return std::nullopt;
    }

    std::string makeKey(const Task& task) const 
    {
        int a = task.first_num;
        int b = task.second_num;
        if ((task.operation == '+' || task.operation == '*') && a > b)
            std::swap(a, b);
        return std::to_string(a) + task.operation + std::to_string(b);
    }

    void writeRecord(const Task& task) 
    {
        cache_[makeKey(task)] = task;

        std::string query = 
            "INSERT INTO operations (first_num, second_num, operation, result, status) VALUES (" +
            std::to_string(task.first_num) + ", " +
            std::to_string(task.second_num) + ", '" +
            task.operation + "', " +
            std::to_string(task.result) + ", " +
            std::to_string(task.status) + ")";

        PGresultPtr res(PQexec(connection_.get(), query.c_str()));
        if (PQresultStatus(res.get()) != PGRES_COMMAND_OK) 
            throw std::runtime_error("writeRecord failed: " + std::string(PQerrorMessage(connection_.get())));
    }

    void warmUpCach() {
        PGresultPtr res(PQexec(connection_.get(), 
            "SELECT first_num, second_num, operation, result, status FROM operations"));
        
        if (PQresultStatus(res.get()) != PGRES_TUPLES_OK) 
            throw std::runtime_error("warmUpCach failed");

        int rows = PQntuples(res.get());
        for (int i = 0; i < rows; i++) 
        {
            Task t;
            t.first_num  = std::stoi(PQgetvalue(res.get(), i, 0));
            t.second_num = std::stoi(PQgetvalue(res.get(), i, 1));
            t.operation  = PQgetvalue(res.get(), i, 2)[0];
            t.result     = std::stoi(PQgetvalue(res.get(), i, 3));
            t.status     = std::stoi(PQgetvalue(res.get(), i, 4));
            cache_[makeKey(t)] = t;
        }
    }


private:
    Config config_;
    std::unique_ptr<PGconn, PGconnDeleter> connection_;
    std::unordered_map<std::string, Task> cache_;
};

class Application
{
    public:
        Application()
        {
            dataBase_.connect();
            dataBase_.warmUpCach();
        }

        ~Application()
        {
            dataBase_.disconnect();
        }
        void run(int argc, char** argv)
        {
            getTask(argc, argv);
            if (task_.show_help)
            {
                printHelp();
                return;
            }

            std::optional<Task> dbRecord = dataBase_.getRecord(task_);
            if (!dbRecord)
            {
                Logger::getInstance().info("Cache miss, calculating...");
                try
                {
                    makeCalculate();
                    task_.status = 0;
                }
                catch(const std::overflow_error& e)
                {
                    Logger::getInstance().error(e.what());
                    task_.result = 0;
                    task_.status = 1;
                }
                catch (const std::runtime_error& e)
                {
                    Logger::getInstance().error(e.what());
                    task_.result = 0;
                    task_.status = 2;                    
                }
                dataBase_.writeRecord(task_);
            }
            else
            {
                Logger::getInstance().info("Cache hit!");
                task_ = *dbRecord;
            }
            printResult();
        }
    private:
        void printHelp() 
        {
            printf("Usage:\n");
            printf("  calculator '{\"a\":5,\"b\":3,\"op\":\"+\"}'\n\n");

            printf("JSON fields:\n");
            printf("  a   First integer number\n");
            printf("  b   Second integer number\n");
            printf("  op  Operation\n\n");

            printf("Operations:\n");
            printf("  +   Addition\n");
            printf("  -   Subtraction\n");
            printf("  *   Multiplication\n");
            printf("  /   Division\n");
            printf("  ^   Power\n");
            printf("  !   Factorial\n\n");

            printf("Examples:\n");
            printf("  calculator '{\"a\":5,\"b\":3,\"op\":\"+\"}'\n");
            printf("  calculator '{\"a\":10,\"b\":2,\"op\":\"/\"}'\n");
            printf("  calculator '{\"a\":5,\"op\":\"!\"}'\n");
        }
        void getTask(int argc, char** argv)
        {
            if (argc < 2)
                throw std::runtime_error("No input provided. Use --help for usage.");

            if (std::string(argv[1]) == "--help" || std::string(argv[1]) == "-h") 
            {
                task_.show_help = true;
                return;
            }
            nlohmann::json j = nlohmann::json::parse(argv[1]); 
            task_.first_num  = j.at("a").get<int>();
            task_.second_num = j.value("b", 0);
            task_.operation  = j.at("op").get<std::string>()[0];
        }
        void makeCalculate()
        {
            switch (task_.operation)
            {
                case '+':
                    math_lib::Math::calculateSum(task_.first_num, task_.second_num, task_.result);
                    break;
                case '-':
                    math_lib::Math::calculateDifference(task_.first_num, task_.second_num,
                                                        task_.result);
                    break;
                case '*':
                    math_lib::Math::calculateMult(task_.first_num, task_.second_num, task_.result);
                    break;
                case '/':
                     math_lib::Math::calculateDivision(task_.first_num, task_.second_num,
                                                        task_.result);
                    break;
                case '^':
                    math_lib::Math::calculatePower(task_.first_num, task_.second_num, task_.result);
                    break;
                case '!':
                    math_lib::Math::calculateFactorial(task_.first_num, task_.result);
                    break;
            }
        }
        void printResult() const
        {
            if (task_.status == 0)
                printf("%d\n", task_.result);
            else
                printf("Error: operation failed with status %d\n", task_.status);
        }

    private:
        Task task_;    
        DataBase dataBase_;
};

}

int main(int argc, char* argv[])
{
    try
    {
        calculator::Application application;
        application.run(argc, argv);
    }
    catch (const nlohmann::json::exception& e) 
    {
        Logger::getInstance().error(e.what());
        std::cout << "Error: JSON parse error: " << e.what() << std::endl;
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().error(e.what());
        std::cout << "Error: " << e.what() << '\n';
    }
    return 0;
}
