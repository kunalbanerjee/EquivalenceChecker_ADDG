void linear()
{
    long argument , k , l , i, kb5i;
    long sa[101], sb[101], b5[101], stb5[1];

    for ( l=1 ; l<=1000 ; l+=1 ){
        for ( k=0 ; k<101 ; k+=1 ){
            b5[k] = sa[k] + stb5[0]*sb[k];
        }
        for ( i=1 ; i<=101 ; i+=1 ){
            b5[101-i] = sa[101-i] + stb5[0]*sb[101-i];
        }
    }
}
