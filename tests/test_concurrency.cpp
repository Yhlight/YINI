#include "YINI/YiniManager.hpp"
#include <gtest/gtest.h>
#include <thread>
#include <vector>
#include <fstream>
#include <cstdio>

TEST(ConcurrencyTest, ConcurrentWritesToYiniManager)
{
  // This test will simulate multiple threads writing to the same YINI file
  // through a shared YiniManager instance. The goal is to ensure that the
  // mutexes prevent data corruption and that the final state is consistent.

  // 1. Setup: Create a temporary YINI file for the test.
  const std::string test_file = "concurrency_test.yini";
  std::ofstream(test_file, std::ios::trunc) << "[Settings]\n";

  // 2. Create a shared YiniManager instance.
  auto manager = std::make_shared<YINI::YiniManager>(test_file);
  ASSERT_TRUE(manager->isLoaded());

  // 3. Define the concurrent task. Each thread will write a unique value.
  auto writer_task = [&](int thread_id) {
    std::string key = "key" + std::to_string(thread_id);
    int value = thread_id;
    manager->setIntValue("Settings", key, value);
  };

  // 4. Spawn multiple threads to execute the task concurrently.
  const int num_threads = 10;
  std::vector<std::thread> threads;
  for (int i = 0; i < num_threads; ++i)
  {
    threads.emplace_back(writer_task, i);
  }

  // 5. Wait for all threads to complete.
  for (auto &t : threads)
  {
    t.join();
  }

  // 6. Verification: Reload the manager and check the final state.
  YINI::YiniManager final_manager(test_file);
  ASSERT_TRUE(final_manager.isLoaded());
  YINI::YiniDocument doc = final_manager.getDocument();
  const auto *section = doc.findSection("Settings");
  ASSERT_NE(section, nullptr);

  // Check that all key-value pairs were written correctly.
  ASSERT_EQ(section->pairs.size(), num_threads);
  for (int i = 0; i < num_threads; ++i)
  {
    std::string key = "key" + std::to_string(i);
    auto it = std::find_if(section->pairs.begin(), section->pairs.end(),
                           [&](const auto &p) { return p.key == key; });
    ASSERT_NE(it, section->pairs.end());
    ASSERT_TRUE(std::holds_alternative<int>(it->value.data));
    EXPECT_EQ(std::get<int>(it->value.data), i);
  }

  // 7. Cleanup
  std::remove(test_file.c_str());
  std::remove((test_file + ".ymeta").c_str());
  for (int i = 1; i <= 5; ++i)
  {
    std::string backup_path = test_file + ".ymeta.bak" + std::to_string(i);
    std::remove(backup_path.c_str());
  }
}