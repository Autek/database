#include <gtest/gtest.h>
#include <fcntl.h>
#include <sys/types.h>
#include <fstream>
#include <sstream>
#include <iostream>

#include "unistd.h"
extern "C"
{
    #include "ex1.h"
    #include "common.h"
}

const size_t k_ntests = 3;

TEST(unittest, search_by_book_title)
{
  Book_t book1 = {1, "book1", 100, 2020, nullptr};
  Book_t book2 = {2, "book2", 200, 2021, &book1};
  Book_t book3 = {3, "book3", 300, 2022, &book2};

  Book_t* b = search_book_by_title(&book3, "book1");
  assert(b != nullptr);
  EXPECT_EQ(b->page_cnt, 100);
}

TEST(unittest, search_by_price)
{
  Book_t book1 = {1, "book1", 100, 2020, nullptr};
  Book_t book2 = {2, "book2", 200, 2021, &book1};
  Book_t book3 = {3, "book3", 300, 2022, &book2};

  Book_t* b = search_book_by_page_count(&book3, 200);
  assert(b != nullptr);
  EXPECT_EQ(b->page_cnt, 200);

  b = search_book_by_page_count(&book3, 100);
  assert(b != nullptr);
  EXPECT_EQ(b->page_cnt, 100);  
}

TEST(unittest, search_by_id)
{
  Book_t book1 = {1, "book1", 100, 2020, nullptr};
  Book_t book2 = {2, "book2", 200, 2021, &book1};
  Book_t book3 = {3, "book3", 300, 2022, &book2};

  Book_t* b = search_book_by_id(&book3, 1);
  assert(b != nullptr);
  EXPECT_EQ(b, &book1);

  b = search_book_by_page_count(&book3, 12);
  EXPECT_EQ(b, nullptr);  
}

TEST(unittest, empty_handles)
{
  Book_t* b = search_book_by_title(nullptr, "book1");
  EXPECT_EQ(b, nullptr);

  b = search_book_by_page_count(nullptr, 100);
  EXPECT_EQ(b, nullptr);
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
