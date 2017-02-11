#ifndef NORMALIZATION
#define NORMALIZATION

/**********************************************************************/
/*      This is the header file which contain the definitions    */
/*      and the prototypes of the functions which were used for  */
/*      the creation of normalized cells	                     */
/**********************************************************************/

typedef struct normalized_cell NC;

struct normalized_cell
{
	NC  *list;
	char type;
	int  inc;
	NC  *link;
};

typedef NC N_sum, N_term, N_primary;


//Symbol Table
#define SYMTABSIZE 50

//Structure for symbol table
typedef struct sym_table
{
	char *sym_tab[SYMTABSIZE];
	int   val_tab[SYMTABSIZE];
	int   numsymbols;
}SYMTAB;


typedef struct variables {
	int   var_val[SYMTABSIZE];    // integer value of the variable
	char *var_string[SYMTABSIZE]; // variable
	int   no_of_elements;         // number of elements in the list
}var_list;



//This structure is used to hold the information of an assignment operation
typedef struct Assign
{
	int lhs; // left hand side (lhs) variable's symbol table index is stored here
	NC *LHS; // lhs (array) variable along with its indices is stored here
	NC *rhs; // right hand side of the assignment is stored as an expression tree
}DATA_TRANS;


/**********************************************************************/

//Functions related to normalization

//Functions related normalized arithmetic expressions
//Creation of normalized "primary" expressions
NC* createVariable( char* );
NC* createConstant( char* );
NC* createArray      ( NC*, NC* );
NC* createExpression_Mod_Div( NC*, NC*, char );

NC* addArgument   ( NC*, NC* );
NC* createFunction( NC*, NC* );

//Creation of data transformations
DATA_TRANS* createDataTransVar  ( char*, NC* );
DATA_TRANS* createDataTransArray( NC*,   NC* );
void markOutputVariables( int );

NC* negateExpression( NC* );


//Functions related to normalized conditional expressions
//Creation of normalized conditional expressions
int getRelOperatorIndex( char* );
NC* createConditionalExpression( NC*, char*, NC* );

//Functions for performing Boolean operations - NOT, AND, OR
NC* computeNegation( NC* );
NC* negateConditionalExpression( NC* );

NC* createAndExpression( NC*, NC* );

NC* compareRelations   ( NC*, NC* );
NC* createOrExpression ( NC*, NC* );


/**************************************************************/

NC*  copylist     ( NC* );
int  compare_trees( NC*, NC* );
void write_lists  ( NC* );

/************************/

int isconstant( char* );
int constval  ( char* );

NC* create_expr( char*, char, char* );
NC* create_term( char*, int );

/************************/

N_sum* simplify_sum  ( N_sum*, N_sum* );
N_sum* simplify_sum_1( N_sum*, N_sum*, int );
int Compare_Sums     ( N_sum*, N_sum* );
int Check_c1_c2_And_R1_R2( int, int, int, int );
int A_implies_B        ( NC*, NC* );
int Compare_Conditions ( NC*, NC* );
int Search_cond_in_expr( NC*, NC* );

NC* Delete_cond_from_expr              ( NC*, NC*, int, NC* );
NC* Remove_mult_occurence_cond_in_expr ( NC*, NC*, NC* );
NC* Remove_all_mult_occurence_in_expr_1( NC*, NC*, NC* );
NC* Remove_all_mult_occurence_in_expr  ( NC*, NC* );
NC* Substitute_in_condition   ( NC*, N_primary*, N_sum*, NC* );
N_sum* SubExpr_simplify_sum   ( N_sum*, N_sum* );
N_sum* SubExpr_simplify_terms ( N_sum*, int, N_sum* );
NC* SubExpr_simplify_condition( NC*, NC* );

/************************/

N_sum* Add_Sums( N_sum*, N_sum*, N_sum*	);
N_sum* Add_Sums_1( N_sum*, N_sum*, N_sum* );
int Compare_Primaries( N_primary*, N_primary* );
int Compare_Terms( N_term*, N_term* );

N_term* Mult_Term_With_Term_1   ( N_term*, N_term*, N_term* );
N_term* Mult_Term_With_Term     ( N_term*, N_term*, N_term* );
N_sum* 	Mult_Sum_With_Term_1    ( N_sum*, N_term*, N_sum*   );
N_sum* 	Mult_Sum_With_Term      ( N_sum*, N_term*, N_sum*   );
N_sum* 	Mult_Sum_With_Sum       ( N_sum*, N_sum*, N_sum*    );
N_sum* 	Mult_Sum_With_Sum_1     ( N_sum*, N_sum*, N_sum*    );
N_sum* 	Mult_sum_with_constant  ( N_sum*, int, N_sum*    );
N_sum* 	Mult_sum_with_constant_1( N_sum*, int, N_sum*    );
N_term* Mult_constant_with_term ( int , N_term*, N_term* );
N_term* Mult_primary_with_term  ( N_primary*, N_term*, N_term* );
N_term* Mult_primary_with_term_1( N_primary*, N_term*, N_term* );
N_sum*  Replace_Primary     ( N_term*, N_primary*, N_sum*, N_term*, N_term*, N_sum*, N_sum* );
N_sum*  Replace_Primary_1   ( N_term*, N_primary*, N_sum*, N_term*, N_term*, N_sum*, N_sum* );
N_sum*  Substitute_In_Sum   ( N_sum*, N_primary*, N_sum*, N_sum* );
N_sum*  Substitute_In_Sum_1 ( N_sum*, N_primary*, N_sum*, N_sum* );
int     negateoperator( int );

void cal_V0_intersection_V1          ( var_list* );
void cal_V1_minus_V0_intersection_V1 ( var_list* );
int  var_in_V1_minus_V0_intersection_V1 ( int    );
void cal_V1_minus_V0_intersection_V1 ( var_list* );
int  indexof_symtab  ( char* );
int  indexof_varlist ( char*, var_list* );
void symbol_for_index( int, char* );

/**************************************************************/

#endif
