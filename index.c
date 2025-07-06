#include "index.h"

int max(int a, int b){
    return a>b?a:b;
}

int getBalance(Node* node) {
    return node ? height(node->left) - height(node->right) : 0;
}

int height(struct Node *N)
{
    if(N==NULL)
        return 0;
    return N->height;
}

struct Node* newNode(long long int key, int page, int slot)
{
    Node* node=(Node*)malloc(sizeof(Node));
    node->key=key;
    node->row_num=slot;
    node->page_num=page;
    node->left=NULL;
    node->right=NULL;
    node->height=1;  
    return node;
}

Node* rightRotate(Node* y) {
    Node* x = y->left;
    Node* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max(height(y->left), height(y->right)) + 1;
    x->height = max(height(x->left), height(x->right)) + 1;

    return x;
}


Node* leftRotate(Node* x) {
    Node* y = x->right;
    Node* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max(height(x->left), height(x->right)) + 1;
    y->height = max(height(y->left), height(y->right)) + 1;

    return y;
}

bool index_find(long long int key, int *page, int *slot)
{
    Node* cur_node=avl_root;
    while(cur_node!=NULL)
    {
        if(key==cur_node->key){
            *page=cur_node->page_num;
            *slot=cur_node->row_num;
            return true;
        }
        else if(key<cur_node->key)
            cur_node=cur_node->left;
        else if(key>cur_node->key)
            cur_node=cur_node->right;
    }
    return false;
}


Node* index_delete(Node* root, long long key) {
    if (root==NULL){
        return NULL;
    }
    else if (root->key<key){
        root->right=index_delete(root->right, key); 
    }
    else if (root->key>key){
        root->left=index_delete(root->left,key);
    }
    else{
        if (root->left==NULL || root->right==NULL){
            if (root->left==NULL && root->right==NULL){
                free(root);
                return NULL;
            }
            else if (root->left==NULL){
                Node *temp = root->right;
                free(root);
                return temp;             
                   
            }
            else{
                Node *temp = root->left;
                free(root);
                return temp;
            }
        }
        Node* min=root->right;
        while(min->left!=NULL){
            min=min->left;
        }
        long long int minvalue = min->key;
        root->key=minvalue;    
        root->page_num = min->page_num;
        root->row_num = min->row_num;
    
        root->right=index_delete(root->right,min->key);
        if (height(root->right)>=height(root->left)){
            root->height=1+height(root->right);
        }
        else{
            root->height=1+ height(root->left);
        }
        int balance = getBalance(root);

        if (balance > 1 && getBalance(root->left) >= 0){
            return rightRotate(root);
        }        
        if (balance > 1 && getBalance(root->left) < 0) {
            root->left = leftRotate(root->left);
            return rightRotate(root);
        }

        if (balance < -1 && getBalance(root->right) <= 0){
            return leftRotate(root);
        }
        if (balance < -1 && getBalance(root->right) > 0) {
            root->right = rightRotate(root->right);
            return leftRotate(root);
        }
        
        }

        return root;
        
    }



Node* insertTree(Node* node, int key, int page, int slot){
    if(!node){
        return newNode(key, page, slot);
    }
    if (key<node->key){
        node->left= insertTree(node->left, key, page,slot);
    }
    else if(key > node->key){
        node->right= insertTree(node->right, key, page, slot);
    }
    else{
        return node;
    }
    node->height = 1+max(height(node->left), height(node->right));
    int balance = getBalance(node);

    if (balance > 1 && key < node->left->key) return rightRotate(node);
    if (balance < -1 && key > node->right->key) return leftRotate(node);
    if (balance > 1 && key > node->left->key) {
        node->left = leftRotate(node->left);
        return rightRotate(node);
    }
    if (balance < -1 && key < node->right->key) {
        node->right = rightRotate(node->right);
        return leftRotate(node);
    }

    return node;
}

void index_insert(long long key, int page, int slot) {
    avl_root = insertTree(avl_root, key, page, slot);
}