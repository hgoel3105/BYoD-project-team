#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "row.h"
#include "page.h"
#include "table.h"
#include "btree.h"
#include "wal.h"

int remove_bit(uint8_t *bitmap, int i);
int check_bit(uint8_t *bitmap, int i);

void delete_row(Table* table, int id) 
{
    tx_begin();
    uint32_t page_num;
    uint16_t row_num;
    if(!btree_find(&table->btree_meta, id, &page_num, &row_num)){
        printf("Row with ID %d not found.\n", id);
        tx_abort(table);
        return;
    }
    
    // Load page if not in memory
    Page* page = table_get_page(table, page_num);
    if(page == NULL || check_bit(page->bitmap, row_num) == 0){
        printf("Row with ID %d not found in memory.\n", id);
        tx_abort(table);
        return;
    }
    Page before, after;
    memcpy(&before, page, sizeof(Page));

    // Clear the row data
    memset(&page->rows[row_num], 0, sizeof(Row));

    if(remove_bit(page->bitmap, row_num) == -1){
        printf("Error removing bit from bitmap at index %d.\n", row_num);
        tx_abort(table);
        return;
    }
    
    page->num_rows--;
    
    memcpy(&after, page, sizeof(Page));
    log_write(current_tx_id, page_num, &before, &after);
    printf("Written log for transaction id %d\n", current_tx_id);

    // Write updated page back to disk
    table_write_page(table, page_num, page);
    
    // Delete from B-Tree index
    if (!btree_delete(&table->btree_meta, id)) {
        printf("Warning: Failed to delete from B-Tree index.\n");
        tx_abort(table);
    }
    
    printf("Row with ID %d deleted successfully.\n", id);
    tx_commit();
}
