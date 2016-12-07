#include <gtest/gtest.h>
#include <stdexcept>
#include <string>
#include "../util/stopwatch.h"

using namespace order_concepts::util;

class StopwatchTest : public ::testing::Test {
protected:
    virtual void SetUp() {}
    virtual void TearDown() {}
};


TEST_F(StopwatchTest, returnNonNegativeValue)
{
    const int NTRY = 10000000;
    Stopwatch<std::chrono::milliseconds> sw;
    int sum = 0;
    for (int i = 0; i < NTRY; i++) {
        sum += i - std::rand();
    }
    sw.stop();
    ASSERT_GE(sw.elapsed(), 0);
}

TEST_F(StopwatchTest, hasCorrectUnitSecond) {
    const std::string SEC = "s";
    Stopwatch<std::chrono::seconds> sw;
    ASSERT_STREQ(sw.unit().c_str(), SEC.c_str());
}

TEST_F(StopwatchTest, hasCorrectUnitMillisecond) {
    const std::string MILL = "ms";
    Stopwatch<std::chrono::milliseconds> sw;
    ASSERT_STREQ(sw.unit().c_str(), MILL.c_str());
}

TEST_F(StopwatchTest, hasCorrectUnitMicrosecond) {
    const std::string MICRO = "us";
    Stopwatch<std::chrono::microseconds> sw;
    ASSERT_STREQ(sw.unit().c_str(), MICRO.c_str());
}

TEST_F(StopwatchTest, hasCorrectUnitNanosecond) {
    const std::string NANO = "ns";
    Stopwatch<std::chrono::nanoseconds> sw;
    ASSERT_STREQ(sw.unit().c_str(), NANO.c_str());
}


