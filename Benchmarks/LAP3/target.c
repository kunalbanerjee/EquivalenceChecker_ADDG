void laplace()
{
  int i, j;
  int image1[100][100], image2[100][100], temp1[100][100], temp2[100][100];

  for (i=1; i<=50; i+=1) {
    for (j=1; j<100; j+=1) {
      temp1[i][j] = image1[i-1][j-1] - 2 * image1[i][j-1] + image1[i+1][j-1] - 2 * image1[i-1][j] + 4 * image1[i][j] +
                    image1[i-1][j+1] - 2 * (image1[i+1][j] + image1[i][j+1]) + image1[i+1][j+1];
    }
  }

 for (i=51; i<100; i+=1) {
    for (j=1; j<100; j+=1) {
      temp2[i][j] = image1[i-1][j-1] - 2 * image1[i][j-1] + image1[i+1][j-1] - 2 * image1[i-1][j] + 4 * image1[i][j] +
                    image1[i-1][j+1] - 2 * image1[i+1][j] - 2 * image1[i][j+1] + image1[i+1][j+1];
    }
  }

 for (i=1; i<=50; i+=1) {
    for (j=1; j<100; j+=1) {
      image2[i][j] = temp1[i][j];
    }
  }

  for (i=51; i<100; i+=1) {
    for (j=1; j<100; j+=1) {
      image2[i][j] = temp2[i][j];
    }
  }
}
