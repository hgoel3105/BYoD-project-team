#include "btree.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Initialize B-Tree with disk file
bool btree_init(const char* filename, BTreeMeta* meta) {
    if (!meta || !filename) {
        return false;
    }
    
    meta->file = fopen(filename, "r+b");
    if (!meta->file) {
        // Create new file if it doesn't exist
        meta->file = fopen(filename, "w+b");
        if (!meta->file) {
            printf("Failed to create B-Tree file: %s\n", strerror(errno));
            return false;
        }
        
        // Initialize new B-Tree
        meta->root_page_id = 1;
        meta->next_page_id = 2;
        meta->num_pages = 1;
        
        // Create root node
        BTreeNode root;
        memset(&root, 0, sizeof(BTreeNode));
        root.page_id = 1;
        root.is_leaf = true;
        root.num_keys = 0;
        root.num_children = 0;
        root.is_dirty = false;
        
        if (!btree_write_node(meta, 1, &root)) {
            fclose(meta->file);
            return false;
        }
        
        // Write metadata to beginning of file
        fseek(meta->file, 0, SEEK_SET);
        fwrite(meta, sizeof(BTreeMeta) - sizeof(FILE*), 1, meta->file);
        fflush(meta->file);
    } else {
        // Read existing metadata
        fseek(meta->file, 0, SEEK_SET);
        if (fread(meta, sizeof(BTreeMeta) - sizeof(FILE*), 1, meta->file) != 1) {
            fclose(meta->file);
            return false;
        }
        // Reset file pointer as it's not stored in file
        fseek(meta->file, 0, SEEK_SET);
        fread(meta, sizeof(BTreeMeta) - sizeof(FILE*), 1, meta->file);
    }
    
    return true;
}

// Close B-Tree and save metadata
void btree_close(BTreeMeta* meta) {
    if (!meta || !meta->file) {
        return;
    }
    
    // Write metadata to file
    fseek(meta->file, 0, SEEK_SET);
    fwrite(meta, sizeof(BTreeMeta) - sizeof(FILE*), 1, meta->file);
    fflush(meta->file);
    
    fclose(meta->file);
    meta->file = NULL;
}

// Read a B-Tree node from disk
bool btree_read_node(BTreeMeta* meta, uint32_t page_id, BTreeNode* node) {
    if (!meta || !meta->file || !node || page_id == 0) {
        return false;
    }
    
    long offset = sizeof(BTreeMeta) - sizeof(FILE*) + (page_id - 1) * PAGE_SIZE;
    
    if (fseek(meta->file, offset, SEEK_SET) != 0) {
        printf("Failed to seek to page %u\n", page_id);
        return false;
    }
    
    if (fread(node, sizeof(BTreeNode), 1, meta->file) != 1) {
        printf("Failed to read page %u\n", page_id);
        return false;
    }
    
    return true;
}

// Write a B-Tree node to disk
bool btree_write_node(BTreeMeta* meta, uint32_t page_id, BTreeNode* node) {
    if (!meta || !meta->file || !node || page_id == 0) {
        return false;
    }
    
    long offset = sizeof(BTreeMeta) - sizeof(FILE*) + (page_id - 1) * PAGE_SIZE;
    
    if (fseek(meta->file, offset, SEEK_SET) != 0) {
        printf("Failed to seek to page %u\n", page_id);
        return false;
    }
    
    if (fwrite(node, sizeof(BTreeNode), 1, meta->file) != 1) {
        printf("Failed to write page %u\n", page_id);
        return false;
    }
    
    fflush(meta->file);
    return true;
}

// Allocate a new page ID
uint32_t btree_allocate_page(BTreeMeta* meta) {
    if (!meta) {
        return 0;
    }
    
    uint32_t new_page_id = meta->next_page_id++;
    meta->num_pages++;
    return new_page_id;
}

// Find key in B-Tree node
static int btree_find_key(BTreeNode* node, long long key) {
    int i = 0;
    while (i < node->num_keys && key > node->entries[i].key) {
        i++;
    }
    return i;
}

// Split a full child node
static bool btree_split_child(BTreeMeta* meta, BTreeNode* parent, int child_index) {
    BTreeNode full_child, new_child;
    
    if (!btree_read_node(meta, parent->children[child_index], &full_child)) {
        return false;
    }
    
    // Create new child node
    memset(&new_child, 0, sizeof(BTreeNode));
    new_child.page_id = btree_allocate_page(meta);
    new_child.is_leaf = full_child.is_leaf;
    new_child.num_keys = MAX_KEYS / 2;
    new_child.is_dirty = true;
    
    // Move half of the keys to new child
    for (int i = 0; i < MAX_KEYS / 2; i++) {
        new_child.entries[i] = full_child.entries[i + MAX_KEYS / 2 + 1];
    }
    
    // Move half of the children if not leaf
    if (!full_child.is_leaf) {
        new_child.num_children = MAX_KEYS / 2 + 1;
        for (int i = 0; i < MAX_KEYS / 2 + 1; i++) {
            new_child.children[i] = full_child.children[i + MAX_KEYS / 2 + 1];
        }
    }
    
    // Update full child
    full_child.num_keys = MAX_KEYS / 2;
    full_child.is_dirty = true;
    
    // Move parent's children to make room
    for (int i = parent->num_children; i > child_index + 1; i--) {
        parent->children[i] = parent->children[i - 1];
    }
    parent->children[child_index + 1] = new_child.page_id;
    parent->num_children++;
    
    // Move parent's keys to make room
    for (int i = parent->num_keys; i > child_index; i--) {
        parent->entries[i] = parent->entries[i - 1];
    }
    parent->entries[child_index] = full_child.entries[MAX_KEYS / 2];
    parent->num_keys++;
    parent->is_dirty = true;
    
    // Write nodes back to disk
    if (!btree_write_node(meta, full_child.page_id, &full_child) ||
        !btree_write_node(meta, new_child.page_id, &new_child)) {
        return false;
    }
    
    return true;
}

// Insert into non-full node
static bool btree_insert_non_full(BTreeMeta* meta, uint32_t page_id, long long key, uint32_t page_num, uint16_t slot_num) {
    BTreeNode node;
    
    if (!btree_read_node(meta, page_id, &node)) {
        return false;
    }
    
    int i = node.num_keys - 1;
    
    if (node.is_leaf) {
        // Shift keys to make room
        while (i >= 0 && key < node.entries[i].key) {
            node.entries[i + 1] = node.entries[i];
            i--;
        }
        
        // Insert new key
        node.entries[i + 1].key = key;
        node.entries[i + 1].page_num = page_num;
        node.entries[i + 1].slot_num = slot_num;
        node.num_keys++;
        node.is_dirty = true;
        
        return btree_write_node(meta, page_id, &node);
    } else {
        // Find child to insert into
        while (i >= 0 && key < node.entries[i].key) {
            i--;
        }
        i++;
        
        BTreeNode child;
        if (!btree_read_node(meta, node.children[i], &child)) {
            return false;
        }
        
        // Split child if full
        if (child.num_keys == MAX_KEYS) {
            if (!btree_split_child(meta, &node, i)) {
                return false;
            }
            
            // Re-read node after split
            if (!btree_read_node(meta, page_id, &node)) {
                return false;
            }
            
            if (key > node.entries[i].key) {
                i++;
            }
        }
        
        return btree_insert_non_full(meta, node.children[i], key, page_num, slot_num);
    }
}

// Insert key into B-Tree
bool btree_insert(BTreeMeta* meta, long long key, uint32_t page_num, uint16_t slot_num) {
    if (!meta) {
        return false;
    }
    
    BTreeNode root;
    if (!btree_read_node(meta, meta->root_page_id, &root)) {
        return false;
    }
    
    // Check if root is full
    if (root.num_keys == MAX_KEYS) {
        // Create new root
        BTreeNode new_root;
        memset(&new_root, 0, sizeof(BTreeNode));
        new_root.page_id = btree_allocate_page(meta);
        new_root.is_leaf = false;
        new_root.num_keys = 0;
        new_root.num_children = 1;
        new_root.children[0] = meta->root_page_id;
        new_root.is_dirty = true;
        
        // Split old root
        if (!btree_split_child(meta, &new_root, 0)) {
            return false;
        }
        
        // Update root page ID
        meta->root_page_id = new_root.page_id;
        
        if (!btree_write_node(meta, new_root.page_id, &new_root)) {
            return false;
        }
        
        return btree_insert_non_full(meta, meta->root_page_id, key, page_num, slot_num);
    } else {
        return btree_insert_non_full(meta, meta->root_page_id, key, page_num, slot_num);
    }
}

// Search for key in B-Tree
static bool btree_search_node(BTreeMeta* meta, uint32_t page_id, long long key, uint32_t* page_num, uint16_t* slot_num) {
    if (page_id == 0) {
        return false;
    }
    
    BTreeNode node;
    if (!btree_read_node(meta, page_id, &node)) {
        return false;
    }
    
    int i = 0;
    while (i < node.num_keys && key > node.entries[i].key) {
        i++;
    }
    
    if (i < node.num_keys && key == node.entries[i].key) {
        // Found the key
        *page_num = node.entries[i].page_num;
        *slot_num = node.entries[i].slot_num;
        return true;
    }
    
    if (node.is_leaf) {
        return false;  // Key not found
    }
    
    // Search in child
    return btree_search_node(meta, node.children[i], key, page_num, slot_num);
}

// Find key in B-Tree
bool btree_find(BTreeMeta* meta, long long key, uint32_t* page_num, uint16_t* slot_num) {
    if (!meta || !page_num || !slot_num) {
        return false;
    }
    
    return btree_search_node(meta, meta->root_page_id, key, page_num, slot_num);
}

// Find predecessor key in a subtree
static void btree_get_predecessor(BTreeMeta* meta, BTreeNode* node, int idx, long long* key, uint32_t* page_num, uint16_t* slot_num) {
    BTreeNode current = *node;
    
    // Keep going to the right most node of left subtree
    if (!current.is_leaf) {
        btree_read_node(meta, current.children[idx], &current);
        while (!current.is_leaf) {
            btree_read_node(meta, current.children[current.num_children - 1], &current);
        }
    }
    
    // Get the last key
    *key = current.entries[current.num_keys - 1].key;
    *page_num = current.entries[current.num_keys - 1].page_num;
    *slot_num = current.entries[current.num_keys - 1].slot_num;
}

// Find successor key in a subtree
static void btree_get_successor(BTreeMeta* meta, BTreeNode* node, int idx, long long* key, uint32_t* page_num, uint16_t* slot_num) {
    BTreeNode current = *node;
    
    // Keep going to the left most node of right subtree
    if (!current.is_leaf) {
        btree_read_node(meta, current.children[idx + 1], &current);
        while (!current.is_leaf) {
            btree_read_node(meta, current.children[0], &current);
        }
    }
    
    // Get the first key
    *key = current.entries[0].key;
    *page_num = current.entries[0].page_num;
    *slot_num = current.entries[0].slot_num;
}

// Delete from non-empty node
static bool btree_delete_from_node(BTreeMeta* meta, uint32_t page_id, long long key) {
    BTreeNode node;
    
    if (!btree_read_node(meta, page_id, &node)) {
        return false;
    }
    
    int i = 0;
    while (i < node.num_keys && key > node.entries[i].key) {
        i++;
    }
    
    if (i < node.num_keys && key == node.entries[i].key) {
        // Key found in this node
        if (node.is_leaf) {
            // Simple case: delete from leaf
            for (int j = i; j < node.num_keys - 1; j++) {
                node.entries[j] = node.entries[j + 1];
            }
            node.num_keys--;
            node.is_dirty = true;
            return btree_write_node(meta, page_id, &node);
        } else {
            // Internal node: replace with predecessor or successor
            long long pred_key;
            uint32_t pred_page;
            uint16_t pred_slot;
            
            btree_get_predecessor(meta, &node, i, &pred_key, &pred_page, &pred_slot);
            
            // Replace current key with predecessor
            node.entries[i].key = pred_key;
            node.entries[i].page_num = pred_page;
            node.entries[i].slot_num = pred_slot;
            node.is_dirty = true;
            
            if (!btree_write_node(meta, page_id, &node)) {
                return false;
            }
            
            // Delete the predecessor from left subtree
            return btree_delete_from_node(meta, node.children[i], pred_key);
        }
    } else {
        // Key not in this node, go to appropriate child
        if (node.is_leaf) {
            return true; // Key not found, but that's OK
        }
        
        return btree_delete_from_node(meta, node.children[i], key);
    }
}

// Delete key from B-Tree
bool btree_delete(BTreeMeta* meta, long long key) {
    if (!meta) {
        return false;
    }
    
    return btree_delete_from_node(meta, meta->root_page_id, key);
}
