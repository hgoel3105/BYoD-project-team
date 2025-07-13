#include"table.h"
#include<stdbool.h>
#define HASH_SIZE 10007

typedef struct Double_Node{
    int page_id;
    bool dirty;
    Page* pg_address;
    Page* dirty_page;
    struct Double_Node* prev;
    struct Double_Node* next;
}Double_Node;

typedef struct Entry{
    int key;
    Double_Node* node;
    struct Entry* next;
}Entry;

typedef struct LRUCache{
    int capacity;
    int sz;
    Entry* hashMap[HASH_SIZE]; 
    Double_Node* head; 
    Double_Node* tail;
}LRUCache;

Double_Node* createnode(int page_id,Page* pg_address);
void removenode(Double_Node* node);
void addnode(LRUCache* cache,Double_Node* node);
int hash(int key);
void add_map(LRUCache* cache,int page_id,Double_Node* node);
Double_Node* get_map(LRUCache* cache,int page_id);
void remove_map(LRUCache* cache,int page_id);
LRUCache* create(int capacity);
Double_Node* cache_get(LRUCache* cache,int page_id);
void cache_put(LRUCache* cache,int page_id,Page* pg_address);
void cache_free(LRUCache* cache);