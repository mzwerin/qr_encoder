#include "FPToolkit.c"

// SIMPLE FUNCTION TO TEST FUTURE PLANS OF
// ADJUSTABLE COLORS FOR QR CODE

// bounds of rgb
#define UPPER 1.0
#define LOWER 0.0
double r, g, b;

void draw() {
    G_rgb(0,0,0);
    G_clear();
    G_rgb(r, g, b);
    G_fill_circle(400,400,100);
}

void init() {
    G_init_graphics(800,800);
    r = 1; g = 0; b = 0;
}

int main() {
    init();

    int key;
    while(1) {
        draw();
        key = G_wait_key();
        if (key == 'q') exit(0);
        else if (key == 'r') {
            r -= 0.05;
            if (r < LOWER) r = LOWER;
        }
        else if (key == 'g') {
            g -= 0.05;
            if (g < LOWER) g = LOWER;
        }
        else if (key == 'b') {
            b -= 0.05;
            if (b < LOWER) b = LOWER;
        }
        else if (key == 'R') {
            r += 0.05;
            if (r > UPPER) r = UPPER;
        }
        else if (key == 'G') {
            g += 0.05;
            if (g > UPPER) g = UPPER;
        }
        else if (key == 'B') {
            b += 0.05;
            if (b > UPPER) b = UPPER;
        }
    }
}