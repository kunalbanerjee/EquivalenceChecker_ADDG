void wave()
{
  int i, j, filter;
  int wksp[200], a[100], C0, C1, C2, C3;

  if (filter == 0) {
    for (i=0,j=0; j<48; j+=2, i+=1) {
      wksp[i] = 6*a[j+1] + 5*a[j] + 7*a[j+2] + 8*a[j+3];
      wksp[i+100] = 8*a[j] - 7*a[j+1] + 6*a[j+2] - 5*a[j+3];
    }
  }
  else {
    for (i=0,j=0; j<48; j+=2, i+=1) {
      wksp[i] = 5*a[j+2] + 6*a[j+3] + 7*a[j+4] + 8*a[j+5];
      wksp[i+100] = 8*a[j+2] - 7*a[j+3] + 6*a[j+4] - 5*a[j+5];
    }
  }
}
