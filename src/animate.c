#include "FPToolkit.c"
#include "M2d_matrix_tools.c"

#define NUM_POINTS 50
#define BOX_SIZE 100

int NXY = NUM_POINTS;
double X[1000], Y[1000];
double radius = 45.0; // Radius of the circle
double centerX = 400.0, centerY = 400.0; // Center of the circle

int clip_size = 80;
int NCLIP = 4;
double XCLIP[4];
double YCLIP[4];

int grid[100][100];
int modules;

int in_out (double x[], double y[], int n, double P[2], int I, int J)
// return 1 if point P is on the correct side of the line
// using the convex polygon to find center mass
// else return 0
{
    int i ;
    double MX = 0, MY = 0 ;
    for (i = 0 ; i < n ; i++) {
        MX += x[i] ; MY += y[i] ;
    }

    MX /= n ; MY /= n ;

    double a = (y[J] - y[I]);
    double b = (x[J] - x[I]);
    double c = (x[J] - x[I]) * y[I] - (y[J] - y[I]) * x[I] ;

    double signM = a * MX - b * MY + c ;
    double signP = a * P[0] - b * P[1] + c ;

    if (signM > 0 && signP < 0) return 0 ;
    if (signM < 0 && signP > 0) return 0 ;

    return 1 ;
}


int intersect_2_lines (double A[2], double B[2],
                       double C[2], double D[2],
                       double intersection[2])
// return 0 if lines do NOT intersect
// return 1 if they do
{
    double m1, m2, b1, b2 ;

    if (A[0] == B[0] && C[0] == D[0]) return 0; // both lines are vertical

    if (A[0] == B[0]) { // line AB is vertical

        intersection[0] = A[0];
        m2 = (D[1] - C[1]) / (D[0] - C[0]);
        b2 = C[1] - m2 * C[0];

        intersection[1] = m2 * intersection[0] + b2;

    } else if (C[0] == D[0]) { // line CD is vertical

        intersection[0] = C[0];
        m1 = (B[1] - A[1]) / (B[0] - A[0]);
        b1 = A[1] - m1 * A[0];

        intersection[1] = m1 * intersection[0] + b1;

    } else { // normal lines

        m1 = (B[1] - A[1]) / (B[0] - A[0]);
        m2 = (D[1] - C[1]) / (D[0] - C[0]);

        if (m1 == m2) return 0; // lines are parallel

        b1 = A[1] - m1 * A[0];
        b2 = C[1] - m2 * C[0];

        intersection[0] = (b2 - b1) / (m1 - m2);
        intersection[1] = m1 * intersection[0] + b1;
    }

    return 1;
}


int clip_line(double A[], double B[], int NAB,
              double X[], double Y[], int NXY,
              int a, int b)
// AB is the clipping polygon
// XY is the original polygon
// clip XY using line A[a],B[a] to A[b],B[b]
{
    double P[2], intersection[2] ;
    double aa[2], bb[2], xx[2], yy[2] ;
    int inout1, inout2, i, j ;

    double tempx[1000], tempy[1000] ;
    int ntemp = 0 ;

    aa[0] = A[a] ; aa[1] = B[a] ;
    bb[0] = A[b] ; bb[1] = B[b] ;

    G_rgb(0,1,1) ;
    G_polygon(A, B, NAB) ;

    G_rgb(1,0,1) ;
    G_line(A[a],B[a], A[b],B[b]) ;

    for (i = 0 ; i < NXY; i++) {

        j = (i + 1) % NXY ;

        xx[0] = X[i] ; xx[1] = Y[i] ;
        yy[0] = X[j] ; yy[1] = Y[j] ;

        P[0] = X[i] ; P[1] = Y[i] ;
        inout1 = in_out(A, B, NAB, P, a, b) ;

        P[0] = X[j] ; P[1] = Y[j] ;
        inout2 = in_out(A, B, NAB, P, a, b) ;

        if (inout1 == 1 && inout2 == 1) {
            // good to good
            tempx[ntemp] = X[j] ;
            tempy[ntemp] = Y[j] ;
            ntemp++ ;

        } else if (inout1 == 1 && inout2 == 0) {
            // good to bad
            intersect_2_lines(aa,bb, xx,yy, intersection) ;
            G_rgb(0,1,0) ;
            G_fill_circle(intersection[0], intersection[1], 3) ;
            tempx[ntemp] = intersection[0] ;
            tempy[ntemp] = intersection[1] ;
            ntemp++ ;

        } else if (inout1 == 0 && inout2 == 1) {
            // bad to good
            intersect_2_lines(aa,bb, xx,yy, intersection) ;
            G_rgb(0,1,0) ;
            G_fill_circle(intersection[0], intersection[1], 3) ;
            tempx[ntemp] = intersection[0] ;
            tempy[ntemp] = intersection[1] ;
            ntemp++ ;
            tempx[ntemp] = X[j] ;
            tempy[ntemp] = Y[j] ;
            ntemp++ ;

        }
    }

    G_rgb(1,0,0) ;
    G_polygon(X, Y, NXY) ;

    G_rgb(0,1,1) ;
    G_polygon(A, B, NAB) ;

    G_rgb(0,1,0) ;
    G_polygon(tempx, tempy, ntemp) ;

    for (i = 0 ; i < ntemp ; i++) {
        X[i] = tempx[i] ;
        Y[i] = tempy[i] ;
    }

    return ntemp ;
}


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


void scale_circle(double scaleFactor) {
    double A[3][3], B[3][3], C[3][3];

    // Translate to origin
    M2d_make_translation(A, -centerX, -centerY);
    M2d_mat_mult_points(X, Y, A, X, Y, NUM_POINTS);

    // Scale
    M2d_make_scaling(B, scaleFactor, scaleFactor);
    M2d_mat_mult_points(X, Y, B, X, Y, NUM_POINTS);

    // Translate back to original position
    M2d_make_translation(C, centerX, centerY);
    M2d_mat_mult_points(X, Y, C, X, Y, NUM_POINTS);
}


void draw() {
    G_rgb(1,0,0);
    G_fill_polygon(X, Y, NXY);
    G_rgb(1,1,1);
    G_polygon(XCLIP, YCLIP, NCLIP);
}


void animate(int flag) {
    int key;
    if (flag == 1) { // scale up
        while (radius < clip_size) {
            //scale_circle(1.1);
            radius *= 1.1;

            init_circle();
            NXY = NUM_POINTS;
            for (int i = 0; i < NCLIP; i++) {
                int j = (i + 1) % NCLIP;
                NXY = clip_line(XCLIP, YCLIP, NCLIP, X, Y, NXY, i, j);
            }
            clear_screen();
            draw();

            G_display_image();
            usleep(40000);
        }
    }
    else { // scale down
        while (radius > 45) {


            //scale_circle(0.9);
            radius *= 0.9;

            init_circle();
            NXY = NUM_POINTS;
            for (int i = 0; i < NCLIP; i++) {
                int j = (i + 1) % NCLIP;
                NXY = clip_line(XCLIP, YCLIP, NCLIP, X, Y, NXY, i, j);
            }

            clear_screen();
            draw();

            G_display_image();
            usleep(40000);
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

