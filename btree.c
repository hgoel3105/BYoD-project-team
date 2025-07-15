#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#define PAGE_SIZE 4096
#define KEY_SIZE sizeof(int)
#define PTR_SIZE sizeof(int)
#define ENTRY_SIZE (KEY_SIZE + PTR_SIZE)
#define META_SIZE (sizeof(bool) + sizeof(int))
#define MAX_KEYS ((PAGE_SIZE - META_SIZE) / ENTRY_SIZE - 1)

// B-Tree node stored in a 4KB page
typedef struct
{
    bool is_leaf;
    int num_keys;
    int keys[MAX_KEYS];         // sorted keys
    int children[MAX_KEYS + 1]; // disk page numbers
} BTreeNode;

// File to store B-Tree
static const char *BTREE_FILE = "btree.dat";
static FILE *btree_fp = NULL;
static int root_page = -1;
static int next_free_page = 0;

int allocate_page()
{
    int page_num = next_free_page++;
    // Update metadata in file header
    fseek(btree_fp, sizeof(int), SEEK_SET); // Skip root_page
    fwrite(&next_free_page, sizeof(int), 1, btree_fp);
    fflush(btree_fp);
    return page_num;
}

void write_node(int page_num, BTreeNode *node)
{
    fseek(btree_fp, page_num * PAGE_SIZE, SEEK_SET);
    fwrite(node, sizeof(BTreeNode), 1, btree_fp);
    fflush(btree_fp);
}

void read_node(int page_num, BTreeNode *node)
{
    fseek(btree_fp, page_num * PAGE_SIZE, SEEK_SET);
    fread(node, sizeof(BTreeNode), 1, btree_fp);
}

BTreeNode *create_node(bool is_leaf)
{
    BTreeNode *node = calloc(1, sizeof(BTreeNode));
    node->is_leaf = is_leaf;
    node->num_keys = 0;
    return node;
}

int btree_search(int page_num, int key)
{
    BTreeNode node;
    read_node(page_num, &node);

    int i = 0;
    while (i < node.num_keys && key > node.keys[i])
    {
        i++;
    }
    if (i < node.num_keys && key == node.keys[i])
    {
        return page_num; 
    }
    if (node.is_leaf)
    {
        return -1; 
    }
    else
    {
        return btree_search(node.children[i], key);
    }
}

void split_child(int parent_page, int idx, int child_page)
{
    BTreeNode parent, child;
    read_node(parent_page, &parent);
    read_node(child_page, &child);

    BTreeNode *new_node = create_node(child.is_leaf);
    int new_page = allocate_page();

    int t = MAX_KEYS / 2;
    new_node->num_keys = child.num_keys - t - 1;

    for (int j = 0; j < new_node->num_keys; j++)
    {
        new_node->keys[j] = child.keys[j + t + 1];
    }

    if (!child.is_leaf)
    {
        for (int j = 0; j <= new_node->num_keys; j++)
        {
            new_node->children[j] = child.children[j + t + 1];
        }
    }
    child.num_keys = t;

    for (int j = parent.num_keys; j >= idx + 1; j--)
    {
        parent.children[j + 1] = parent.children[j];
    }
    parent.children[idx + 1] = new_page;

    for (int j = parent.num_keys - 1; j >= idx; j--)
    {
        parent.keys[j + 1] = parent.keys[j];
    }
    parent.keys[idx] = child.keys[t];
    parent.num_keys++;

    // Write nodes back to disk
    write_node(child_page, &child);
    write_node(new_page, new_node);
    write_node(parent_page, &parent);
    free(new_node);
}

void insert_nonfull(int page_num, int key)
{
    BTreeNode node;
    read_node(page_num, &node);

    int i = node.num_keys - 1;
    if (node.is_leaf)
    {
        // Shift keys to insert
        while (i >= 0 && key < node.keys[i])
        {
            node.keys[i + 1] = node.keys[i];
            i--;
        }
        node.keys[i + 1] = key;
        node.num_keys++;
        write_node(page_num, &node);
    }
    else
    {
        // Find child index
        while (i >= 0 && key < node.keys[i])
            i--;
        i++;
        BTreeNode child;
        read_node(node.children[i], &child);
        // Split if full
        if (child.num_keys == MAX_KEYS)
        {
            split_child(page_num, i, node.children[i]);
            read_node(page_num, &node);
            if (key > node.keys[i])
                i++;
        }
        insert_nonfull(node.children[i], key);
    }
}

void btree_insert(int key)
{
    if (root_page == -1)
    {
        BTreeNode *root = create_node(true);
        root_page = allocate_page();
        write_node(root_page, root);

        // Update root_page in file header
        fseek(btree_fp, 0, SEEK_SET);
        fwrite(&root_page, sizeof(int), 1, btree_fp);
        fflush(btree_fp);

        free(root);
    }

    BTreeNode root;
    read_node(root_page, &root);

    if (root.num_keys == MAX_KEYS)
    {
        // Create new root
        BTreeNode *new_root = create_node(false);
        int new_root_page = allocate_page();
        new_root->children[0] = root_page;
        write_node(new_root_page, new_root);

        split_child(new_root_page, 0, root_page);
        root_page = new_root_page;

        // Update root_page in file header
        fseek(btree_fp, 0, SEEK_SET);
        fwrite(&root_page, sizeof(int), 1, btree_fp);
        fflush(btree_fp);

        free(new_root);
        insert_nonfull(root_page, key);
    }
    else
    {
        insert_nonfull(root_page, key);
    }
}

int btree_init()
{
    // Try to open existing file first
    btree_fp = fopen(BTREE_FILE, "r+b");

    if (btree_fp)
    {
        fseek(btree_fp, 0, SEEK_SET);
        if (fread(&root_page, sizeof(int), 1, btree_fp) != 1 ||
            fread(&next_free_page, sizeof(int), 1, btree_fp) != 1)
        {
            perror("Failed to read B-tree metadata");
            fclose(btree_fp);
            return -1;
        }
        printf("Loaded existing B-tree: root at page %d, next free page %d\n",
               root_page, next_free_page);
    }
    else
    {
        btree_fp = fopen(BTREE_FILE, "w+b");
        if (!btree_fp)
        {
            perror("Failed to create B-Tree file");
            return -1;
        }

        root_page = -1;
        next_free_page = 1; // Reserve page 0 for metadata

        // Write header (root_page and next_free_page)
        fseek(btree_fp, 0, SEEK_SET);
        fwrite(&root_page, sizeof(int), 1, btree_fp);
        fwrite(&next_free_page, sizeof(int), 1, btree_fp);
        fflush(btree_fp);

        printf("Created new B-tree file\n");
    }

    return 0;
}

void remove_from_node(BTreeNode *node, int idx) {
    for (int i = idx + 1; i < node->num_keys; i++) {
        node->keys[i - 1] = node->keys[i];
    }
    node->num_keys--;
}

int get_predecessor(int page_num) {
    BTreeNode node;
    read_node(page_num, &node);
    while (!node.is_leaf) {
        page_num = node.children[node.num_keys];
        read_node(page_num, &node);
    }
    return node.keys[node.num_keys - 1];
}

int get_successor(int page_num) {
    BTreeNode node;
    read_node(page_num, &node);
    while (!node.is_leaf) {
        page_num = node.children[0];
        read_node(page_num, &node);
    }
    return node.keys[0];
}

void merge_nodes(int parent_page, int idx, int left_page, int right_page) {
    BTreeNode parent, left, right;
    read_node(parent_page, &parent);
    read_node(left_page, &left);
    read_node(right_page, &right);

    // Bring down parent key
    left.keys[left.num_keys] = parent.keys[idx];

    // Append right node's keys and children
    for (int i = 0; i < right.num_keys; i++) {
        left.keys[left.num_keys + 1 + i] = right.keys[i];
    }
    if (!right.is_leaf) {
        for (int i = 0; i <= right.num_keys; i++) {
            left.children[left.num_keys + 1 + i] = right.children[i];
        }
    }
    left.num_keys += right.num_keys + 1;

    // Remove key from parent
    for (int i = idx + 1; i < parent.num_keys; i++) {
        parent.keys[i - 1] = parent.keys[i];
        parent.children[i] = parent.children[i + 1];
    }
    parent.num_keys--;

    write_node(left_page, &left);
    write_node(parent_page, &parent);
    // right_page is now unused 
}

void fill_child(int parent_page, int idx) {
    BTreeNode parent, child;
    read_node(parent_page, &parent);
    read_node(parent.children[idx], &child);

    if (idx > 0) {
        BTreeNode left_sibling;
        read_node(parent.children[idx - 1], &left_sibling);
        if (left_sibling.num_keys >= (MAX_KEYS + 1) / 2) {
            // Borrow from left
            for (int i = child.num_keys; i > 0; i--) {
                child.keys[i] = child.keys[i - 1];
            }
            if (!child.is_leaf) {
                for (int i = child.num_keys + 1; i > 0; i--) {
                    child.children[i] = child.children[i - 1];
                }
            }
            child.keys[0] = parent.keys[idx - 1];
            if (!child.is_leaf)
                child.children[0] = left_sibling.children[left_sibling.num_keys];
            parent.keys[idx - 1] = left_sibling.keys[left_sibling.num_keys - 1];
            left_sibling.num_keys--;
            child.num_keys++;

            write_node(parent.children[idx - 1], &left_sibling);
            write_node(parent.children[idx], &child);
            write_node(parent_page, &parent);
            return;
        }
    }

    if (idx < parent.num_keys) {
        BTreeNode right_sibling;
        read_node(parent.children[idx + 1], &right_sibling);
        if (right_sibling.num_keys >= (MAX_KEYS + 1) / 2) {
            // Borrow from right
            child.keys[child.num_keys] = parent.keys[idx];
            if (!child.is_leaf)
                child.children[child.num_keys + 1] = right_sibling.children[0];
            parent.keys[idx] = right_sibling.keys[0];

            for (int i = 1; i < right_sibling.num_keys; i++) {
                right_sibling.keys[i - 1] = right_sibling.keys[i];
            }
            if (!right_sibling.is_leaf) {
                for (int i = 1; i <= right_sibling.num_keys; i++) {
                    right_sibling.children[i - 1] = right_sibling.children[i];
                }
            }
            right_sibling.num_keys--;
            child.num_keys++;

            write_node(parent.children[idx + 1], &right_sibling);
            write_node(parent.children[idx], &child);
            write_node(parent_page, &parent);
            return;
        }
    }

    // Merge either left or right
    if (idx < parent.num_keys) {
        merge_nodes(parent_page, idx, parent.children[idx], parent.children[idx + 1]);
    } else {
        merge_nodes(parent_page, idx - 1, parent.children[idx - 1], parent.children[idx]);
    }
}

void delete_from_node(int page_num, int key) {
    BTreeNode node;
    read_node(page_num, &node);

    int idx = 0;
    while (idx < node.num_keys && key > node.keys[idx])
        idx++;

    if (idx < node.num_keys && key == node.keys[idx]) {
        // Key found in this node

        if (node.is_leaf) {
            // Case 1: Leaf node – just remove it
            remove_from_node(&node, idx);
            write_node(page_num, &node);
        } else {
            // Case 2: Internal node
            int pred_page = node.children[idx];
            int succ_page = node.children[idx + 1];
            BTreeNode pred, succ;
            read_node(pred_page, &pred);
            read_node(succ_page, &succ);

            if (pred.num_keys >= (MAX_KEYS + 1) / 2) {
                // Case 2a: Replace with predecessor
                int pred_key = get_predecessor(pred_page);
                node.keys[idx] = pred_key;
                write_node(page_num, &node);
                delete_from_node(pred_page, pred_key);
            } else if (succ.num_keys >= (MAX_KEYS + 1) / 2) {
                // Case 2b: Replace with successor
                int succ_key = get_successor(succ_page);
                node.keys[idx] = succ_key;
                write_node(page_num, &node);
                delete_from_node(succ_page, succ_key);
            } else {
                // Case 2c: Merge and recurse
                merge_nodes(page_num, idx, pred_page, succ_page);
                delete_from_node(pred_page, key);
            }
        }
    } else {
        // Key not in this node
        if (node.is_leaf) {
            // Case 3: Key not found and node is leaf
            return; 
        }

        bool last_child = (idx == node.num_keys);

        BTreeNode child;
        read_node(node.children[idx], &child);

        if (child.num_keys < (MAX_KEYS + 1) / 2) {
            fill_child(page_num, idx);
            read_node(page_num, &node); // reload parent in case it changed
        }

        // Recompute child index if merge occurred
        if (last_child && idx > node.num_keys)
            idx--;

        delete_from_node(node.children[idx], key);
    }
}

// It handles the root changes
void btree_delete(int key) {
    if (root_page == -1)
        return;

    delete_from_node(root_page, key);

    BTreeNode root;
    read_node(root_page, &root);

    if (root.num_keys == 0) {
        if (!root.is_leaf) {
            int old_root = root_page;
            root_page = root.children[0];
            fseek(btree_fp, 0, SEEK_SET);
            fwrite(&root_page, sizeof(int), 1, btree_fp);
            fflush(btree_fp);
        } else {
            // The Tree becomes empty
            root_page = -1;
            fseek(btree_fp, 0, SEEK_SET);
            fwrite(&root_page, sizeof(int), 1, btree_fp);
            fflush(btree_fp);
        }
    }
}


// Function to save B-tree metadata before closing
void btree_close()
{
    if (btree_fp)
    {
        // Write current metadata to file header
        fseek(btree_fp, 0, SEEK_SET);
        fwrite(&root_page, sizeof(int), 1, btree_fp);
        fwrite(&next_free_page, sizeof(int), 1, btree_fp);
        fflush(btree_fp);
        fclose(btree_fp);
        btree_fp = NULL;
    }
}

void print_node(int page_num, int level)
{
    BTreeNode node;
    read_node(page_num, &node);

    // Print indentation based on level
    for (int i = 0; i < level; i++)
    {
        printf("    ");
    }

    // Print keys in this node
    printf("Node(page %d, %s): [", page_num, node.is_leaf ? "leaf" : "internal");
    for (int i = 0; i < node.num_keys; i++)
    {
        printf("%d", node.keys[i]);
        if (i < node.num_keys - 1)
        {
            printf(", ");
        }
    }
    printf("]\n");

    // Recursively print children if not leaf
    if (!node.is_leaf)
    {
        for (int i = 0; i <= node.num_keys; i++)
        {
            print_node(node.children[i], level + 1);
        }
    }
}

void print_btree()
{
    if (root_page == -1)
    {
        printf("B-tree is empty\n");
        return;
    }

    printf("\n=== B-Tree Structure ===\n");
    print_node(root_page, 0);
}

int main() {
    if (btree_init() != 0)
        return 1;

    printf("Inserting keys: ");
    for (int i = 1; i <= 20; i++) {
        printf("%d ", i);
        btree_insert(i);
    }
    print_btree();

    printf("\n");

    printf("Deleting keys: ");
    for (int i = 5; i <= 15; i++) {
        printf("%d ", i);
        btree_delete(i);
    }
    print_btree();

    printf("\n");

    printf("Searching for 10: %s\n", btree_search(root_page, 10) != -1 ? "Found" : "Not Found");
    printf("Searching for 3: %s\n", btree_search(root_page, 3) != -1 ? "Found" : "Not Found");

    fclose(btree_fp);
    return 0;
}
