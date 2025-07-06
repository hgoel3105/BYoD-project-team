#include<stdio.h>
#include<stdlib.h>

#include "row.h"
#include "page.h"
#include "table.h"
#include "index.h"

int remove_bit(uint8_t *bitmap, int i);

void delete_row(Table* table, int id) 
{
    int page_num, row_num;
    if(!index_find(id, &page_num, &row_num)){
        printf("Row with ID %d not found.", id);
        return;
    }
    Page* page = table->pages[page_num];
    if(page == NULL || page->row_ptr[row_num]== NULL){
        printf("Row with ID %d not found in memory.", id);
        return;
    }
    free(page->row_ptr[row_num]);
    page->row_ptr[row_num]=NULL;

    if(remove_bit(page->bitmap, row_num)==-1){
        printf("Error removing bit from bitmap at index %d.\n", row_num);
        return;
    }
    page->num_rows--;
    if(page->num_rows==0){
        free(page);
        table->pages[page_num]=NULL;
        table->num_pages--;
    }
    avl_root=index_delete(avl_root,id);
    printf("Row with ID %d deleted successfully.\n", id);
}