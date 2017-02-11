void sor()
{
  long a[200][200], b[200][200], c[200][200], d[200][200], e[200][200], f[200][200], u[200][200];
  long resid[200][200];
  int j, l;
  long temp1[200][200],temp2[200][200],temp3[200][200];

  for (j=2; j<200; j+=1) {
    for (l=1; l<200; l+=2) {
      temp1[j][l] = a[j][l]*u[j+1][l] + b[j][l]*u[j-1][l];
    }
  }
  for (j=2; j<200; j+=1)
    for (l=1; l<200; l+=2) {
      temp2[j][l] = c[j][l]*u[j][l+1] + d[j][l]*u[j][l-1];
      temp3[j][l] = e[j][l]*u[j][l] - f[j][l];
    }

  for (j=2; j<200; j+=1) {
    for (l=1; l<200; l+=2) {
      resid[j][l] = temp1[j][l] + temp2[j][l] + temp3[j][l];
    }
  }
}
