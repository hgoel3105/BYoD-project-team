#ifndef TABLE_H
#define TABLE_H

#include "page.h"
#include "btree.h"
#include "page_cache.h"
#include <stdio.h>

#define max_pages 100

//array of pointers to 100 pages
typedef struct{
    Page* pages[max_pages];
    int num_pages; //number of pages in the table
    
    // Disk-based storage
    FILE* data_file;
    char* data_filename;
    
    // B-Tree index
    BTreeMeta btree_meta;
    
    // Page cache
    PageCache cache;
}Table;

void print_table(Table* table);
void insert(Table* table);
void delete_row(Table* table, int id);
int scan_table(Table* table, int ID);

// New disk-based operations
bool table_open(Table* table, const char* data_filename, const char* index_filename);
void table_close(Table* table);
bool table_write_page(Table* table, int page_num, Page* page);
bool table_read_page(Table* table, int page_num, Page* page);
void table_sync(Table* table);
Page* table_get_page(Table* table, int page_num);

#endif
