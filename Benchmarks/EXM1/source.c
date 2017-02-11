void main ()
{
	int A[100], In[100], N;
	int i, out;

	A[0] = In[0];

	for( i = 1; i < N; i=i+1 ){
		A[i] = f( In[i] ) + g( A[i-1] );
	}

	out = A[N-1];
}
