#include <stdio.h>
#include <unistd.h>

#include "../math_lib/math_lib.h"

struct Task {
    double first_num;
    double second_num;
    char operation;
    double result;
    bool status = false;
};

// DRY
double charToInt(const char* str) {
    double num = 0;
    int sign = 1;
    int i = 0;
    if (str[i] == '-') {
        sign = -1;
        i++;
    }
    for (i; str[i] != '\0'; ++i) {
        num = num * 10 + (str[i] - '0');
    }
    return num * sign;
}

void makeTask(int argc, char* argv[], Task& t) {
    int opt;

    while ((opt = getopt(argc, argv, "a:b:o:")) != -1) {
        switch (opt) {
            case 'a':
                t.first_num = charToInt(optarg);
                break;
            case 'b':
                t.second_num = charToInt(optarg);
                break;
            case 'o':
                t.operation = optarg[0];
                break;
        }
    }
}

void makeCalculate(Task& t) {
    switch (t.operation) {
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

void printResult(Task& t) {
    if (t.status) {
        double intPart = (long long)t.result;
        if (t.result == intPart) {
            printf("%.0f\n", t.result);
        } else {
            printf("%.2f\n", t.result);
        }
    } else {
        printf("type overflow error occurred\n");
    }
}

void applicationRun(int argc, char* argv[]) {
    Task t;
    makeTask(argc, argv, t);
    makeCalculate(t);
    printResult(t);
}

int main(int argc, char* argv[]) {
    applicationRun(argc, argv);
    return 0;
}
