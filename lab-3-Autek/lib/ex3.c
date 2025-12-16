#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ex3.h"
#include "common.h"

bp_tree_t *bp_init_index(void)
{
	bp_tree_t* tree = calloc(1, sizeof(bp_tree_t));
	*tree = (bp_tree_t){.m = 5, .root = calloc(1, sizeof(bp_tree_node_t))}; 
	tree->root->is_leaf_node = true;
	return tree;
}

Book_t *bp_get_book(bp_tree_t *tree, int key)
{
	bp_tree_node_t* node = tree->root;
	while(!node->is_leaf_node) {
		for(int i = 0; i < node->n; i++) {
			if (node->keys[i] > key) {
				node = node->children[i];
				break;
			} 
			else if (i + 1 == node->n) {
				node = node->children[i + 1];
				break;
			}
		}
	}
	for(int i = 0; i < node->n; i++) {
		if (node->keys[i] == key) {
			return node->books[i];
		}
	}
	return NULL;
}

void splitLeaf(bp_tree_node_t*, int, int, Book_t*, bp_tree_t*);
void insert_node(bp_tree_node_t*, int, int, bp_tree_node_t*, Book_t*, bp_tree_t*);

bool alreadyPresent(bp_tree_node_t* node, int key) {
	for(int i = 0; i < node->n; i++) {
		if(node->keys[i] == key) {
			return true;
		}
	}
	return false;
}

int bp_insert_book(bp_tree_t *tree, int key, Book_t *book)
{
    // Check empty case

    // Check if book is already indexed

    // Get the corresponding leaf node

    // Insert 

    // If there is split needed handle that (you may implement some helper functions then use it here.)

	bp_tree_node_t* node = tree->root;
	if (alreadyPresent(node, key)) {
		return 1;
	}
	int m = tree->m;
	while(!node->is_leaf_node) {
		bool changed = false;
		for(int i = 0; i < node->n; i++) {
			if (node->keys[i] > key) {
				node = node->children[i];
				changed = true;
				break;
			} 
		}
		printf("key: %d\n", node->keys[0]);
		// node as not been changed
		if (!changed) {
			node = node->children[node->n];
		}
	}

	insert_node(node, key, m, NULL, book, tree);
	return 0;
}

int find_index(bp_tree_node_t* node, int key) {
	int index = -1;
	for(int i = 0; i < node->n; i++) {
		if (node->keys[i] > key) {
			index = i;
			break;
		}
	}
	if (index == -1) {
		index = node->n;
	}
	return index;
}


void splitLeaf(bp_tree_node_t* node, int key, int m, Book_t* book, bp_tree_t* tree) {
	int midpoint = (m+1) / 2;
	int index = find_index(node, key);

	int keys[m+1];
	Book_t* books[m+1];

	for(int i = 0; i <= m; i++) {
		if(i == index) {
			keys[i] = key;
			books[i] = book;
			break;
		}
		keys[i] = node->keys[i];
		books[i] = node->books[i];
	}

	bp_tree_node_t* left_node = node;

	left_node->n = midpoint;

	bp_tree_node_t* right_node = calloc(1, sizeof(bp_tree_node_t));
	right_node->n = midpoint;
	right_node->is_leaf_node = true;
	for(int i = 0; i < right_node->n; i++) {
		right_node->keys[i] = keys[midpoint + i];
		right_node->books[i] = books[midpoint + i];
	}

	left_node->next = right_node;
	
	if (left_node->parent == NULL) {
		right_node->parent = calloc(1, sizeof(bp_tree_node_t));
		left_node->parent = right_node->parent;
		right_node->parent->n = 1;
		right_node->parent->children[0] = left_node;
		right_node->parent->children[1] = right_node;
		right_node->parent->keys[0] = keys[midpoint];
		tree->root = right_node->parent;
	} else {
		right_node->parent = left_node->parent;
		insert_node(left_node->parent, keys[midpoint], m, right_node, books[midpoint], tree);
	}
}

void splitNode(bp_tree_node_t* node, int key, int m, bp_tree_node_t* child, bp_tree_t* tree) {
	int midpoint = (m+1) / 2;
	int index = find_index(node, key);

	// loading the keys and childrens
	int keys[m+1];
	bp_tree_node_t* children[m+2];
	for(int i = 0; i < index; i++) {
		keys[i] = node->keys[i];
		children[i] = node->children[i];
	}
	children[index] = node->children[index];
	keys[index] = key;
	children[index + 1] = child;
	for(int i = index + 1; i <= m; i++) {
		keys[i] = node->keys[i];
		children[i + 1] = node->children[i + 1];
	}
	// loading the keys and childrens

	bp_tree_node_t* left_node = node;
	left_node->n = midpoint;

	bp_tree_node_t* right_node = calloc(1, sizeof(bp_tree_node_t));
	right_node->n = midpoint - 1;
	right_node->parent = left_node->parent;
	for(int i = 0; i < right_node->n; i++) {
		right_node->keys[i] = keys[midpoint + 1 + i];
		right_node->children[i] = children[midpoint + 1 + i];
	}
	right_node->children[right_node->n] = children[m+1];

	if (left_node->parent == NULL) {
		right_node->parent = calloc(1, sizeof(bp_tree_node_t));
		left_node->parent = right_node->parent;
		right_node->parent->n = 1;
		right_node->parent->children[0] = left_node;
		right_node->parent->children[1] = right_node;
		right_node->parent->keys[0] = keys[midpoint];
		tree->root = right_node->parent;
	} else {
		right_node->parent = left_node->parent;
		insert_node(right_node->parent, keys[midpoint], m, right_node, NULL, tree);
	}
}


void insert_node(bp_tree_node_t* node, int key, int m, bp_tree_node_t* child, Book_t * book, bp_tree_t* tree) {
	if (node->n == m) {
		if (node->is_leaf_node) {
			splitLeaf(node, key, m, book, tree);
		} else {
			splitNode(node, key, m,  child, tree);
		}
		return;
	}
	int index = find_index(node, key);
	node->n++;

	int next1 = key;
	bp_tree_node_t* next2 = child;
	Book_t* next3 = book;

	for(int i = index; i < node->n; i++) {
		int tmp1 = node->keys[i];
		bp_tree_node_t* tmp2 = node->children[i+1];
		Book_t* tmp3 = node->books[i];
		node->keys[i] = next1;
		node->children[i+1] = next2;
		node->books[i] = next3;
		next1 = tmp1;
		next2 = tmp2;
		next3 = tmp3;
	}
}

void bp_destroy_rec(bp_tree_node_t*);

void bp_destroy_tree(bp_tree_t *tree)
{
	bp_tree_node_t* node = tree->root;
	bp_destroy_rec(node);
	free(tree);
}

void bp_destroy_rec(bp_tree_node_t *node) {
	for(int i = 0; i < node->n; i++) {
		if (node->is_leaf_node) {
			free(node->books[i]);
		}
		else {
			bp_destroy_rec(node->children[i]);
		}
	}
	free(node);
}

void print_tree(bp_tree_node_t* node) {
	if (!node) return;

	bp_tree_node_t** array = calloc(1000, sizeof(bp_tree_node_t));
	bp_tree_node_t** temp_array = calloc(1000, sizeof(bp_tree_node_t));
	if (!array || !temp_array) return;

	size_t array_size = 1;
	size_t temp_size = 0;
	array[0] = node;


	while (array_size != 0) {
		for (size_t i = 0; i < array_size; i++) {
			for (size_t j = 0; j < array[i]->n; j++) {
				printf("%d ", array[i]->keys[j]);
			}
			if(i != array_size-1) {
				printf("- ");
			}
		}
		printf("\n");

		temp_size = 0;
		for (size_t i = 0; i < array_size; i++) {
			for (size_t j = 0; j <= array[i]->n; j++) {
				if (!array[i]->is_leaf_node) {
					//printf("child index: %d\n", j);
					temp_array[temp_size++] = array[i]->children[j];
					bp_tree_node_t test = *array[i]->children[j];
				}
			}
		}

		// Swap pointers instead of leaking memory
		bp_tree_node_t** tmp = array;
		array = temp_array;
		temp_array = tmp;

		array_size = temp_size;
	}

	free(array);
	free(temp_array);
}
