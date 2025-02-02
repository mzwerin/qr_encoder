#include "FPToolkit.c"

int grid[100][100];
int modules = 80;
int level = 0;
int binary[50000];

const int format_bits[8][15] = {
        {1,1,1,0,1,1,1,1,1,0,0,0,1,0,0},
        {1,1,1,0,0,1,0,1,1,1,1,0,0,1,1},
        {1,1,1,1,1,0,1,1,0,1,0,1,0,1,0},
        {1,1,1,1,0,0,0,1,0,0,1,1,1,0,1},
        {1,1,0,0,1,1,0,0,0,1,0,1,1,1,1},
        {1,1,0,0,0,1,1,0,0,0,1,1,0,0,0},
        {1,1,0,1,1,0,0,0,1,0,0,0,0,0,1},
        {1,1,0,1,0,0,1,0,1,1,1,0,1,1,0}
};
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
    double size = floor(600.0 / modules);

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

int main() {
    init_grid();

    init_timing_patterns();
    init_finder_patterns();
    init_dummy_format_bits();
    init_alignment_patterns();

    G_init_graphics(800, 800);
    G_rgb(1,1,1);
    G_clear();

    for (int i = 0; i < 5000; i++) {
        double x = drand48();

        if (x < 0.6) binary[i] = 0;
        else binary[i] = 1;
    }

    put_binary_in_grid(binary);

    display_grid(0);

    while (1) {
        int q = G_wait_key();
        if (q == 'q') exit(0);
    }
}
