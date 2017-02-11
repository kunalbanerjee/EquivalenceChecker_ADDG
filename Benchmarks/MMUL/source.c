void matrix_multiplication()
{
  int i, j, k;
  int A[20][20], B[20][20]; 
  int E[20][20][21];

  for(i = 0; i < 20; i++)
    for(j = 0; j < 20; j++)
    {
      E[i][j][0] = 0;
      for(k = 0; k < 20; k++)
        E[i][j][k+1] = E[i][j][k] + A[i][k]*B[k][j];
    }
}
