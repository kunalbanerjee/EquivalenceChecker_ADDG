void main ()
{
	int A[100], In[100], B[100], C[100], N;
	int i, out;

	A[0] = In[0];

	for( i = 1; i < N; i++ ) {
		if( i%2 == 0 ) {
			B[i] = f( In[i] )  + 2;
			C[i] = g( A[i-1] ) + 2;
		}
		else {
			B[i] = g( A[i-1] ) + 2;
			C[i] = f( In[i] )  + 2;
		}
		A[i] = B[i] + C[i];
	}

	out = A[N-1];
}
