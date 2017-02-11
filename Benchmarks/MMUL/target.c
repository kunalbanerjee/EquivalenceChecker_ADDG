void matrix_multiplication()
{
  int i, j, k;
  int ii, jj, kk;
  int A[20][20], B[20][20];
  int E[20][20][21];

  for(i = 0; i < 20; i++)
    for(j = 0; j < 20; j++)
      E[i][j][0] = 0;

  for(i = 0; i < 20; i+=5)
    for(k = 0; k < 20; k+=5)
      for(j = 0; j < 20; j+=5)
        for(ii = i; ii < i+5; ii++)
          for(kk = k; kk < k+5; kk++)
            for(jj = j; jj < j+5; jj++) 
              E[ii][jj][kk+1] = E[ii][jj][kk] + A[ii][kk]*B[kk][jj];
}
