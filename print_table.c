#include<stdio.h>
#include<stdlib.h>

#include "row.h"
#include "page.h"
#include "table.h"

void print_table(Table* table) 
{
    //printf("Printing full table:\n");

    if (table->num_pages == 0) {
        printf("Table is empty.\n");
        return;
    }

    for (int i = 0; i < table->num_pages; i++) 
    {
        Page* page = table->pages[i];
        if (page != NULL) 
        {
            printf("Page %d:\n", i + 1);
            for (int j = 0; j < rows_per_page; j++) 
            {
                Row* row = page->row_ptr[j];
                if (row != NULL) 
                {
                    printf("  Row %d: ID=%d, Name=%s, Branch=%s, City=%s, MTH=%d, PHY=%d, CHM=%d, TA=%d, LIF=%d\n",
                           j + 1, row->ID, row->name, row->branch, row->city,
                           row->MTH, row->PHY, row->CHM, row->TA, row->LIF);
                }
                else
                {
                    printf("  Row %d: [empty]\n", j + 1);
                }
            }
        } 
        else 
        {
            printf("Page %d: [uninitialized]\n", i + 1);
        }
    }
}
