void fdtd()
{
  int t, i, j;
  int In1[3000], In2[3000];
  int hz[3000][3000][3000], hz2[3000][3000][3000], ex[3000][3000][3000], ey[3000][3000][3000];
  int temp1[3000][3000][3000], temp2[3000][3000][3000];

  for(t=0; t<128; t++)
  {
    for (j=0; j<2048; j++)
    {
      ey[t][0][j] = In1[j];
      ex[t][j][0] = In2[j];
    }

    for (i=1; i<2048; i++)
      for (j=0; j<2048; j++)
        ey[t][i][j] = ey[t][i-1][j] - (hz[t][i][j]-hz[t][i-1][j]) / 2;

    for (i=0; i<2048; i++)
      for (j=1; j<2048; j++)
        ex[t][i][j] = ex[t][i][j-1] - (hz[t][i][j]-hz[t][i][j-1]) / 2;

    for (i=0; i<2048; i++)
      for (j=0; j<2048; j++)
      {
        temp1[t][i][j] = ex[t][i][j+1]-ex[t][i][j];
        temp2[t][i][j] = ey[t][i+1][j]-ey[t][i][j];
        hz2[t][i][j] = hz[t][i][j] - (temp1[t][i][j] + temp2[t][i][j]) * 7/10;
      }
  }
}
