#ifndef TABLE_H
#define TABLE_H

#include "page.h"
#define max_pages 100
//array of pointers to 100 pages
typedef struct{
    Page* pages[max_pages];
    int num_pages; //number of pages in the table
}Table;

#endif
