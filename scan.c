#include <stdio.h>
#include <stdlib.h>

#include "row.h"
#include "page.h"
#include "table.h"
#include "index.h"

int scan_table(Table* table, int ID) 
{
    int page_num, row_num;

    if (!index_find(ID, &page_num, &row_num)) {
        printf("Row with ID %d not found in index.\n", ID);
        return 0;
    }

    Page* page = table->pages[page_num];
    if (page == NULL || page->row_ptr[row_num] == NULL) {
        printf("Row with ID %d not found in memory.\n", ID);
        return 0;
    }

    Row* row = page->row_ptr[row_num];
    printf("Row Found: ID=%d, Name=%s, Branch=%s, City=%s, MTH=%d, PHY=%d, CHM=%d, TA=%d, LIF=%d\n", 
           row->ID, row->name, row->branch, row->city, 
           row->MTH, row->PHY, row->CHM, row->TA, row->LIF);

    return 1;
}