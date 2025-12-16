#ifndef EX1_H
#define EX1_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "common.h"

Book_t* search_book_by_title(Book_t* books, const char* name);
Book_t* search_book_by_page_count(Book_t* books, int page_count);
Book_t *search_book_by_id(Book_t *book_list, unsigned id);

#ifdef __cplusplus
}
#endif
#endif

