void wave()
{
  int i, j, filter;
  int wksp[200], a[100];

    for (i=0,j=0; j<48; j+=2, i+=1) {
		if(filter == 0)
		{
      		wksp[i] = 5*a[j] + 6*a[j+1] + 7*a[j+2] + 8*a[j+3];
      		wksp[i+100] = 8*a[j] - 7*a[j+1] + 6*a[j+2] - 5*a[j+3];
		}
		else
		{
			wksp[i] = 5*a[j+2] + 6*a[j+3] + 7*a[j+4] + 8*a[j+5];
      		wksp[i+100] = 8*a[j+2] - 7*a[j+3] + 6*a[j+4] - 5*a[j+5];
		}
    }
}
