#include <unity.h>

// Unity が各テストの前後で呼ぶ。使わなくても定義は必要。
void setUp(void) {}
void tearDown(void) {}

void test_sample(void) { TEST_ASSERT_EQUAL_INT(3, 1 + 2); }

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_sample);
  return UNITY_END();
}
