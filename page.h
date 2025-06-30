#include<stdlib.h>
#include"row.h"
#define size 4096
#define ele 60
//array of pointers to each row inserted in a single page
typedef struct page{
    Row* row_ptr[size/ele];
}Page;
