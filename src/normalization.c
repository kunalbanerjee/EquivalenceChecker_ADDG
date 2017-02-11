#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "normalization.h"


extern var_list V0, V1, V0_V1, V1_minus_V0_V1;
extern SYMTAB stab;


//This function calculates the list of variables which are present in both
//var_list 'V0' and var_list 'V1' in var_list 'V0_V1'.
void cal_V0_intersection_V1( var_list *V0_V1 )
{
	int i, j;
	for( i = 0; i < V0.no_of_elements; i++ )
	{
		for( j = 0; j < V1.no_of_elements; j++ )
		{
			if( V0.var_val[i] == V1.var_val[j] )
			{
				V0_V1->var_val[V0_V1->no_of_elements] = V0.var_val[i];
				V0_V1->var_string[V0_V1->no_of_elements] = ( char * ) malloc( strlen( V0.var_string[i] ) +1 );
				strcpy( V0_V1->var_string[V0_V1->no_of_elements], V0.var_string[i] );
				V0_V1->no_of_elements++;
			}
		}
	}
}



//This function checks if a variable 'var' is present in var_list 'V0_V1'.
//It returns '1' if the variable is present , else returns '-1'.
int var_in_V0_V1( int var )
{
	int i;
	for( i = 0; i < V0_V1.no_of_elements; i++ )
	{
		if( var == V0_V1.var_val[i] )
			return 1;
	}
	return -1;
}



//This function checks if a variable 'var' is present in var_list 'V1_minus_V0_intersection_V1'.
//It returns '1' if the variable is present , else returns '-1'.
int var_in_V1_minus_V0_intersection_V1( int var )
{
	int i;
	for( i = 0; i < V1_minus_V0_V1.no_of_elements; i++ )
	{
		if( var == V1_minus_V0_V1.var_val[i] )
			return 1;
	}
	return -1;
}



void cal_V1_minus_V0_intersection_V1( var_list *V1_minus_V0_V1 )
{
	int i, j, flag;
	for( i = 0; i < V1.no_of_elements; i++ )
	{
		flag = 0;
		for( j = 0; j < V0_V1.no_of_elements; j++ )
		{
			if( V1.var_val[i] == V0_V1.var_val[j] )
			{
				flag = 1;
				break;
			}
		}
		if( flag == 0 )
		{
			V1_minus_V0_V1->var_val[V1_minus_V0_V1->no_of_elements] = V1.var_val[i];
			V1_minus_V0_V1->var_string[V1_minus_V0_V1->no_of_elements] = ( char * ) malloc( strlen( V1.var_string[i] ) +1 );
			strcpy( V1_minus_V0_V1->var_string[V1_minus_V0_V1->no_of_elements], V1.var_string[i] );
			V1_minus_V0_V1->no_of_elements++;
		}
	}//end for
}



//This function is used to create a copy of the list structure
NC* copylist( NC *source )
{
	NC *temp;
	if(source != NULL)
	{
		temp=(NC *)malloc(sizeof(NC));
		temp->type = source->type;
		temp->inc  = source->inc;
		temp->link = (NC *)malloc(sizeof(NC));
		temp->link = copylist(source->link);
		temp->list = (NC *)malloc(sizeof(NC));
		temp->list = copylist(source->list);
		return temp;
	}
	return NULL;
}



//This function is used to print the normalized expressions
//It will take the root of the binary tree as input and print the corresponding expression
void write_lists( NC *root )
{
	char *sym_value;
	sym_value = (char*) malloc (100 * sizeof(char));
	if( root != NULL )
	{
		if(root->type == 0) root->type = 'S';
		if( root->type == 'R' || root->type == 'O' )
		{
			if( root->type == 'R' )
				printf( " ( " );
			write_lists( root->link );
		}

		switch( root->type )
		{
			case 'f':
				symbol_for_index( root->inc, sym_value );
				printf( "* %s( ", sym_value );
				break;
			case 'v':
				symbol_for_index( root->inc, sym_value );
				printf( "* %s ", sym_value );
				break;
			case 'T':
				printf( "%c %d ", ( root->inc >= 0 )?'+':'-', abs( root->inc ) );
				break;
			case 'S':
				printf( "%d ", root->inc );
				break;
			case 'R':
				switch( root->inc )
				{
					case 0: printf( ">= 0" ); break;
					case 1: printf( "> 0"  ); break;
					case 2: printf( "<= 0" ); break;
					case 3: printf( "< 0"  ); break;
					case 4: printf( "== 0" ); break;
					case 5: printf( "!= 0" ); break;
				}; // switch( root->inc )
				printf( " ) " );
				if( root->list != NULL )
					printf( " OR " );
				break;
			case 'A':
				break;
			case 'O':
				if( root->list != NULL )
					printf( " AND " );
				break;
			case 'D':
				printf( " * ( /   " );
				break;
			case 'M':
				printf( " * (%%   " );
				break;
			case 'w':
				printf( "write ( " );
				break;
			case 'y':
				symbol_for_index(root->inc, sym_value);
				printf( "%s, ", sym_value );
				break;
			case 'a':
				symbol_for_index(root->inc, sym_value);
				printf( "* read ( %s, ", sym_value );
				break;
			default:
				break;
		}; // switch( root->type )

		if( root->type != 'R' && root->type != 'O' )
			write_lists( root->link );
		if( root->type == 'f' || root->type == 'w') //2nd clause is for arrays
			printf( ")" );
		if(root->type == 'y')
			printf(", ");
		//A closing bracket has to be put explicitly in case the arrays
		//are of only one dimension
		if(root->type == 'a' && root->link->list == NULL)
			printf( ")" );

		if( root->type == 'S' && root->list != NULL )
		{
			printf( ", " );
			write_lists( root->list );
			if(root->list->list == NULL)
				printf(")");
			return;
		}
		write_lists( root->list );
	}
	return;
}



//This function is used to find the integer value represented by a string
int constval( char *s )
{
	int i, val = 0;
	if( s[0] != '-' )
	{
		for( i = 0; s[i] != '\0'; i++ )
			val = val * 10 + s[i] - 48;
	} // calculating constant value
	else
	{
		for( i = 1; s[i] != '\0'; i++ )
			val = val * 10 + s[i] - 48;
		val = -1 * val;
	}
	return val;
}



//This function is used to find the index value of a symbol in a symbol table
//If the symbol is already in the table then it will return the index  value
//otherwise it will add the symbol into the symbol table
//if the string is null then it will return -1
int indexof_symtab( char *symbol )
{
	int i;
	if( strcmp( symbol, "\0" ) )  // if the symbol is null string then return -1
	{
		for( i = 0; i < stab.numsymbols; i++ )
			if( !strcmp( stab.sym_tab[i], symbol ) )
			{
				return stab.val_tab[i];
			}
		i = stab.numsymbols++; // symbol not found in the symbol table
		stab.sym_tab[i] = ( char * ) malloc( strlen( symbol ) +1 );
		strcpy( stab.sym_tab[i], symbol );//copy the symbol into the symbol table
		stab.val_tab[i] = constval( symbol );
		return stab.val_tab[i];
	}
	return -1;
}


//This function is used to find the index value of a symbol in a var_list.
//var_list is the list of variable which are present in the FSMD's.
//If the symbol is already in the var_list then it will return the index value
//otherwise it will add the symbol into the var_list
//if the string is null then it will return -1
int indexof_varlist( char *symbol, var_list *V )
{
	int i;
	if( strcmp( symbol, "\0" ) )
	{ // if the symbol is null string then return -1
		for( i = 0; i < V->no_of_elements; i++ )
		if( !strcmp( V->var_string[i], symbol ) )
		{
			return 1;
		}

		i = V->no_of_elements++; // symbol not found in the symbol table
		V->var_string[i] = ( char * ) malloc( strlen( symbol ) +1 );
		strcpy( V->var_string[i], symbol );//copy the symbol into the symbol table
		V->var_val[i] = constval( symbol );
		return 2;
	}
	return -1;
}



//This function returns the string value of an index.
//This index is the integer value of a variable present in the FSMD.
void symbol_for_index( int n , char *sym_value )
{
	int i;
	for( i = 0; i < stab.numsymbols; i++ )
	{
		if( stab.val_tab[i] == n )
		{
			strcpy( sym_value, stab.sym_tab[i] );
		}
	}
}



//This function checks whether two normalized expressions are equivalent or not
int compare_trees( NC *t1, NC *t2 )
{
	if(t1!=NULL&&t2!=NULL)
	{
		//KB start
		//if(t1->type == 0) t1->type = 'S';
		//if(t2->type == 0) t2->type = 'S';
		//KB end

		if(t1->type==t2->type&&t1->inc==t2->inc)
			return compare_trees(t1->link,t2->link)&compare_trees(t1->list,t2->list);
	}
	else if(t1==NULL && t2==NULL)
		return 1;

	return 0;
}



//This function adds two normalized sums and returns a normalized sum
N_sum* Add_Sums( N_sum *sum1, N_sum *sum2, N_sum *add_result )
{
	if( sum1 == NULL && sum2 == NULL )
		return NULL;
	if( sum1 == NULL && sum2 != NULL )
	{
		add_result = copylist( sum2 );
		add_result->list = NULL;
	}
	if( sum1 != NULL && sum2 == NULL )
	{
		add_result = copylist( sum1 );
		add_result->list = NULL;
	}
	if( sum1 != NULL && sum2 != NULL )
	{
		add_result->type = 'S';
		add_result->inc = sum1->inc + sum2->inc;
		add_result->list = NULL;
		add_result->link = Add_Sums_1( sum1->link, sum2->link, add_result->link );
	}
	return add_result;
}



//This function works on the suffixes s1 and s2 of two  normalized sums
//to return their resultant normalized sum.
//This function adds the terms of the normalized sum s1 with the
//terms of the normalized sum s2 and returns the resultant normalized sum.
N_sum* Add_Sums_1( N_sum *sum1, N_sum *sum2, N_sum *add_result )
{
	int m;

	if( sum1 == NULL && sum2 == NULL )
		return NULL;
	if( sum1 == NULL && sum2 != NULL )
	{
		add_result = copylist( sum2 );
		add_result->type = 'T';
		add_result->list = Add_Sums_1( NULL, sum2->list, add_result->list );
	}
	if( sum1 != NULL && sum2 == NULL )
	{
		add_result = copylist( sum1 );
		add_result->type = 'T';
		add_result->list = Add_Sums_1( sum1->list, NULL, add_result->list );
	}

	if( sum1 != NULL && sum2 != NULL )
	{
		m = Compare_Terms( sum1->link, sum2->link );

		if( m == 0 )
		{
			if( sum1->inc + sum2->inc != 0 )
			{
				add_result = copylist( sum2 );
				add_result->type = 'T';
				add_result->inc = sum1->inc + sum2->inc;
				add_result->list = Add_Sums_1( sum1->list, sum2->list, add_result->list );
			}
			else
			{
				add_result = Add_Sums_1( sum1->list, sum2->list, add_result );
			}
		}
		else if( m < 0 )
		{
			add_result = copylist( sum1 );
			add_result->type = 'T';
			add_result->list = Add_Sums_1( sum1->list, sum2, add_result->list );
		}
		else
		{//if( m > 0 )
			add_result = copylist( sum2 );
			add_result->type = 'T';
			add_result->list = Add_Sums_1( sum1, sum2->list, add_result->list );
		}
	}
	return add_result;
}



//This function compares the two terms in the style of string compare
//Here t1 is the pointer to the first primary and t2 is the pointer to the
//first primary of the term t1 and t2 respectively
int Compare_Terms( N_term *t1, N_term *t2 )
{
	int v;

	if( t1 == NULL && t2 == NULL )
		return 0;
	if( t1 == NULL && t2 != NULL )
		return -1;
	if( t1 != NULL && t2 == NULL )
		return 1;

	v = Compare_Primaries( t1, t2 );
	if( v == 0 )
		v = Compare_Terms( t1->list, t2->list );
	else if( v < 0 )
		return -1;
	else
		return 1;

	return v;
}



//This function compares the two primaries in the style of string compare
int Compare_Primaries( N_primary *primary1, N_primary *primary2 )
{
	int m, m1, m2;

	//updating Compare_Primaries
	//with precedence order D,M > w,a > v
	if( ( primary1->type == 'D' || primary1->type == 'M' ) && ( primary2->type == 'D' || primary2->type == 'M' ) )
	{
		if( primary1->type == primary2->type )
		{
			m1 = Compare_Sums( primary1->link->link, primary2->link->link );
			m2 = Compare_Sums( primary1->link->list->link, primary2->link->list->link );

			if( m1 == 0 && ( primary1->link->inc ==  primary2->link->inc ) )
				m1 = 0;
			else if( m1 == 0 && ( primary1->link->inc <  primary2->link->inc ) )
				m1 = -1;
			else if( m1 == 0 && ( primary1->link->inc >  primary2->link->inc ) )
				m1 =  1;

			if( m2 == 0 && ( primary1->link->list->inc ==  primary2->link->list->inc ) )
				m2 = 0;
			else if( m2 == 0 &&  ( primary1->link->list->inc <  primary2->link->list->inc ) )
				m2 = -1;
			else if( m2 == 0 && ( primary1->link->list->inc >  primary2->link->list->inc ) )
				m2 =  1;

			switch( m1 )
			{
				case 0:
					if( m2 == 0 )
						m = 0;
					else if( m2 == -1 )
						m = -1;
					else if( m2 == 1 )
						m = 1;
					break;
				case -1:
					m = -1;
					break;
				case 1:
					m = 1;
					break;
			}
			return m;
		}
		else if( primary1->type == 'D' )
			return 1;
		else
			return -1;
	}
	else if( ( primary1->type == 'D' || primary1->type == 'M' ) )
	{
		return 1;
	}
	else if( ( primary1->type == 'w' ) && ( primary2->type == 'D' || primary2->type == 'M' ) )
	{
		return -1;
	}
	else if( primary1->type == 'w' && primary2->type == 'w' )
	{
		m1 = Compare_Sums( primary1->link->link, primary2->link->link );
		m2 = Compare_Sums( primary1->link->list->link, primary2->link->list->link );

		if( m1 == 0 && ( primary1->link->inc ==  primary2->link->inc ) )
			m1 = 0;
		else if( m1 == 0 && ( primary1->link->inc <  primary2->link->inc ) )
			m1 = -1;
		else if( m1 == 0 && ( primary1->link->inc >  primary2->link->inc ) )
			m1 =  1;

		if( m2 == 0 && ( primary1->link->list->inc ==  primary2->link->list->inc ) )
			m2 = 0;
		else if( m2 == 0 && ( primary1->link->list->inc <  primary2->link->list->inc ) )
			m2 = -1;
		else if( m2 == 0 && ( primary1->link->list->inc >  primary2->link->list->inc ) )
			m2 =  1;

		switch( m1 )
		{
			case 0:
				if( m2 == 0 )
					m = 0;
				else if( m2 == -1 )
					m = -1;
				else if( m2 == 1 )
					m = 1;
				break;
			case -1:
				m = -1;
				break;
			case 1:
				m = 1;
				break;
		}
		return m;
	}
	else if( primary1->type == 'a' && primary2->type == 'a' )
	{
		if ( primary1->inc < primary2->inc )
			return -1;
		else if( primary1->inc > primary2->inc )
			return 1;

		m = Compare_Sums( primary1->link, primary2->link );

		return m;
	}
	else if( primary1->type == 'a' && primary2->type == 'w' )
	{
		if ( primary1->inc < primary2->link->inc )
			return -1;
		else if( primary1->inc > primary2->link->inc )
			return 1;

		m = Compare_Sums( primary1->link, primary2->link->link );

		return m;
	}
	else if( primary1->type == 'w' && primary2->type == 'a' )
	{
		if ( primary1->link->inc < primary2->inc )
			return -1;
		else if( primary1->link->inc > primary2->inc )
			return 1;

		m = Compare_Sums( primary1->link->link, primary2->link );

		return m;
	}
	else if( primary1->type == 'w' && primary2->type == 'v' )
	{
		return 1;
	}
	else if( primary1->type == 'v' && primary2->type == 'w' )
	{
		return -1;
	}
	else if( primary1->type == 'a' && primary2->type == 'v' )
	{
		return 1;
	}
	else if( primary1->type == 'v' && primary2->type == 'a' )
	{
		return -1;
	}
	else //( primary1->type == 'v' ) && ( primary2->type == 'v' )
	{
		if( primary1->inc == primary2->inc )
			return 0;
		else if ( primary1->inc < primary2->inc )
			return -1;
		else
			return 1;
	}
}



//This function multiplies a normalized term 't1' with another normalized
//term 't2' and returns the resultant normalized term.
N_term* Mult_Term_With_Term( N_term *t1, N_term *t2, N_term *mult_TT )
{
	if( t1 == NULL && t2 == NULL )
		return NULL;
	if( t1 == NULL && t2 != NULL )
	{
		mult_TT->inc = t2->inc;
		mult_TT->link = ( NC * )malloc( sizeof( NC ) );
		mult_TT->link = Mult_Term_With_Term_1( NULL, t2->link, mult_TT->link);
	}
	if( t1 != NULL && t2 == NULL )
	{
		mult_TT->inc = t1->inc;
		mult_TT->link = ( NC * )malloc( sizeof( NC ) );
		mult_TT->link = Mult_Term_With_Term_1( t1->link, NULL, mult_TT->link);
	}
	if( t1 != NULL && t2 != NULL )
	{
		mult_TT->inc = t1->inc * t2->inc;
		mult_TT->link = ( NC * )malloc( sizeof( NC ) );
		mult_TT->link = Mult_Term_With_Term_1( t1->link, t2->link, mult_TT->link);
	}
	return mult_TT;
}



//This function multiplies all the primaries of a normalized term 't1'
//with all the primaries of another normalized  term 't2' and returns
//the resultant normalized term.
N_term* Mult_Term_With_Term_1( N_term *t1, N_term *t2, N_term *mult_TT )
{
	int v;

	if( t1 == NULL && t2 == NULL )
		return NULL;
	if( t1 == NULL && t2 != NULL )
	{
		mult_TT = copylist( t2 );
		mult_TT->list = ( NC * )malloc( sizeof( NC ) );
		mult_TT->list = Mult_Term_With_Term_1( NULL, t2->list, mult_TT->list);
	}
	if( t1 != NULL && t2 == NULL )
	{
		mult_TT = copylist( t1 );
		mult_TT->list = ( NC * )malloc( sizeof( NC ) );
		mult_TT->list = Mult_Term_With_Term_1( t1->list, NULL, mult_TT->list);
	}
	if( t1 != NULL && t2 != NULL )
	{
		v = Compare_Primaries( t1, t2 );
		if( v <= 0 )
		{
			mult_TT = copylist( t1 );
			mult_TT->list = ( NC * )malloc( sizeof( NC ) );
			mult_TT->list = Mult_Term_With_Term_1( t1->list, t2, mult_TT->list );
		}
		else
		{
			mult_TT = copylist( t2 );
			mult_TT->list = ( NC * )malloc( sizeof( NC ) );
			mult_TT->list = Mult_Term_With_Term_1( t1, t2->list, mult_TT->list );
		}
	}
	return mult_TT;
}



//This function multiplies a normalized sum with a normalized term and
//returns  the resultant normalized sum.
N_sum* Mult_Sum_With_Term( N_sum *sum, N_term *t, N_sum *result )
{
	N_term *term1;
	N_sum *sum2;

	if( sum == NULL && t == NULL )
		return NULL;
	if( sum == NULL && t != NULL )
		return NULL;
	if( sum != NULL && t == NULL )
		return NULL;

	if( sum != NULL && t != NULL )
	{
		result = ( NC * )malloc( sizeof( NC ) );
		result->type = 'S';
		result->inc = 0;
		result->list = NULL;

		term1 = ( NC * )malloc( sizeof( NC ) );
		term1 = copylist( t );

		term1->inc = sum->inc * t->inc;
		sum2 = ( NC * )malloc( sizeof( NC ) );
		sum2->type = 'T';
		sum2->list = NULL;

		if( sum->link != NULL )
		{
			sum2 = Mult_Sum_With_Term_1( sum->link, t, sum2 );
		}
		else
		{
			result->link = copylist( term1 );
			return result;
		}
		if( sum->inc != 0 )
		{
			result->link = Add_Sums_1( term1, sum2, result->link );
			return result;
		}
		else
		{
			result->link = copylist( sum2 );
			return result;
		}
	}
	return result;
}



//This function  multiplies all the terms of a normalized sum with a
//normalized term , and returns the resultant normalized sum.
N_sum* Mult_Sum_With_Term_1( N_sum *sum, N_term *t, N_sum *result )
{
	N_term *term1, *term2;

	if( sum == NULL && t == NULL )
		return NULL;
	if( sum == NULL && t != NULL )
		return NULL;
	if( sum != NULL && t == NULL )
		return NULL;
	if( sum != NULL && t != NULL )
	{
		term1 = ( NC * )malloc( sizeof( NC ) );
		term1->type = 'T';

		term2 = ( NC * )malloc( sizeof( NC ) );
		term2->type = 'T';
		term2 = copylist( sum );
		term2->list = NULL;

		term1 = Mult_Term_With_Term( term2, t, term1 );

		result= ( NC * )malloc( sizeof( NC ) );
		result->type = 'T';
		result = Add_Sums_1( term1, Mult_Sum_With_Term_1( sum->list, t, result->list ), result );
		return result;
	}
}



//This function multiplies a normalized sum 'sum1' with another normalized
//sum 'sum2' and returns  the resultant normalized sum.
N_sum* Mult_Sum_With_Sum( N_sum *sum1, N_sum *sum2, N_sum *result )
{
	N_sum *temp1, *temp2;

	if( sum1 == NULL && sum2 == NULL )
		return NULL;
	if( sum1 != NULL && sum2 == NULL )
		return NULL;
	if( sum1 == NULL && sum2 != NULL )
		return NULL;

	if( sum1 != NULL && sum2 != NULL )
	{
		if( sum1->link == NULL )
		{
			//Mult_sum_with_constant
			result = ( NC *) malloc( sizeof( NC ) );
			result->type = 'S';
			result->list = NULL;
			result = Mult_sum_with_constant( sum2, sum1->inc, result );
			return result;
		}
		else
		{
			if( sum2->inc != 0 )
			{
				temp1 = ( NC *) malloc( sizeof( NC ) );
				temp1->type = 'S';
				temp1->list = NULL;
				temp1 = Mult_Sum_With_Sum_1( sum1, sum2->link, temp1 );

				temp2 = ( NC *) malloc( sizeof( NC ) );
				temp2->type = 'S';
				temp2->list = NULL;
				temp2 = Mult_sum_with_constant( sum1, sum2->inc, temp2 );

				result = ( NC *) malloc( sizeof( NC ) );
				result->type = 'S';
				result->list = NULL;
				result = Add_Sums( temp1, temp2, result );
			}
			else
			{
				result = ( NC *) malloc( sizeof( NC ) );
				result->type = 'S';
				result->list = NULL;
				result = Mult_Sum_With_Sum_1( sum1, sum2->link, result );
			}
		}
	}
	return result;
}



//This function multiplies all the terms of the normalized sum 'sum2'
//with the normalized sum 'sum1' and returns the resultant normalized sum.
N_sum* Mult_Sum_With_Sum_1( N_sum *sum1, N_sum *sum2, N_sum *result )
{
	NC * term1, *temp1;

	if( sum1 == NULL && sum2 == NULL )
		return NULL;
	if( sum1 != NULL && sum2 == NULL )
		return NULL;
	if( sum1 == NULL && sum2 != NULL )
		return NULL;

	if( sum1 != NULL && sum2 != NULL )
	{
		term1 = ( NC *) malloc( sizeof( NC ) );
		term1 = copylist( sum2 );
		term1->list = NULL;

		temp1 = ( NC *) malloc( sizeof( NC ) );
		temp1->type = 'S';
		temp1 = Mult_Sum_With_Term( sum1, term1, temp1 );

		result = ( NC *) malloc( sizeof( NC ) );
		result->type = 'S';

		result = Add_Sums( temp1, Mult_Sum_With_Sum_1( sum1, sum2->list, result->list ), result );
	}
	return result;
}



//This function multiplies a normalized sum with a constant and returns
//the resultant normalized sum.
N_sum* Mult_sum_with_constant( N_sum *sum, int n, N_sum *result )
{
	if( sum == NULL )
		return NULL;

	if( sum->inc != 0 )
	{
		result = ( NC *) malloc( sizeof( NC ) );
		result->type = 'S';
		result->inc = sum->inc * n;

		result->link = ( NC *) malloc( sizeof( NC ) );
		result->link->type = 'T';
		result->link = Mult_sum_with_constant_1( sum->link, n, result->link );
	}
	else
	{
		result = ( NC *) malloc( sizeof( NC ) );
		result->type = 'S';
		result->inc = 0;

		result->link = ( NC *) malloc( sizeof( NC ) );
		result->link->type = 'T';
		result->list = NULL;
		result->link = Mult_sum_with_constant_1( sum->link, n, result->link );
	}
	return result;
}



//This function multiplies all the terms of a normalized sum with a
//constant and returns the resultant sum of terms.
N_sum* Mult_sum_with_constant_1( N_sum *sum, int n, N_sum  *result )
{
	if( sum == NULL )
		return NULL;
	else
	{
		result = ( NC *) malloc( sizeof( NC ) );
		result->type = 'T';
		result = copylist( sum );
		result->inc = sum->inc * n ;
		result->list = ( NC *) malloc( sizeof( NC ) );
		result->list->type = 'T';
		result->list = Mult_sum_with_constant_1( sum->list, n, result->list );
	}
	return result;
}



//This function multiplies a constant with a normalized term, and returns
//the resultant normalized term.
N_term* Mult_constant_with_term( int n, N_term *t, N_term *result )
{
	result = ( NC *) malloc( sizeof( NC ) );
	result->type = 'T';
	result = copylist( t );
	result->inc = t->inc * n;
	return result;
}



//This function multiplies a primary with a normalized term and returns
//the resultant normalized term.
N_term* Mult_primary_with_term( N_primary *p, N_term *t, N_term *result )
{
	if( p == NULL && t == NULL )
		return NULL;

	if( p != NULL && t == NULL )
	{
		result = ( NC *) malloc( sizeof( NC ) );
		result->type = 'T';
		result->list = NULL;
		result->inc = 1;
		result->link = ( NC *) malloc( sizeof( NC ) );
		result->link = Mult_primary_with_term_1( p, NULL, result->link );
	}

	if( p != NULL && t != NULL )
	{
		result = ( NC *) malloc( sizeof( NC ) );
		result->type = 'T';
		result->list = NULL;
		result->inc = t->inc;
		result->link = Mult_primary_with_term_1( p, t->link, result->link );
	}
	return result;
}



//This function multiplies all the primaries of the term 't' with
//the primary 'p' and retuns the resultant normalized term.
N_term* Mult_primary_with_term_1( N_primary *p, N_term *t, N_term *result )
{
	int v;

	if( p == NULL && t == NULL )
		return NULL;

	if( p == NULL && t != NULL )
	{
		result = ( NC *) malloc( sizeof( NC ) );
		result = copylist( t );

		result->list = ( NC *) malloc( sizeof( NC ) );
		result->list = Mult_primary_with_term_1( NULL, t->list, result->list );
	}

	if( p != NULL && t == NULL )
	{
		result = ( NC *) malloc( sizeof( NC ) );
		result = copylist( p );

		result->list = ( NC *) malloc( sizeof( NC ) );
		result->list = Mult_primary_with_term_1( NULL, NULL, result->list );
	}

	if( p != NULL && t != NULL )
	{
		v = Compare_Primaries( p, t );

		if( v <= 0 )
		{
			result = ( NC *) malloc( sizeof( NC ) );
			result = copylist( p );

			result->list = ( NC *) malloc( sizeof( NC ) );
			result->list = Mult_primary_with_term_1( NULL, t, result->list );
		}
		else
		{
			result = ( NC *) malloc( sizeof( NC ) );
			result = copylist( t );

			result->list = ( NC *) malloc( sizeof( NC ) );
			result->list = Mult_primary_with_term_1( p, t->list, result->list );
		}
	}
	return result;
}



//This function works on the term 't' , a primary 'x' , a normalized sum
//'s', a normalized term 'p' , a normalized term 'q' , and another
//normalized sum 'z' .
//Here 'x' is to be replaced for by 's' in 't' , 'p' is formed by multiplying
//the constant and the other primaries that do not match 'x' , and 'q' is
//formed by multiplying all the primaries from 't' that match 'x' ; for
//every primary in 'q' , 'z' is multiplied by 's' and then 'p' is multiplied
//to 'z' and returned .
//Initially it should be invoked with { p = NULL , q = NULL , z = 1 }
N_sum* Replace_Primary( N_term *t, N_primary *x, N_sum *s, N_term *p, N_term *q, N_sum *z, N_sum *result )
{
	NC *p1;

	if( t == NULL )
		return NULL;

	if( t != NULL )
	{
		p1 = ( NC *) malloc( sizeof( NC ) );
		p1->type = 'T';
		p1->inc = t->inc;
		p1->list = NULL;
		p1->link = NULL;

		result = ( NC *) malloc( sizeof( NC ) );
		result->type = 'S';
		result->list = NULL;
		result = Replace_Primary_1( t->link, x, s, p1, q, z, result );
		return result;
	}
	return NULL;
}



//This function works on all the primaries of the term 't' , a primary 'x' ,
//a normalized sum  's', a normalized term 'p' , a normalized term 'q' ,
//and another normalized sum 'z' .
N_sum* Replace_Primary_1( N_term *t, N_primary *x, N_sum *s, N_term *p, N_term *q, N_sum *z, N_sum *result )
{
	int v, flag;
	N_sum *z1, *prev, *next;
	N_term *q1, *p1, *term;
	N_primary *pr;

	if( t == NULL && q == NULL )
	{
		result = ( NC *) malloc( sizeof( NC ) );
		result->type = 'S';
		result->list = NULL;

		term = ( NC *) malloc( sizeof( NC ) );
		term->type = 'T';
		term->inc = p->inc ;
		term->list = NULL;
		term->link = copylist( p->link );

		if( term->link != NULL )
		{
			result = Mult_Sum_With_Term( z, term, result );
		}
		else
		{
			result = Mult_sum_with_constant( z, term->inc, result );
		}
		return result;
	}

	if( t == NULL && q != NULL )
	{
		z1 = ( NC *) malloc( sizeof( NC ) );
		z1->type = 'S';
		z1->list = NULL;
		z1 = Mult_Sum_With_Sum( z, s, z1 );

		result = ( NC *) malloc( sizeof( NC ) );
		result->type = 'S';
		result->list = NULL;
		result = Replace_Primary_1( NULL, x, s, p, q->list, z1, result );
	}

	if( t != NULL )
	{
		if( t->type == 'M' || t->type == 'D' )
		{
			result = ( NC *) malloc( sizeof( NC ) );
			result->type = t->type;
			result->inc = 0;
			result->list = NULL;
			result->link = Substitute_In_Sum( t->link, x, s, result->link );
			result->link->list = Substitute_In_Sum( t->link->list, x, s, result->link->list );
		}
		else if( t->type == 'f' )
		{
			result = ( NC *) malloc( sizeof( NC ) );
			result->type = t->type;
			result->inc = t->inc;
			result->list = NULL;
			result->link = NULL;
			flag = 1;
			next = t->link;
			while(next != NULL)
			{
				if(flag)
				{
					result->link = Substitute_In_Sum( next, x, s, result->link );
					prev = result->link;
					flag = 0;
				}
				else
				{
					prev->list = Substitute_In_Sum( next, x, s, prev->list );
					prev = prev->list;
				}
				next = next->list;
			}//end while
		}
		else
		{
			v = Compare_Primaries( t, x );

			if( v == 0 )
			{
				q1 = ( NC *) malloc( sizeof( NC ) );
				q1->type = 'T';
				q1->inc = 1;
				q1->list = NULL;

				if(t->type == 'v')
				{
					pr = ( NC *) malloc( sizeof( NC ) );
					pr->type = 'v';
					pr->inc = t->inc;
					pr->list = NULL;
					pr->link = NULL;
				}
				else
				{
					pr = ( NC *) malloc( sizeof( NC ) );
					pr = copylist( t );
					pr->list = NULL;
				}

				q1->link = Mult_primary_with_term_1( pr, q, q1->link );

				result = ( NC *) malloc( sizeof( NC ) );
				result->type = 'S';
				result->list = NULL;
				result = Replace_Primary_1( t->list, x, s, p, q1->link, z, result  );
			}
			else
			{
				p1 = ( NC *) malloc( sizeof( NC ) );
				p1->type = 'T';
				p1->list = NULL;
				p1->link = NULL;

				if(t->type == 'v')
				{
					pr = ( NC *) malloc( sizeof( NC ) );
					pr->type = 'v';
					pr->inc = t->inc;
					pr->list = NULL;
				}
				else
				{
					pr = ( NC *) malloc( sizeof( NC ) );
					pr = copylist( t );
					pr->list = NULL;
				}

				p1 = Mult_primary_with_term( pr, p, p1 );
				result = ( NC *) malloc( sizeof( NC ) );
				result->type = 'S';
				result->list = NULL;
				result = Replace_Primary_1( t->list, x, s, p1, q, z, result );
			}
		}
	}
	return result;
}



//This function works on the normalized sum 's' to substitute each
//occurrence of primary 'x' in 's' with the normalized sum 'z'.
N_sum* Substitute_In_Sum( N_sum *s, N_primary *x, N_sum *z, N_sum *result )
{
	N_sum *sum, *result1, *temp;

	if( s == NULL )
		return NULL;

	if( s != NULL )
	{
		sum = ( NC *) malloc( sizeof( NC ) );
		sum->type = 'S';
		sum->inc = s->inc;
		sum->list = NULL;
		sum->link = NULL;

		result = ( NC *) malloc( sizeof( NC ) );
		result->type = 'S';
		result->inc  = 0;
		result->list = NULL;
		result->link = NULL;

		result1 = Substitute_In_Sum_1( s->link, x, z, result1 );
		if(result1 == NULL)
		{
			return sum;
		}
		if(result1->type == 'D' || result1->type == 'M' || result1->type == 'f')
		{
			temp = copylist(result1);
			result1 = ( NC *) malloc( sizeof( NC ) );
			result1->type = 'S';
			result1->inc  = 0;
			result1->list = NULL;

			result1->link = ( NC *) malloc( sizeof( NC ) );
			result1->link->type = 'T';
			result1->link->inc  = 1;
			result1->link->link = temp;
		}
		result = Add_Sums( sum, result1, result );

		return result;
	}
}



//This function works on all the terms of the normalized sum 's' to
//substitute each occurrence of primary 'x'  in all the terms of 's' with
//the normalized sum 'z'.
N_sum* Substitute_In_Sum_1( N_sum *s, N_primary *x, N_sum *z, N_sum *result )
{
	N_sum *z_sum, *temp1, *temp2, *temp3;
	N_term *term;

	if( s == NULL )
		return NULL;

	if( s != NULL )
	{
		if( s->link != NULL )
		{
			z_sum = ( NC *) malloc( sizeof( NC ) );
			z_sum->type = 'S';
			z_sum->inc = 1;
			z_sum->list = NULL;
			z_sum->link = NULL;

			term = ( NC *) malloc( sizeof( NC ) );
			term = copylist( s );
			term->type = 'T';
			term->list = NULL;

			result = ( NC *) malloc( sizeof( NC ) );
			result->type = 'S';

			temp1 =  ( NC *) malloc( sizeof( NC ) );
			temp1->type = 'S';
			temp1->list = NULL;
			temp1 = Replace_Primary( term, x, z, NULL, NULL, z_sum, temp1 );

			temp2 =  Substitute_In_Sum_1( s->list, x, z, result->list );
			if(temp2 != NULL && temp2->type != 'S')
			{
				temp3 = temp2;

				temp2 = ( NC *) malloc( sizeof( NC ) );
				temp2->type = 'S';
				temp2->inc  =  0;
				temp2->list = NULL;

				temp2->link = ( NC *) malloc( sizeof( NC ) );
				temp2->link->type = 'T';
				temp2->link->inc  =  1;
				temp2->link->list = NULL;
				temp2->link->link = temp3;
			}
			result = Add_Sums( temp1, temp2, result );

			return result;
		}
	}
}



//This funtion does the arithmetic simplification at of a normalized sum.
//The common constant factors are extracted from the normalized sum.
N_sum* simplify_sum( N_sum *sum, N_sum *result)
{
	int min;
	N_sum *sum1, *sum2;

	sum1 = ( NC * )malloc( sizeof( NC ) );
	sum1 = copylist( sum );
	sum2 = ( NC * )malloc( sizeof( NC ) );
	sum2 = copylist( sum );

	min = abs( sum->inc );
	sum1 = sum1->link;

	while( sum1 != NULL )
	{
		if( abs( sum1->inc ) < min )
			min = abs( sum1->inc );
		else
			min = min;

		sum1 = sum1->list;
	}

	if( min != 1 && min != 0 )
	{
		result->inc = sum2->inc / min;
		result->link = simplify_sum_1( sum2->link,result->link, min );
		return result;
	}

	return result;
}



//This funtion does the arithmetic simplification at of a normalized sum.
//The common constant factors are extracted from the normalized sum.
//The common constant factor is 'min' which is calculated from the function
//"simplify_sum".
N_sum* simplify_sum_1( N_sum *sum, N_sum *result, int min )
{
	if( sum == NULL)
		return NULL;

	if( sum != NULL )
	{
		result->inc = sum->inc / min;
		result->list = simplify_sum_1( sum->list, result->list, min );;
	}

	return result;
}



//This funtion does the arithmetic simplification at of a normalized conditional expression.
//The common constant factors are extracted from the normalized conditional expression.
NC* simplify_condition( NC * condition, NC * result )
{
	NC *condition1;

	if( condition == NULL )
		return NULL;

	if( condition != NULL)
	{
		condition1 = copylist( condition );
		result->link->link = simplify_sum( condition1->link->link, result->link->link );

		result->list = simplify_condition( condition->list, result->list );
	}
	return result;
}



//This function compares two sums  'sum1 and 'sum2' without including constant factor.
//All the terms of the normalized sums  'sum1' and 'sum2' and returns
//'0' if sum1 == sum2 , returns -1 if sum1 < sum2 else returns +1.
int Compare_Sums( N_sum *sum1, N_sum *sum2 )
{
	N_term *t1, *t2;
	int m;

	if( sum1 == NULL && sum2 == NULL )
		return 0;
	if( sum1 == NULL && sum2 != NULL )
		return -1;
	if( sum1 != NULL && sum2 == NULL )
		return 1;
	if( sum1 != NULL && sum2 != NULL )
	{
		t1 = ( NC * )malloc( sizeof( NC ) );
		t2 = ( NC * )malloc( sizeof( NC ) );

		t1 = copylist( sum1 );
		t1->list = NULL;

		t2 = copylist( sum2 );
		t2->list = NULL;

		m = Compare_Terms( t1->link, t2->link );

		if( m == 0 && t1->inc == t2->inc )
		{
			m = Compare_Sums( sum1->list, sum2->list );
		}
		else if( m == 0 && t1->inc < t2->inc )
			return -1;
		else if( m == 0 && t1->inc > t2->inc )
			return 1;
		else if( m < 0 )
			return -1;
		else // if m > 0
			return 1;
	}
	return m;
}



//This function compares constants c1 and c2 depending upon the relations R1 and R2,
//which must be satisfied for A to imply B ( A -> B ) .
//It returns '1' if there is success else returns -1.
int Check_c1_c2_And_R1_R2( int c1, int c2, int R1, int R2 )
{
	// case 1
	if( R2 == 4 )
	{
		if( R1 == 4 && c1 == c2 )
			return 1;
		else
			return -1;
	}

	// case 2
	if( R2 == 0 )
	{
		if( R1 == 4 && c2 >= c1 )
			return 1;
		if( R1 == 0 && c2 >= c1 )
			return 1;
		else
			return -1;
	}

	//case 3
	if( R2 == 5 )
	{
		if( R1 == 0 && c2 > c1 )
			return 1;
		if( R1 == 5 && c1 == c2 )
			return 1;
		if( R1 == 2 && c2 < c1 )
			return 1;
		if( R1 == 4 && c1 != c2 )
			return 1;
		else
			return -1;
	}

	//case 4
	if( R2 == 2 )
	{
		if( R1 == 4 && c2 <= c1 )
			return 1;
		if( R1 == 2 && c2 <= c1 )
			return 1;
		else
			return -1;
	}
}



//This function checks if condition A implies condition B.
//It returns '1' if A implies B  else returns -1.
int A_implies_B( NC *condition_A, NC *condition_B )
{
	N_sum *sum1, *sum2;
	NC *condition1, *condition2;
	int R1, R2, c1, c2, s, c;

	if( condition_A == NULL || condition_B == NULL )
		return -1;
	if( condition_A != NULL && condition_B != NULL )
	{
		condition1 = ( NC * )malloc( sizeof( NC ) );
		condition2 = ( NC * )malloc( sizeof( NC ) );

		sum1 = ( NC * )malloc( sizeof( NC ) );
		sum2 = ( NC * )malloc( sizeof( NC ) );

		if( condition_A->type == 'A' )
		{
			condition1 = copylist( condition_A );
			condition2 = copylist( condition_B );

			R1 = condition1->link->link->inc;
			R2 = condition2->link->link->inc;

			c1 = condition1->link->link->link->inc;
			c2 = condition2->link->link->link->inc;

			sum1 = condition1->link->link->link->link;
			sum2 = condition2->link->link->link->link;
		}
		else if( condition_A->type == 'O' )
		{
			condition1 = copylist( condition_A );
			condition2 = copylist( condition_B );

			R1 = condition1->link->inc;
			R2 = condition2->link->inc;

			c1 = condition1->link->link->inc;
			c2 = condition2->link->link->inc;

			sum1 = condition1->link->link->link;
			sum2 = condition2->link->link->link;
		}

		s = Compare_Sums( sum1, sum2 );

		c = Check_c1_c2_And_R1_R2( c1, c2, R1, R2 );

		if( s == 0 && c == 1 )
			return 1;
		else
			return -1;
	}
}



//This function compares two conditions and returns '0' if both are equal else returns '-1'.
int Compare_Conditions( NC *condition1, NC *condition2 )
{
	int number;

	if( condition1 == NULL && condition2 == NULL )
		return 1;
	if( condition1 == NULL && condition2 != NULL )
		return -1;
	if( condition1 != NULL && condition2 == NULL )
		return -1;
	if( condition1 != NULL && condition2 != NULL )
	{
		if( condition1->type == 'A' )
		{
			if( condition1->inc == condition2->inc )
				number = Compare_Conditions( condition1->link, condition2->link );
			else
				return -1;
		}

		if( condition1->type == 'O' )
		{
			if( condition1->inc == condition2->inc )
				number = Compare_Conditions( condition1->link, condition2->link );
			else
				return -1;
		}

		if( condition1->type == 'R' )
		{
			if( condition1->inc == condition2->inc )
				number = Compare_Conditions( condition1->link, condition2->link );
			else
				return -1;
		}

		if( condition1->type == 'S' )
		{
			number = Compare_Sums( condition1->link, condition2->link );

			if( number == 0 && condition1->inc == condition2->inc )
				return 0;
			if( number == 0 && condition1->inc < condition2->inc )
				return -1;
			if( number == 0 && condition1->inc > condition2->inc )
				return 1;

			if( number < 0 )
				return -1;
			else
				return 1;
		}

	}
	return number;
}



//This function calculates the number of occurrences of a condition in conditional
//expression and returns the count / number.
int Search_cond_in_expr( NC *expression, NC *condition )
{
	NC *expression1, *temp_cond, *condition1;
	int count = 0, number;

	if( expression == NULL )
		return 0;
	if( expression != NULL )
	{
		expression1 = ( NC * )malloc( sizeof( NC ) );
		expression1 = copylist( expression );

		condition1 = ( NC * )malloc( sizeof( NC ) );
		condition1 = copylist( condition );

		if( expression1->type == 'A' && condition1->type == 'O' )
			expression1 = expression1->link;
		if( expression1->type == 'O' && condition1->type == 'A' )
			condition1 = condition1->link;
		if( expression1->type == 'A' && condition1->type == 'A' )
		{
			expression1 = expression1->link;
			condition1 = condition1->link;
		}

		temp_cond = ( NC * )malloc( sizeof( NC ) );
		while( expression1 != NULL )
		{
			temp_cond = copylist( expression1 );
			temp_cond->list = NULL;

			number = Compare_Conditions( temp_cond, condition1 );

			if( number == 0 )
				count = count + 1;
			else
				count = count;

			expression1 = expression1->list;
		}
	}
	return count ;
}



//This function deletes 'count' number of occurrences of a condition ,from the
//conditional expression and returns the resultant conditional expression.
//The number of occurrences of a condition in a conditional expression can be found
//by using the function "Search_cond_in_expr( )".
//Both the expression and the condtion must be at the 'O' level.
NC* Delete_cond_from_expr( NC *expression, NC *condition, int count, NC *result )
{
	NC *temp_condition;
	int number;

	temp_condition = ( NC* ) malloc(sizeof( NC ) );

	if( expression == NULL && count == 0 )
		return NULL;
	if( count == 0 && expression != NULL )
	{
		result = copylist( expression );
		return result;
	}

	if( count != 0 && expression != NULL )
	{
		temp_condition = copylist( expression );
		temp_condition->list = NULL;

		number = Compare_Conditions( temp_condition, condition );

		if( number == 0 )
		{
			count--;
			result = Delete_cond_from_expr( expression->list, condition, count, result );
		}
		else
		{
			result = copylist( temp_condition );
			result->list = Delete_cond_from_expr( expression->list, condition, count, result->list );
		}

		return result;
	}
	return result;
}



//This function removes the multiple occurrences of a condition ( eg. a>b ) from the
//conditional expression ( eg. a>b&&c>2&&b==2&&a>b)and returns
//the resultant conditional expression.
NC* Remove_mult_occurence_cond_in_expr( NC *expression, NC* condition, NC *result )
{
	int count;
	NC *expression1;

	expression1 = copylist( expression );

	if( expression1 == NULL )
		return NULL;

	if( expression1 != NULL )
	{
		if( expression1->type == 'A' && condition->type == 'O' )
			expression1 = expression1->link;
		if( expression1->type == 'O' && condition->type == 'A' )
			condition = condition->link;
		if( expression1->type == 'A' && condition->type == 'A' )
		{
			expression1 = expression1->link;
			condition = condition->link;
		}

		count = Search_cond_in_expr( expression1, condition );

		if( count > 1 )
		{
			result = Delete_cond_from_expr( expression1, condition, count-1, result );
			return result;
		}
		else
			return expression1;
	}
}



//This function removes all the multiple occurrences of all the conditions
//( in conditional expression2 ) from conditional expression1 and returns
//the resultant conditional expression.
NC* Remove_all_mult_occurence_in_expr_1( NC *expression1, NC *expression2, NC *result )
{
	NC *condition;

	if( expression2 == NULL )
		return expression1;
	if( expression2 != NULL )
	{
		condition = ( NC * )malloc( sizeof( NC ) );
		condition = copylist( expression2 );
		condition->list = NULL;

		result = Remove_mult_occurence_cond_in_expr( expression1, condition, result );
		result = Remove_all_mult_occurence_in_expr_1( result, expression2->list, result );
	}

	return result;
}



//This function removes all the multiple occurrences of all the conditions from
//conditional expression and returns the resultant conditional expression.
NC* Remove_all_mult_occurence_in_expr( NC *expression, NC *result )
{
	NC *expression1;

	if( expression == NULL )
		return NULL;

	if( expression != NULL )
	{
		expression1 = ( NC * )malloc( sizeof( NC ) );
		expression1 = copylist( expression );

		result = copylist( expression );
		result->list = NULL;

		result->link = Remove_all_mult_occurence_in_expr_1( expression1->link, expression1->link, result->link );
	}
	return result;
}



//This function returns the negation of relational operators.
//Eg: The negate of > is <=.
int negateoperator( int op )
{
	switch(op)
	{
		case 0:
			return 3;
		case 1:
			return 2;
		case 2:
			return 1;
		case 3:
			return 0;
		case 4:
			return 5;
		case 5:
			return 4;
	}

	return op;
}
