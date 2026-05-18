#include "math_lib.h"
#include <getopt.h>  
#include <stdio.h>
#include <unistd.h>

struct Task
{
    int first_num;
    int second_num;
    char operation;
    int result;
    bool status = false;
    bool show_help = false;
};

// DRY
int charToInt(const char* str)
{
    int num = 0;
    int sign = 1;
    int i = 0;
    if (str[i] == '-')
    {
        sign = -1;
        i++;
    }
    for (i; str[i] != '\0'; ++i)
    {
        num = num * 10 + (str[i] - '0');
    }
    return num * sign;
}

void printHelp() {
    printf("Usage: calc -a <num1> -b <num2> -o <operation>\n");
    printf("Options:\n");
    printf("  -a <num1>       First integer number\n");
    printf("  -b <num2>       Second integer number\n");
    printf("  -o <operation>  Operation to perform\n");
    printf("  -h, --help      Show this help message\n\n");
    printf("Operations:\n");
    printf("  +   Addition        (a + b)\n");
    printf("  -   Subtraction     (a - b)\n");
    printf("  *   Multiplication  (a * b)\n");
    printf("  /   Division        (a / b)\n");
    printf("  ^   Power           (a ^ b)\n");
    printf("  !   Factorial       (a!)\n\n");
    printf("Examples:\n");
    printf(" -a 5 -b 3 -o +\n");
    printf(" -a 5 -b 3 -o /\n");
    printf(" -a 5 -o !\n");
}

void makeTask(int argc, char* argv[], Task& t)
{
    int opt;

    static struct option long_options[] = {
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "a:b:o:h", long_options, NULL)) != -1)
    {
        switch (opt)
        {
            case 'a':
                t.first_num = charToInt(optarg);
                break;
            case 'b':
                t.second_num = charToInt(optarg);
                break;
            case 'o':
                t.operation = optarg[0];
                break;
            case 'h':
                t.show_help = true;
                break;
        }
    }
}

void makeCalculate(Task& t)
{
    switch (t.operation)
    {
        case '+':
            t.status =
                math_lib::calculateSum(t.first_num, t.second_num, t.result);
            break;
        case '-':
            t.status = math_lib::calculateDifference(t.first_num, t.second_num,
                                                     t.result);
            break;
        case '*':
            t.status =
                math_lib::calculateMult(t.first_num, t.second_num, t.result);
            break;
        case '/':
            t.status = math_lib::calculateDivision(t.first_num, t.second_num,
                                                   t.result);
            break;
        case '^':
            t.status =
                math_lib::calculatePower(t.first_num, t.second_num, t.result);
            break;
        case '!':
            t.status = math_lib::calculateFactorial(t.first_num, t.result);
            break;
    }
}

void printResult(Task& t)
{
    if (t.status)
    {
        printf("%d\n", t.result);
    }
    else
    {
        printf("type overflow error occurred\n");
    }
}

void applicationRun(int argc, char* argv[])
{
    Task t;
    makeTask(argc, argv, t);
    if (t.show_help) {
        printHelp();
        return;
    }
    makeCalculate(t);
    printResult(t);
}

int main(int argc, char* argv[])
{
    applicationRun(argc, argv);
    return 0;
}
