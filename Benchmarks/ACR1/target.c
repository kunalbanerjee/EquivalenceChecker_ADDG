int across()
{
	int a[100], b[100], c[100];
	int e[100];
	int t1[100], t2[100];
	int i;

	for(i = 2; i <= 50; i=i+1)
	{
		t2[i] = a[i-1] + c[i];
	}
	for(i = 2; i <= 50; i=i+1)
	{
		t1[i] = (b[i]+ c[i-1]) * 2;
	}
	for(i = 2; i <= 50; i=i+1)
	{
		e[i] = t2[i-2] + t1[i];
	}

	return 0;
}
