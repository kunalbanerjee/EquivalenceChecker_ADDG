//int f(int);
//int g(int);

void nonuniform()
{
  int i, N, in, out, A[100];
  
  A[0] = in;
  for(i = 1; i <= N ; ++i)
  {
    A[i] = f(g(A[i/2]));
  }
  out = g(A[N]);
}
