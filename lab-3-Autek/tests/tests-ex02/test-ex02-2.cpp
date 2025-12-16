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

const size_t k_ntests = 4;

TEST(unittest, insert_single_book)
{
  hash_index_t* index = h_init_index(5);
  assert(index != nullptr);

  Book_t* book1 = (Book_t*) malloc(sizeof(Book_t));
  book1->id = 1; strcpy(book1->title, "book1"); book1->page_cnt = 100; book1->next = nullptr;

  h_insert_book(index, book1);
  Book_t* b = h_get_book(index, "book1");
  ASSERT_EQ(b, book1);

  h_destroy_index(index);
}

TEST(unittest, insert_multiple_books)
{
  hash_index_t* index = h_init_index(5);
  assert(index != nullptr);

  Book_t* book1 = (Book_t*) malloc(sizeof(Book_t));
  book1->id = 1; strcpy(book1->title, "book1"); book1->page_cnt = 100; book1->next = nullptr;
  Book_t* book2 = (Book_t*) malloc(sizeof(Book_t));
  book2->id = 2; strcpy(book2->title, "book2"); book2->page_cnt = 200; book2->next = nullptr;
  Book_t* book3 = (Book_t*) malloc(sizeof(Book_t));
  book3->id = 3; strcpy(book3->title, "book3"); book3->page_cnt = 300; book3->next = nullptr;
  Book_t* book4 = (Book_t*) malloc(sizeof(Book_t));
  book4->id = 4; strcpy(book4->title, "book4"); book4->page_cnt = 400; book4->next = nullptr;

  h_insert_book(index, book1);
  h_insert_book(index, book2);
  h_insert_book(index, book3);
  h_insert_book(index, book4);
  
  Book_t* b = h_get_book(index, "book1");
  ASSERT_EQ(b, book1);
  Book_t* b2 = h_get_book(index, "book2");
  ASSERT_EQ(b2, book2);
  Book_t* b3 = h_get_book(index, "book3");
  ASSERT_EQ(b3, book3);
  Book_t* b4 = h_get_book(index, "book4");
  ASSERT_EQ(b4, book4);
  
  h_destroy_index(index);
}

TEST(unittest, remove_test)
{
  hash_index_t* index = h_init_index(5);
  assert(index != nullptr);

  Book_t* book1 = (Book_t*) malloc(sizeof(Book_t));
  book1->id = 1; strcpy(book1->title, "book1"); book1->page_cnt = 100; book1->next = nullptr;

  h_insert_book(index, book1);
  Book_t* b = h_get_book(index, "book1");
  ASSERT_EQ(b, book1);

  h_delete_book(index, book1->title);
  Book_t* b2 = h_get_book(index, "book1");
  ASSERT_EQ(b2, nullptr);

  h_destroy_index(index);
}

TEST(unittest, remove_test2)
{
  hash_index_t* index = h_init_index(5);
  assert(index != nullptr);

  Book_t* book1 = (Book_t*) malloc(sizeof(Book_t));
  book1->id = 1; strcpy(book1->title, "book1"); book1->page_cnt = 100; book1->next = nullptr;

  h_insert_book(index, book1);

  int ret = h_delete_book(index, "book1");
  ASSERT_EQ(ret, 0);
  ret = h_delete_book(index, "book1");
  ASSERT_EQ(ret, 1);

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
