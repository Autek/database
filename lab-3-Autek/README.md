[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/fcaYt3Ds)
[![Open in Visual Studio Code](https://classroom.github.com/assets/open-in-vscode-2e0aaae1b6195c2367325f4f02e2d04e9abb55f0b24a779b69b11b9e10269abc.svg)](https://classroom.github.com/online_ide?assignment_repo_id=19518616&assignment_repo_type=AssignmentRepo)
# CS300 - Lab 3

This lab was designed in three exercises focusing on indexing structures. In the first exercise, you will implement linked list functions to find specific records. In the second exercise, you will use the hash index to speed up the lookup time. In the last exercise, you will implement a B+ tree data structure.

```
- lab3
    - lib
        - ex1.c                 # Code for Exercise 1 - Linear Search
        - ex2.c                 # Code for Exercise 2 - Hash Map
        - ex2.c                 # Code for Exercise 3 - B+ Tree
    - inc
        - ex1.h                 # Header file for Exercise 1
        - ex2.h                 # Header file for Exercise 2
        - ex3.h                 # Header file for Exercise 3
        - common.h              # Header file for const
    - tests
        - tests-ex01            # Tests for Exercise 1
        - tests-ex02            # Tests for Exercise 2
        - tests-ex03            # Tests for Exercise 3
    - benchmarks                # Benchmarks to see indexing structures
```
You will write the code in the ex1.c, ex2.c and ex3.c files in the lib folder. The inc folder contains the header files describing the data structures you will use in your code. The tests folder contains the respective test cases for testing your implementation.

To run all test cases for all exercises, execute the following command:
```
make
```
You can also run tests separately by
```
make test-ex0[1-3]
```

## General Description
This lab will try to show the importance of indexing in the databases. Through the lab, you will implement a B+ tree and a hash index. In our "imaginary" database we have a table called `Book` which has the following attributes:  

```
Book {
    id:             primary-key
    title:          string
    page_cnt:       int
    publish_year:   int
}
```

## Exercise 1: Linear Scan (5 Points)
In this exercise, you will implement a search function for a linked list. You will be given a variable called `Book_t* books` which is a linked list where you're going to traverse and find the data records. For this exercise, implement these three functions:

- `Book_t* search_book_by_title(Book_t* book_list, const char* title)` 
- `Book_t* search_book_by_page_count(Book_t* book_list, int page_count)`
- `Book_t *search_book_by_id(Book_t * book_list, unsigned id)`

Great! Now, we can continue with the next exercise. We know that using a linked list may not be a good idea for big tables since searching for a specific key can take O(n) time. In the next exercise, you will implement a hash index to speed up this process.

## Exercise 2: Hash Index (10 Points)
In this exercise, you're going to build the hash index on the "Title" attribute of the `Book` table. 

### Small Reminder for Hash Index
A hash index is a data structure (kind of table) that stores records which allow direct access using a hash function. You can think hash function as a black box which produces a deterministic value from a key. `hash(key) -> hashed_value`. Once the value hashed, you can retrieve the corresponding record in O(1) time without traversing the entire table. However, if you have the same `hashed_value` for different keys, you need to chain these records in a linked list (See bucket 3). 

You can see the example of a hash table with size 6 below. 

```
  Bucket 0: [Book_t*] -> [Book_t*] -> NULL
  |
  Bucket 1: [Book_t*] -> NULL
  |
  Bucket 2: NULL
  |
  Bucket 3: [Book_t*] -> [Book_t*] -> [Book_t*] -> NULL
  |
  Bucket 4: NULL
  |
  Bucket 5: [Book_t*] -> NULL
```

You can assume there is a single data entry in the bucket. Also, you can assume there will be no same book title for different books, and you will not be inserting the same book into the hash index. To decide the bucket index to insert, you need to get the mod of the hash value by the size of the hash index.

### Implementation

You're going to implement these functions:

- `hash_index_t *h_init_index(int size)`: Build an empty hash table and return. If the size is 0 or less than 0 return `NULL`.
- `void h_destroy_index(hash_index_t *index)`:  Destroy the hash table.
- `unsigned hash(const char *str)`: Here you need to implement the `djb2` hash function which is widely used for string hashing. See the pseudocode down below.
- `Book_t *h_get_book(hash_index_t *index, const char *title)`: Get book with title, if no book found return `NULL`. 
- `void h_insert_book(hash_index_t *index, Book_t *book)`: Insert the book into the hash index, if you have a collision you need to chain the books. 
- `int h_delete_book(hash_index_t *index, const char *title)`: Delete the book entry from the hash index. For successful cases return 0, if there is no such book or any other error return 1.

For the `djb2` function please see the following pseudocode.

```
function hash(str) -> unsigned
    hash = 5381
    c = 0
    while (c = *str++) != null
        hash = ((hash << 5) + hash) + c
    return hash
```

## Exercise 3: B+ Tree Index (35 Points)

In this exercise, you're going to implement a B+Tree index for attribute `id`.

### Small Reminder for B+ Trees 

B+ tree is a widely used data structure in database indexing. It is a balanced search tree, and data records can reside only in the leaf nodes. Moreover, leaf nodes are linked to each other and they can grow from bottom to up and shrink dynamically.

To visualize how the B+ tree works you can use the following [website](https://www.cs.usfca.edu/~galles/visualization/BPlusTree.html).

To illustrate an example, let's create a B+tree that can store a maximum of 4 keys in a single node. As an initialization, let's insert `1 2 3 4`. Now, let's add `5`. Since a leaf can hold at most `2d` keys, it needs to split. In the split phase, it grew. Therefore, after the insertion, the tree will look like this.

![](sources/2024-04-04-07-12-28.png)

As you can notice 3 is duplicated on the root node. However, this will not happen when an index node splits. Again let's insert values until `12` now the tree will look like this.

![](sources/2024-04-04-07-32-23.png)

Now let's add `13` and see what will happen.

![](sources/2024-04-04-07-32-48.png)

Now the root node split but `7` is not duplicated.

### Implementation

In your implementation, you need to use these structs.

```c
typedef struct bp_tree
{
    bp_tree_node_t *root;
    int m;
} bp_tree_t;
```

```c
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
```

You will implement an unclustered B+ tree in which a single node can store at most `5 keys`. After you need to implement `get` and `insert` operations on top of it. 

- `bp_tree_t * bp_init_index()`: Create an empty B+ tree, and set `m` to the number of books per node. 
- `int bp_insert_book(bp_tree_t* tree, int key, Book_t *book);` Insert the book into B+ tree using the key. This is the hardest function you're going to implement. First find the correct place for key, then add book and update `n`. You need to think about the cases where you need to split the leaf nodes and intermediate nodes. After successful split you will have 3 keys on left and 3 keys on right. On successful inserts return `0`, if the book is already in the table return `1`.
- `Book_t * bp_get_book(bp_tree_t* tree, int key)`: Return the book which has a following `key`. If there is no book with that key return `NULL`.
- `void bp_destroy_tree(bp_tree_t* tree)`: Destroy the tree and free the `books` on leaf nodes. 

## Submit
You must submit (commit) your code for lab3 before Monday 23.59, 2nd of June 2025.
You can find the steps to submit your lab [here](https://github.com/cs300-epfl/lab-setup?tab=readme-ov-file). After the submission, your results will be available online on the PR. 

We recommend you pass the local test before submitting because GitHub limits the length of the error messages, so you may not see all the errors if there are too many.

### timeout
For macOS users, use
```
brew install coreutils
```
To install timeout

### Address sanitizer
When we compile your code, we add `-fsanitize=address` flags which enables the address sanitizer to check whether your code contains any invalid memory accesses. Whenever you code triggers an invalid memory access, the sanitizer will report the backtrace of where the invalid access is from.

If you're getting DEADLYSIGNAL from address sanitizer just use:

```
sudo sysctl vm.mmap_rnd_bits=28
cd your_repo_dir
make clean
make
```
