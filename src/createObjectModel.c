#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "normalization.h"
#include "addg.h"


//This function creates an object of type DATA_TRANS
DATA_TRANS* createDataTrans( NC *lhs, char *operator, DATA_TRANS *rhs, bool flag )
{
	DATA_TRANS *newStmt;
	NC *newrhs;

	#ifdef Debug
	printf("In createDataTrans\n");
	#endif

	newStmt = (DATA_TRANS*) malloc (sizeof(DATA_TRANS));

	//flag = false implies that we need to store only the rhs expression
	//flag = true  implies that we need to create entire DATA_TRANS
	if( !flag )
	{
		newStmt->lhs = -1;
		newStmt->LHS = (NC*)NULL;
		newStmt->rhs = lhs;
		return newStmt;
	}

	newStmt->lhs = -1;
	newStmt->LHS = lhs;

	newrhs = (NC*) malloc (sizeof(NC));
	//Determine the rhs expression based on the operator
	if( strcmp(operator,"=")==0 )
		newrhs = rhs->rhs;
	else if( strcmp(operator,"*=")==0 )
		newrhs = Mult_Sum_With_Sum(lhs, rhs->rhs, newrhs);
	else if( strcmp(operator,"/=")==0 )
		newrhs = createExpression_Mod_Div(lhs, rhs->rhs, '/');
	else if( strcmp(operator,"%=")==0 )
		newrhs = createExpression_Mod_Div(lhs, rhs->rhs, '%');
	else if( strcmp(operator,"+=")==0 )
		newrhs = Add_Sums(lhs, rhs->rhs, newrhs);
	else if( strcmp(operator,"-=")==0 )
		newrhs = Add_Sums(lhs, negateExpression(rhs->rhs), newrhs);
	else
	{
		printf("Operator not supported: %s\nExiting System.\n",operator);
		exit(0);
	}
	newStmt->rhs = newrhs;

	#ifdef Debug
	printDataTrans(newStmt);
	printf("Out of createDataTrans\n");
	#endif

	return newStmt;
}


//This function prints an object of type DATA_TRANS
void printDataTrans( DATA_TRANS *dataTrans )
{
	char *lhsVariable;

	#ifdef Debug
	printf("In printDataTrans\n");
	#endif

	if(dataTrans == (DATA_TRANS*)NULL)
		return;

	if(dataTrans->lhs != -1)
	{
		lhsVariable = (char*) malloc (MAX_NAME * sizeof(char));
		symbol_for_index(dataTrans->lhs, lhsVariable);
		printf("\n %s := ", lhsVariable);
	}

	if(dataTrans->LHS != (NC*)NULL)
	{
		write_lists(dataTrans->LHS);
		printf(" := ");
	}

	if(dataTrans->rhs != (NC*)NULL)
	{
		write_lists(dataTrans->rhs);
	}

	#ifdef Debug
	printf("Out of printDataTrans\n");
	#endif
}



//This function returns a copy of a DATA_TRANS
DATA_TRANS* copyDataTrans( DATA_TRANS *source )
{
	DATA_TRANS *dest;

	if(source == (DATA_TRANS*)NULL)
		return NULL;

	dest = (DATA_TRANS*) malloc (sizeof(DATA_TRANS));
	dest->lhs = source->lhs;
	dest->LHS = copylist(source->LHS);
	dest->rhs = copylist(source->rhs);
	return dest;
}



//This function prints an object of type STMTBLK
void printSingleStatement( SINGSTMT *singStmt )
{
	int i;

	if(singStmt == (SINGSTMT*)NULL)
		return;

	printf("\n***Start of Single Statement***");

	if(singStmt->count_for != 0)
	{
		printf("\nEncompassing FOR statements:");
		for(i = singStmt->count_for-1; i >= 0; i--)
			printForStatement(singStmt->forStmt[i]);
	}

	if(singStmt->count_if != 0)
	{
		printf("\nEncompassing IF statements:\n");
		for(i = singStmt->count_if-1; i >= 0; i--)
			printIfStatement(singStmt->ifStmt[i]);
	}

	if(singStmt->count_for==0 && singStmt->count_if==0)
		printf("\n");

	printDataTrans(singStmt->singleStmt);

	printf("\n***End of Single Statement***\n");
}



//This function creates an object of type IFSTMT
STMTBLK* assignIfStatement( STMTBLK *condition, STMTBLK *trueChild, STMTBLK *falseChild )
{
	IFSTMT *newIfStmtTrue, *newIfStmtFalse;
	unsigned int i;

	newIfStmtTrue = (IFSTMT*) malloc (sizeof(IFSTMT));
	newIfStmtTrue->condition = copylist(condition->statement[0]->rhs);

	updateStatementBlock(trueChild, NULL, newIfStmtTrue, 1);

	if(falseChild == (STMTBLK*)NULL)
		return trueChild;

	newIfStmtFalse = (IFSTMT*) malloc (sizeof(IFSTMT));
	newIfStmtFalse->condition = negateConditionalExpression( condition->statement[0]->rhs );

	updateStatementBlock(falseChild, NULL, newIfStmtFalse, 1);

	for(i = 0; i < falseChild->count_stmts; i++)
	{
		trueChild->statement[trueChild->count_stmts + i] = falseChild->statement[i];
		trueChild->stmtNumber[trueChild->count_stmts + i] = falseChild->stmtNumber[i];
	}

	trueChild->count_stmts = trueChild->count_stmts + falseChild->count_stmts;

	return trueChild;
}



//This function prints an object of type IFSTMT
void printIfStatement( IFSTMT *ifStmt )
{
	if(ifStmt == (IFSTMT*)NULL)
		return;


	printf("\n*Start of If Statement*");

	if(ifStmt->condition != (NC*)NULL)
	{
		printf("\nCondition: ");
		write_lists(ifStmt->condition);
	}

	printf("\n*End of If Statement*\n");
}



//This function creates an object of type FORSTMT
STMTBLK* assignForStatement( STMTBLK *init, STMTBLK *condition, STMTBLK *incr, STMTBLK *body )
{
	FORSTMT *newForStmt;
	unsigned int i;

	newForStmt = (FORSTMT*) malloc (sizeof(FORSTMT));

	newForStmt->count_init = init->count_stmts;
	for(i = 0; i < init->count_stmts; i++)
		newForStmt->init[i] = copyDataTrans(init->statement[i]);

	newForStmt->condition = copylist(condition->statement[0]->rhs);

	newForStmt->count_incr = incr->count_stmts;
	for(i = 0; i < incr->count_stmts; i++)
		newForStmt->incr[i] = copyDataTrans(incr->statement[i]);

	updateStatementBlock( body, newForStmt, NULL, 0);

	return body;
}



//This function prints an object of type FORSTMT
void printForStatement( FORSTMT *forStmt )
{
	unsigned int i;

	if(forStmt == (FORSTMT*)NULL)
		return;

	printf("\n*Start of For Statement*");

	printf("\nInitialization:\n");
	for(i = 0; i < forStmt->count_init; i++)
	{
		printDataTrans(forStmt->init[i]);
	}

	if(forStmt->condition != (NC*)NULL)
	{
		printf("\nCondition: ");
		write_lists(forStmt->condition);
	}

	printf("\nIncrementation:\n");
	for(i = 0; i < forStmt->count_incr; i++)
	{
		printDataTrans(forStmt->incr[i]);
	}

	printf("\n*End of For Statement*\n");
}


extern SINGSTMT *G_PtrStmt[MAX_SINGSTMT];
extern unsigned int G_countStmt;

//This function prints the entire program (object model)
void printProgram( void )
{
	unsigned int i;

	#ifdef Debug
	printf("Starting printProgram\n");
	#endif

	printf("tt %u\n",G_countStmt);

	for(i = 0; i < G_countStmt; i++)
		printSingleStatement(G_PtrStmt[i]);

	#ifdef Debug
	printf("End of printProgram\n");
	#endif
}



//This function checks whether a SingleStatement corresponding to a stement
//in the program already exists or not. If it exists then the pointer to
//that SingleStatement is returned, otherwise a new SingleStatement is
//created and then that pointer is returned.
//NB: The stmtNumber associated with each SingleStatement permits us to
//uniquely identify whether a SingleStatement exists or not
SINGSTMT* existSingleStatement( DATA_TRANS *dt, unsigned int stmtNumber )
{
	unsigned int i;

	for(i = 0; i < G_countStmt; i++)
	{
		if(stmtNumber == G_PtrStmt[i]->stmtNumber)
				return G_PtrStmt[i];
	}

	//A new SingleStatement has to be created
	G_PtrStmt[G_countStmt] = (SINGSTMT*) malloc (sizeof(SINGSTMT));
	G_PtrStmt[G_countStmt]->count_for = 0;
	G_PtrStmt[G_countStmt]->count_if = 0;
	G_PtrStmt[G_countStmt]->singleStmt = dt;
	G_PtrStmt[G_countStmt]->stmtNumber = stmtNumber;
	G_countStmt++;
	return G_PtrStmt[G_countStmt - 1];
}



//This function creates and updates all SingleStatement's in a StatementBlock
void updateStatementBlock( STMTBLK *blk, FORSTMT *forstmt, IFSTMT *ifstmt, int flag )
{
	SINGSTMT *stmt;
	unsigned int i;

	for(i = 0; i < blk->count_stmts; i++)
	{
		stmt = existSingleStatement(blk->statement[i], blk->stmtNumber[i]);

		switch(flag)
		{
			case 0: //FOR statement
				stmt->forStmt[stmt->count_for] = forstmt;
				(stmt->count_for)++;
				break;
			case 1: //IF statement
				stmt->ifStmt[stmt->count_if] = ifstmt;
				(stmt->count_if)++;
				break;
			case 2: //Expression statement (not enclosed in FOR or IF)
				//nothing to do, existSingleStatement has already taken care of it
				break;
			default:
				printf("Entered updateStatementBlock with incompatible flag value.\nExiting System.\n");
				exit(0);
		}
	}//end for blk->count_stmts
}



//This function prints a statement block
void printStatementBlock( STMTBLK *blk )
{
	unsigned int i;

	if(blk == (STMTBLK*)NULL)
		return;

	printf("Number of statements: %u\n", blk->count_stmts);
	for(i = 0; i < blk->count_stmts; i++)
	{
		printf("Statement Id: %u\n", blk->stmtNumber[i]);
		printDataTrans(blk->statement[i]);
	}
}



//This function merges two StatementBlock's and returns the resulting
//combined StatementBlock
STMTBLK* mergeStatementBlocks( STMTBLK *blk1, STMTBLK *blk2 )
{
	STMTBLK *combBlk;
	unsigned int i;

	if(blk1==(STMTBLK*)NULL && blk2==(STMTBLK*)NULL)
		return (STMTBLK*)NULL;
	if(blk1==(STMTBLK*)NULL)
		return blk2;
	if(blk2==(STMTBLK*)NULL)
		return blk1;

	combBlk = (STMTBLK*) malloc (sizeof(STMTBLK));

	for(i = 0; i < blk1->count_stmts; i++)
	{
		combBlk->statement[i] = copyDataTrans(blk1->statement[i]);
		combBlk->stmtNumber[i] = blk1->stmtNumber[i];
	}
	for(i = 0; i < blk2->count_stmts; i++)
	{
		combBlk->statement[blk1->count_stmts + i] = copyDataTrans(blk2->statement[i]);
		combBlk->stmtNumber[blk1->count_stmts + i] = blk2->stmtNumber[i];
	}

	combBlk->count_stmts = blk1->count_stmts + blk2->count_stmts;

	return combBlk;
}
