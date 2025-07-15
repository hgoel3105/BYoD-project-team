#ifndef PAGE_CACHE_H
#define PAGE_CACHE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "btree.h"

#define CACHE_SIZE 32  // Number of pages to cache in memory
#define INVALID_PAGE_ID 0xFFFFFFFF

// Cache entry structure
typedef struct CacheEntry {
    uint32_t page_id;
    BTreeNode* node;
    bool is_dirty;
    bool is_valid;
    uint32_t access_time;
    struct CacheEntry* next;
    struct CacheEntry* prev;
} CacheEntry;

// Page cache structure
typedef struct {
    CacheEntry entries[CACHE_SIZE];
    CacheEntry* lru_head;  // Most recently used
    CacheEntry* lru_tail;  // Least recently used
    uint32_t access_counter;
    BTreeMeta* btree_meta;
    
    // Statistics
    uint32_t cache_hits;
    uint32_t cache_misses;
} PageCache;

// Cache operations
bool cache_init(PageCache* cache, BTreeMeta* btree_meta);
void cache_destroy(PageCache* cache);
BTreeNode* cache_get_page(PageCache* cache, uint32_t page_id);
bool cache_put_page(PageCache* cache, uint32_t page_id, BTreeNode* node);
bool cache_flush_page(PageCache* cache, uint32_t page_id);
bool cache_flush_all(PageCache* cache);
void cache_mark_dirty(PageCache* cache, uint32_t page_id);

// LRU operations
void cache_move_to_head(PageCache* cache, CacheEntry* entry);
CacheEntry* cache_remove_tail(PageCache* cache);
void cache_add_to_head(PageCache* cache, CacheEntry* entry);

// Cache statistics
void cache_print_stats(PageCache* cache);

#endif
