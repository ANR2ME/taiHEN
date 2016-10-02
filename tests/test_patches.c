/* test_patches.c -- unit tests for patches.c
 *
 * Copyright (C) 2016 Yifan Lu
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>

#include "../taihen.h"
#include "../taihen_internal.h"
#include "../patches.h"

/** Macro for printing test messages with an identifier */
#define TEST_MSG(fmt, ...) printf("[%s] " fmt "\n", name, ##__VA_ARGS__)

/**
 * @brief      Creates a random shuffling of integers 0..count
 *
 *             count MUST BE PRIME! This works because any number (except 0) is
 *             an additive generator modulo a prime. Number theory!
 *
 * @param[out] ordering  The ordering
 * @param[in]  count     The count (MUST BE PRIME)
 */
static inline void shuffle_choices(int *ordering, int count) {
  ordering[0] = rand() % count;
  if (ordering[0] == 0) ordering[0]++;
  for (int i = 1; i < count; i++) {
    ordering[i] = (ordering[i-1] + ordering[0]) % count;
  }
}

/** Number of random hooks */
#define TEST_1_NUM_HOOKS      31

/**
 * @brief      Test random hooks
 *
 * @param[in]  name    The name of the test
 * @param[in]  flavor  The flavor of the test
 *
 * @return     Success
 */
int test_scenario_1(const char *name, int flavor) {
  tai_hook_t *hooks[TEST_1_NUM_HOOKS];
  int start[TEST_1_NUM_HOOKS];
  int ret;

  shuffle_choices(start, TEST_1_NUM_HOOKS);

  for (int i = 0; i < TEST_1_NUM_HOOKS; i++) {
    uintptr_t addr;

    if (flavor == 1) {
      addr = (start[i] % 12) * 4;
    } else {
      addr = start[i] * 16;
    }
    TEST_MSG("Attempting to add hook at addr:%lx", addr);
    if (taiHookFunctionAbs(&hooks[i], 0, (void *)addr, NULL) < 0) {
      TEST_MSG("Failed to hook addr:%lx", addr);
      hooks[i] = NULL;
    } else {
      TEST_MSG("Successfully hooked addr:%lx", addr);
    }
  }
  TEST_MSG("Cleanup");
  for (int i = 0; i < TEST_1_NUM_HOOKS; i++) {
    if (hooks[i] != NULL) {
      ret = taiHookRelease(hooks[i]);
      assert(ret == 0);
    }
  }
  return 0;
}


/** Number of random injections */
#define TEST_2_NUM_INJECT      31

/**
 * @brief      Test random injections
 *
 * @param[in]  name    The name of the test
 * @param[in]  flavor  The flavor of the test
 *
 * @return     Success
 */
int test_scenario_2(const char *name, int flavor) {
  tai_inject_t *injections[TEST_2_NUM_INJECT];
  int start[TEST_2_NUM_INJECT];
  int off[TEST_2_NUM_INJECT];
  int sz[TEST_2_NUM_INJECT];
  int ret;

  shuffle_choices(start, TEST_2_NUM_INJECT);
  shuffle_choices(off, TEST_2_NUM_INJECT);
  shuffle_choices(sz, TEST_2_NUM_INJECT);

  for (int i = 0; i < TEST_2_NUM_INJECT; i++) {
    uintptr_t addr;
    size_t size;

    addr = start[i] * 0x10 + off[i] * 0x10;
    size = sz[i] * 0x10;
    TEST_MSG("Attempting to add injection at addr:%lx, size:%zx", addr, size);
    if (taiInjectAbs(&injections[i], 0, (void *)addr, NULL, size) < 0) {
      TEST_MSG("Failed to inject addr:%lx, size:%zx", addr, size);
      injections[i] = NULL;
    } else {
      TEST_MSG("Successfully injected addr:%lx", addr);
    }
  }
  TEST_MSG("Cleanup");
  for (int i = 0; i < TEST_2_NUM_INJECT; i++) {
    if (injections[i] != NULL) {
      ret = taiInjectRelease(injections[i]);
      assert(ret == 0);
    }
  }
  return 0;
}

/**
 * @brief      Randomly pick between test 1 or test 2
 *
 * @param[in]  name    The name of the test
 * @param[in]  flavor  Unused
 *
 * @return     Success
 */
int test_scenario_3(const char *name, int flavor) {
  int test = rand() % 2;
  flavor = rand() % 2;

  TEST_MSG("Running test:%d flavor:%d", test, flavor);
  if (test) {
    return test_scenario_1(name, flavor);
  } else {
    return test_scenario_2(name, flavor);
  }
}

/**
 * @brief      Arguments for test thread
 */
struct thread_args {
  int (*test) (const char *, int);
  const char *prefix;
  int index;
};

/**
 * @brief      Pthreads start for a test
 *
 * @param      arg   The argument
 *
 * @return     NULL
 */
void *start_test(void *arg) {
  struct thread_args *targs = (struct thread_args *)arg;
  char name[256];
  snprintf(name, 256, "%s-thread-%d", targs->prefix, targs->index);
  targs->test(name, 0);
  return NULL;
}

/** Number of threads for tests. */
#define TEST_NUM_THREADS 32

int main(int argc, const char *argv[]) {
  const char *name = "INIT";
  pthread_t threads[TEST_NUM_THREADS];
  struct thread_args args[TEST_NUM_THREADS];
  
  int seed = atoi(argv[1]);

  if (argc > 1) {
    TEST_MSG("Seeding PRNG: %d", seed);
    srand(seed);
  }

  TEST_MSG("Setup patches");
  patches_init();

  TEST_MSG("Phase 1: Single threaded");
  test_scenario_1("hooks_test_1", 0);
  test_scenario_1("hooks_test_2", 1);
  test_scenario_2("injection_test", 0);

  TEST_MSG("Phase 2: Multi threaded");
  TEST_MSG("scenario 1");
  for (int i = 0; i < TEST_NUM_THREADS; i++) {
    args[i].test = test_scenario_1;
    args[i].index = i;
    args[i].prefix = "hooks";
    pthread_create(&threads[i], NULL, start_test, &args[i]);
  }
  TEST_MSG("cleanup");
  for (int i = 0; i < TEST_NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  TEST_MSG("scenario 2");
  for (int i = 0; i < TEST_NUM_THREADS; i++) {
    args[i].test = test_scenario_2;
    args[i].index = i;
    args[i].prefix = "injections";
    pthread_create(&threads[i], NULL, start_test, &args[i]);
  }
  TEST_MSG("cleanup");
  for (int i = 0; i < TEST_NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }
  TEST_MSG("scenario 3");
  for (int i = 0; i < TEST_NUM_THREADS; i++) {
    args[i].test = test_scenario_3;
    args[i].index = i;
    args[i].prefix = "mixed";
    pthread_create(&threads[i], NULL, start_test, &args[i]);
  }
  TEST_MSG("cleanup");
  for (int i = 0; i < TEST_NUM_THREADS; i++) {
    pthread_join(threads[i], NULL);
  }

  TEST_MSG("Cleanup patches");
  patches_deinit();
  return 0;
}
