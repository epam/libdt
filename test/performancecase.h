#ifndef PERFORMANCECASE_H
#define PERFORMANCECASE_H
#define _VARIADIC_MAX 10
#include <gtest/gtest.h>

class PerformanceCase: public ::testing::Test
{
public:
    PerformanceCase();
};

#endif // PERFORMANCECASE_H
