void lowpass()
{
   int i, j;
   int a1[100][100], a2[100][100], a3[100][100];
   int a4[100][100], a5[100][100], a6[100][100];
   int a7[100][100], a8[100][100], a9[100][100];
   int a[100][100];

   for (i=1; i<100; i+=1) {
     for (j=1; j<100; j+=1) {
        a[i][j] = 9 * a1[i-1][j-1] + a2[i-1][j]     + 2 * a3[i-1][j+1] +
                  3 * a4[i][j-1]   + 4 * a5[i][j]   + 5 * a6[i][j+1]   +
                  6 * a7[i+1][j-1] + 7 * a8[i+1][j] + 8 * a9[i+1][j+1] ;
     }
   }
 }
