/*
 * job_processing.c -- File-based job execution with streaming chunk processing
 */

#include "./job_processing.h"

/*
 * determine_job_type() -- parse job metadata to identify job type
 *
 * Extracts the first word from the buffer (up to space), moves remaining
 * content to start of buffer, and returns the corresponding JTYPE constant
 */
int determine_job_type(unsigned char buf[MAXBUFSIZE], int size){
    char keyword[size];
    int i = 0;
    int type = -1;

    for (; i < size; i++){
        if (buf[i] == ' '){
            break;
        }
    }

    strncpy(keyword, (char *)buf, i);
    keyword[i] = '\0';

    i++;
    memmove(buf, buf+i, MAXBUFSIZE-i);

    if (strlen(keyword) == 0){
        printf("huh\n");
        return type;
    }

    if (strcmp(keyword, "wordcount") == 0){
        type =  JTYPE_WORDCOUNT;
    }

    if (strcmp(keyword, "echo") == 0){
        type = JTYPE_ECHO;
    }

    if (strcmp(keyword, "capitalize") == 0){
        type = JTYPE_CAPITALIZE;
    }

    if (strcmp(keyword, "charcount") == 0){
        type = JTYPE_CHARCOUNT;
    }

    if (strcmp(keyword, "csvsort") == 0){
        type = JTYPE_CSVSORT;
    }

    if (strcmp(keyword, "csvstats") == 0){
        type = JTYPE_CSVSTATS;
    }

    if (strcmp(keyword, "csvfilter") == 0){
        type = JTYPE_CSVFILTER;
    }

    if (strcmp(keyword, "scale") == 0){
        type = JTYPE_SCALE;
    }

    if (strcmp(keyword, "resize") == 0){
        type = JTYPE_RESIZE;
    }

    if (strcmp(keyword, "filter") == 0){
        type = JTYPE_FILTER;
    }

    if (strcmp(keyword, "flipx") == 0){
        type = JTYPE_FLIPX;
    }

    if (strcmp(keyword, "flipy") == 0){
        type = JTYPE_FLIPY;
    }

    if (strcmp(keyword, "rotate") == 0){
        type = JTYPE_ROTATE;
    }

    if (strcmp(keyword, "charcoal_filter") == 0){
        type = JTYPE_CHARCOAL;
    }

    if (strcmp(keyword, "grayscale_filter") == 0){
        type = JTYPE_MONOCHROME;
    }

    if (strcmp(keyword, "stencil_filter") == 0){
        type = JTYPE_STENCIL;
    }

    return type;
}

/*
 * strip_whitespace() -- remove leading spaces from string in-place
 *
 * Uses memmove (not strcpy) because source and dest overlap.
 * Example: "   hello" → "hello"
 */
void strip_whitespace(char *string){
    int len = strlen(string);
    int i = 0;

    for (; i < len; i++){
        if (string[i] != ' ') break;
    }

    memmove(string, string+i, len-i);
}

/*
 * extract_first_word() -- parse first space-delimited word from string
 *
 * Copies first word to dest, moves remainder to start of orig, strips leading spaces.
 * Example: orig="City Portland" → dest="City", orig="Portland"
 */
void extract_first_word(char *dest, char *orig){
    int j = 0;
    for (; j < strlen(orig); j++){
        if (orig[j] == ' ') break;
    }
    strncpy(dest, orig, j);
    dest[j] = '\0';

    memmove(orig, orig+j, MAXBUFSIZE-j);
    strip_whitespace(orig);
}

/*
 * job_wordcount() -- count words in content string
 *
 * Words are defined as sequences of non-space characters separated by spaces
 */
int job_wordcount(FILE *results, FILE *content){
    char content_read[MAXFILEREAD];
    int bytes_read;
    int wordcount = 0;
    int seen_space = 1;

    while ((bytes_read = fread(content_read, sizeof(char), MAXFILEREAD, content)) > 0){

        for (int i = 0; i < bytes_read; i++){
            if (content_read[i] == ' ') seen_space = 1;
            else if (seen_space == 1) {
                wordcount++;
                seen_space = 0;
            }
        }
    }

    fprintf(results, "%d total words", wordcount);
    return 1;
}

/*
 * job_charcount() -- count non-space characters in content string
 */
int job_charcount(FILE *results, FILE *content){
    char content_read[MAXFILEREAD];
    int bytes_read;
    int charcount = 0;

    while ((bytes_read = fread(content_read, sizeof(char), MAXFILEREAD, content)) > 0){
        for (int i = 0; i < bytes_read; i++){
            if (content_read[i] != ' ') charcount++;
        }
    }

    fprintf(results, "%d total characters", charcount);
    return 1;
}

/*
 * job_echo() -- echo content back as result
 */
int job_echo(FILE *results, FILE *content){
    char content_read[MAXFILEREAD];
    int bytes_read;

    while ((bytes_read = fread(content_read, sizeof(char), MAXFILEREAD, content)) > 0){
        fprintf(results, "%s", content_read);
    }
    return 1;
}

/*
 * job_capitalize() -- convert all lowercase letters in content to uppercase
 */
int job_capitalize(FILE *results, FILE *content){
    char content_read[MAXFILEREAD];
    int bytes_read;

    while ((bytes_read = fread(content_read, sizeof(char), MAXFILEREAD, content)) > 0){
        for (int i = 0; i < bytes_read; i++){
            if (islower(content_read[i])) {
                fprintf(results, "%c", toupper(content_read[i]));
                continue;
            }
            fprintf(results, "%c", content_read[i]);
        }
    }
    return 1;
}

/*
 * job_csvstats() -- count CSV rows and columns, write to results
 *
 * Uses get_size() from parse_csv.c to count dimensions without full parsing.
 * Output format: "N total entries, M columns"
 */
int job_csvstats(FILE *results, FILE *content, unsigned char header[MAXBUFSIZE]){
    int rows=0;
    int cols=0;

    get_size(content, &rows, &cols);
    fprintf(results, "%d total entries, %d columns", rows, cols);
    return 1;
}

/*
 * job_csvsort_mergesort_helper() -- merge two sorted index subarrays
 *
 * Merges idx_arr[left..middle] and idx_arr[middle+1..right] based on
 * string comparison of csv[idx_arr[i]][col]. Sorts indices, not actual rows.
 *
 * Note: Uses strcmp (lexicographic), so "9" > "100" because '9' > '1'.
 * Would need atoi() for proper numeric sorting.
 */
void job_csvsort_mergesort_helper(char ***csv, int col, int *idx_arr, int left, int middle, int right){
    int i = left;
    int j = middle+1;


    int temp[right-left + 1];
    int temp_idx = 0;

    while (i <= middle && j <= right){
        if (strcmp(csv[idx_arr[i]][col], csv[idx_arr[j]][col]) > 0){ // TODO: currently does not support strong string-integer comparison 
            temp[temp_idx++] = idx_arr[i++];                        // (eg '32' < '5' with this scheme)
        } else {
            temp[temp_idx++] = idx_arr[j++]; 
        }
    }

    while (j <= right){
        temp[temp_idx++] = idx_arr[j++];
    }

    while (i <= middle){
        temp[temp_idx++] = idx_arr[i++];
    }

    // Copy merged result back to idx_arr
    temp_idx = 0;
    for (int i = left; i <= right; i++){
        idx_arr[i] = temp[temp_idx++];
    }
}

/*
 * job_csvsort_mergesort() -- recursive merge sort on index array
 *
 * Classic divide-and-conquer: sort left half, sort right half, merge.
 * Base case: single-element array is already sorted.
 */
void job_csvsort_mergesort(char ***csv, int col, int *idx_arr, int left, int right){
    int middle = (left + right) / 2;

    if (left >= right) return;

    // Recursively sort halves
    job_csvsort_mergesort(csv, col, idx_arr, left, middle);
    job_csvsort_mergesort(csv, col, idx_arr, middle+1, right);

    // Merge sorted halves
    job_csvsort_mergesort_helper(csv, col, idx_arr, left, middle, right);
}

/*
 * job_csvsort() -- sort CSV by specified column using index array strategy
 *
 * Parses column name from header, finds column index in CSV header row,
 * creates index array [1,2,3,...], sorts it based on column values,
 * outputs rows in sorted index order.
 *
 * Index array strategy: Moves integers instead of entire rows (faster).
 * Example: idx_sort=[3,1,2] outputs rows in order: csv[3], csv[1], csv[2]
 */
int job_csvsort(FILE *results, FILE *content, unsigned char header[MAXBUFSIZE]){
    strip_whitespace((char *)header);

    char filter_keyword[MAXFILEPATH];

    extract_first_word(filter_keyword, (char *)header);

    struct CSV *csv = malloc(sizeof *csv);
    csv->cols = 0;
    csv->rows = 0;
    parse_csv(csv, content);

    int col_idx = -1;

    for(int j = 0; j < csv->cols; j++){
        if (strcmp(csv->csv_data[0][j], filter_keyword) == 0){
            col_idx = j;
            break;
        }
    }
    if (col_idx == -1) return -1;  // Column not found

    // Initialize index array [1, 2, 3, ..., rows-1] (skip header row 0)
    int idx_sort[csv->rows - 1];
    for (int i = 1; i < csv->rows; i++){
        idx_sort[i-1] = i;
    }

    for (int j = 0; j < csv->cols; j++){
        if (j > 0) fprintf(results, ",");
        fprintf(results, "%s", csv->csv_data[0][j]);
    }
    fprintf(results, "\n");

    // Sort index array by column values (lexicographic comparison)
    job_csvsort_mergesort(csv->csv_data, col_idx, idx_sort, 0, csv->rows - 2);

    // Output rows in sorted index order
    for (int i = 0; i < csv->rows-1; i++){
        for (int j = 0; j < csv->cols; j++){
            if (j > 0) fprintf(results, ",");
            fprintf(results, "%s", csv->csv_data[idx_sort[i]][j]);
        }
        fprintf(results, "\n");
    }

    return 1;
}

/*
 * job_csvfilter() -- filter CSV rows by column value match
 *
 * Header format: "csvfilter [column_name] [filter_value]"
 * Example: "csvfilter City Portland" returns all rows where City="Portland"
 *
 * Algorithm:
 * 1. Parse column name and filter value from header
 * 2. Parse CSV structure
 * 3. Find column index by searching header row
 * 4. Output header + matching rows only
 */
int job_csvfilter(FILE *results, FILE *content, unsigned char header[MAXBUFSIZE]){
    strip_whitespace((char *)header);

    char filter_keyword[MAXFILEPATH];
    char filter_key[MAXFILEPATH];

    extract_first_word(filter_keyword, (char *)header);  // Column name
    extract_first_word(filter_key, (char *)header);      // Value to match

    struct CSV *csv = malloc(sizeof *csv);
    csv->cols = 0;
    csv->rows = 0;
    parse_csv(csv, content);

    int col_idx = -1;

    for(int j = 0; j < csv->cols; j++){
        if (strcmp(csv->csv_data[0][j], filter_keyword) == 0){
            col_idx = j;
            break;
        }
    }
    if (col_idx == -1) return -1;  // Column not found
    
    // Write header row
    for (int j = 0; j < csv->cols; j++){
        if (j > 0) fprintf(results, ",");
        fprintf(results, "%s", csv->csv_data[0][j]);
    }
    fprintf(results, "\n");

    // Write matching rows only
    for (int i = 0; i < csv->rows; i++){
        if (strcmp(csv->csv_data[i][col_idx], filter_key) == 0){
            for (int j = 0; j < csv->cols; j++){
                if (j > 0) fprintf(results, ",");
                fprintf(results, "%s", csv->csv_data[i][j]);
            }
            fprintf(results, "\n");
        }
    }

    return 1;
}

int job_scale(unsigned char header[MAXBUFSIZE], char* img_path, char *output_path){
    printf("scaling\n");
    MagickWand *magick_wand;
    MagickBooleanType status;

    MagickWandGenesis();
    magick_wand = NewMagickWand();

    status = MagickReadImage(magick_wand, img_path);
    if (status == MagickFalse){
        fprintf(stderr, "Error reading file %s\n", img_path);
        return -1;
    }

    int img_width = MagickGetImageWidth(magick_wand);
    int img_height = MagickGetImageHeight(magick_wand);

    printf("img dimensions: %d x %d (wxh)\n", img_width, img_height);

    char factor_c[MAXFILEPATH];
    strip_whitespace(header);
    extract_first_word(factor_c, header);

    char *endptr;
    double scale_factor = strtod(factor_c, &endptr);

    printf("scale factor: %f\n", scale_factor);

    status = MagickResizeImage(magick_wand, img_width*scale_factor, img_height*scale_factor, LanczosFilter, 1.0);
    if (status == MagickFalse){
        fprintf(stderr, "Failed to resize image %s\n", img_path);
        return -1;
    }

    status = MagickWriteImage(magick_wand, output_path);
    if (status == MagickFalse) {
        fprintf(stderr, "Error writing image\n");
        return 1;
    }

    magick_wand = DestroyMagickWand(magick_wand);
    MagickWandTerminus();

    return 1;
}

int job_resize(unsigned char header[MAXBUFSIZE], char* img_path, char *output_path){
    printf("resizing\n");
    MagickWand *magick_wand;
    MagickBooleanType status;

    MagickWandGenesis();
    magick_wand = NewMagickWand();

    status = MagickReadImage(magick_wand, img_path);
    if (status == MagickFalse){
        fprintf(stderr, "Error reading file %s\n", img_path);
        return -1;
    }

    int img_width = MagickGetImageWidth(magick_wand);
    int img_height = MagickGetImageHeight(magick_wand);

    printf("img dimensions: %d x %d (wxh)\n", img_width, img_height);

    char new_dimension[MAXFILEPATH];
    strip_whitespace(header);
    extract_first_word(new_dimension, header);

    int i = 0;
    for (; i < strlen(new_dimension); i++){
        if (new_dimension[i] == 'x'){
            new_dimension[i] == '\0';
            i++;
            break;
        }
    }

    int new_width = atoi(new_dimension);
    int new_height = atoi(new_dimension+i);

    printf("new dimensions: %d x %d\n", new_width, new_height);

    status = MagickResizeImage(magick_wand, new_width, new_height, LanczosFilter, 1.0);
    if (status == MagickFalse){
        fprintf(stderr, "Failed to resize image %s\n", img_path);
        return -1;
    }

    status = MagickWriteImage(magick_wand, output_path);
    if (status == MagickFalse) {
        fprintf(stderr, "Error writing image\n");
        return 1;
    }

    magick_wand = DestroyMagickWand(magick_wand);
    MagickWandTerminus();

    return 1;
}

int job_filter_img(unsigned char header[MAXBUFSIZE], char* img_path, char *output_path){
    printf("filtering!\n");
    return 0;
}

int job_flipy_img(unsigned char header[MAXBUFSIZE], char* img_path, char *output_path){
    printf("flipping!\n");
    MagickWand *magick_wand;
    MagickBooleanType status;

    MagickWandGenesis();
    magick_wand = NewMagickWand();

    status = MagickReadImage(magick_wand, img_path);
    if (status == MagickFalse){
        fprintf(stderr, "Error reading file %s\n", img_path);
        return -1;
    }

    int img_width = MagickGetImageWidth(magick_wand);
    int img_height = MagickGetImageHeight(magick_wand);

    printf("img dimensions: %d x %d (wxh)\n", img_width, img_height);


    status = MagickFlipImage(magick_wand);
    if (status == MagickFalse){
        fprintf(stderr, "Failed to resize image %s\n", img_path);
        return -1;
    }

    status = MagickWriteImage(magick_wand, output_path);
    if (status == MagickFalse) {
        fprintf(stderr, "Error writing image\n");
        return 1;
    }

    magick_wand = DestroyMagickWand(magick_wand);
    MagickWandTerminus();

    return 1;
}

int job_flipx_img(unsigned char header[MAXBUFSIZE], char* img_path, char *output_path){
    printf("flipping!\n");
    MagickWand *magick_wand;
    MagickBooleanType status;

    MagickWandGenesis();
    magick_wand = NewMagickWand();

    status = MagickReadImage(magick_wand, img_path);
    if (status == MagickFalse){
        fprintf(stderr, "Error reading file %s\n", img_path);
        return -1;
    }

    int img_width = MagickGetImageWidth(magick_wand);
    int img_height = MagickGetImageHeight(magick_wand);

    printf("img dimensions: %d x %d (wxh)\n", img_width, img_height);


    status = MagickFlopImage(magick_wand);
    if (status == MagickFalse){
        fprintf(stderr, "Failed to resize image %s\n", img_path);
        return -1;
    }

    status = MagickWriteImage(magick_wand, output_path);
    if (status == MagickFalse) {
        fprintf(stderr, "Error writing image\n");
        return 1;
    }

    magick_wand = DestroyMagickWand(magick_wand);
    MagickWandTerminus();

    return 1;
}

int job_rotate_img(unsigned char header[MAXBUFSIZE], char* img_path, char *output_path){
    printf("rotate\n");
    MagickWand *magick_wand;
    MagickBooleanType status;

    MagickWandGenesis();
    magick_wand = NewMagickWand();

    PixelWand *bg = NewPixelWand();
    PixelSetColor(bg, "black");

    status = MagickReadImage(magick_wand, img_path);
    if (status == MagickFalse){
        fprintf(stderr, "Error reading file %s\n", img_path);
        return -1;
    }

    int img_width = MagickGetImageWidth(magick_wand);
    int img_height = MagickGetImageHeight(magick_wand);

    printf("img dimensions: %d x %d (wxh)\n", img_width, img_height);

    char degree[MAXFILEPATH];
    strip_whitespace(header);
    extract_first_word(degree, header);

    char *endptr;
    double degrees = strtod(degree, &endptr);

    status = MagickRotateImage(magick_wand, bg, degrees);
    if (status == MagickFalse){
        fprintf(stderr, "Failed to resize image %s\n", img_path);
        return -1;
    }

    status = MagickWriteImage(magick_wand, output_path);
    if (status == MagickFalse) {
        fprintf(stderr, "Error writing image\n");
        return 1;
    }

    magick_wand = DestroyMagickWand(magick_wand);
    MagickWandTerminus();

    return 1;
}

int job_charcoal_img(unsigned char header[MAXBUFSIZE], char* img_path, char *output_path){
    printf("charcoal\n");
    MagickWand *magick_wand;
    MagickBooleanType status;

    MagickWandGenesis();
    magick_wand = NewMagickWand();

    status = MagickReadImage(magick_wand, img_path);
    if (status == MagickFalse){
        fprintf(stderr, "Error reading file %s\n", img_path);
        return -1;
    }

    int img_width = MagickGetImageWidth(magick_wand);
    int img_height = MagickGetImageHeight(magick_wand);

    printf("img dimensions: %d x %d (wxh)\n", img_width, img_height);

    char radius_s[MAXFILEPATH];
    strip_whitespace(header);
    extract_first_word(radius_s, header);

    char *endptr;
    double radius = strtod(radius_s, &endptr);

    char omega_s[MAXFILEPATH];
    strip_whitespace(header);
    extract_first_word(omega_s, header);

    double omega = strtod(omega_s, &endptr);

    status = MagickCharcoalImage(magick_wand, radius, omega);
    if (status == MagickFalse){
        fprintf(stderr, "Failed to resize image %s\n", img_path);
        return -1;
    }

    status = MagickWriteImage(magick_wand, output_path);
    if (status == MagickFalse) {
        fprintf(stderr, "Error writing image\n");
        return 1;
    }

    magick_wand = DestroyMagickWand(magick_wand);
    MagickWandTerminus();

    return 1;
}

int job_monochrome_img(unsigned char header[MAXBUFSIZE], char* img_path, char *output_path){
    printf("mono\n");
    MagickWand *magick_wand;
    MagickBooleanType status;

    MagickWandGenesis();
    magick_wand = NewMagickWand();

    status = MagickReadImage(magick_wand, img_path);
    if (status == MagickFalse){
        fprintf(stderr, "Error reading file %s\n", img_path);
        return -1;
    }

    int img_width = MagickGetImageWidth(magick_wand);
    int img_height = MagickGetImageHeight(magick_wand);

    printf("img dimensions: %d x %d (wxh)\n", img_width, img_height);

    status = MagickTransformImageColorspace(magick_wand, GRAYColorspace);
    if (status == MagickFalse){
        fprintf(stderr, "Failed to resize image %s\n", img_path);
        return -1;
    }

    status = MagickWriteImage(magick_wand, output_path);
    if (status == MagickFalse) {
        fprintf(stderr, "Error writing image\n");
        return 1;
    }

    magick_wand = DestroyMagickWand(magick_wand);
    MagickWandTerminus();

    return 1;
}

int job_stencil_img(unsigned char header[MAXBUFSIZE], char* img_path, char *output_path){
    printf("stencil\n");
    MagickWand *magick_wand;
    MagickBooleanType status;

    MagickWandGenesis();
    magick_wand = NewMagickWand();

    status = MagickReadImage(magick_wand, img_path);
    if (status == MagickFalse){
        fprintf(stderr, "Error reading file %s\n", img_path);
        return -1;
    }

    int img_width = MagickGetImageWidth(magick_wand);
    int img_height = MagickGetImageHeight(magick_wand);

    printf("img dimensions: %d x %d (wxh)\n", img_width, img_height);

    status = MagickTransformImageColorspace(magick_wand, GRAYColorspace);
    if (status == MagickFalse){
        fprintf(stderr, "Failed to resize image %s\n", img_path);
        return -1;
    }

    status = MagickEdgeImage(magick_wand, 1);
    if (status == MagickFalse){
        fprintf(stderr, "Failed to resize image %s\n", img_path);
        return -1;
    }

    status = MagickNegateImage(magick_wand, MagickFalse);
    if (status == MagickFalse){
        fprintf(stderr, "Failed to resize image %s\n", img_path);
        return -1;
    }

    status = MagickThresholdImage(magick_wand, 0.7 * QuantumRange);
    if (status == MagickFalse) {
        fprintf(stderr, "Error writing image\n");
        return 1;
    }


    status = MagickWriteImage(magick_wand, output_path);
    if (status == MagickFalse) {
        fprintf(stderr, "Error writing image\n");
        return 1;
    }

    magick_wand = DestroyMagickWand(magick_wand);
    MagickWandTerminus();

    return 1;
}


/*
 * process_job() -- route job to appropriate handler based on type
 *
 * Determines job type from content, calls appropriate job function, returns result or error code
 */
int process_job(unsigned char header[MAXBUFSIZE], char dir[MAXFILEPATH], char ext[MAXFILEEXT]){
    int job_type = determine_job_type(header, strlen((char *)header));
    int rv = 1;

    if (job_type == -1){
        return WERR_INVALIDJOB;
    }

    char fcontent[MAXFILEPATH];
    strcpy(fcontent, dir);

    char fresults[MAXFILEPATH];
    strcpy(fresults, dir);

    if (strcmp(ext, ".txt") == 0){
        printf("txt job.\n");
        strcat(fresults, "results.txt");
        fclose(fopen(fresults, "w"));
        strcat(fcontent, "content.txt");
        FILE *results_file = fopen(fresults ,"w");
        FILE *content_file = fopen(fcontent ,"r");

        if (job_type == JTYPE_WORDCOUNT){
            FILE *results_file = fopen(fresults ,"w");
            FILE *content_file = fopen(fcontent ,"r");
            rv = job_wordcount(results_file, content_file);
        } 
        
        else if (job_type == JTYPE_CHARCOUNT){
            FILE *results_file = fopen(fresults ,"w");
            FILE *content_file = fopen(fcontent ,"r");
            rv = job_charcount(results_file, content_file);
        }

        else if (job_type == JTYPE_ECHO){
            FILE *results_file = fopen(fresults ,"w");
            FILE *content_file = fopen(fcontent ,"r");
            rv = job_echo(results_file, content_file);
        }

        else if (job_type == JTYPE_CAPITALIZE){
            FILE *results_file = fopen(fresults ,"w");
            FILE *content_file = fopen(fcontent ,"r");
            rv = job_capitalize(results_file, content_file);
        }

        else if (job_type == JTYPE_CSVFILTER){
            FILE *results_file = fopen(fresults ,"w");
            FILE *content_file = fopen(fcontent ,"r");
            rv = job_csvfilter(results_file, content_file, header);
        }

        else if (job_type == JTYPE_CSVSORT){
            FILE *results_file = fopen(fresults ,"w");
            FILE *content_file = fopen(fcontent ,"r");
            rv = job_csvsort(results_file, content_file, header);
        }

        else if (job_type == JTYPE_CSVSTATS){
            FILE *results_file = fopen(fresults ,"w");
            FILE *content_file = fopen(fcontent ,"r");
            rv = job_csvstats(results_file, content_file, header);
        }

        fclose(results_file);
        fclose(content_file);

    } else if (strcmp(ext, ".jpg") == 0){
        printf("img job.\n");
        strcat(fresults, "results.jpg");
        fclose(fopen(fresults, "wb"));
        strcat(fcontent, "content.jpg");
        FILE *results_file = fopen(fresults ,"wb");
        FILE *content_file = fopen(fcontent ,"rb");

        if (job_type == JTYPE_SCALE){
            rv = job_scale(header, fcontent, fresults);
        }

        else if (job_type == JTYPE_RESIZE){
            rv = job_resize(header, fcontent, fresults);
        }

        else if (job_type == JTYPE_FILTER){
            rv = job_filter_img(header, fcontent, fresults);
        }

        else if (job_type == JTYPE_FLIPX){
            rv = job_flipx_img(header, fcontent, fresults);
        }

        else if (job_type == JTYPE_FLIPY){
            rv = job_flipy_img(header, fcontent, fresults);
        }

        else if (job_type == JTYPE_ROTATE){
            rv = job_rotate_img(header, fcontent, fresults);
        }

        else if (job_type == JTYPE_CHARCOAL){
            rv = job_charcoal_img(header, fcontent, fresults);
        }

        else if (job_type == JTYPE_MONOCHROME){
            rv = job_monochrome_img(header, fcontent, fresults);
        }

        else if (job_type == JTYPE_STENCIL){
            rv = job_stencil_img(header, fcontent, fresults);
        }



        fclose(results_file);
        fclose(content_file);
    }
    return rv;
}

