#include "table.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Open table with disk-based storage
bool table_open(Table* table, const char* data_filename, const char* index_filename) {
    if (!table || !data_filename || !index_filename) {
        return false;
    }
    
    memset(table, 0, sizeof(Table));
    
    // Store data filename
    table->data_filename = malloc(strlen(data_filename) + 1);
    if (!table->data_filename) {
        return false;
    }
    strcpy(table->data_filename, data_filename);
    
    // Open data file
    table->data_file = fopen(data_filename, "r+b");
    if (!table->data_file) {
        // Create new file if it doesn't exist
        table->data_file = fopen(data_filename, "w+b");
        if (!table->data_file) {
            printf("Failed to create data file: %s\n", strerror(errno));
            free(table->data_filename);
            return false;
        }
    }
    
    // Initialize B-Tree index
    if (!btree_init(index_filename, &table->btree_meta)) {
        printf("Failed to initialize B-Tree index\n");
        fclose(table->data_file);
        free(table->data_filename);
        return false;
    }
    
    // Initialize page cache
    if (!cache_init(&table->cache, &table->btree_meta)) {
        printf("Failed to initialize page cache\n");
        btree_close(&table->btree_meta);
        fclose(table->data_file);
        free(table->data_filename);
        return false;
    }
    
    // Initialize in-memory page array
    for (int i = 0; i < max_pages; i++) {
        table->pages[i] = NULL;
    }
    
    return true;
}

// Close table and cleanup
void table_close(Table* table) {
    if (!table) {
        return;
    }
    
    // Sync all changes to disk
    table_sync(table);
    
    // Cleanup cache
    cache_destroy(&table->cache);
    
    // Close B-Tree
    btree_close(&table->btree_meta);
    
    // Close data file
    if (table->data_file) {
        fclose(table->data_file);
    }
    
    // Free filename
    free(table->data_filename);
    
    // Free in-memory pages
    for (int i = 0; i < max_pages; i++) {
        if (table->pages[i]) {
            free(table->pages[i]);
        }
    }
}

// Write page to disk
bool table_write_page(Table* table, int page_num, Page* page) {
    if (!table || !table->data_file || !page || page_num < 0) {
        return false;
    }
    
    // Calculate offset for this page
    long offset = page_num * sizeof(Page);
    
    if (fseek(table->data_file, offset, SEEK_SET) != 0) {
        printf("Failed to seek to page %d in data file\n", page_num);
        return false;
    }
    
    if (fwrite(page, sizeof(Page), 1, table->data_file) != 1) {
        printf("Failed to write page %d to data file\n", page_num);
        return false;
    }
    
    fflush(table->data_file);
    return true;
}

// Read page from disk
bool table_read_page(Table* table, int page_num, Page* page) {
    if (!table || !table->data_file || !page || page_num < 0) {
        return false;
    }
    
    // Calculate offset for this page
    long offset = page_num * sizeof(Page);
    
    if (fseek(table->data_file, offset, SEEK_SET) != 0) {
        printf("Failed to seek to page %d in data file\n", page_num);
        return false;
    }
    
    if (fread(page, sizeof(Page), 1, table->data_file) != 1) {
        // Page might not exist yet (new page)
        memset(page, 0, sizeof(Page));
        page->num_rows = 0;
        return true;
    }
    
    return true;
}

// Sync all changes to disk
void table_sync(Table* table) {
    if (!table) {
        return;
    }
    
    // Flush page cache
    cache_flush_all(&table->cache);
    
    // Write all in-memory pages to disk
    for (int i = 0; i < table->num_pages; i++) {
        if (table->pages[i]) {
            table_write_page(table, i, table->pages[i]);
        }
    }
    
    // Sync data file
    if (table->data_file) {
        fflush(table->data_file);
    }
}

// Load page into memory if not already loaded
Page* table_get_page(Table* table, int page_num) {
    if (!table || page_num < 0 || page_num >= max_pages) {
        return NULL;
    }
    
    // Check if page is already in memory
    if (table->pages[page_num]) {
        return table->pages[page_num];
    }
    
    // Allocate new page
    table->pages[page_num] = malloc(sizeof(Page));
    if (!table->pages[page_num]) {
        printf("Failed to allocate memory for page %d\n", page_num);
        return NULL;
    }
    
    // Read page from disk
    if (!table_read_page(table, page_num, table->pages[page_num])) {
        free(table->pages[page_num]);
        table->pages[page_num] = NULL;
        return NULL;
    }
    
    return table->pages[page_num];
}
