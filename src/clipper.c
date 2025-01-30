// ARGS: double clip_windowX[], double clip_windowY[], int N_clip,
//       double clipX[], double clipY[], int *NXY)
// RETURN: void

// IMPORTANT NOTE: Use &NXY to get new count for clipped polygon

// All graphics have been commented out with FPToolkit removed
// To view clipping process:
//   - #include "FPToolkit.c"
//   - uncomment all G_ calls

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
// AB is the clipping window.
// XY is the polygon to be clipped.
// clip XY using line A[a],B[a] to A[b],B[b].
{
    double P[2], intersection[2] ;
    double aa[2], bb[2], xx[2], yy[2] ;
    int inout1, inout2, i, j ;

    double tempx[1000], tempy[1000] ;
    int ntemp = 0 ;

    aa[0] = A[a] ; aa[1] = B[a] ;
    bb[0] = A[b] ; bb[1] = B[b] ;

    // G_rgb(0,1,1) ;
    // G_polygon(A, B, NAB) ;

    // G_rgb(1,0,1) ;
    // G_line(A[a],B[a], A[b],B[b]) ;

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
            // G_rgb(0,1,0) ;
            // G_fill_circle(intersection[0], intersection[1], 3) ;
            tempx[ntemp] = intersection[0] ;
            tempy[ntemp] = intersection[1] ;
            ntemp++ ;

        } else if (inout1 == 0 && inout2 == 1) {
            // bad to good
            intersect_2_lines(aa,bb, xx,yy, intersection) ;
            // G_rgb(0,1,0) ;
            // G_fill_circle(intersection[0], intersection[1], 3) ;
            tempx[ntemp] = intersection[0] ;
            tempy[ntemp] = intersection[1] ;
            ntemp++ ;
            tempx[ntemp] = X[j] ;
            tempy[ntemp] = Y[j] ;
            ntemp++ ;

        }
    }

    // G_rgb(1,0,0) ;
    // G_polygon(X, Y, NXY) ;

    // G_rgb(0,1,1) ;
    // G_polygon(A, B, NAB) ;

    // G_rgb(0,1,0) ;
    // G_polygon(tempx, tempy, ntemp) ;

    for (i = 0 ; i < ntemp ; i++) {
        X[i] = tempx[i] ;
        Y[i] = tempy[i] ;
    }

    return ntemp ;
}


void clip(double clip_windowX[], double clip_windowY[], int N_clip,
          double clipX[], double clipY[], int *NXY) {

    for (int i = 0; i < N_clip; i++) {
        int j = (i + 1) % N_clip;
        *NXY = clip_line(clip_windowX, clip_windowY, N_clip,
                         clipX, clipY, *NXY, i, j);
        // to view clipping in action
        // G_wait_key()
    }

}

