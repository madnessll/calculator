#include <stdio.h>
#include <string>
#include <iostream>
#include "math_lib.h"
#include <nlohmann/json.hpp>
#include "logger.h"

namespace calculator
{

class Application
{
    public:
        void run(int argc, char** argv)
        {
            getTask(argc, argv);
                if (task_.show_help) 
                    {
                        printHelp();
                        return;
                    }
            Logger::getInstance().info("Starting calculation");
            makeCalculate();
            Logger::getInstance().info("Calculation successful");
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
            printf("%d\n", task_.result);
        }
    private:
        struct Task
        {
            int first_num;
            int second_num;
            char operation;
            int result;
            bool show_help = false;
        };
    private:
        Task task_;    

};

}

int main(int argc, char* argv[])
{
    calculator::Application application;
    try
    {
        application.run(argc, argv);
    }
    catch (const std::overflow_error& e) 
    {
        Logger::getInstance().error(e.what());
        std::cout << "Error: " << e.what() << std::endl;
    }
    catch (const std::runtime_error& e) 
    {
        Logger::getInstance().error(e.what());
        std::cout << "Error: " << e.what() << std::endl;
    }
    catch (const nlohmann::json::exception& e) 
    {
        Logger::getInstance().error(e.what());
        std::cout << "Error: JSON parse error: " << e.what() << std::endl;
    }
    return 0;
}
