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

size_t k_ntests = 2;


TEST(unittest, check_leaf_size_simple)
{
  bp_tree_t* bp_tree = bp_init_index();
  // assert(bp_tree != nullptr);
  Book_t* book1 = create_book(1);
  Book_t* book2 = create_book(2);
  Book_t* book3 = create_book(3);
  Book_t* book4 = create_book(4);
  Book_t* book5 = create_book(5);
  Book_t* book6 = create_book(6);
  
  bp_insert_book(bp_tree, 1, book1);
  bp_insert_book(bp_tree, 2, book2);
  bp_insert_book(bp_tree, 3, book3);
  bp_insert_book(bp_tree, 4, book4);
  bp_insert_book(bp_tree, 5, book5);
  
  ASSERT_EQ(bp_tree->root->n, 5);

  bp_insert_book(bp_tree, 6, book6);
  ASSERT_EQ(bp_tree->root->n, 1);
  
  bp_destroy_tree(bp_tree);
}

TEST(unittest, check_sorting_leafs)
{
  bp_tree_t* bp_tree = bp_init_index();
  assert(bp_tree != nullptr);

  Book_t* book1 = create_book(1);
  Book_t* book2 = create_book(2);
  Book_t* book3 = create_book(3);

  bp_insert_book(bp_tree, 3, book1);
  ASSERT_EQ(bp_tree->root->keys[0], 3);
  bp_insert_book(bp_tree, 2, book2);
  ASSERT_EQ(bp_tree->root->keys[0], 2);
  bp_insert_book(bp_tree, 1, book3);
  ASSERT_EQ(bp_tree->root->keys[0], 1);

  bp_destroy_tree(bp_tree);
}

TEST(unittest, check_leaf_linkage_simple)
{
  bp_tree_t* bp_tree = bp_init_index();
  assert(bp_tree != nullptr);

  Book_t* book1 = create_book(1);
  Book_t* book2 = create_book(2);
  Book_t* book3 = create_book(3);
  Book_t* book4 = create_book(4);
  Book_t* book5 = create_book(5);
  Book_t* book6 = create_book(6);
  Book_t* book7 = create_book(7);

  bp_insert_book(bp_tree, 1, book1);
  bp_insert_book(bp_tree, 2, book2);
  bp_insert_book(bp_tree, 3, book3);
  bp_insert_book(bp_tree, 4, book4);
  bp_insert_book(bp_tree, 5, book5);
  bp_insert_book(bp_tree, 6, book6);
  bp_insert_book(bp_tree, 7, book7);

  ASSERT_EQ(bp_tree->root->children[0]->next, bp_tree->root->children[1]); 
  
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
