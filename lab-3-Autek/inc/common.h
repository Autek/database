#ifndef COMMON_H
#define COMMON_H
#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#define NUM_BOOKS 100

typedef struct Book {
    unsigned id;
    char title [1024];
    int page_cnt;
    int publish_year; 
    struct Book* next;
} Book_t;

#ifdef __cplusplus
}
#endif
#endif