#include <gtest/gtest.h>
#include "math_lib.h"

// Сложение
TEST(MathLibTest, SumPositive) 
{
    int result;
    math_lib::Math::calculateSum(2, 3, result);
    EXPECT_EQ(result, 5);
}

TEST(MathLibTest, SumOverflow) 
{
    int result;
    EXPECT_THROW(math_lib::Math::calculateSum(2147483647, 1, result), std::overflow_error);
}

// Деление
TEST(MathLibTest, DivisionByZero) 
{
    int result;
    EXPECT_THROW(math_lib::Math::calculateDivision(5, 0, result), std::runtime_error);
}

TEST(MathLibTest, DivisionInteger) 
{
    int result;
    math_lib::Math::calculateDivision(7, 2, result);
    EXPECT_EQ(result, 3);
}

// Факториал
TEST(MathLibTest, FactorialBasic) 
{
    int result;
    math_lib::Math::calculateFactorial(5, result);
    EXPECT_EQ(result, 120);
}

TEST(MathLibTest, FactorialNegative) 
{
    int result;
    EXPECT_THROW(math_lib::Math::calculateFactorial(-1, result), std::runtime_error);
}

TEST(MathLibTest, FactorialZero) 
{
    int result;
    math_lib::Math::calculateFactorial(0, result);
    EXPECT_EQ(result, 1);
}

// Степень
TEST(MathLibTest, PowerNegative)
{
    int result;
    EXPECT_THROW(math_lib::Math::calculatePower(3, -2, result), std::runtime_error);
}

TEST(MathLibTest, PowerBasic) 
{
    int result;
    math_lib::Math::calculatePower(5, 3, result);
    EXPECT_EQ(result, 125);
}

// Вычитание
TEST(MathLibTest, DifferencePositive) 
{
    int result;
    math_lib::Math::calculateDifference(5, 3, result);
    EXPECT_EQ(result, 2);
}

TEST(MathLibTest, DifferenceOverflow) 
{
    int result;
    EXPECT_THROW(math_lib::Math::calculateDifference(-2147483648, 1, result), std::overflow_error);
}