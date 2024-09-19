#include <gtest/gtest.h>
#include "../build/Driver_Common/ctypes.h"

class SafetyTest : public ::testing::Test {
protected:
  void SetUp() override { }
};

// Sample Test Case
TEST_F(SafetyTest, SAMPLE_TEST_CASE) {
  ASSERT_EQ(3, 1 + 2);
}
