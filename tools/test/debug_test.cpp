#include "gtest/gtest.h"
#include "gmock/gmock.h"
#define DEBUG //not sure how to get around defining here and doing a seperate file for the other code path
#include "../debug.h"
#include <string>
#include <iostream>
using namespace std;

FILE* pFile;
TEST(debug_tests, DEBUG_) {
#ifndef DEBUG

#define DEBUG
#endif
    int i = 0;
    string test {"DEBUG: "};
    debug("%s%d\n", test.c_str(), 392);
    debug("%s%u\n", test.c_str(), 7235);
    debug("%s%o\n", test.c_str(), 610);
    debug("%s%x\n", test.c_str(), 700);
    debug("%s%X\n", test.c_str(), 700);
    debug("%s%e\n", test.c_str(), 39265.23);
    debug("%s%E\n", test.c_str(), 39265.23);
    debug("%s%g\n", test.c_str(), 39265.23);
    debug("%s%G\n", test.c_str(), 39265.23);
    debug("%s%a\n", test.c_str(), 39265.23);
    debug("%s%A\n", test.c_str(), 39265.23);
    debug("%s%c\n", test.c_str(), 'a');
    debug("%s%p\n", test.c_str(), *(test.c_str()));
    debug("%s%%\n", test.c_str());
    ASSERT_EQ(i,0);
#undef DEBUG
}
