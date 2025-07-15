#ifndef WAL_H
#define WAL_H
#include "table.h"
#include <stdbool.h>

#define WAL_SIZE 4096 // Size of the Write-Ahead Log (WAL) in bytes
#define WAL_FILE "wal.log" // Name of the WAL file

typedef struct {
    int tx_id;
    int page_num;
    Page before; // Data before the change
    Page after;  // Data after the change
    int lsn;
    bool committed; // Indicates if the transaction is committed
} WALRecord;

void log_write(int tx_id, int page_num, Page* before, Page* after);
void wal_recover(Table* table);
bool was_committed(int tx_id);
void mark_commit(int tx_id);


void tx_commit();
void tx_abort(Table* table);
void tx_begin();

extern int current_tx_id; // Current transaction ID
#endif