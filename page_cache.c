#include "page_cache.h"
#include <stdlib.h>
#include <string.h>

// Initialize cache
bool cache_init(PageCache* cache, BTreeMeta* btree_meta) {
    if (!cache || !btree_meta) {
        return false;
    }
    
    memset(cache, 0, sizeof(PageCache));
    cache->btree_meta = btree_meta;
    
    for (int i = 0; i < CACHE_SIZE; i++) {
        cache->entries[i].node = (BTreeNode*)malloc(sizeof(BTreeNode));
        if (!cache->entries[i].node) {
            // Handle memory allocation failure
            cache_destroy(cache);
            return false;
        }
        cache->entries[i].page_id = INVALID_PAGE_ID;
        cache->entries[i].is_valid = false;
        cache->entries[i].is_dirty = false;
    }
    
    return true;
}

// Destroy cache
void cache_destroy(PageCache* cache) {
    if (!cache) {
        return;
    }
    
    // Write back all dirty pages
    cache_flush_all(cache);
    
    for (int i = 0; i < CACHE_SIZE; i++) {
        free(cache->entries[i].node);
        cache->entries[i].node = NULL;
    }
}

// Get page from cache
BTreeNode* cache_get_page(PageCache* cache, uint32_t page_id) {
    if (!cache) {
        return NULL;
    }
    
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->entries[i].is_valid && cache->entries[i].page_id == page_id) {
            // Move accessed page to head of LRU
            cache_move_to_head(cache, &cache->entries[i]);
            cache->cache_hits++;
            cache->entries[i].access_time = cache->access_counter++;
            return cache->entries[i].node;
        }
    }
    
    // Page not in cache (cache miss)
    cache->cache_misses++;
    
    // Find an empty slot or evict LRU
    CacheEntry* entry = NULL;
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache->entries[i].is_valid) {
            entry = &cache->entries[i];
            break;
        }
    }
    
    if (!entry) {
        entry = cache_remove_tail(cache);
        if (!entry) {
            return NULL;
        }
        
        // Write back if dirty
        if (entry->is_dirty) {
            btree_write_node(cache->btree_meta, entry->page_id, entry->node);
        }
    }
    
    // Load page from disk
    if (!btree_read_node(cache->btree_meta, page_id, entry->node)) {
        return NULL;
    }
    
    // Setup cache entry
    entry->page_id = page_id;
    entry->is_valid = true;
    entry->is_dirty = false;
    entry->access_time = cache->access_counter++;
    
    // Add entry to head of LRU
    cache_add_to_head(cache, entry);
    
    return entry->node;
}

// Put page into cache
bool cache_put_page(PageCache* cache, uint32_t page_id, BTreeNode* node) {
    if (!cache || !node) {
        return false;
    }
    
    CacheEntry* entry = NULL;
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->entries[i].is_valid && cache->entries[i].page_id == page_id) {
            entry = &cache->entries[i];
            break;
        }
    }
    
    if (!entry) {
        // Find empty slot or evict LRU
        for (int i = 0; i < CACHE_SIZE; i++) {
            if (!cache->entries[i].is_valid) {
                entry = &cache->entries[i];
                break;
            }
        }
        
        if (!entry) {
            entry = cache_remove_tail(cache);
            if (!entry) {
                return false;
            }
            
            // Write back if dirty
            if (entry->is_dirty) {
                btree_write_node(cache->btree_meta, entry->page_id, entry->node);
            }
        }
    }
    
    // Copy node to cache entry
    memcpy(entry->node, node, sizeof(BTreeNode));
    entry->page_id = page_id;
    entry->is_valid = true;
    entry->is_dirty = true;
    entry->access_time = cache->access_counter++;
    
    // Add or move entry to head of LRU
    cache_add_to_head(cache, entry);
    
    return true;
}

// Flush a page to disk
bool cache_flush_page(PageCache* cache, uint32_t page_id) {
    if (!cache) {
        return false;
    }
    
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->entries[i].is_valid && cache->entries[i].page_id == page_id) {
            if (cache->entries[i].is_dirty) {
                if (!btree_write_node(cache->btree_meta, page_id, cache->entries[i].node)) {
                    return false;
                }
                cache->entries[i].is_dirty = false;
            }
            return true;
        }
    }
    return false;
}

// Flush all dirty pages to disk
bool cache_flush_all(PageCache* cache) {
    if (!cache) {
        return false;
    }
    
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->entries[i].is_valid && cache->entries[i].is_dirty) {
            if (!btree_write_node(cache->btree_meta, cache->entries[i].page_id, cache->entries[i].node)) {
                return false;
            }
            cache->entries[i].is_dirty = false;
        }
    }
    return true;
}

// Mark a page as dirty
void cache_mark_dirty(PageCache* cache, uint32_t page_id) {
    if (!cache) {
        return;
    }
    
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache->entries[i].is_valid && cache->entries[i].page_id == page_id) {
            cache->entries[i].is_dirty = true;
            return;
        }
    }
}

// Move entry to head of LRU
void cache_move_to_head(PageCache* cache, CacheEntry* entry) {
    if (!cache || !entry) {
        return;
    }
    
    if (entry == cache->lru_head) {
        return;  // Already at head
    }

    // Remove entry from its current position
    if (entry->prev) {
        entry->prev->next = entry->next;
    }
    if (entry->next) {
        entry->next->prev = entry->prev;
    }
    if (entry == cache->lru_tail) {
        cache->lru_tail = entry->prev;
    }

    // Attach entry at head
    entry->prev = NULL;
    entry->next = cache->lru_head;
    if (cache->lru_head) {
        cache->lru_head->prev = entry;
    }
    cache->lru_head = entry;
    if (!cache->lru_tail) {
        cache->lru_tail = entry;
    }
}

// Remove tail (LRU) from cache
CacheEntry* cache_remove_tail(PageCache* cache) {
    if (!cache || !cache->lru_tail) {
        return NULL;
    }
    
    CacheEntry* entry = cache->lru_tail;

    // Move tail
    if (entry->prev) {
        entry->prev->next = NULL;
    }
    cache->lru_tail = entry->prev;

    if (!cache->lru_tail) {
        cache->lru_head = NULL;
    }

    return entry;
}

// Add entry to head of LRU
void cache_add_to_head(PageCache* cache, CacheEntry* entry) {
    if (!cache || !entry) {
        return;
    }

    entry->prev = NULL;
    entry->next = cache->lru_head;

    if (cache->lru_head) {
        cache->lru_head->prev = entry;
    }

    cache->lru_head = entry;

    if (!cache->lru_tail) {
        cache->lru_tail = entry;
    }
}

// Print cache statistics
void cache_print_stats(PageCache* cache) {
    if (!cache) {
        return;
    }
    
    printf("Cache hits: %u, Cache misses: %u\n", cache->cache_hits, cache->cache_misses);
}
