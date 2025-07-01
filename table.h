#ifndef TABLE_H
#define TABLE_H

#include "page.h"
#define max_pages 100
//array of pointers to 100 pages
typedef struct{
    Page* pages[max_pages];
    int num_pages; //number of pages in the table
}Table;

void print_table(Table* table);
void insert(Table* table);
void delete_row(Table* table, int id);
int scan_table(Table* table, int ID);
#endif
