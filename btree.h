#ifndef BTREE_H
#define BTREE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define PAGE_SIZE 4096
#define BTREE_ORDER 128  // Chosen to fit in 4KB page
#define MAX_KEYS (BTREE_ORDER - 1)
#define MAX_CHILDREN BTREE_ORDER

// B-Tree node structure aligned to 4KB pages
typedef struct {
    uint32_t page_id;           // Page ID on disk
    bool is_leaf;               // True if leaf node
    uint16_t num_keys;          // Number of keys in this node
    uint16_t num_children;      // Number of children (internal nodes only)
    bool is_dirty;              // True if node has been modified
    
    // Keys and their corresponding page/slot locations
    struct {
        long long key;          // Primary key value
        uint32_t page_num;      // Data page number
        uint16_t slot_num;      // Slot within the page
    } entries[MAX_KEYS];
    
    uint32_t children[MAX_CHILDREN];  // Child page IDs (internal nodes only)
    
    // Padding to align to 4KB
    uint8_t padding[PAGE_SIZE - 
        (sizeof(uint32_t) + sizeof(bool) + 2*sizeof(uint16_t) + sizeof(bool) +
         MAX_KEYS * (sizeof(long long) + sizeof(uint32_t) + sizeof(uint16_t)) +
         MAX_CHILDREN * sizeof(uint32_t))];
} BTreeNode;

// B-Tree metadata structure
typedef struct {
    uint32_t root_page_id;
    uint32_t next_page_id;
    uint32_t num_pages;
    FILE* file;
} BTreeMeta;

// B-Tree operations
bool btree_init(const char* filename, BTreeMeta* meta);
void btree_close(BTreeMeta* meta);
bool btree_insert(BTreeMeta* meta, long long key, uint32_t page_num, uint16_t slot_num);
bool btree_find(BTreeMeta* meta, long long key, uint32_t* page_num, uint16_t* slot_num);
bool btree_delete(BTreeMeta* meta, long long key);

// Node I/O operations
bool btree_read_node(BTreeMeta* meta, uint32_t page_id, BTreeNode* node);
bool btree_write_node(BTreeMeta* meta, uint32_t page_id, BTreeNode* node);
uint32_t btree_allocate_page(BTreeMeta* meta);

#endif
