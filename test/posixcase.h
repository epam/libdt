#ifndef BASICCASE_H
#define BASICCASE_H
#define _VARIADIC_MAX 10
#include <gtest/gtest.h>
class PosixCase : public ::testing::Test
{
public:
    PosixCase();
};

#endif // BASICCASE_H
