# Week 11: CSV + Image Processing Jobs

**Days 51-55 | March 9-13, 2026**

Taking the file-based task queue from Week 10 and making it work with structured data and binary image jobs. The system could already handle plain text files, but what about CSVs? Spreadsheets? Exported datasets? And once that worked, the next question was: can the exact same worker pipeline also process JPGs using MagickWand?

Building a CSV parser from scratch taught me why libraries exist. Adding image jobs on top of that taught me how quickly a "simple job worker" turns into a protocol and file-handling problem.

---

## What Changed From Week 10

**Week 10 system:**
- Workers process plain text files (wordcount, charcount, capitalize, echo)
- Simple byte-by-byte or word-by-word operations
- No structured data parsing

**Week 11 system:**
- Workers can parse CSV files into structured data (rows/columns)
- Workers can also process JPG files through MagickWand
- CSV job types: csvstats, csvfilter, csvsort
- Image job types: scale, resize, flipx, flipy, rotate, charcoal_filter, grayscale_filter, stencil_filter
- Two-pass CSV parsing: first pass counts dimensions, second pass allocates and populates
- Buffer boundary handling for chunked file reading
- Merge sort implementation for CSV sorting
- File-type aware result delivery so the client can receive either `.txt` or `.jpg` results

CSV files require actual parsing. Can't just count bytes or capitalize characters. Need to understand structure: rows, columns, quoted fields, commas inside quotes.

Image jobs introduced a different class of problems. Instead of parsing structured text, the worker has to decode a binary file, apply a transform, write a new image, and make sure the server/client protocol knows whether the result coming back is text or binary.

---

## What This Does

Same infrastructure as Week 10, but now supporting both structured text jobs and image transforms.

**CSV Job Types:**

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

**Image Job Types:**

- `scale [factor]` - Resize image proportionally using a scale factor
  ```bash
  ./client submit "scale 0.5" ./client_storage/space.jpg
  # Output: JPG result at half the original width/height
  ```

- `resize [width]x[height]` - Resize to exact dimensions
  ```bash
  ./client submit "resize 300x300" ./client_storage/space.jpg
  # Output: JPG resized to 300x300
  ```

- `flipx` - Horizontal flip
  ```bash
  ./client submit "flipx" ./client_storage/space.jpg
  ```

- `flipy` - Vertical flip
  ```bash
  ./client submit "flipy" ./client_storage/space.jpg
  ```

- `rotate [degrees]` - Rotate image by a degree amount
  ```bash
  ./client submit "rotate 90" ./client_storage/space.jpg
  ```

- `charcoal_filter [radius] [sigma]` - Apply charcoal effect
  ```bash
  ./client submit "charcoal_filter 1.5 0.5" ./client_storage/space.jpg
  ```

- `grayscale_filter` - Convert image to grayscale
  ```bash
  ./client submit "grayscale_filter" ./client_storage/space.jpg
  ```

- `stencil_filter` - Edge-detect + threshold into a stencil-like output
  ```bash
  ./client submit "stencil_filter" ./client_storage/space.jpg
  ```

**Architecture:**
- Client sends file + job specification
- Server routes the job to a worker (same queueing/scheduling system as Week 10)
- Worker determines whether the job is text, CSV, or image-based
- CSV jobs parse data into `csv_data[row][col]`
- Image jobs load JPGs through MagickWand and write a new JPG to worker storage
- Server returns either a message or a file transfer packet
- Client receives results as either `results.txt` or `results.jpg`

**Old jobs still work:** wordcount, charcount, capitalize, echo are still functional.

**Current note:** there is still a generic `filter` image hook in the code path, but the documented image operations above are the ones with real behavior implemented right now.

# Compiling

## client: `gcc client.c ./utils/buffer_manipulation.c ./utils/file_transfer.c ./utils/epoll_helper.c -o client`

### ex usage: 

`./client submit "scale 0.5" "./client_storage/space.jpg"`

## server: `gcc server.c ./utils/workers.c ./utils/buffer_manipulation.c ./utils/time_custom.c ./utils/jobs.c ./utils/job_queue.c ./utils/file_transfer.c ./utils/epoll_helper.c -o server`

### ex usage: 

`./server`

## worker: `gcc $(pkg-config --cflags MagickCore MagickWand) worker.c ./utils/buffer_manipulation.c ./utils/job_processing.c ./utils/file_transfer.c ./utils/epoll_helper.c ./utils/csv/parse_csv.c -o worker $(pkg-config --libs MagickCore MagickWand)`

Requires ImageMagick / MagickWand development headers and libraries to be installed so `pkg-config` can resolve both include paths and linker flags.

### ex usage: 

`./worker`