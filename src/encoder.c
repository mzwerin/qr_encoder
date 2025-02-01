#include "FPToolkit.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_SIZE 50000

/*----------------CONSTANTS----------------*/

const int ECC_LENGTHS[10][4] = {
        {7, 10, 13, 17},    // Version 1
        {10, 16, 22, 28},   // Version 2
        {15, 26, 36, 44},   // Version 3
        {20, 36, 52, 64},   // Version 4
        {26, 48, 72, 88},   // Version 5
        {36, 64, 96, 112},  // Version 6
        {40, 72, 108, 130}, // Version 7
        {48, 88, 132, 156}, // Version 8
        {60, 110, 160, 192},// Version 9
        {72, 130, 192, 224} // Version 10
};

/*-----------------STRUCTS-----------------*/

typedef struct {
    int index;
    char character;
    char hex_value[3]; // 2 hex digits + null terminator
    char bits[9];      // 8 bits + null terminator
} CharInfo;

/*---------------CONVERSIONS---------------*/

void char_to_binary(char ch, char *binary) {
    for (int i = 7; i >= 0; --i) {
        binary[7 - i] = (ch & (1 << i)) ? '1' : '0';
    }
    binary[8] = '\0';
}

void binary_to_hex(const char* binary, char* hex, int hex_size) {
    int bin_len = strlen(binary);

    // Ensure the binary string length is a multiple of 4 for hex conversion
    if (bin_len % 4 != 0) {
        printf("Error: Binary string length must be a multiple of 4.\n");
        return;
    }

    // Calculate the required length of the hex string
    int hex_len = bin_len / 4;

    // Check if the provided hex buffer size is sufficient
    if (hex_size < hex_len + 1) { // +1 for null terminator
        printf("Error: Output buffer size too small.\n");
        return;
    }

    // Process 4 binary digits at a time
    for (int i = 0; i < hex_len; i++) {
        char segment[5]; // Temporary storage for 4 binary digits + '\0'
        strncpy(segment, binary + i * 4, 4);
        segment[4] = '\0'; // Null terminate the segment

        // Convert binary segment to integer
        int value = strtol(segment, NULL, 2); // Base 2 conversion

        // Map the integer to its hexadecimal representation
        hex[i] = "0123456789ABCDEF"[value];
    }

    hex[hex_len] = '\0'; // Null terminate the final string
}

const char* hexToBin(char hex) {
    switch (hex) {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': case 'a': return "1010";
        case 'B': case 'b': return "1011";
        case 'C': case 'c': return "1100";
        case 'D': case 'd': return "1101";
        case 'E': case 'e': return "1110";
        case 'F': case 'f': return "1111";
        default: return NULL; // Invalid character
    }
}

int hexStringToBinString(const char* hexStr, char* binStr, size_t binStrSize) {
    if (hexStr == NULL || binStr == NULL) return -1;

    size_t hexLen = strlen(hexStr);
    size_t requiredSize = hexLen * 4 + 1; // 4 bits per hex digit + 1 for null terminator

    if (binStrSize < requiredSize) {
        return -1; // Buffer size insufficient
    }

    binStr[0] = '\0'; // Initialize the binary string
    for (size_t i = 0; i < hexLen; i++) {
        const char* binChunk = hexToBin(hexStr[i]);
        if (binChunk == NULL) { // Invalid hex character
            return -1;
        }
        strcat(binStr, binChunk);
    }

    return 0; // Success
}

void string_to_charinfo(const char *s, CharInfo info_array[], int *out_size) {
    int i = 0;
    while (s[i] != '\0') {
        info_array[i].index = i;
        info_array[i].character = s[i];
        snprintf(info_array[i].hex_value, sizeof(info_array[i].hex_value), "%02X", s[i]);
        char_to_binary(s[i], info_array[i].bits);
        i++;
    }
    *out_size = i; // Return size of array
}

/*------------------LOGIC------------------*/

int determine_qr_version(int size) {
    if (size <= 17) return 1;   // Version 1  (152 bits)
    if (size <= 32) return 2;   // Version 2  (272 bits)
    if (size <= 53) return 3;   // Version 3  (440 bits)
    if (size <= 78) return 4;   // Version 4  (640 bits)
    if (size <= 106) return 5;  // Version 5  (864 bits)
    if (size <= 134) return 6;  // Version 6  (1088 bits)
    if (size <= 154) return 7;  // Version 7  (1248 bits)
    if (size <= 192) return 8;  // Version 8  (1552 bits)
    if (size <= 230) return 9;  // Version 9  (1856 bits)
    if (size <= 271) return 10; // Version 10 (2192 bits)

    // Need to extend for versions > 10

    return -1; // Input too large for supported QR versions
}

int get_max_bits_for_version(int version) {
    switch (version) {
        case 1: return 152;   // Version 1
        case 2: return 272;   // Version 2
        case 3: return 440;   // Version 3
        case 4: return 640;   // Version 4
        case 5: return 864;   // Version 5
        case 6: return 1088;  // Version 6
        case 7: return 1248;  // Version 7
        case 8: return 1552;  // Version 8
        case 9: return 1856;  // Version 9
        case 10: return 2192; // Version 10
            // Extend for versions 11–40 as needed
        default:
            printf("Unsupported QR version: %d\n", version);
            return -1; // Error: Unsupported version
    }
}

int add_qr_padding(char *data_bits, int size, int version, int *out_size) {
    int total_bits = strlen(data_bits);
    int max_bits = get_max_bits_for_version(version);

    // Add terminator (up to 4 bits or remaining space)
    int terminator_bits = (total_bits + 4 <= max_bits) ? 4 : (max_bits - total_bits);
    for (int i = 0; i < terminator_bits; i++) {
        strcat(data_bits, "0");
    }

    // Add padding bytes alternately
    const char *pad1 = "11101100";
    const char *pad2 = "00010001";
    while (strlen(data_bits) < max_bits) {
        strcat(data_bits, (strlen(data_bits) % 16 == 0) ? pad1 : pad2);
    }


    return *out_size = total_bits;
}

void print_charinfo(const CharInfo info_array[], int size) {
    printf("Index\tChar\tHex\tBinary\n");
    printf("--------------------------------\n");
    for (int i = 0; i < size; ++i) {
        printf("%d\t%c\t%s\t%s\n",
               info_array[i].index,
               info_array[i].character,
               info_array[i].hex_value,
               info_array[i].bits);
    }
}

void call_reedsolomon(const char *hex_string, int error_level, char *output, size_t output_size) {
    char command[8192];

    // Construct the Python command
    snprintf(command, sizeof(command),
             "python3 -c \"import reedsolo; "
             "rs = reedsolo.RSCodec(%d); "
             "data = bytes.fromhex('%s'); "
             "encoded_data = rs.encode(data); "
             "parity = encoded_data[len(data):]; "
             "print(parity.hex().upper())\"",
             error_level, hex_string);

    // Execute the command and capture the output
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        perror("Failed to run Python command");
        exit(1);
    }

    // Read the output from Python script
    if (fgets(output, output_size, fp) == NULL) {
        perror("Failed to read Python output");
        exit(1);
    }
    output[strcspn(output, "\n")] = 0;

    pclose(fp);
}

/*--------------INIT GRAPHICS--------------*/

int grid[100][100];
int modules;
int format_bits[8][15] =   {{1,1,1,0,1,1,1,1,1,0,0,0,1,0,0},{1,1,1,0,0,1,0,1,1,1,1,0,0,1,1},{1,1,1,1,1,0,1,1,0,1,0,1,0,1,0},{1,1,1,1,0,0,0,1,0,0,1,1,1,0,1},
                            {1,1,0,0,1,1,0,0,0,1,0,1,1,1,1},{1,1,0,0,0,1,1,0,0,0,1,1,0,0,0},{1,1,0,1,1,0,0,0,1,0,0,0,0,0,1},{1,1,0,1,0,0,1,0,1,1,1,0,1,1,0}};

// LEVEL 0 = Low
// LEVEL 1 = Medium
// LEVEL 2 = High
// LEVEL 3 = Quartile
int level = 0;

void init_grid() {
    for (int i = 0; i < modules; i++) {
        for (int j = 0; j < modules; j++) {
            grid[i][j] = 2;  // Usable grid when location == 2
        }
    }
}

void init_timing_patterns() {
    for (int i = 0; i < modules; i++) {
        grid[i][modules-7] = 1 - (i % 2);
        grid[6][i] = 1 - (i % 2);
    }
}

void init_finder_patterns() {
    int c = modules - 1;

    // white
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            grid[i][j] = 0;
            grid[i][c-7+j] = 0;
            grid[c-7+i][c-7+j] = 0;
        }
    }

    // outer black
    for (int i = 0; i < 7; i++) {
        grid[i][0] = 1;
        grid[0][i] = 1;
        grid[i][6] = 1;
        grid[6][i] = 1;

        grid[i][c] = 1;
        grid[0][c-i] = 1;
        grid[i][c-6] = 1;
        grid[6][c-i] = 1;

        grid[c-6+i][c] = 1;
        grid[c-6][c-6+i] = 1;
        grid[c-6+i][c-6] = 1;
        grid[c][c-6+i] = 1;
    }

    // inner black
    for (int i = 2; i < 5; i++) {
        for (int j = 2; j < 5; j++) {
            grid[i][j] = 1;
            grid[i][c-6+j] = 1;
            grid[c-6+i][c-6+j] = 1;
        }
    }
}

void init_dummy_format_bits() {
    int c = modules - 1;

    for (int i = 0; i < 8; i++) {
        grid[c-i][c-8] = 4;
        grid[8][i] = 4;
        if (grid[i][c-8] != 1) grid[i][c-8] = 4;
        if (grid[8][c-i] != 1) grid[8][c-i] = 4;
    }
    grid[8][c-8] = 4;

    if (level == 0) {
        grid[8][0] = 4;
        grid[8][1] = 4;
        grid[0][modules-9] = 4;
        grid[1][modules-9] = 4;
    }

    grid[8][7] = 1;
}

void draw_single_box(double x, double y, double size, int c) {
    if (c == 0) G_rgb(1, 1, 1);
    else if (c == 1) G_rgb(0, 0, 0);
    else G_rgb(0.9, 0.2, 0.7);

    G_fill_rectangle(x, y, size, size);
}

void update(int i, int j) {
    double size = floor(600 / modules);
    double x = 100 + i * size;
    double y = 100 + j * size;
    draw_single_box(x, y, size, grid[i][j]);
    int q = 0;
    q = G_wait_key();
    if (q == 'q') exit(0);
}

void init_format_bits(int mask) {
    int c = modules - 1;
    int bits[15];
    int bitcount = 0;

    for (int i = 0; i < 15; i++) {
        bits[i] = format_bits[mask][i];
    }

    // upper left x
    for (int i = 0; i < 8; i++) {
        if (grid[i][c-8] == 1) continue;
        grid[i][c-8] = bits[bitcount];
        bitcount++;
    }

    // upper left y
    for (int i = 0; i < 9; i++) {
        if (grid[8][c-8+i] == 1) continue;
        grid[8][c-8+i] = bits[bitcount];
        bitcount++;
    }

    // lower left y
    bitcount = 0;
    for (int i = 0; i < 7; i++) {
        grid[8][i] = bits[bitcount];
        bitcount++;
    }

    // upper right x
    for (int i = 0; i < 8; i++) {
        grid[c-7+i][c-8] = bits[bitcount];
        bitcount++;
    }

}

void init_alignment_patterns() {
    int c = modules - 1;
    for (int i = 4; i < 9; i++) {
        for (int j = 4; j < 9; j++) {
            grid[c - i][j] = 1;
        }
    }

    for (int i = 5; i < 8; i++) {
        for (int j = 5; j < 8; j++) {
            grid[c - i][j] = 0;
        }
    }

    grid[c - 6][6] = 1;

}

void display_grid(int mask) {
    G_rgb(1,1,1);
    G_clear();
    double size = floor(600 / modules);

    init_timing_patterns();
    init_finder_patterns();

    if (mask >= 0 && mask < 8) init_format_bits(mask);
    else init_dummy_format_bits();

    init_alignment_patterns();

    for (int i = 0; i < modules; i++) {
        for (int j = 0; j < modules; j++) {
            double x = 100 + i * size;
            double y = 100 + j * size;

            draw_single_box(x, y, size, grid[i][j]);
        }
    }
}

void put_binary_in_grid(int binary[]) {
    int x = modules - 1;  // Start at the bottom row
    int y = 0;            // Start at the rightmost column
    int count = 0;        // Binary data index

    grid[x][y] = binary[count]; count++;

    while (x >= 0) { // While not past the 0th row
        while (y < modules) {  // Zigzag up

            // Move LEFT
            x--;
            if (x >= 0 && y < modules) {
                if (grid[x][y] == 2) {
                    grid[x][y] = binary[count];
                    count++;
                }
            }

            // Diagonal RIGHT-UP
            x++;
            y++;
            if (x >= 0 && y < modules) {
                if (grid[x][y] == 2) {
                    grid[x][y] = binary[count];
                    count++;
                }
            }
        }

        // Move LEFT
        x-=2;
        if (x == 6) x--;
        if (x >= 0 && y < modules) {
            if (grid[x][y] == 2) {
                grid[x][y] = binary[count];
                count++;
            }
        }

        while (y >= 0) { // Zigzag down
            // Move LEFT
            x--;
            if (x >= 0 && y < modules) {
                if (grid[x][y] == 2) {
                    grid[x][y] = binary[count];
                    count++;
                }
            }
            // Diagonal RIGHT-DOWN
            x++;
            y--;
            if (x >= 0 && y < modules) {
                if (grid[x][y] == 2) {
                    grid[x][y] = binary[count];
                    count++;
                }
            }
        }
        // Move LEFT
        x-=2;
        if (x >= 0 && y < modules) {
            if (grid[x][y] == 2) {
                grid[x][y] = binary[count];
                count++;
                draw_single_box(x,y,modules,binary[count]); G_wait_key();
            }
        }
    }
}

void apply_mask(int mask) {
    for (int x = 0; x < modules; x++) {
        for (int y = 0; y < modules; y++) {
            // Skip fixed patterns (finder, timing, and format)
            if ((grid[x][y] == 2) || (grid[x][y] == 3)) continue;

            if (mask == 0 && (x + y) % 2 == 0) grid[x][y] ^= 1;
            if (mask == 1 && x % 2 == 0) grid[x][y] ^= 1;
            if (mask == 2 && y % 3 == 0) grid[x][y] ^= 1;
            if (mask == 3 && (x + y) % 3 == 0) grid[x][y] ^= 1;
            if (mask == 4 && ((floor(x / 2)) + (int)(floor(y / 3)) % 2 == 0)) grid[x][y] ^= 1;
            if (mask == 5 && ((x * y) % 2 + (x * y) % 3 == 0)) grid[x][y] ^= 1;
            if (mask == 6 && (((x * y) % 2 + (x * y) % 3) % 2 == 0)) grid[x][y] ^= 1;
            if (mask == 7 && (((x + y) % 2 + (x * y) % 3) % 2 == 0)) grid[x][y] ^= 1;
        }
    }
}

void test_masks() {
    int q = 0;

    while (q != 'q') {
        q = G_wait_key();
        if (q == 'q') exit(0);

        q = q - 48;

        char text[100];
        // Copy original grid
        int temp_grid[100][100];
        memcpy(temp_grid, grid, sizeof(grid));

        // Apply mask
        apply_mask(q);

        display_grid(q);

        if (q >= 0 && q < 8) snprintf(text, sizeof(text), "applying mask : %d", q);
        else snprintf(text, sizeof(text), "no mask applied");

        G_rgb(0,0,0);
        G_draw_string(text, 100, 745);

        // Restore grid before testing next mask
        memcpy(grid, temp_grid, sizeof(grid));
    }
}


void loading(int speed) {
    int bar_length = 25;
    int total = 100;
    for (int i = 0; i <= total; i++) {
        double x = speed * drand48();
        i = i + (int) x;
        if (i > total) i = total;

        int percentage = (i * total) / total;
        int numBars = (i * bar_length) / total;

        printf("\r     > Progress : [");

        for (int j = 0; j < bar_length; j++) {
            if (j < numBars) {
                printf("#");
            } else {
                printf(" ");
            }
        }
        printf("] %d%%", percentage);
        fflush(stdout);
        usleep(50000);
    }
    printf("\n");
}

int main() {
    char input[MAX_SIZE];

    printf("\n     ----------------------------------------------------------------\n");
    printf("     Paste link : ");
    fgets(input, sizeof(input), stdin);

    // Remove newline
    input[strcspn(input, "\n")] = 0;

    CharInfo info_array[MAX_SIZE];
    int size;

    string_to_charinfo(input, info_array, &size);

    char seg_count[9] ;
    char_to_binary(size, seg_count);

    // Determine version
    int version = determine_qr_version(size);
    if (version == -1) {
        printf("Input size too large for supported QR versions.\n");
        return 1;
    }

    modules = (int)version * 4 + 17;

    // Prepare data bits
    char data_bits[MAX_SIZE] = "0100"; // Add mode indicator for byte mode
    strcat(data_bits, seg_count);  // Add segment count
    for (int i = 0; i < size; i++) {
        strcat(data_bits, info_array[i].bits);
    }

    add_qr_padding(data_bits, size, version, &size);

    char hex[MAX_SIZE];
    binary_to_hex(data_bits, hex, sizeof(hex));

    int ECC_LEN;
    ECC_LEN = ECC_LENGTHS[version - 1][level];

    char ECC[MAX_SIZE];
    call_reedsolomon(hex, ECC_LEN, ECC, sizeof(ECC));

    char bin[MAX_SIZE];
    hexStringToBinString(ECC, bin, sizeof(bin));

    strcat(data_bits, bin); // put all info into data_bits

    int length = strlen(data_bits);

    // Convert char array to int array
    int binary[MAX_SIZE];
    for (int i = 0; i < length; i++) {
        binary[i] = data_bits[i] - '0'; // Convert '0' or '1' to 0 or 1

    }

    loading(7);

    /*-------- GRAPHICS --------*/

    init_grid();
    init_timing_patterns();
    init_finder_patterns();
    init_dummy_format_bits();
    if (version > 1) init_alignment_patterns();

    G_init_graphics(800, 800);

    G_rgb(1,1,1);
    G_clear();

    put_binary_in_grid(binary);

    apply_mask(0);
    display_grid(0);

    printf("     Success!\n\n");
    printf("     Press 's' to save as .bmp and exit\n"
           "     Press 'q' to exit without saving\n");

    /*------- FILE SAVER -------*/

    char *file = "output.bmp";

    while (1) {
        int q = G_wait_key();
        if (q == 's') {
            FILE *f = fopen(file, "w+");
            if (f == NULL) {
                printf("error: could not open file '%s'\n", file);
                exit(1);
            }
            G_save_to_bmp_file(file);
            printf("\n     Success!\n");
            break;
        }
        if (q == 'q') {
            exit(0);
        }
    }
    printf("     ----------------------------------------------------------------\n\n");
    exit(0);
}
