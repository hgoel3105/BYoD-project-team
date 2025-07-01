#include <stdio.h>
#include <stdlib.h>
#include "table.h"

int main()
{
    Table* table= (Table*)malloc(sizeof(Table));
    if (table == NULL) {
        fprintf(stderr, "Memory allocation failed for table.\n");
        return EXIT_FAILURE;
    }
    table->num_pages = 0;
    for (int i = 0; i < max_pages; i++) {
        table->pages[i] = NULL;
    }
    int choice;
    while(1){
        printf("1. Insert a new Row\n");
        printf("2. Delete Row by id\n");
        printf("3. Scan Table for a specific entry by ID\n");
        printf("4. To print the table\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch(choice){
            case 1:
                // Function to insert a new row
                // insert_row();
                printf("Inserting a new row...\n");
                insert(table);
                break;
            case 2:
                // Function to delete row by ID
                // delete_row();
                printf("Deleting a row by ID...\n");
                int id_to_delete;
                printf("Enter ID of the row to delete: ");
                scanf("%d", &id_to_delete);
                delete_row(table, id_to_delete);
                break;
            case 3:
                // Function to scan the table
                // scan_table();
                printf("Scanning table for a specific entry by ID...\n");
                int id_to_scan;
                printf("Enter ID to scan: ");
                scanf("%d", &id_to_scan);
                if (scan_table(table, id_to_scan) == 0) {
                    printf("No entry found with ID %d.\n", id_to_scan);
                } else {
                    printf("Entry found with ID %d.\n", id_to_scan);
                }

                break;
            case 4:
                // Function to print the table
                // print_table();
                printf("Printing the table...\n");
                print_table(table);
                break;
            case 5:
                printf("Exiting\n");
                free(table);
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}