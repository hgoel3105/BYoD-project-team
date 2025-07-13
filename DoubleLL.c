#include"cache.h"
Double_Node* createnode(int page_id,Page* pg_address)
{
    Double_Node* node=(Double_Node*)malloc(sizeof(Double_Node));
    node->page_id=page_id;
    node->pg_address=pg_address;
    node->dirty=false;
    node->dirty_page=NULL;
    node->prev=NULL;
    node->next=NULL;
    return node;
}

void removenode(Double_Node* node) 
{
    node->prev->next=node->next;
    node->next->prev=node->prev;
}

void addnode(LRUCache* cache,Double_Node* node) 
{
    node->next=cache->head->next;
    node->prev=cache->head;
    cache->head->next->prev=node;
    cache->head->next=node;
}
