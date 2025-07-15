#include <stdio.h>

#include "wal.h"
static int global_lsn = 0;
int current_tx_id=-1;

void log_write(int tx_id, int page_num, Page* before, Page* after) {
    WALRecord record;
    record.tx_id = tx_id;
    record.page_num = page_num;
    record.before = *before;
    record.after = *after;
    record.lsn = global_lsn++;
    record.committed = false;

    FILE* f = fopen(WAL_FILE, "ab");
    if (!f) {
        perror("Failed to open WAL file for writing");
        return;
    }
    fwrite(&record, sizeof(WALRecord), 1, f);
    fclose(f);
}


void wal_recover(Table* table) {
    FILE* f = fopen(WAL_FILE, "r+b");
    if (!f) {
        f = fopen(WAL_FILE, "w+b");
        if (!f) {
            perror("Failed to open WAL file for recovery");
            return;
        }
        fclose(f);
        return; // No WAL file exists, nothing to recover
    }
    WALRecord record;
    while (fread(&record, sizeof(WALRecord), 1, f) == 1) {
        Page* page_recover = record.committed ? &record.after : &record.before;
        if(!table_write_page(table, record.page_num, page_recover)) {
            printf("Failed to write page %d during recovery\n", record.page_num);
            fclose(f);
            return;

        }
    }
    fclose(f);
}

bool was_committed(int tx_id) {
    FILE* f = fopen(WAL_FILE, "rb");
    if (!f) {
        perror("Failed to open WAL file for checking commit status");
        return false;
    }
    WALRecord record;
    while (fread(&record, sizeof(WALRecord), 1, f) == 1) {
        if (record.tx_id == tx_id) {
            fclose(f);
            return record.committed;
        }
    }
    fclose(f);
    return false;
}

void mark_commit(int tx_id) {
    FILE* f = fopen(WAL_FILE, "r+b");
    if (!f) {
        perror("Failed to open WAL file for marking commit");
        return;
    }
    WALRecord record;
    while (fread(&record, sizeof(WALRecord), 1, f) == 1) {
        if (record.tx_id == tx_id) {
            record.committed = true;
            fseek(f, -(long)sizeof(WALRecord), SEEK_CUR);
            fwrite(&record, sizeof(WALRecord), 1, f);
            fflush(f);
            fclose(f);
            return;
        }
    }
    fclose(f);
    printf("Transaction ID %d not found in WAL for commit marking\n", tx_id);
}

void tx_begin(){
    static int next_tx_id = 1;
    current_tx_id = next_tx_id++;
    printf("Transaction %d started\n", current_tx_id);
}

void tx_commit() {
    if (current_tx_id < 0) {
        printf("No active transaction to commit\n");
        return;
    }
    mark_commit(current_tx_id);
    printf("Transaction %d committed\n", current_tx_id);
    current_tx_id = -1; // Reset current transaction ID
}

void tx_abort(Table* table) {
    if (current_tx_id < 0) {
        printf("No active transaction to abort\n");
        return;
    }

    FILE* f = fopen(WAL_FILE, "rb");
    if (!f) {
        perror("Failed to open WAL file for aborting transaction");
        return;
    }
    WALRecord record;
    while (fread(&record, sizeof(WALRecord), 1, f) == 1) {
        if (record.tx_id == current_tx_id) {
            Page* page = &record.before;
            if (!table_write_page(table, record.page_num, page)) {
                printf("Failed to write page %d during transaction abort\n", record.page_num);
            }
        }
    }
    fclose(f);
    printf("Transaction %d aborted\n", current_tx_id);
    current_tx_id = -1; // Reset current transaction ID
}