#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "unistd.h"
extern "C"
{
    #include "ex2.h"
    #include "common.h"
}

const size_t k_ntests = 3;

TEST(unittest, h_init_index)
{
  hash_index_t* index = h_init_index(0);
  ASSERT_EQ(index, nullptr);

  hash_index_t* index2 = h_init_index(10);
  ASSERT_NE(index2, nullptr);
  ASSERT_EQ(index2->size, 10);

  h_destroy_index(index2);
}

TEST(unittest, check_hash_function)
{
  const char * str = "cs300-epfl";
  ASSERT_EQ(hash(str), 219334818);

  const char * str2 = "databases";
  ASSERT_EQ(hash(str2), 3262565773);
  return ;
}

TEST(unittest, get_null_book)
{
  hash_index_t* index = h_init_index(100);
  assert(index != nullptr);

  Book_t* b = h_get_book(index, "Not exist");
  ASSERT_EQ(b, nullptr);

  h_destroy_index(index);
}

int
main (int argc, char **argv)
{  
  for (size_t i = 0; i < k_ntests; i++){
    std::string filename = "test";
    filename += ('1' + i);
    if (access(filename.c_str(), F_OK) == 0){
      remove(filename.c_str());
    }
  }
  ::testing::InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}
