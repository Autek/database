#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ex2.h"
#include "ex1.h"
#include "common.h"

// djb2
unsigned hash(const char *str)
{
	unsigned hash = 5381;
	char c = 0;
	while ((c = *str++)) {
		hash = ((hash << 5) + hash) + c;
	}
	return hash;
}

hash_index_t *h_init_index(int size)
{
	if (size <= 0) {
		return NULL;
	}
	hash_index_t * table = malloc(sizeof(hash_index_t));
	*table = (hash_index_t){.size = size, .buckets = calloc(size, sizeof(hash_node_t*))};
	return table;
}

void h_destroy_index(hash_index_t *index)
{
	hash_node_t ** table = index->buckets;
	for(int i = 0; i < index->size; i++) {
		hash_node_t * list = table[i];
		while(list) {
			hash_node_t * tmp = list;
			list = list->next;
			free(tmp->book);
			free(tmp);
		}

	}
	free(index->buckets);
	free(index);
}

Book_t *h_get_book(hash_index_t *index, const char *title)
{
	unsigned h = hash(title) % index->size;
	hash_node_t ** table = index->buckets;
	hash_node_t * list = table[h];
	while(list) {
		if (!strcmp(list->book->title, title)) {
			return list->book;
		}
		list = list->next;
	}
	return NULL;
}

void h_insert_book(hash_index_t *index, Book_t *book)
{
	unsigned h = hash(book->title) % index->size;
	hash_node_t ** table = index->buckets;
	hash_node_t * list = table[h];
	if (!list) {
		table[h] = malloc(sizeof(hash_node_t));
		*table[h] = (hash_node_t){.book = book, .next = NULL};
		return;
	}
	while (list) {
		if (list->next == NULL) {
			list->next = malloc(sizeof(hash_node_t));
			*list->next = (hash_node_t){.book = book, .next = NULL};
			return;
		}
		list = list->next;
	}
}

int h_delete_book(hash_index_t *index, const char *title)
{
	unsigned h = hash(title) % index->size;
	hash_node_t ** table = index->buckets;
	hash_node_t * list = table[h];
	if (!list) {
		return 1;
	}
	if (!strcmp(list->book->title, title)) {
		table[h] = list->next;
		free(list->book);
		free(list);
		return 0;
	}
	while (list->next) {
		if (!strcmp(list->next->book->title, title)){
			free(list->next->book);
			hash_node_t * tmp = list->next->next;
			free(list->next);
			list->next = tmp;
			return 0;
		}
		list = list->next;
	}
	return 1;
}
