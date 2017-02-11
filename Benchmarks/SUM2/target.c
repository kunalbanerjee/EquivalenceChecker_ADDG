void main ()
{
	int A[3][100], In1[100], In2[100], In3[100], N;
	int temp1[100][100], temp2[100][100];
	int i, j, out;

	for(i = 0; i < N; i++)
		A[i][0] = In1[0] + In2[0] * (-1);

	for( i = N-1; i >= 0; i=i-1 ){
		for( j = 1; j < N; j=j+1 ){
			temp1[i][j]= In2[j] * (-1) + In1[j];
			temp2[i][j] = temp1[i][j] + A[i][j-1];
			A[i][j] = temp2[i][j];
		}
	}

	S[0] = A[0][N-1];
	for( i = 1; i < N; i++ )
		S[i] = A[i][N-1] + S[i-1];

	out = S[N-1];
}
