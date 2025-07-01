#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "row.h"
#include "page.h"
#include "table.h"

// remaining code - bitmap instead of linear search
// error handling in insert_row

int integer_input()
{
    char buffer[20];
    int value=0;
    if(fgets(buffer, sizeof(buffer), stdin) !=NULL )
    {
        buffer[strcspn(buffer, "\n")] = 0; 
        int buffer_size = strlen(buffer);
        for(int i = 0; i < buffer_size; i++)
        {
            if (buffer[i] < '0' || buffer[i] > '9') 
            {
                printf("Invalid input. Please enter an integer.\n");
                return -1; 
            }
            value = value * 10 + (buffer[i] - '0');
        }
    }
    else
    {
        printf("Error reading input.\n");
        return -1; 
    }

    return value;
}

int string_input(char* input,int max_length)
{
    if (fgets(input, max_length, stdin) != NULL)
    {
        input[strcspn(input, "\n")] = 0; 
        if (strlen(input) > max_length - 1)
        {
            printf("Input exceeds maximum length of %d characters.\n", max_length - 1);
            return -1; 
        }

        for (int i = 0; i < strlen(input); i++)
        {
            if (input[i] <= 'Z' || input[i] >= 'A' || input[i] <= 'z' || input[i] >= 'a') 
            {
                continue; 
            }
            else
            {
                printf("Invalid character in input. Please use alphabet characters only.\n");
                return -1; 
            }
        }
        return 0; 
    }
    else
    {
        printf("Error reading input.\n");
        return -1; 
    }
    return 0;
}

void insert_row(Row* input_row)
{
    int error = 0;
    char buffer[100];

    printf("Please provide integer type Student ID, and press enter");
    input_row->ID = integer_input();
    if (input_row->ID == -1)
    {
        error = 1; 
    }

    printf("Please provide Student name (maximum 19 character), and press enter");
    if (string_input(buffer,name_size) == -1)
    {
        error = 1; 
    }
    else
    {
        strncpy(input_row->name, buffer, name_size - 1);
        input_row->name[name_size - 1] = '\0'; 
    }

    printf("Please provide Student branch (maximum 3 character), and press enter");
    if (string_input(buffer, branch_size) == -1)
    {
        error = 1; 
    }
    else
    {
        strncpy(input_row->branch, buffer, branch_size - 1);
        input_row->branch[branch_size - 1] = '\0'; 
    }

    printf("Please provide Student city (maximum 11 character), and press enter");
    if (string_input(buffer, loc_size) == -1)
    {
        error = 1; 
    }
    else
    {
        strncpy(input_row->city, buffer, loc_size - 1);
        input_row->city[loc_size - 1] = '\0'; 
    }

    printf("Please provide Student marks in MTH, and press enter");
    input_row->MTH = integer_input();
    if (input_row->MTH == -1)
    {
        error = 1; 
    }

    printf("Please provide Student marks in PHY, and press enter");
    input_row->PHY = integer_input();
    if (input_row->PHY == -1)
    {
        error = 1; 
    }

    printf("Please provide Student marks in CHM, and press enter");
    input_row->CHM = integer_input();
    if (input_row->CHM == -1)
    {
        error = 1; 
    }

    printf("Please provide Student marks in TA, and press enter");
    input_row->TA = integer_input();
    if (input_row->TA == -1)
    {
        error = 1; 
    }

    printf("Please provide Student marks in LIF, and press enter");
    input_row->LIF = integer_input();
    if (input_row->LIF == -1)
    {
        error = 1; 
    }

    if (error == 1)
    {
        printf("Error: Invalid input. Please try again.\n");
        return;
    }
    printf("Row inserted successfully!\n");
}

void find_empty_row(Page* page)
{
    for (int i = 0; i < rows_per_page; i++)
    {
        if (page->row_ptr[i] == NULL) // Assuming NULL means empty row
        {
            printf("Found empty row at index %d\n", i);
            Row* new_row = (Row*)malloc(sizeof(Row));
            if (new_row == NULL) 
            {
                printf("Memory allocation failed for new row at index %d.\n", i);
                return;
            } 

            insert_row(new_row); // Insert data into the new row
            page->row_ptr[i] = new_row; 
            page->num_rows++; 
            return;
        }
    }
    printf("No empty row found in the page.\n");
}

void find_empty_page(Table* table)
{
    for (int i = 0; i < max_pages; i++)
    {
        if (table->pages[i] == NULL) // Assuming NULL means empty page
        {
            printf("Found empty page at index %d\n", i);
            table->pages[i] = (Page*)malloc(sizeof(Page)); 

            if (table->pages[i] == NULL) 
            {
                printf("Memory allocation failed for new page at index %d.\n", i);
                return;
            }

            table->pages[i]->num_rows = 0; 
            find_empty_row(table->pages[i]); 
            return;
        }

        int max_rows = rows_per_page; 

        if (table->pages[i]->num_rows < max_rows) 
        {
            printf("Found non-full page at index %d with space for more rows.\n", i);
            find_empty_row(table->pages[i]);

            return;
        }
    }

    printf("No empty page found in the table.\n");
}

void insert(Table* table)
{
    find_empty_page(table);
}