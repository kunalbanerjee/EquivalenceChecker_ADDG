void dct()
{
  int i, j, k;
  int cos1[2000][2000];
  int temp2d[2000][2000][2000], block[2000][2000], block2[2000][2000], sum2[2000][2000][2000];

  for (i = 0; i < 1024; i++)
    for (j = 0; j < 1024; j++)
    {
      temp2d[i][j][0] = 0;
      for (k = 0; k < 1024; k++)
        temp2d[i][j][k+1] = temp2d[i][j][k] + block[i][k] * cos1[j][k];
    }

  for (i = 0; i < 1024; i++)
    for (j = 0; j < 1024; j++)
    {
      sum2[i][j][0] = 0;
      for (k = 0; k < 1024; k++)
        sum2[i][j][k+1] = sum2[i][j][k] + cos1[i][k] * temp2d[k][j][1024];
    
      block2[i][j] = sum2[i][j][1024];
    }
}
