#ifndef EX3_H
#define EX3_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "common.h"

#define BP_TREE_ORDER 5

typedef struct bp_tree_node
{
    int n;                                                      // Number of keys in the node
    bool is_leaf_node;                                          // Is this node a leaf?
    
    int keys[BP_TREE_ORDER];                                    // Keys
    Book_t* books[BP_TREE_ORDER];                               // Pointers to books

    struct bp_tree_node* children[BP_TREE_ORDER + 1];           // Pointers to children
    struct bp_tree_node* parent;
    
    struct bp_tree_node* next;
} bp_tree_node_t;

typedef struct bp_tree
{
    bp_tree_node_t *root;
    int m;
} bp_tree_t;

bp_tree_t* bp_init_index(void);
int bp_insert_book(bp_tree_t *tree, int key, Book_t *book);
Book_t * bp_get_book(bp_tree_t *tree, int key);
void bp_destroy_tree(bp_tree_t *tree);
void print_tree(bp_tree_node_t* node);

#ifdef __cplusplus
}
#endif
#endif

