# Week 11: CSV Processing Jobs

**Days 51-55 | March 9-13, 2026**

Taking the file-based task queue from Week 10 and making it work with structured data. The system could handle text files, but what about CSVs? Spreadsheets? Databases exports? This week is about parsing CSV files and implementing useful operations: filtering, sorting, statistics.

Building a CSV parser from scratch taught me why libraries exist.

---

## What Changed From Week 10

**Week 10 system:**
- Workers process plain text files (wordcount, charcount, capitalize, echo)
- Simple byte-by-byte or word-by-word operations
- No structured data parsing

**Week 11 system:**
- Workers can parse CSV files into structured data (rows/columns)
- Three new job types: csvstats, csvfilter, csvsort
- Two-pass parsing: first pass counts dimensions, second pass allocates and populates
- Buffer boundary handling for chunked file reading
- Merge sort implementation for sorting

CSV files require actual parsing. Can't just count bytes or capitalize characters. Need to understand structure: rows, columns, quoted fields, commas inside quotes.

---

## What This Does

Same infrastructure as Week 10, but with CSV-specific jobs:

**New Job Types:**

- `csvstats` - Count rows and columns
  ```bash
  ./client submit csvstats employees.csv
  # Output: "145 total entries, 4 columns"
  ```

- `csvfilter [column] [value]` - Return rows where column equals value
  ```bash
  ./client submit "csvfilter City Portland" employees.csv
  # Output: All rows where City column equals "Portland"
  ```

- `csvsort [column]` - Sort CSV by column (lexicographic)
  ```bash
  ./client submit "csvsort Age" employees.csv
  # Output: CSV sorted by Age column (string comparison)
  ```

**Architecture:**
- Client sends CSV file + job specification
- Server routes to worker (same Week 10 infrastructure)
- Worker parses CSV into 3D array: `csv_data[row][col]` → string
- Worker executes job, writes results as CSV
- Client retrieves results file

**Old jobs still work:** wordcount, charcount, capitalize, echo all functional

# Compiling

client: `gcc client.c ./utils/buffer_manipulation.c ./utils/file_transfer.c ./utils/epoll_helper.c -o client`

server: `gcc server.c ./utils/workers.c ./utils/buffer_manipulation.c ./utils/time_custom.c ./utils/jobs.c ./utils/job_queue.c ./utils/file_transfer.c ./utils/epoll_helper.c -o server`

worker: `gcc `Wand-config --cflags --cppflags` worker.c ./utils/buffer_manipulation.c ./utils/job_processing.c ./utils/file_transfer.c ./utils/epoll_helper.c ./utils/csv/parse_csv.c -o worker `Wand-config --ldflags --libs``