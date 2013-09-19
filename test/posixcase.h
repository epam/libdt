#ifndef POSIXCASE_H
#define POSIXCASE_H
#define _VARIADIC_MAX 10
#include <gtest/gtest.h>
class PosixCase : public ::testing::Test
{
public:
    PosixCase();
};

#endif // POSIXCASE_H
