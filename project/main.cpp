#include "logger.h"
#include "math_lib.h"

#include <libpq-fe.h>

#include <nlohmann/json.hpp>

#include <cstdio>
#include <iostream>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <unordered_map>
#include <atomic>
#include <csignal>
#include <thread>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

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
        if (conn != nullptr)
        {
            PQfinish(conn);
        }
    }
};

struct PGresultDeleter
{
    void operator()(PGresult* res) const
    {
        if (res != nullptr)
        {
            PQclear(res);
        }
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

    DataBase() : connection_(nullptr)
    {}

    ~DataBase()
    {
        disconnect();
    }

    DataBase(const DataBase&) = delete;
    DataBase& operator=(const DataBase&) = delete;

    DataBase(DataBase&&) = default;
    DataBase& operator=(DataBase&&) = default;

    void connect()
    {
        const std::string connStr =
            "host=" + config_.host + " port=" + config_.port +
            " user=" + config_.username + " password=" + config_.password +
            " dbname=" + config_.dbname;

        connection_.reset(PQconnectdb(connStr.c_str()));

        if (PQstatus(connection_.get()) != CONNECTION_OK)
        {
            const std::string err = PQerrorMessage(connection_.get());
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
        const std::string key = makeKey(task);
        auto iter = cache_.find(key);
        if (iter != cache_.end())
        {
            return iter->second;
        }
        return std::nullopt;
    }

    static std::string makeKey(const Task& task)
    {
        int firstNum = task.first_num;
        int secondNum = task.second_num;
        if ((task.operation == '+' || task.operation == '*') &&
            firstNum > secondNum)
        {
            std::swap(firstNum, secondNum);
        }
        return std::to_string(firstNum) + task.operation +
               std::to_string(secondNum);
    }

    void writeRecord(const Task& task)
    {
        cache_[makeKey(task)] = task;

        const std::string query =
            "INSERT INTO operations (first_num, second_num, "
            "operation, result, status) VALUES (" +
            std::to_string(task.first_num) + ", " +
            std::to_string(task.second_num) + ", '" + task.operation + "', " +
            std::to_string(task.result) + ", " + std::to_string(task.status) +
            ")";

        const PGresultPtr res(PQexec(connection_.get(), query.c_str()));
        if (PQresultStatus(res.get()) != PGRES_COMMAND_OK)
        {
            throw std::runtime_error(
                "writeRecord failed: " +
                std::string(PQerrorMessage(connection_.get())));
        }
    }

    void warmUpCach()
    {
        const PGresultPtr res(PQexec(connection_.get(),
                                     "SELECT first_num, second_num, operation, "
                                     "result, status FROM operations"));

        if (PQresultStatus(res.get()) != PGRES_TUPLES_OK)
        {
            throw std::runtime_error("warmUpCach failed");
        }

        const int rows = PQntuples(res.get());
        for (int i = 0; i < rows; i++)
        {
            Task task;
            task.first_num = std::stoi(PQgetvalue(res.get(), i, 0));
            task.second_num = std::stoi(PQgetvalue(res.get(), i, 1));
            const std::string opStr = PQgetvalue(res.get(), i, 2);
            task.operation = opStr.at(0);
            task.result = std::stoi(PQgetvalue(res.get(), i, 3));
            task.status = std::stoi(PQgetvalue(res.get(), i, 4));
            cache_[makeKey(task)] = task;
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
        : acceptor_(ioContext_, tcp::endpoint(tcp::v4(), kPort_))
    {
        dataBase_.connect();
        dataBase_.warmUpCach();
    }

    ~Application()
    {
        dataBase_.disconnect();
    }

    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;

    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    void run()
    {
        // блокируем сигналы в главном потоке
        sigset_t signals;
        sigemptyset(&signals);
        sigaddset(&signals, SIGTERM);
        sigaddset(&signals, SIGINT);
        pthread_sigmask(SIG_BLOCK, &signals, nullptr);

        // запускаем сигнальный поток
        std::thread signalThread([this, &signals]()
        {
            int sig = 0;
            sigwait(&signals, &sig);
            Logger::getInstance().info("Signal received, stopping...");
            running_ = false;
            ioContext_.stop();
        });

        // основной цикл
        Logger::getInstance().info("Service started");
        while (running_)
        {
            try
            {
                tcp::socket socket(ioContext_);
                acceptor_.accept(socket);
                handleClient(socket);
            }
            catch (const boost::system::system_error& err)
            {
                if (!running_)
                {
                    break;
                }
                Logger::getInstance().error(err.what());
            }
        }

        signalThread.join();
        Logger::getInstance().info("Service stopped");
    }

  private:
    void handleClient(tcp::socket& socket)
    {
        boost::asio::streambuf buffer;
        boost::system::error_code errorCode;
        boost::asio::read_until(socket, buffer, '\n', errorCode);
        if (errorCode)
        {
            return;
        }

        std::istream stream(&buffer);
        std::string line;
        std::getline(stream, line);

        try
        {
            nlohmann::json json = nlohmann::json::parse(line);
            task_.first_num = json.at("a").get<int>();
            task_.second_num = json.value("b", 0);
            const std::string opStr = json.at("op").get<std::string>();
            task_.operation = opStr.at(0);

            std::optional<Task> dbRecord = dataBase_.getRecord(task_);
            if (!dbRecord)
            {
                Logger::getInstance().info("Cache miss, calculating...");
                try
                {
                    makeCalculate();
                    task_.status = 0;
                }
                catch (const std::overflow_error& err)
                {
                    Logger::getInstance().error(err.what());
                    task_.result = 0;
                    task_.status = 1;
                }
                catch (const std::runtime_error& err)
                {
                    Logger::getInstance().error(err.what());
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

            nlohmann::json response;
            response["result"] = task_.result;
            response["status"] = task_.status;
            const std::string responseStr = response.dump() + "\n";
            boost::asio::write(socket, boost::asio::buffer(responseStr), errorCode);
        }
        catch (const std::exception& err)
        {
            Logger::getInstance().error(err.what());
            const std::string errResponse = 
                std::string(R"({"error":")") + err.what() + "\"}\n";
            boost::asio::write(socket, boost::asio::buffer(errResponse), errorCode);
        }
    }
    void makeCalculate()
    {
        switch (task_.operation)
        {
            case '+':
                math_lib::Math::calculateSum(task_.first_num, task_.second_num,
                                             task_.result);
                break;
            case '-':
                math_lib::Math::calculateDifference(
                    task_.first_num, task_.second_num, task_.result);
                break;
            case '*':
                math_lib::Math::calculateMult(task_.first_num, task_.second_num,
                                              task_.result);
                break;
            case '/':
                math_lib::Math::calculateDivision(
                    task_.first_num, task_.second_num, task_.result);
                break;
            case '^':
                math_lib::Math::calculatePower(task_.first_num,
                                               task_.second_num, task_.result);
                break;
            case '!':
                math_lib::Math::calculateFactorial(task_.first_num,
                                                   task_.result);
                break;
            default:
                throw std::runtime_error("Unknown operation");
        }
    }
    boost::asio::io_context ioContext_;
    tcp::acceptor acceptor_;
    std::atomic<bool> running_{true};
    Task task_;
    DataBase dataBase_;
    static constexpr int kPort_ = 12345;
};

} // namespace calculator

int main()
{
    try
    {
        calculator::Application application;
        application.run();
    }
    catch (const std::exception& e)
    {
        Logger::getInstance().error(e.what());
        std::cout << "Error: " << e.what() << '\n';
    }
    return 0;
}
