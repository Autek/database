#include <gtest/gtest.h>
#include <cstdlib>
#include <ctime>
#include <random>
#include "unistd.h"

extern "C"
{
    #include "buffer_manager.h"
}

const size_t k_ntests = 5;

TEST (unittest, singleNewPage){
  const char* db_name = "test1.db";
  const size_t buffer_pool_size = 3;
  const size_t k = 1;

  StorageManager *sm = 0;
  BufferManager *bm = 0;

  std::random_device r;
  std::default_random_engine rng(r());
  std::uniform_int_distribution<uint8_t> uniform_dist(0);

  int ret = InitStorageManager(db_name, &sm);
  assert(ret == 0);
  ret = InitBufferManager(buffer_pool_size, k, sm, &bm);
  assert(ret == 0);

  block_id bids[buffer_pool_size];
  Page* pages[buffer_pool_size];
  pages[0] = NewPage(&bids[0], bm);
  pages[1] = NewPage(&bids[1], bm);
  pages[2] = NewPage(&bids[2], bm);
  for (size_t i = 0; i < buffer_pool_size; i++){
    ASSERT_NE(nullptr, pages[i]);
    EXPECT_EQ(i, bids[i]);
  }
  block_id fail_id = 0xdeadbeaf;
  Page *fail_page = NewPage(&fail_id, bm);
  EXPECT_EQ(fail_id, 0xdeadbeaf);

  StopStorageManager(sm);
  StopBufferManager(bm);
  remove("test1.db");
}

TEST (unittest, singleUnpinPage){
  const char* db_name = "test2.db";
  const size_t buffer_pool_size = 3;
  const size_t k = 1;

  StorageManager *sm = 0;
  BufferManager *bm = 0;

  std::random_device r;
  std::default_random_engine rng(r());
  std::uniform_int_distribution<uint8_t> uniform_dist(0);

  int ret = InitStorageManager(db_name, &sm);
  assert(ret == 0);
  ret = InitBufferManager(buffer_pool_size, k, sm, &bm);
  assert(ret == 0);

  block_id bids[buffer_pool_size];
  Page* pages[buffer_pool_size];
  pages[0] = NewPage(&bids[0], bm);
  pages[1] = NewPage(&bids[1], bm);
  pages[2] = NewPage(&bids[2], bm);
  block_id fail_id = 0xdeadbeaf;
  Page *fail_page = NewPage(&fail_id, bm);
  EXPECT_EQ(fail_id, 0xdeadbeaf);
  
  // Unpin now and try
  EXPECT_EQ(pages[1]->pin_count_, 1);
  EXPECT_EQ(pages[1]->is_dirty_, false);
  bool ret_code = UnpinPage(1, true, bm);
  EXPECT_EQ(ret_code, true);
  EXPECT_EQ(pages[1]->pin_count_, 0);
  EXPECT_EQ(pages[1]->is_dirty_, true);

  fail_page = NewPage(&fail_id, bm);
  EXPECT_EQ(fail_id, 3);
  EXPECT_EQ(fail_page, pages[1]);
  EXPECT_EQ(fail_page->is_dirty_, false);
  EXPECT_EQ(fail_page->pin_count_, 1);

  ret_code = UnpinPage(4, true, bm);
  EXPECT_EQ(ret_code, false);

  StopStorageManager(sm);
  StopBufferManager(bm);
  remove("test2.db");
}

TEST (unittest, singleFlushFetchPage){
  const char* db_name = "test3.db";
  const size_t buffer_pool_size = 3;
  const size_t k = 1;

  StorageManager *sm = 0;
  BufferManager *bm = 0;

  std::random_device r;
  std::default_random_engine rng(r());
  std::uniform_int_distribution<uint8_t> uniform_dist(0);

  int ret = InitStorageManager(db_name, &sm);
  assert(ret == 0);
  ret = InitBufferManager(buffer_pool_size, k, sm, &bm);
  assert(ret == 0);

  block_id bids[buffer_pool_size];
  Page* pages[buffer_pool_size];
  pages[0] = NewPage(&bids[0], bm);
  pages[1] = NewPage(&bids[1], bm);
  pages[2] = NewPage(&bids[2], bm);
  char random_binary_data[PAGE_SIZE];
  for (char &i : random_binary_data) {
    i = uniform_dist(rng);
  }
  memcpy(pages[1]->data_, random_binary_data, PAGE_SIZE);
  EXPECT_EQ(0, memcmp(pages[1]->data_, random_binary_data, PAGE_SIZE));
  bool ret_code = FlushPage(1, bm);
  EXPECT_EQ(ret_code, true);
  EXPECT_EQ(pages[1]->is_dirty_, false);

  ret_code = UnpinPage(2, false, bm);
  EXPECT_EQ(ret_code, true);
  EXPECT_EQ(pages[2]->is_dirty_, false);
  
  Page* new_page = FetchPage(1, bm);
  EXPECT_EQ(new_page, pages[1]);
  EXPECT_EQ(0, memcmp(new_page->data_, random_binary_data, PAGE_SIZE));

  block_id new_id = 0xdeadbeaf;
  new_page = NewPage(&new_id, bm);
  EXPECT_EQ(new_id, 3);
  EXPECT_EQ(new_page, pages[2]);

  new_page = FetchPage(2, bm);
  EXPECT_EQ(new_page, nullptr);


  StopStorageManager(sm);
  StopBufferManager(bm);
  remove("test3.db");
}

TEST (unittest, singleDeletePage)
{
  const char* db_name = "test4.db";
  const size_t buffer_pool_size = 3;
  const size_t k = 1;

  StorageManager *sm = 0;
  BufferManager *bm = 0;

  std::random_device r;
  std::default_random_engine rng(r());
  std::uniform_int_distribution<uint8_t> uniform_dist(0);

  int ret = InitStorageManager(db_name, &sm);
  assert(ret == 0);
  ret = InitBufferManager(buffer_pool_size, k, sm, &bm);
  assert(ret == 0);

  block_id bids[buffer_pool_size];
  Page* pages[buffer_pool_size];
  pages[0] = NewPage(&bids[0], bm);
  pages[1] = NewPage(&bids[1], bm);
  pages[2] = NewPage(&bids[2], bm);
  char random_binary_data[PAGE_SIZE];
  for (char &i : random_binary_data) {
    i = uniform_dist(rng);
  }
  memcpy(pages[1]->data_, random_binary_data, PAGE_SIZE);
  EXPECT_EQ(0, memcmp(pages[1]->data_, random_binary_data, PAGE_SIZE));
  bool ret_code = FlushPage(1, bm);
  EXPECT_EQ(ret_code, true);
  EXPECT_EQ(pages[1]->is_dirty_, false);

  ret_code = DeletePage(3, bm);
  EXPECT_EQ(ret_code, true);

  ret_code = DeletePage(2, bm);
  EXPECT_EQ(ret_code, false);

  ret_code = UnpinPage(2, false, bm);
  EXPECT_EQ(ret_code, true);
  EXPECT_EQ(pages[2]->is_dirty_, false);

  ret_code = DeletePage(2, bm);
  EXPECT_EQ(ret_code, true);
  
  block_id new_id = 0xdeadbeaf;
  Page *new_page = NewPage(&new_id, bm);
  EXPECT_EQ(new_id, 3);
  EXPECT_EQ(new_page, pages[2]);

  StopStorageManager(sm);
  StopBufferManager(bm);
  remove("test4.db");
}

int
main (int argc, char **argv)
{
  for (size_t i = 0; i < k_ntests; i++){
    std::string filename = "test";
    filename += ('1' + i);
    filename += ".db";
    if (access(filename.c_str(), F_OK) == 0){
      remove(filename.c_str());
    }
  }
  ::testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}