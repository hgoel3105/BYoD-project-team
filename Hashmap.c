#include"cache.h"
int hash(int key) 
{
    return(key%HASH_SIZE + HASH_SIZE)%HASH_SIZE;
}

void add_map(LRUCache* cache,int page_id,Double_Node* node)
{
    int h=hash(page_id);
    Entry* entry=(Entry*)malloc(sizeof(Entry));
    entry->key=page_id;
    entry->node=node;
    entry->next=cache->hashMap[h];
    cache->hashMap[h]=entry;
}

Double_Node* get_map(LRUCache* cache,int page_id) 
{
    int h=hash(page_id);
    Entry* entry=cache->hashMap[h];
    while(entry) 
    {
        if (entry->key==page_id)
            return entry->node;
        entry=entry->next;
    }
    return NULL;
}

void remove_map(LRUCache* cache,int page_id) 
{
    int h=hash(page_id);
    Entry* entry=cache->hashMap[h];
    Entry* prev=NULL;
    while (entry) 
    {
        if (entry->key==page_id) 
        {
            if(prev)
                prev->next=entry->next;
            else 
                cache->hashMap[h]=entry->next;
            free(entry);
            return;
        }
        prev=entry;
        entry=entry->next;
    }
}