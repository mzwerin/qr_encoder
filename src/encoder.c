#include "FPToolkit.c"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*----------------CONSTANTS----------------*/

#define MAX_SIZE 50000

char input[MAX_SIZE];
int binary[MAX_SIZE];
int grid[100][100];
int modules;

// LEVEL 0 = Low
// LEVEL 1 = Medium
// LEVEL 2 = Quartile
// LEVEL 3 = High
int level = 0;
int version;

const int ECC_LENGTHS[10][4] = {
        {7, 10, 13, 17},     // Version 1
        {10, 16, 22, 28},    // Version 2
        {15, 26, 36, 44},    // Version 3
        {20, 36, 52, 64},    // Version 4
        {26, 48, 72, 88},    // Version 5
        {36, 64, 96, 112},   // Version 6
        {40, 72, 108, 130},  // Version 7
        {48, 88, 132, 156},  // Version 8
        {60, 110, 160, 192}, // Version 9
        {72, 130, 192, 224}  // Version 10
};

const int format_bits[4][8][15] = {
        { // LOW
            {1,1,1,0,1,1,1,1,1,0,0,0,1,0,0}, // L 0
            {1,1,1,0,0,1,0,1,1,1,1,0,0,1,1}, // L 1
            {1,1,1,1,1,0,1,1,0,1,0,1,0,1,0}, // L 2
            {1,1,1,1,0,0,0,1,0,0,1,1,1,0,1}, // L 3
            {1,1,0,0,1,1,0,0,0,1,0,1,1,1,1}, // L 4
            {1,1,0,0,0,1,1,0,0,0,1,1,0,0,0}, // L 5
            {1,1,0,1,1,0,0,0,1,0,0,0,0,0,1}, // L 6
            {1,1,0,1,0,0,1,0,1,1,1,0,1,1,0}  // L 7
            },{ // MEDIUM
            {1,0,1,0,1,0,0,0,0,0,1,0,0,1,0}, // M 0
            {1,0,1,0,0,0,1,0,0,1,0,0,1,0,1}, // M 1
            {1,0,1,1,1,1,0,0,1,1,1,1,1,0,0}, // M 2
            {1,0,1,1,0,1,1,0,1,0,0,1,0,1,1}, // M 3
            {1,0,0,0,1,0,1,1,1,1,1,1,0,0,1}, // M 4
            {1,0,0,0,0,0,0,1,1,0,0,1,1,1,0}, // M 5
            {1,0,0,1,1,1,1,1,0,0,1,0,1,1,1}, // M 6
            {1,0,0,1,0,1,0,1,0,1,0,0,0,0,0}  // M 7
            },{ // QUARTILE
            {0,1,1,0,1,0,1,0,1,0,1,1,1,1,1}, // Q 0
            {0,1,1,0,0,0,0,0,1,1,0,1,0,0,0}, // Q 1
            {0,1,1,1,1,1,1,0,0,1,1,0,0,0,1}, // Q 2
            {0,1,1,1,0,1,0,0,0,0,0,0,1,1,0}, // Q 3
            {0,1,0,0,1,0,0,1,0,1,1,0,1,0,0}, // Q 4
            {0,1,0,0,0,0,1,1,0,0,0,0,0,1,1}, // Q 5
            {0,1,0,1,1,1,0,1,1,0,1,1,0,1,0}, // Q 6
            {0,1,0,1,0,1,1,1,1,1,0,1,1,0,1}  // Q 7
            },{ // HIGH
            {0,0,1,0,1,1,0,1,0,0,0,1,0,0,1}, // H 0
            {0,0,1,0,0,1,1,1,0,1,1,1,1,1,0}, // H 1
            {0,0,1,1,1,0,0,1,1,1,0,0,1,1,1}, // H 2
            {0,0,1,1,0,0,1,1,1,0,1,0,0,0,0}, // H 3
            {0,0,0,0,1,1,1,0,1,1,0,0,0,1,0}, // H 4
            {0,0,0,0,0,1,0,0,1,0,1,0,1,0,1}, // H 5
            {0,0,0,1,1,0,1,0,0,0,0,1,1,0,0}, // H 6
            {0,0,0,1,0,0,0,0,0,1,1,1,0,1,1}  // H 7
            }
};

const int alignment_pattern_locations[10][3] = {
        {-1, -1, -1},  // Version 1
        {6, 18, -1},   // Version 2
        {6, 22, -1},   // Version 3
        {6, 26, -1},   // Version 4
        {6, 30, -1},   // Version 5
        {6, 34, -1},   // Version 6
        {6, 22, 38},   // Version 7
        {6, 24, 42},   // Version 8
        {6, 26, 46},   // Version 9
        {6, 28, 50}    // Version 10
};


/*-----------------STRUCTS-----------------*/

typedef struct {
    int index;
    char character;
    char hex_value[3]; // 2 hex digits + null terminator
    char bits[9];      // 8 bits + null terminator
} CharInfo;

/*---------------CONVERSIONS---------------*/

void char_to_binary(char ch, char *bin) {
    for (int i = 7; i >= 0; --i) {
        bin[7 - i] = (ch & (1 << i)) ? '1' : '0';
    }
    bin[8] = '\0';
}

void binary_to_hex(const char* bin, char* hex, int hex_size) {
    int bin_len = strlen(bin);

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
        strncpy(segment, bin + i * 4, 4);
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

int get_max_bits_for_version() {
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

        // Need to extend for versions > 10

        default:
            printf("Unsupported QR version: %d\n", version);
            return -1;
    }
}

int add_qr_padding(char *data_bits, int size, int *out_size) {
    int total_bits = strlen(data_bits);
    int max_bits = get_max_bits_for_version();

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
    if (fgets(output, (int) output_size, fp) == NULL) {
        perror("Failed to read Python output");
        exit(1);
    }
    output[strcspn(output, "\n")] = 0;

    pclose(fp);
}

/*--------------INIT GRAPHICS--------------*/

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

    //if (level == 0) {
        grid[8][0] = 4;
        grid[8][1] = 4;
        grid[0][modules-9] = 4;
        grid[1][modules-9] = 4;
    //}

    grid[8][7] = 1;
}

void draw_single_box(double x, double y, double size, int c) {
    if (c == 0) G_rgb(1, 1, 1);
    else if (c == 1) G_rgb(0, 0, 0);
    else G_rgb(0.9, 0.2, 0.7);

    G_fill_rectangle(x, y, size, size);
}

void update(int i, int j) {
    double size = floor(600.0 / modules);
    double x = 100 + i * size;
    double y = 100 + j * size;
    draw_single_box(x, y, size, grid[i][j]);
    int q = G_wait_key();
    if (q == 'q') exit(0);
}

void init_format_bits(int mask) {
    int c = modules - 1;
    int bits[15];
    int bitcount = 0;

    for (int i = 0; i < 15; i++) {
        bits[i] = format_bits[level][mask][i];
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

void individual_alignment(int x, int y) {
    int out_x = x - 2, out_y = y - 2;
    int in_x = x - 1, in_y = y - 1;
    for (int i = 0; i < 5; i++) { // Outer layer
        for (int j = 0; j < 5; j++) {
            grid[out_x + i][out_y + j] = 1;
        }
    }
    for (int i = 0; i < 3; i++) { // Outer layer
        for (int j = 0; j < 3; j++) {
            grid[in_x + i][in_y + j] = 0;
        }
    }
    grid[x][y] = 1;
}

void init_alignment() {
    int a = alignment_pattern_locations[version-1][0];
    int b = alignment_pattern_locations[version-1][1];
    int c = alignment_pattern_locations[version-1][2];

    if (version > 0 && version < 7) {

        individual_alignment(b, a);

    } else if (version >= 7) {

        individual_alignment(a, b);
        individual_alignment(b, a);
        individual_alignment(b, b);
        individual_alignment(b, c);
        individual_alignment(c, a);
        individual_alignment(c, b);

    }
}

void display_grid(int mask) {
    G_rgb(1,1,1);
    G_clear();
    double size = floor(600.0 / modules);

    init_timing_patterns();
    init_finder_patterns();

    if (mask >= 0 && mask < 8) init_format_bits(mask);
    else init_dummy_format_bits();

    init_alignment();

    for (int i = 0; i < modules; i++) {
        for (int j = 0; j < modules; j++) {
            double x = 100 + i * size;
            double y = 100 + j * size;

            draw_single_box(x, y, size, grid[i][j]);
        }
    }
}

void put_binary_in_grid(int bin[]) {
    int x = modules - 1;  // Start at the bottom row
    int y = 0;            // Start at the rightmost column
    int count = 0;        // Binary data index

    grid[x][y] = bin[count]; count++;

    while (x >= 0) { // While not past the 0th row
        while (y < modules) {  // Zigzag up

            // Move LEFT
            x--;
            if (x >= 0 && y < modules) {
                if (grid[x][y] == 2) {
                    grid[x][y] = bin[count];
                    count++;
                }
            }

            // Diagonal RIGHT-UP
            x++;
            y++;
            if (x >= 0 && y < modules) {
                if (grid[x][y] == 2) {
                    grid[x][y] = bin[count];
                    count++;
                }
            }
        }

        // Move LEFT
        x-=2;
        if (x == 6) x--;
        if (x >= 0 && y < modules) {
            if (grid[x][y] == 2) {
                grid[x][y] = bin[count];
                count++;
            }
        }

        while (y >= 0) { // Zigzag down
            // Move LEFT
            x--;
            if (x >= 0 && y < modules) {
                if (grid[x][y] == 2) {
                    grid[x][y] = bin[count];
                    count++;
                }
            }
            // Diagonal RIGHT-DOWN
            x++;
            y--;
            if (x >= 0 && y < modules) {
                if (grid[x][y] == 2) {
                    grid[x][y] = bin[count];
                    count++;
                }
            }
        }
        // Move LEFT
        x-=2;
        if (x >= 0 && y < modules) {
            if (grid[x][y] == 2) {
                grid[x][y] = bin[count];
                count++;
                draw_single_box(x,y,modules,bin[count]); G_wait_key();
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

void load_graphics() {
    init_grid();
    init_timing_patterns();
    init_finder_patterns();
    init_dummy_format_bits();
    init_alignment();

    G_init_graphics(800, 800);

    G_rgb(1,1,1);
    G_clear();

    put_binary_in_grid(binary);

    apply_mask(0);
    display_grid(0);

    printf("     Success!\n\n");
}

/*-------------INPUT PROCESSOR-------------*/

void get_level_input() {
    char level_input;

    printf("\n     L = low (~7%%), M = medium (~25%%), Q = quartile (~25%%), H = high (~30%%)\n");
    printf("     Enter desired level of error correction : ");
    while (1) {
        scanf("%c", &level_input);
        getchar();
        if (level_input == 'L' || level_input == 'l') {
            level = 0;
            break;
        } else if (level_input == 'M' || level_input == 'm') {
            level = 1;
            break;
        } else if (level_input == 'Q' || level_input == 'q') {
            level = 2;
            break;
        } else if (level_input == 'H' || level_input == 'h') {
            level = 3;
            break;
        }
    }


}

void get_link_input() {
    printf("\n     Paste link : ");
    fgets(input, sizeof(input), stdin);


    // Remove newline
    input[strcspn(input, "\n")] = 0;
}

/*--------------ENCODE BINARY--------------*/

void load_binary() {
    CharInfo info_array[MAX_SIZE];
    int size;

    string_to_charinfo(input, info_array, &size);

    char seg_count[9] ;
    char_to_binary(size, seg_count);

    // Determine version
    version = determine_qr_version(size);
    if (version == -1) {
        printf("Input size too large for supported QR versions.\n");
        return;
    }

    modules = (int) version * 4 + 17;

    // Prepare data bits
    char data_bits[MAX_SIZE] = "0100"; // Add mode indicator for byte mode
    strcat(data_bits, seg_count);  // Add segment count
    for (int i = 0; i < size; i++) {
        strcat(data_bits, info_array[i].bits);
    }

    add_qr_padding(data_bits, size, &size);

    char hex[MAX_SIZE];
    binary_to_hex(data_bits, hex, sizeof(hex));

    int ECC_LEN;
    ECC_LEN = ECC_LENGTHS[version - 1][level];

    char ECC[MAX_SIZE];
    call_reedsolomon(hex, ECC_LEN, ECC, sizeof(ECC));

    char bin[MAX_SIZE];
    hexStringToBinString(ECC, bin, sizeof(bin));

    strcat(data_bits, bin); // Put all info into data_bits

    int length = strlen(data_bits);

    // Convert char array to int array
    for (int i = 0; i < length; i++) {
        binary[i] = data_bits[i] - '0'; // Converts '0' or '1' to 0 or 1
    }
}

/*---------------END PROGRAM---------------*/

void save_file() {
    char *home = getenv("HOME"); // get ~/Desktop path
    if (home == NULL) {
        printf("Error: Could not get HOME directory\n");
        exit(0);
    }

    char file[512];
    snprintf(file, sizeof(file), "%s/Desktop/output.bmp", home);

    FILE *f = fopen(file, "w+");

    if (f == NULL) {
        printf("Error: Could not open file '%s'\n", file);
        exit(0);
    }

    int b = G_save_to_bmp_file(file);
    if (b == 1) printf("\n     File has been saved to desktop.\n");
    else printf("\n     Error saving file.\n");
}

void save_or_quit() {
    printf("     Press 's' to save as .bmp and exit\n"
           "     Press 'q' to exit without saving\n");

    while (1) {
        int q = G_wait_key();
        if (q == 's') {
            save_file();
            break;
        }
        if (q == 'q') {
            break;
        }
    }
    printf("     ----------------------------------------------------------------\n\n");
    exit(1);
}

int main() {

    // Error correction level and pasted link
    printf("\n     ----------------------------------------------------------------");
    //get_level_input();
    level = 0;
    get_link_input();

    load_binary();

    load_graphics();

    save_or_quit();
}
