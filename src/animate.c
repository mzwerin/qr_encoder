#include "FPToolkit.c"
#include "clipper.c"

#define NUM_POINTS 50
#define BOX_SIZE 100
#define SLEEP 40000

int NXY = NUM_POINTS;
double X[1000], Y[1000];
double radius = BOX_SIZE/2.1; // Radius of the circle
double centerX = 400.0, centerY = 400.0; // Center of the circle

int clip_size = 80;
int NCLIP = 4;
double XCLIP[4];
double YCLIP[4];

int grid[100][100];
int modules;

void clear_screen() {
    G_rgb(0, 0, 0);
    G_clear();
}


void draw_single_box(double x, double y, double size, int c) {
    if (c == 0) G_rgb(1, 1, 1);
    else if (c == 1) G_rgb(0, 0, 0);
    else G_rgb(0.9, 0.2, 0.7);

    G_fill_rectangle(x, y, size, size);
}


void display_grid() {
    modules = 21;

    double size = floor(600 / modules);

    for (int i = 0; i < modules; i++) {
        for (int j = 0; j < modules; j++) {
            double x = 100 + i * size;
            double y = 100 + j * size;

            draw_single_box(x, y, size, grid[i][j]);
        }
    }
}


void init_circle() {
    for (int i = 0; i < NUM_POINTS; i++) {
        double angle = 2 * M_PI * i / NUM_POINTS;
        X[i] = centerX + radius * cos(angle);
        Y[i] = centerY + radius * sin(angle);
    }
}

void draw() {
    G_rgb(1,0,0);
    G_fill_polygon(X, Y, NXY);
    G_rgb(1,1,1);
    //G_polygon(XCLIP, YCLIP, NCLIP);
}

void buffer() {
    clear_screen();
    draw();
    G_display_image();
    usleep(SLEEP);
}


void animate(int flag) {
    if (flag == 1) { // scale up
        while (radius < clip_size) {
            radius *= 1.01;

            printf("%lf\n", BOX_SIZE/radius);

            init_circle();
            NXY = NUM_POINTS;
            clip(XCLIP, YCLIP,NCLIP,X, Y, &NXY);

            buffer();
            
            int q = G_wait_key();
            if (q == 'q') exit(0);
        }
    }
    else { // scale down
        while (radius > 45) {
            radius *= 0.9;

            printf("%lf\n", BOX_SIZE/radius);

            init_circle();
            NXY = NUM_POINTS;
            clip(XCLIP, YCLIP,NCLIP,X, Y, &NXY);

            buffer();

            int q = G_wait_key();
            if (q == 'q') exit(0);
        }
    }
}

void set_clipping_box() {
    int H = BOX_SIZE/2;

    XCLIP[0] = centerX - H; XCLIP[1] = centerX + H;
    XCLIP[2] = centerX + H; XCLIP[3] = centerX - H;

    YCLIP[0] = centerY - H; YCLIP[1] = centerY - H;
    YCLIP[2] = centerY + H; YCLIP[3] = centerY + H;
}

int main() {
    int key;
    int flag = 0; // Scale up
    set_clipping_box();

    G_init_graphics(800, 800);
    init_circle();

    while (1) {
        clear_screen();
        draw();

        key = G_wait_key();

        if (key == 'q') break; // Quit program
        if (flag == 0) flag = 1;
        else flag = 0;

        animate(flag);
    }

    return 0;
}


