#ifndef INDEX_H
#define INDEX_H
#include "stdlib.h"
#include "stdbool.h"
typedef struct Node{
    int key;
    struct Node *left;
    struct Node *right;
    int height;
    int page_num;
    int row_num;
}Node;

extern Node* avl_root;

int height(struct Node *N);

struct Node* newNode(long long int key, int page, int slot);
void index_insert(long long int key, int page, int slot);
bool index_find(long long int key, int *page, int *slot);
Node* index_delete(Node* root, long long int key);
Node* rightRotate(Node* y);
Node* leftRotate(Node* x);

#endif