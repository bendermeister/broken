# broken
Testing Framework for good ol' C.

## unit testing
You can perform simple unit tests using `broken/unit.h`.

``` c
#include <broken/unit.h>

static test_t TEST_something(void) {
  // create a default test (name will be equal to the function name)
  test_t test = TEST_MAKE();

  // test if 35 + 34 == 69
  TEST_INT(&test, 35 + 34, 69, NULL);

  // test which will fail
  TEST_BOOL(&test, true, false, NULL);

  // test with a context
  TEST_STR(&test, "these strings", "do not match",
           TEST_MAKE_STR("%s %s", "this is how you can create a context",
                         "with simple printf string generation"));

  return test;
}

int main(void) {
  // this is how you run a test
  TEST_RUN(TEST_something());

  // cleanup function to free needed resources
  TEST_CLEANUP();
  return 0;
}
```
Try this code in the `example` directory.
