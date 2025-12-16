#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ex1.h"
#include "common.h"

Book_t *search_book_by_title(Book_t *book_list, const char *title)
{
	while(book_list) {
		if (!strcmp(book_list->title, title)) {
			return book_list;
		}
		book_list = book_list->next;
	}
	return NULL;
}

Book_t *search_book_by_page_count(Book_t *book_list, int page_count)
{
	while(book_list) {
		if (book_list->page_cnt == page_count) {
			return book_list;
		}
		book_list = book_list->next;
	}
	return NULL;
}

Book_t *search_book_by_id(Book_t *book_list, unsigned id)
{
	while(book_list) {
		if (book_list->id == id) {
			return book_list;
		}
		book_list = book_list->next;
	}
	return NULL;
}

