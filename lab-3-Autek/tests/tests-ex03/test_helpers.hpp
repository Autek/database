#ifndef TEST_HELPERS
#define TEST_HELPERS

extern "C"
{
    #include "ex3.h"
    #include "common.h"
}
#include <vector>
#include <string>
#include <cstdlib>
#include <random>
#include <iostream>


size_t TREE_ORDER = 5;
const uint64_t seed = 41;

Book_t* create_book(int id)
{
    Book_t* book = (Book_t*) malloc(sizeof(Book_t));
    book->id = id;
    sprintf(book->title, "book%d", id);
    book->page_cnt = 5;
    book->next = nullptr;
    return book;
}


#endif 