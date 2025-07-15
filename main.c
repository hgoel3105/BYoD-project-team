#include <stdio.h>
#include <stdlib.h>
#include "table.h"
#include "wal.h"

void flush_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void free_table(Table* table) {
    if (table == NULL) return;

    for (int i = 0; i < max_pages; i++) {
        Page* page = table->pages[i];
        if (page != NULL) {
            free(page);
        }
    }
    free(table);
}

int main()
{
    Table* table= (Table*)malloc(sizeof(Table));
    if (table == NULL) {
        fprintf(stderr, "Memory allocation failed for table.\n");
        return EXIT_FAILURE;
    }
    
    // Initialize disk-based storage
    if (!table_open(table, "student_data.db", "student_index.db")) {
        fprintf(stderr, "Failed to open table files.\n");
        free(table);
        return EXIT_FAILURE;
    }
    wal_recover(table);
    printf("Database opened successfully.\n");
    printf("Data file: student_data.db\n");
    printf("Index file: student_index.db\n");
    printf("Page cache initialized with %d slots.\n", CACHE_SIZE);
    int choice;
    while(1){
        printf("\n1. Insert a new Row\n");
        printf("2. Delete Row by id\n");
        printf("3. Scan Table for a specific entry by ID\n");
        printf("4. To print the table\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        flush_stdin();
        
        switch(choice){
            case 1:
                printf("Inserting a new row...\n");
                insert(table);
                break;
            case 2:
                printf("Deleting a row by ID...\n");
                int id_to_delete;
                printf("Enter ID of the row to delete: ");
                scanf("%d", &id_to_delete);
                flush_stdin();
                delete_row(table, id_to_delete);
                break;
            case 3:
                printf("Scanning table for a specific entry by ID...\n");
                int id_to_scan;
                printf("Enter ID to scan: ");
                scanf("%d", &id_to_scan);
                flush_stdin();
                if (scan_table(table, id_to_scan) == 0) {
                    printf("No entry found with ID %d.\n", id_to_scan);
                } else {
                    printf("Entry found with ID %d.\n", id_to_scan);
                }

                break;
            case 4:
                printf("Printing the table...\n");
                print_table(table);
                break;
            case 5:
                printf("Syncing data to disk...\n");
                table_sync(table);
                printf("Closing database...\n");
                table_close(table);
                // Print cache statistics
                printf("Exiting\n");
                free(table);
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }
    return 0;
}