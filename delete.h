#include<stdio.h>
#include<stdlib.h>

#include "row.h"
#include "page.h"
#include "table.h"

void delete_row(Table* table, int id) 
{
    for(int i=0;i<table->num_pages;i++)
    {
        Page* page = table->pages[i];
        if (page != NULL) 
        {
            for (int j = 0; j < get_max_rows_per_page(); j++) 
            {
                Row* row = page->row_ptr[j];
                if (row != NULL && row->ID == id) 
                {
                    free(row); 
                    
                    page->row_ptr[j] = NULL; 
                    page->num_rows--; 

                    if(page->num_rows == 0) 
                    {
                        free(page); 
                        table->pages[i] = NULL; 
                        table->num_pages--; 
                    }

                    printf("Row with ID %d deleted successfully.\n", id);
                    return;
                }
            }
        }
    }
    printf("Row with ID %d not found.\n", id);
}