#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

#include "row.h"
#include "page.h"
#include "table.h"

// Bitmap function declaration
int check_bit(uint8_t *bitmap, int i);

void print_table(Table* table) 
{
    // printf("Printing full table:\n");

    bool is_empty = true;

    for (int i = 0; i < max_pages; i++)
    {
        Page* page = table->pages[i];
        
        // If page is not in memory, try to load it from disk
        if (page == NULL) {
            page = table_get_page(table, i);
        }
        
        if (page != NULL && page->num_rows > 0) 
        {
            printf("Page %d:\n", i + 1);
            for (int j = 0; j < rows_per_page; j++) 
            {
                // Check if this slot has a valid row (using bitmap)
                if (check_bit(page->bitmap, j) > 0) 
                {
                    Row* row = &page->rows[j];
                    printf("  Row %d: ID=%d, Name=%s, Branch=%s, City=%s, MTH=%d, PHY=%d, CHM=%d, TA=%d, LIF=%d\n",
                           j + 1, row->ID, row->name, row->branch, row->city,
                           row->MTH, row->PHY, row->CHM, row->TA, row->LIF);
                    is_empty = false;
                }
            }
        } 
    }

    if (is_empty) {
        printf("Table is empty.\n");
    }
}
