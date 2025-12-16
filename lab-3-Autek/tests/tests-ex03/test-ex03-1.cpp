#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "unistd.h"
extern "C"
{
  #include "ex3.h"
  #include "common.h"
}
#include "test_helpers.hpp"

const size_t k_ntests = 1;

TEST(unittest, check_bp_size)
{
  bp_tree_t* bp_tree = bp_init_index();
  assert(bp_tree != nullptr);
  ASSERT_EQ(bp_tree->m, TREE_ORDER);

  bp_destroy_tree(bp_tree);
}

TEST(unittest, single_add)
{
  Book_t* book1 = create_book(1);

  bp_tree_t* bp_tree = bp_init_index();
  assert(bp_tree != nullptr);
  ASSERT_EQ(bp_insert_book(bp_tree, 2, book1), 0);

  bp_destroy_tree(bp_tree);
}

TEST(unittest, check_same_key)
{
  Book_t* book1 = create_book(1);

  bp_tree_t* bp_tree = bp_init_index();
  assert(bp_tree != nullptr);

  // This is ok
  ASSERT_EQ(bp_insert_book(bp_tree, 1, book1), 0);
  // Duplicate key
  ASSERT_EQ(bp_insert_book(bp_tree, 1, book1), 1);

  bp_destroy_tree(bp_tree);
}

TEST(unittest, insert_4keys)
{ 
  Book_t* book1 = create_book(1);
  Book_t* book2 = create_book(2);
  Book_t* book3 = create_book(3);
  Book_t* book4 = create_book(4);

  bp_tree_t* bp_tree = bp_init_index();
  assert(bp_tree != nullptr);

  ASSERT_EQ(bp_insert_book(bp_tree, 1, book1), 0);
  ASSERT_NE(bp_tree->root, nullptr);
  ASSERT_EQ(bp_insert_book(bp_tree, 2, book2), 0);
  ASSERT_EQ(bp_insert_book(bp_tree, 3, book3), 0);
  ASSERT_EQ(bp_insert_book(bp_tree, 4, book4), 0);

  ASSERT_EQ(bp_tree->root->n, 4);

  bp_destroy_tree(bp_tree);
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
