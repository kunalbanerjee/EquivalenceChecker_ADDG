int main()
{
   int i, j;
   int N, M;
   static int image[1000][1000];
   static int Jx[1000][1000];
   static int Jy[1000][1000];
   static int av[1000][1000];


    for (j=2; j < M; j+=1) {
      for (i=2; i < N; i+=1) {
           Jx[j][i] = image[j-1][i+1] + 2 * image[j][i+1] + image[j+1][i+1] - image[j-1][i-1] - 2 * image[j][i-1] - image[j+1][i-1];
      }
   }


   for (j=2; j < M; j+=1) {
      for (i=2; i < N; i+=1) {
	    Jy[j][i] = image[j+1][i-1] + 2 * image[j+1][i] + image[j+1][i+1] - image[j-1][i-1] - 2 * image[j-1][i] - image[j-1][i+1];
      }
   }

   for (j=2; j < M; j+=1) {
      for (i=2; i < N; i+=1) {
             av[j][i] = 4 * Jx[j][i] + 4 * Jy[j][i];
      }
   }

   return 0;
}

