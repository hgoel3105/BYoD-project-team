# Row Store Database Engine

A simple in-memory table-based database engine implemented in C, supporting basic operations such as insert, delete, scan, and print. This project demonstrates fundamental database concepts and provides a minimal row-store architecture.

## Features
- **Insert**: Add new rows to the table.
- **Delete**: Remove rows from the table based on criteria.
- **Scan**: Iterate and retrieve rows from the table.
- **Print**: Display the contents of the table in a readable format.

## Directory Structure
```
assignment_2/BYoD-project-team/
├── delete.c / delete.h      # Delete operation implementation
├── insert.c / insert.h      # Insert operation implementation
├── scan.c / scan.h          # Scan operation implementation
├── print_table.c / print_table.h # Print operation implementation
├── main.c                   # Entry point and CLI
├── page.h                   # Page structure definition
├── row.h                    # Row structure definition
├── table.h                  # Table structure definition
├── Makefile                 # Build instructions
├── README.md                # Project documentation
```

## Prerequisites
- GCC or any compatible C compiler
- Make (for building the project)

## Build Instructions
1. Open a terminal and navigate to the `assignment_2/BYoD-project-team/` directory.
2. Run the following command to build the project:
   
   ```sh
   make
   ```

   This will compile all source files and generate the executable as specified in the `Makefile`.

## Usage
After building, run the executable as follows:

```sh
.\row_store.exe
```

Follow the on-screen prompts to perform insert, delete, scan, and print operations on the in-memory table.

## File Descriptions
- **main.c**: Handles user interaction and calls the appropriate database operations.
- **insert.c/h**: Functions for inserting rows into the table.
- **delete.c/h**: Functions for deleting rows from the table.
- **scan.c/h**: Functions for scanning and retrieving rows.
- **print_table.c/h**: Functions for printing the table contents.
- **row.h, page.h, table.h**: Data structure definitions for rows, pages, and tables.
