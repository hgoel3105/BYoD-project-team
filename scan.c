#include <stdio.h>
#include <stdlib.h>

#include "row.h"
#include "page.h"
#include "table.h"

int scan_page(Page* page, int ID) 
{
    //printf("Scanning page with %d rows:\n", page->num_rows);
    for (int i = 0; i < rows_per_page; i++) 
    {
        Row* row = page->row_ptr[i];
        if (row != NULL && row->ID == ID) 
        {
            printf("Row %d: ID=%d, Name=%s, Branch=%s, City=%s, MTH=%d, PHY=%d, CHM=%d, TA=%d, LIF=%d\n", 
                   i + 1, row->ID, row->name, row->branch, row->city, 
                   row->MTH, row->PHY, row->CHM, row->TA, row->LIF);
            return 1; 
        }
    }
    return 0; // Not found in this page
}

int scan_table(Table* table, int ID) 
{
    //printf("Scanning table with %d pages:\n", table->num_pages);
    for (int i = 0; i < table->num_pages; i++) 
    {
        Page* page = table->pages[i];
        if (page != NULL) 
        {
            //printf("Page %d:\n", i + 1);
            if (scan_page(page, ID))
                return 1; // found
        } 
        else 
        {
            printf("Page %d is empty.\n", i + 1);
        }
    }
    //printf("No row found with ID %d in the table.\n", ID);
    return 0;
}
