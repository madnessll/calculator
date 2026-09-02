#include <gtest/gtest.h>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <chrono>
#include <string>

using boost::asio::ip::tcp;

std::string sendRequest(const std::string& request)
{
    boost::asio::io_context ioContext;
    tcp::socket socket(ioContext);
    tcp::resolver resolver(ioContext);
    
    const auto endpoints = resolver.resolve("localhost", "12345");
    boost::asio::connect(socket, endpoints);
    
    const std::string message = request + "\n";
    boost::asio::write(socket, boost::asio::buffer(message));
    
    boost::asio::streambuf buffer;
    boost::asio::read_until(socket, buffer, '\n');
    
    std::istream stream(&buffer);
    std::string response;
    std::getline(stream, response);
    return response;
}

TEST(ClientTest, Addition)
{
    const std::string response = sendRequest(R"({"a": 5, "b": 3, "op": "+"})");
    const nlohmann::json json = nlohmann::json::parse(response);
    EXPECT_EQ(json["result"].get<int>(), 8);
    EXPECT_EQ(json["status"].get<int>(), 0);
}

TEST(ClientTest, Division)
{
    const std::string response = sendRequest(R"({"a": 10, "b": 2, "op": "/"})");
    const nlohmann::json json = nlohmann::json::parse(response);
    EXPECT_EQ(json["result"].get<int>(), 5);
    EXPECT_EQ(json["status"].get<int>(), 0);
}

TEST(ClientTest, DivisionByZero)
{
    const std::string response = sendRequest(R"({"a": 5, "b": 0, "op": "/"})");
    const nlohmann::json json = nlohmann::json::parse(response);
    EXPECT_EQ(json["status"].get<int>(), 2);
}

TEST(ClientTest, Factorial)
{
    const std::string response = sendRequest(R"({"a": 5, "op": "!"})");
    const nlohmann::json json = nlohmann::json::parse(response);
    EXPECT_EQ(json["result"].get<int>(), 120);
    EXPECT_EQ(json["status"].get<int>(), 0);
}