#include <stdio.h>
#include <stdlib.h>
#include "table.h"

int main(){
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
        printf("3. Scan Table\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch(choice){
            case 1:
                // Function to insert a new row
                // insert_row();
                break;
            case 2:
                // Function to delete row by ID
                // delete_row();
                break;
            case 3:
                // Function to scan the table
                // scan_table();
                break;
            case 4:
                printf("Exiting\n");
                free(table);
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}