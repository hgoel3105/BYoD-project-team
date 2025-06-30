#ifndef PAGE_H
#define PAGE_H

#include "row.h"
#define size 4096 //size of one page
#define ele 60 //size of row data
#define rows_per_page 68
//array of pointers to each row inserted in a single page
typedef struct{
    Row* row_ptr[rows_per_page];
    int num_rows; //number of rows in the page
}Page;

#endif
