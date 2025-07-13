#include"cache.h"
LRUCache* create(int capacity) 
{
    LRUCache* cache=(LRUCache*)malloc(sizeof(LRUCache));
    cache->capacity=capacity;
    cache->sz=0;
    for (int i=0;i<HASH_SIZE;i++)
        cache->hashMap[i]=NULL;

    cache->head=createnode(-1,NULL);
    cache->tail=createnode(-1,NULL);
    cache->head->next=cache->tail;
    cache->tail->prev=cache->head;
    return cache;
}

Double_Node* cache_get(LRUCache* cache,int page_id) 
{
    Double_Node* node=get_map(cache,page_id);
    if (!node)
        return NULL;
    removenode(node);
    addnode(cache,node);
    return node;
}

void cache_put(LRUCache* cache,int page_id,Page* pg_address)
{
    if(cache->sz==cache->capacity) 
    {
        Double_Node* lru=cache->tail->prev;
        removenode(lru);
        remove_map(cache,lru->page_id);
        free(lru);
        cache->sz--;
    }
    Double_Node* newNode=createnode(page_id,pg_address);
    addnode(cache,newNode);
    add_map(cache,page_id,newNode);
    cache->sz++;
}

void cache_free(LRUCache* cache)
{
    Double_Node* curr=cache->head;
    while(curr) 
    {
        Double_Node* temp=curr;
        curr=curr->next;
        free(temp);
    }
    for (int i=0;i<HASH_SIZE;i++) 
    {
        Entry* entry=cache->hashMap[i];
        while(entry) 
        {
            Entry* temp=entry;
            entry=entry->next;
            free(temp);
        }
    }
    free(cache);
}