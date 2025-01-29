#include <stdio.h>
#include <math.h>

/*

 ( x')          (x)
 ( y')  =   M * (y)
 ( 1 )          (1)

instead of (x',y',1) = (x,y,1) * M

*/

int M2d_print_mat(double a[3][3]) {
    int r, c;
    for (r = 0; r < 3; r++) {
        for (c = 0; c < 3; c++) {
            printf(" %12.4lf ", a[r][c]);
        }
        printf("\n");
    }

    return 1;
}

int M2d_copy_mat(double a[3][3], double b[3][3]) {
    // a = b
    int r, c;
    for (r = 0; r < 3; r++) {
        for (c = 0; c < 3; c++) {
            a[r][c] = b[r][c];
        }
    }

    return 1;
}

int M2d_make_identity(double a[3][3]) {
    // a = I
    int r, c;
    for (r = 0; r < 3; r++) {
        for (c = 0; c < 3; c++) {
            if (r == c)
                a[r][c] = 1.0;
            else
                a[r][c] = 0.0;
        }
    }

    return 1;
}

int M2d_make_translation(double a[3][3], double dx, double dy) {
    M2d_make_identity(a);
    a[0][2] = dx;
    a[1][2] = dy;
    return 1;
}

int M2d_make_scaling(double a[3][3], double sx, double sy) {
    M2d_make_identity(a);
    a[0][0] = sx;
    a[1][1] = sy;
    return 1;
}

int M2d_make_rotation_cs(double a[3][3], double cs, double sn) {
    // This one assumes cosine and sine are already known
    M2d_make_identity(a);
    a[0][0] = cs;
    a[0][1] = -sn;
    a[1][0] = sn;
    a[1][1] = cs;

    return 1;
}

int M2d_make_rotation(double a[3][3], double theta) {
    // Generate rotation matrix using angle theta (in radians)
    double cs = cos(theta);
    double sn = sin(theta);
    return M2d_make_rotation_cs(a, cs, sn);
}

int M2d_mat_mult(double res[3][3], double a[3][3], double b[3][3]) {
    // res = a * b
    // This is SAFE, i.e., the user can call M2d_mat_mult(p, p, q) or similar
    double sum;
    int r, c, k;
    double tmp[3][3];

    for (r = 0; r < 3; r++) {
        for (c = 0; c < 3; c++) {
            sum = 0.0;
            for (k = 0; k < 3; k++) {
                sum += a[r][k] * b[k][c];
            }
            tmp[r][c] = sum;
        }
    }

    M2d_copy_mat(res, tmp);

    return 1;
}

int M2d_mat_mult_pt(double P[2], double m[3][3], double Q[2]) {
    // P = m * Q
    // SAFE, user may call M2d_mat_mult_pt(W, m, W);
    double u, v;

    u = m[0][0] * Q[0] + m[0][1] * Q[1] + m[0][2];
    v = m[1][0] * Q[0] + m[1][1] * Q[1] + m[1][2];

    P[0] = u;
    P[1] = v;

    return 1;
}

int M2d_mat_mult_points(double *X, double *Y, double m[3][3], double *x, double *y, int numpoints) {
    // |X0 X1 X2 ...|       |x0 x1 x2 ...|
    // |Y0 Y1 Y2 ...| = m * |y0 y1 y2 ...|
    // | 1  1  1 ...|       | 1  1  1 ...|

    // SAFE, user may call M2d_mat_mult_points(X, Y, m, X, Y, n);
    double u, v;
    int i;

    for (i = 0; i < numpoints; i++) {
        u = m[0][0] * x[i] + m[0][1] * y[i] + m[0][2];
        v = m[1][0] * x[i] + m[1][1] * y[i] + m[1][2];

        X[i] = u;
        Y[i] = v;
    }

    return 1;
}
