#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "row.h"
#include "page.h"
#include "table.h"
#include "btree.h"
#include "wal.h"

int set_bit(uint8_t *bitmap, int i);
int check_bit(uint8_t *bitmap, int i);

int integer_input()
{
    char buffer[20];
    int value = 0;

    if (fgets(buffer, sizeof(buffer), stdin) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = 0;

        if (strlen(buffer) == 0)
        {
            printf("Empty input is not allowed.\n");
            return -1;
        }

        // Check for invalid characters
        for (int i = 0; i < strlen(buffer); i++)
        {
            if (buffer[i] < '0' || buffer[i] > '9')
            {
                printf("Invalid input. Please enter an integer.\n");
                return -1;
            }
        }

        value = atoi(buffer);
        return value;
    }
    else
    {
        printf("Error reading input.\n");

        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
        return -1;
    }
}

int string_input(char *input, int max_length)
{
    if (fgets(input, max_length, stdin) != NULL)
    {
        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0)
        {
            printf("Input cannot be empty.\n");
            return -1;
        }

        for (int i = 0; i < strlen(input); i++)
        {
            char c = input[i];
            if (!((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == ' '))
            {
                printf("Invalid character in input. Use letters and spaces only.\n");
                return -1;
            }
        }

        return 0;
    }
    else
    {
        printf("Error reading input.\n");

        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF)
            ;
        return -1;
    }
}

void safe_copy(char *dest, const char *src, size_t max_size)
{
    strncpy(dest, src, max_size - 1);
    dest[max_size - 1] = '\0';
}

int insert_row(Row *input_row)
{
    int error = 0;
    char buffer[100];

    printf("Enter Student ID (integer): ");
    input_row->ID = integer_input();
    if (input_row->ID == -1)
        error = 1;

    printf("Enter Student name (max 19 characters): ");
    if (string_input(buffer, name_size) == -1)
    {
        error = 1;
        goto error;
    }
    else
    {
        safe_copy(input_row->name, buffer, name_size);
    }

    printf("Enter Student branch (max 3 characters): ");
    if (string_input(buffer, branch_size + 1) == -1)
    {
        error = 1;
        goto error;
    }
    else
    {
        safe_copy(input_row->branch, buffer, branch_size);
    }

    printf("Enter Student city (max 11 characters): ");
    if (string_input(buffer, loc_size) == -1)
    {
        error = 1;
        goto error;
    }
    else
    {
        safe_copy(input_row->city, buffer, loc_size);
    }

    printf("Enter MTH marks: ");
    input_row->MTH = integer_input();
    if (input_row->MTH == -1)
    {
        error = 1;
        goto error;
    }

    printf("Enter PHY marks: ");
    input_row->PHY = integer_input();
    if (input_row->PHY == -1)
    {
        error = 1;
        goto error;
    }

    printf("Enter CHM marks: ");
    input_row->CHM = integer_input();
    if (input_row->CHM == -1)
    {
        error = 1;
        goto error;
    }

    printf("Enter TA marks: ");
    input_row->TA = integer_input();
    if (input_row->TA == -1)
    {
        error = 1;
        goto error;
    }

    printf("Enter LIF marks: ");
    input_row->LIF = integer_input();
    if (input_row->LIF == -1)
    {
        error = 1;
        goto error;
    }

error:
    if (error)
    {
        printf("Error: Invalid input. Row insertion failed.\n");
        return -1;
    }

    printf("Row inserted successfully!\n");
    return 0;
}

bool find_empty_row(Table *table, Page *page, int page_num)
{
    for (int i=0;i<rows_per_page;i++)
    {
        int bit_status = check_bit(page->bitmap, i);
        if(bit_status == 0)
        {
            Row *new_row = (Row *)malloc(sizeof(Row));
            if (!new_row)
            {
                printf("Memory allocation failed for new row.\n");
                return false;
            }

            if (insert_row(new_row) == -1)
            {
                free(new_row);
                return false;
            }

            // Check if ID already exists in B-Tree
            uint32_t existing_page;
            uint16_t existing_slot;
            if (btree_find(&table->btree_meta, new_row->ID, &existing_page, &existing_slot)) {
                printf("Error: ID %d already exists in database.\n", new_row->ID);
                free(new_row);
                return false;
            }
            Page before, after;
            memcpy(&before, page, sizeof(Page));
            
            // Insert into B-Tree index first
            if (!btree_insert(&table->btree_meta, new_row->ID, page_num, i)) {
                printf("Error: Failed to insert into B-Tree index.\n");
                free(new_row);
                return false;
            }
            
            // Only after successful B-Tree insertion, update the page
            page->rows[i] = *new_row;  // Copy the actual row data
            if(set_bit(page->bitmap, i) == -1)
            {
                printf("Error setting bit in bitmap.\n");
                // Rollback B-Tree insertion
                btree_delete(&table->btree_meta, new_row->ID);
                memset(&page->rows[i], 0, sizeof(Row));  // Clear the row data
                free(new_row);
                return false;
            }
            free(new_row);  // Free the temporary row since we copied the data
            page->num_rows++;
            memcpy(&after, page, sizeof(Page));
            
            // Log the change in WAL
            log_write(current_tx_id, page_num, &before, &after);
            printf("Written log for transaction id %d\n", current_tx_id);
            
            // Save page to disk immediately
            table_write_page(table, page_num, page);
            
            return true;
        }
        else if (bit_status == -1)
        {
            printf("Error checking bitmap at index %d.\n", i);
            return false;
        }
    }
    // for (int i = 0; i < rows_per_page; i++)
    // {
    //     if (page->row_ptr[i] == NULL)
    //     {

    //         Row *new_row = (Row *)malloc(sizeof(Row));
    //         if (!new_row)
    //         {
    //             printf("Memory allocation failed for new row.\n");
    //             return;
    //         }

    //         if (insert_row(new_row) == -1)
    //         {
    //             free(new_row);
    //             return;
    //         }

    //         page->row_ptr[i] = new_row;
    //         page->num_rows++;
    //         return;
    //     }
    // }
    // printf("No empty row found in the page.\n");
    printf("No empty row found in the page.\n");
    return false;
}

bool find_empty_page(Table *table)
{
    for (int i = 0; i < max_pages; i++)
    {
        if (table->pages[i] == NULL)
        {
            // printf("Found empty page at index %d\n", i);
            table->pages[i] = (Page *)malloc(sizeof(Page));
            if (!table->pages[i])
            {
                printf("Memory allocation failed for new page.\n");
                return false;
            }

            table->pages[i]->num_rows = 0;
            // Initialize bitmap to all zeros
            memset(table->pages[i]->bitmap, 0, bitmap_size);
            // Initialize all row data to zero
            memset(table->pages[i]->rows, 0, sizeof(Row) * rows_per_page);
            table->num_pages++;

            return find_empty_row(table, table->pages[i], i);
        }

        if (table->pages[i]->num_rows < rows_per_page)
        {
            return find_empty_row(table, table->pages[i], i);
        }
    }

    printf("No empty page found in the table.\n");
    return false;
}

void insert(Table *table)
{
    tx_begin();
    bool status = find_empty_page(table);
    if(!status){
        tx_abort(table);
        return;
    }
    tx_commit();
}
