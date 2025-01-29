#include "FPToolkit.c"

int main() {
    G_init_graphics(800,800);

    G_rgb(0,0,0);
    G_clear();
    G_rgb(1,0,0);
    G_fill_circle(400,400,100);
    G_wait_key();
}