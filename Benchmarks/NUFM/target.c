//int f(int);
//int g(int);

void nonuniform()
{
  int i, N, in, out, A[100];
  
  A[0] = g(in);
  for(i = 1; i <= N ; ++i)
  {
    A[i] = g(f(A[i/2]));
  }
  out = A[N];
}
