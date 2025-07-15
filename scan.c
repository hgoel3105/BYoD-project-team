#include <stdio.h>
#include <stdlib.h>

#include "row.h"
#include "page.h"
#include "table.h"
#include "btree.h"

int check_bit(uint8_t *bitmap, int i);

int scan_table(Table* table, int ID) 
{
    uint32_t page_num;
    uint16_t row_num;

    if (!btree_find(&table->btree_meta, ID, &page_num, &row_num)) {
        printf("Row with ID %d not found in index.\n", ID);
        return 0;
    }

    // Load page if not in memory
    Page* page = table_get_page(table, page_num);
    if (page == NULL || check_bit(page->bitmap, row_num) == 0) {
        printf("Row with ID %d not found in memory.\n", ID);
        return 0;
    }

    Row* row = &page->rows[row_num];
    printf("Row Found: ID=%d, Name=%s, Branch=%s, City=%s, MTH=%d, PHY=%d, CHM=%d, TA=%d, LIF=%d\n", 
           row->ID, row->name, row->branch, row->city, 
           row->MTH, row->PHY, row->CHM, row->TA, row->LIF);

    return 1;
}
