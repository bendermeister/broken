#ifndef BROKEN_UNIT_H
#define BROKEN_UNIT_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include <wchar.h>

typedef union test_value_t test_value_t;
union test_value_t {
  uint64_t as_uint;
  int64_t as_int;
  char *as_str;
  bool as_bool;
};

typedef enum {
  TEST_VALUE_INT,
  TEST_VALUE_UINT,
  TEST_VALUE_STR,
  TEST_VALUE_BOOL,
} test_value_type_t;

typedef struct test_record_t test_record_t;
struct test_record_t {
  char *context;
  test_value_t gotten;
  test_value_t expected;
  test_value_type_t type;
  uint32_t line_number;
};

// TODO: do we have to check malloc?

typedef struct test_t test_t;
struct test_t {
  char *name;
  char *context;
  struct {
    test_record_t *buffer;
    uint32_t length;
    uint32_t capacity;
  } records;
  uint32_t passed;
  uint32_t failed;
};

#define TEST_MAKE(...)                                                         \
  ({                                                                           \
    test_t record = {.name = (char *){__VA_ARGS__}};                           \
    if (!record.name) {                                                        \
      record.name = (char *)__func__;                                          \
    }                                                                          \
    record;                                                                    \
  })

__attribute__((__unused__)) static void test_destroy(test_t test[static 1]) {
  free(test->records.buffer);
}

static test_record_t *test_records_more(test_t test[static 1]) {
  if (test->records.length >= test->records.capacity) {
    test->records.capacity += 1;
    test->records.capacity <<= 1;
    test->records.buffer =
        realloc(test->records.buffer,
                test->records.capacity * sizeof(*test->records.buffer));
  }
  test_record_t *record = &test->records.buffer[test->records.length];
  test->records.length += 1;
  return record;
}

__attribute_maybe_unused__ static void
test_int(test_t test[static 1], const int64_t gotten, const int64_t expected,
         const int line_number, char *context) {
  if (gotten == expected) {
    test->passed += 1;
    return;
  }
  test->failed += 1;

  *test_records_more(test) = (test_record_t){
      .context = context,
      .line_number = line_number,
      .type = TEST_VALUE_INT,
      .gotten.as_int = gotten,
      .expected.as_int = expected,
  };
}

#define TEST_INT(TEST, GOTTEN, EXPECTED, CONTEXT)                              \
  test_int(TEST, GOTTEN, EXPECTED, __LINE__, CONTEXT)

__attribute_maybe_unused__ static void
test_uint(test_t test[static 1], const uint64_t gotten, const uint64_t expected,
          const int line_number, char *context) {
  if (gotten == expected) {
    test->passed += 1;
    return;
  }
  test->failed += 1;

  *test_records_more(test) = (test_record_t){
      .context = context,
      .line_number = line_number,
      .type = TEST_VALUE_UINT,
      .gotten.as_uint = gotten,
      .expected.as_uint = expected,
  };
}

#define TEST_UINT(TEST, GOTTEN, EXPECTED, CONTEXT)                             \
  test_uint(TEST, GOTTEN, EXPECTED, __LINE__, CONTEXT)

__attribute_maybe_unused__ static void test_str(test_t test[static 1],
                                                char *gotten, char *expected,
                                                const int line_number,
                                                char *context) {
  if (strcmp(gotten, expected) == 0) {
    test->passed += 1;
    return;
  }
  test->failed += 1;

  *test_records_more(test) = (test_record_t){
      .context = context,
      .line_number = line_number,
      .type = TEST_VALUE_STR,
      .gotten.as_str = gotten,
      .expected.as_str = expected,
  };
}

#define TEST_STR(TEST, GOTTEN, EXPECTED, CONTEXT)                              \
  test_str(TEST, GOTTEN, EXPECTED, __LINE__, CONTEXT)

__attribute_maybe_unused__ static void test_bool(test_t test[static 1],
                                                 bool gotten, bool expected,
                                                 const int line_number,
                                                 char *context) {
  if (gotten == expected) {
    test->passed += 1;
    return;
  }
  test->failed += 1;

  *test_records_more(test) = (test_record_t){
      .context = context,
      .line_number = line_number,
      .type = TEST_VALUE_BOOL,
      .gotten.as_bool = gotten,
      .expected.as_bool = expected,
  };
}

#define TEST_BOOL(TEST, GOTTEN, EXPECTED, CONTEXT)                             \
  test_bool(TEST, GOTTEN, EXPECTED, __LINE__, CONTEXT)

static struct {
  char buffer[1024];
  struct {
    uint32_t capacity;
    uint32_t length;
    char **buffer;
  } to_free;
} test_str_factory;

__attribute_maybe_unused__ static char *test_internal_make_str(void) {
  if (test_str_factory.to_free.length >= test_str_factory.to_free.capacity) {
    test_str_factory.to_free.capacity += 1;
    test_str_factory.to_free.capacity <<= 1;
    test_str_factory.to_free.buffer =
        realloc(test_str_factory.to_free.buffer,
                test_str_factory.to_free.capacity *
                    sizeof(*test_str_factory.to_free.buffer));
  }

  char *str = malloc(strlen(test_str_factory.buffer) + 1);
  strcpy(str, test_str_factory.buffer);
  test_str_factory.to_free.buffer[test_str_factory.to_free.length] = str;
  test_str_factory.to_free.length += 1;
  return str;
}

#define TEST_MAKE_STR(FMT, ...)                                                \
  ({                                                                           \
    snprintf(test_str_factory.buffer, 1024, FMT, __VA_ARGS__);                 \
    test_internal_make_str();                                                  \
  })

__attribute_maybe_unused__ static void test_analyze(test_t test[static 1]) {
  printf("%s:", test->name);
  if (test->failed == 0) {
    printf("\t\t[SUCCESS]\n");
    return;
  } else {
    printf("\t\t[FAILED]\n");
  }
  for (uint32_t i = 0; i < test->records.length; i += 1) {
    printf("\tline: %d:\n", test->records.buffer[i].line_number);
    switch (test->records.buffer[i].type) {
    case TEST_VALUE_BOOL:
      printf("\t\tgotten: %d\n", test->records.buffer[i].gotten.as_bool);
      printf("\t\texpected: %d\n", test->records.buffer[i].expected.as_bool);

      break;
    case TEST_VALUE_INT:
      printf("\t\tgotten: %ld\n", test->records.buffer[i].gotten.as_int);
      printf("\t\texpected: %ld\n", test->records.buffer[i].expected.as_int);
      break;
    case TEST_VALUE_STR:
      printf("\t\tgotten: %s\n", test->records.buffer[i].gotten.as_str);
      printf("\t\texpected: %s\n", test->records.buffer[i].expected.as_str);
      break;
    case TEST_VALUE_UINT:
      printf("\t\tgotten: %lu\n", test->records.buffer[i].gotten.as_uint);
      printf("\t\texpected: %lu\n", test->records.buffer[i].expected.as_uint);
      break;
    }
    printf("\t\twith context: '%s'\n", test->records.buffer[i].context);
  }
}

#define TEST_RUN(FUNC)                                                         \
  ({                                                                           \
    test_t test = FUNC;                                                        \
    test_analyze(&test);                                                       \
    test_destroy(&test);                                                       \
  })

__attribute_maybe_unused__ static void TEST_CLEANUP(void) {
  for (uint32_t i = 0; i < test_str_factory.to_free.length; i += 1) {
    free(test_str_factory.to_free.buffer[i]);
  }
  free(test_str_factory.to_free.buffer);
}

#endif // BORKEN_UNIT_H_
