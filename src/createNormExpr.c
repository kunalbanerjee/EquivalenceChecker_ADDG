#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "normalization.h"


extern bool flagVar_List;
extern var_list V0, V1;


//Functions related normalized arithmetic expressions
//Creation of normalized "primary" expressions

//Input:  A string x
//Output: A normalized expression with x as primary
NC* createVariable( char *variableName )
{
	NC *sum, *term, *primary;
	int symbolIndex;

	#ifdef Debug
	printf("In createVariable: %s\n", variableName);
	#endif

	symbolIndex = indexof_symtab(variableName);
	#ifdef Debug
	printf("Back from indexof_symtab: %d\n",symbolIndex);
	#endif
	primary = (NC*) malloc (sizeof(NC));
	primary->list = NULL;
	primary->type = 'v';
	primary->inc = symbolIndex;
	primary->link = NULL;

	term = (NC*) malloc (sizeof(NC));
	term->list = NULL;
	term->type = 'T';
	term->inc = 1;
	term->link = primary;

	sum = (NC*) malloc (sizeof(NC));
	sum->list = NULL;
	sum->type = 'S';
	sum->inc = 0;
	sum->link = term;

	//The next statement adds the variable to corresponding var_list
	if(!(flagVar_List))
		indexof_varlist(variableName, &V0);
	else
		indexof_varlist(variableName, &V1);

	#ifdef Debug
	write_lists(sum);
	printf("\nOut of createVariable\n");
	#endif

	return sum;
}



//Input:  A string x (representing a constant integer)
//Output: A normalized expression with x as sum
NC* createConstant( char *constant )
{
	NC *sum;

	#ifdef Debug
	printf("In createConstant\n");
	#endif

	sum = (NC*) malloc (sizeof(NC));
	sum->list = NULL;
	sum->type = 'S';
	sum->inc = constval(constant);
	sum->link = NULL;

	//The next statement adds the constant to corresponding var_list
	//Although it is seemingly unnecessary, it is done to remain compliant
	//with previous code
	if(!(flagVar_List))
		indexof_varlist(constant, &V0);
	else
		indexof_varlist(constant, &V1);

	#ifdef Debug
	write_lists(sum);
	printf("\nOut of createConstant\n");
	#endif

	return sum;
}



//Input:  A normalized variable x and a normalized expression y
//Output: A normalized expression with x as an array
//        and y added to its index expressions
NC* createArray( NC *arrayName, NC *expression )
{
	NC *arrayExpression, *newExpression;
	NC *tempNorm1, *tempNorm2;

	#ifdef Debug
	printf("In createArray\n");
	#endif

	arrayExpression = copylist(arrayName);
	newExpression   = copylist(expression);

	//Change the type form variable 'v' to array 'a'
	arrayExpression->link->link->type = 'a';

	//Append newExpression as the array's last index expression
	tempNorm2 = arrayExpression->link->link->link;
	if(tempNorm2 == (NC*)NULL)
		arrayExpression->link->link->link = newExpression;
	else
	{
		tempNorm1 = arrayExpression->link->link->link;
		tempNorm2 = tempNorm1->list;
		while(tempNorm2 != (NC*)NULL)
		{
			tempNorm1 = tempNorm2;
			tempNorm2 = tempNorm2->list;
		}
		tempNorm1->list = newExpression;
	}

	//Note that the array variable has already been added to the
	//corresponding var_list

	#ifdef Debug
	write_lists(arrayExpression);
	printf("\nOut of createArray\n");
	#endif

	return arrayExpression;
}



//Input:  Two normalized expressions x and y, and a flag (type: character)
//Output: A normalized expression x/y or x%y depending upon the flag
NC* createExpression_Mod_Div( NC *neumerator, NC *denominator, char flag )
{
	NC *finalExpression, *newNeumerator, *newDenominator;
	NC *term, *sum;

	newNeumerator = copylist(neumerator);
	newDenominator = copylist(denominator);

	finalExpression = (NC*) malloc (sizeof(NC));
	finalExpression->list = NULL;
	finalExpression->inc = 0;
	finalExpression->link = newNeumerator;
	newNeumerator->list = newDenominator;


	if(flag == '/')
		finalExpression->type = 'D';
	else
		finalExpression->type = 'M';

	term = (NC*) malloc (sizeof(NC));
	term->list = NULL;
	term->type = 'T';
	term->inc = 1;
	term->link = finalExpression;

	sum = (NC*) malloc (sizeof(NC));
	sum->list = NULL;
	sum->type = 'S';
	sum->inc = 0;
	sum->link = term;

	return sum;
}



//Input:  An argument list x and an argument y
//Output: An argument list x.y (ie, y is appended to x)
NC* addArgument( NC *argList, NC *newArg )
{
	NC *traverseArgList;

	if(argList == NULL && newArg == NULL)
		return NULL;
	if(argList == NULL)
		return newArg;
	if(newArg == NULL)
		return argList;

	traverseArgList = argList;
	while(traverseArgList->list != NULL)
		traverseArgList = traverseArgList->list;

	traverseArgList->list = newArg;

	return argList;
}



//Input:  A function name (type: NC) and an argument list
//Output: The function with its argument list set
NC* createFunction( NC *functionName, NC *argList )
{
	NC *functionPrimary;

	//Note that the function was created using createVariable
	//So, the 'v' in its Primary needs to be replaced by 'f'
	functionPrimary = functionName->link->link;
	functionPrimary->type = 'f';

	functionPrimary->link = argList;

	return functionName;
}



//Input:  A normalized expression x
//Output: Negate of x (Sum's and Term's of x are multiplied by -1)
NC* negateExpression( NC* expression )
{
	NC *temp, *newExpression;

	newExpression = copylist(expression);

	newExpression->inc = (newExpression->inc) * -1;
	temp = newExpression->link;

	while(temp != NULL)
	{
		temp->inc = (temp->inc) * -1;
		temp = temp->list;
	}

	return newExpression;
}


//Functions related to normalized conditional expressions
//Creation of normalized conditional expressions


//Input:  A relational operator (type: character)
//Output: The operator's index (type: int)
int getRelOperatorIndex( char *relOperator )
{
	if(strcmp(relOperator,"!=") == 0)
		return 5;
	if(strcmp(relOperator,"==") == 0)
		return 4;
	if(strcmp(relOperator,">=") == 0)
		return 0;
	if(strcmp(relOperator,">") == 0)
		return 1;
	if(strcmp(relOperator,"<=") == 0)
		return 2;
	if(strcmp(relOperator,"<") == 0)
		return 3;
	return -1;
}



//Input:  Two expressions x and y, and a relational operator op
//Output: One conditional expression x op y
NC* createConditionalExpression( NC *expr1, char *relOp, NC *expr2 )
{
	NC *andExpr, *orExpr, *relExpr;
	NC *newExpr1, *newExpr2;
	NC *newExpr;
	int operator;

	#ifdef Debug
	printf("In createConditionalExpression\n");
	#endif

	operator = getRelOperatorIndex(relOp);
	newExpr1 = copylist(expr1);
	newExpr2 = copylist(expr2);
	newExpr2 = negateExpression(newExpr2);
	newExpr = (NC*) malloc (sizeof(NC));
	Add_Sums(newExpr1, newExpr2, newExpr);


	relExpr = (NC*) malloc (sizeof(NC));
	relExpr->list = NULL;
	relExpr->type = 'R';
	relExpr->inc = operator;
	relExpr->link = newExpr;

	orExpr = (NC*) malloc (sizeof(NC));
	orExpr->list = NULL;
	orExpr->type = 'O';
	orExpr->inc = 0;
	orExpr->link = relExpr;

	andExpr = (NC*) malloc (sizeof(NC));
	andExpr->list = NULL;
	andExpr->type = 'A';
	andExpr->inc = 0;
	andExpr->link = orExpr;

	#ifdef Debug
	write_lists(andExpr);
	printf("\nOut of createConditionalExpression\n");
	#endif

	return andExpr;
}


//Functions for performing Boolean operations - NOT, AND, OR


//Input:  A conditional expression x (at OR level)
//Output: The conditional expression !x
NC* computeNegation( NC *cond )
{
	NC *retOr, *oldOr, *newOr;
	NC *iRel, *oneRel;
	NC *retNegCond, *jRet;

	//base case
	if(cond->list == NULL)
	{
		for(iRel = cond->link; iRel != NULL; iRel = iRel->list)
		{
			oneRel = (NC*) malloc (sizeof(NC));
			oneRel->list = NULL;
			oneRel->type = 'R';
			oneRel->inc = negateoperator(iRel->inc);
			oneRel->link = copylist(iRel->link);

			newOr = (NC*) malloc (sizeof(NC));
			newOr->list = NULL;
			newOr->type = 'O';
			newOr->inc = 0;
			newOr->link = oneRel;

			if(iRel == cond->link)
				retOr = newOr;
			else
				oldOr->list = newOr;

			oldOr = newOr;
		}

		return retOr;
	}

	//inductive case
	retNegCond = computeNegation( cond->list );

	for(iRel = cond->link; iRel != NULL; iRel = iRel->list)
	{
		for(jRet = retNegCond; jRet != NULL; jRet = jRet->list)
		{
			oneRel = (NC*) malloc (sizeof(NC));
			oneRel->list = copylist(jRet->link);
			oneRel->type = 'R';
			oneRel->inc = negateoperator(iRel->inc);
			oneRel->link = copylist(iRel->link);

			newOr = (NC*) malloc (sizeof(NC));
			newOr->list = NULL;
			newOr->type = 'O';
			newOr->inc = 0;
			newOr->link = oneRel;

			if(iRel == cond->link)
				retOr = newOr;
			else
				oldOr->list = newOr;

			oldOr = newOr;
		}
	}

	return retOr;
}



//Input:  A conditional expression x (at AND level)
//Output: The conditional expression !x
NC* negateConditionalExpression( NC *cond )
{
	NC *negCond;

	if(cond == NULL)
		return NULL;

	negCond = (NC*) malloc (sizeof(NC));
	negCond->list = NULL;
	negCond->type = 'A';
	negCond->inc = 0;
	negCond->link = computeNegation(cond->link);

	return negCond;
}



//Input:  Two conditions x and y
//Output: One condition x AND y (i.e., x && y)
NC* createAndExpression( NC *cond1, NC *cond2 )
{
	NC *traverseOr;
	NC *t1, *t2, *node;
	bool flag;

	if(cond1 == NULL && cond2 == NULL)
		return NULL;
	if(cond1 == NULL)
		return cond2;
	if(cond2 == NULL)
		return cond1;

	traverseOr = cond1->link;
	while(traverseOr->list != NULL)
		traverseOr = traverseOr->list;

	//Appending (ANDing) cond1 and cond2
	//traverseOr->list = cond2->link;
	//The above-mentioned simplest strategy is neglected
	//to avoid duplicating the same conjuncts

	t2 = cond2->link;
	while( t2 != (NC*)NULL )
	{
		flag = true;
		t1 = cond1->link;
		while( t1 != (NC*)NULL )
		{
			if(compare_trees(t1->link, t2->link) == 1)
			{
				flag = false;
				break;
			}
			t1 = t1->list;
		}

		if(flag)
		{
			node = (NC*)malloc(sizeof(NC));
			node->list = (NC*)NULL;
			node->type = 'O';
			node->inc = 0;
			node->link = copylist( t2->link );

			traverseOr->list = node;
			traverseOr = node;
		}

		t2 = t2->list;
	}

	return cond1;
}



//Input:  Two relations x = (a or b or c), and y = (b or d)
//Output: One relation z = (a or b or c or d) -- devoid of identical clauses
NC* compareRelations( NC *rel1, NC *rel2 )
{
	NC *copyRel1, *copyRel2;
	NC *r1, *r2, *tempR;
	bool flag;

	if(rel1 == NULL && rel2 == NULL)
		return NULL;
	if(rel1 == NULL)
		return rel2;
	if(rel2 == NULL)
		return rel1;

	copyRel1 = copylist(rel1);
	copyRel2 = copylist(rel2);

	tempR = copyRel1;
	while(tempR->list != NULL)
		tempR = tempR->list;

	for(r2 = copyRel2; r2 != NULL; r2 = r2->list)
	{
		flag = true;
		for(r1 = copyRel1; r1 != NULL; r1 = r1->list)
		{
			if(compare_trees(r1, r2) == 1)
			{
				flag = false;
				break;
			}
		}

		if(flag)
		{
			tempR->list = copylist( r2->link );
			tempR = tempR->list;
		}
	}

	return copyRel1;
}



//Input:  Two conditions x and y
//Output: One condition x OR y (i.e., x || y)
NC* createOrExpression( NC *cond1, NC *cond2 )
{
	NC *iCond1, *jCond2;
	NC *orCond, *node, *oldNode;

	if(cond1 == NULL && cond2 == NULL)
		return NULL;
	if(cond1 == NULL)
		return cond2;
	if(cond2 == NULL)
		return cond1;

	orCond = (NC*) malloc (sizeof(NC));
	orCond->list = NULL;
	orCond->type = 'A';
	orCond->inc = 0;

	for(iCond1 = cond1->link; iCond1 != NULL; iCond1 = iCond1->list)
	{
		for(jCond2 = cond2->link; jCond2 != NULL; jCond2 = jCond2->list)
		{
			node = (NC*) malloc (sizeof(NC));
			node->list = NULL;
			node->type = 'O';
			node->inc = 0;
			node->link = compareRelations(iCond1->link, jCond2->link);

			if(iCond1 == cond1->link && jCond2 == cond2->link)
				orCond->link = node;
			else
				oldNode->link = node;

			oldNode = node;
		}
	}

	return orCond;
}
