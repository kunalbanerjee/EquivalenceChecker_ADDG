void main ()
{
	int A[100][100], In1[100][100], In2[100][100], S[100], N;
	int i, j, out;

	for(i = 0; i < N; i++)
		A[i][0] = In1[0] - In2[0];

	for( i = 0; i < N; i=i+1 ){
		for( j = 1; j < N; j=j+1 ){
			A[i][j] = A[i][j-1] + In1[j] - In2[j];
		}
	}

	S[0] = A[0][N-1];
	for( i = 1; i < N; i++ )
		S[i] = S[i-1] + A[i][N-1];

	out = S[N-1];
}
