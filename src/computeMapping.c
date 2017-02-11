#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "normalization.h"
#include "addg.h"

extern unsigned int G_countOperands;
extern char G_namesOfOperands[3*MAX_INCR][MAX_NAME];

//The following global variable keeps track of RHS operands in an array assignment statement
unsigned int G_countOfRhsOperand;

//This function is used to print the normalized expressions in a string
//It will take the root of the binary tree as input and prints the corresponding expression
//This function is a modified form of write_lists_string in normalization.c
//The last Boolean variable DorM enforces printing the normalized epression in an infix form
//which is understood by ISL, in contrast with write_lists which prints such expressions in
//prefix form
void write_lists_string( NC *root, char str[], bool DorM )
{
	char *sym_value;
	sym_value = (char*) malloc (MAX_NAME * sizeof(char));
	if(root != (NC*)NULL)
	{
		if(root->type == 0) root->type = 'S';
		if(root->type == 'R' || root->type == 'O')
		{
			if(root->type == 'R')
				sprintf(eos(str), " ( ");
			write_lists_string(root->link, str, false);
		}

		switch(root->type)
		{
			case 'f':
				symbol_for_index(root->inc, sym_value);
				sprintf(eos(str), "* %s( ", sym_value);
				break;
			case 'v':
				symbol_for_index(root->inc, sym_value);
				sprintf(eos(str), "* %s ", sym_value);
				break;
			case 'T':
				sprintf(eos(str), "%c %d ", (root->inc >= 0)?'+':'-', abs(root->inc));
				break;
			case 'S':
				sprintf(eos(str), "%d ", root->inc);
				break;
			case 'R':
				switch(root->inc)
				{
					case 0: sprintf(eos(str), ">= 0"); break;
					case 1: sprintf(eos(str), "> 0" ); break;
					case 2: sprintf(eos(str), "<= 0"); break;
					case 3: sprintf(eos(str), "< 0" ); break;
					case 4: sprintf(eos(str),  "= 0");  break; //ISL uses "=" for equality testing (and not "==")
					case 5: sprintf(eos(str), "!= 0"); break;
				};// switch( root->inc)
				sprintf(eos(str), " ) ");
				if(root->list != (NC*)NULL)
					sprintf(eos(str), " or ");
				break;
			case 'A':
				break;
			case 'O':
				if(root->list != (NC*)NULL)
					sprintf(eos(str), " and ");
				break;
			case 'D':
				sprintf(eos(str), " * ( ");
				write_lists_string(root->link, str, true);
				sprintf(eos(str), " ) / ");
				write_lists_string(root->link->list, str, true);
				break;
			case 'M':
				sprintf(eos(str), " * ( ");
				write_lists_string(root->link, str, true);
				sprintf(eos(str), " ) %% ");
				write_lists_string(root->link->list, str, true);
				break;
			case 'w':
				sprintf(eos(str), "write ( ");
				break;
			case 'y':
				symbol_for_index(root->inc, sym_value);
				sprintf(eos(str), "%s, ", sym_value);
				break;
			case 'a':
				symbol_for_index(root->inc, sym_value);
				sprintf(eos(str), "* read ( %s, ", sym_value);
				break;
			default:
				break;
		}; // switch( root->type )

		if(root->type != 'R' && root->type != 'O' && root->type != 'M' && root->type != 'D')
			write_lists_string(root->link, str, false);
		if(root->type == 'f' || root->type == 'w') //2nd clause is for arrays
			sprintf(eos(str), ")");
		if(root->type == 'y')
			sprintf(eos(str), ", ");
		//A closing bracket has to be put explicitly in case the arrays
		//are of only one dimension
		if(root->type == 'a' && root->link->list == (NC*)NULL)
			sprintf(eos(str), ")");

		if(root->type == 'S' && root->list != (NC*)NULL && !DorM)
			sprintf(eos(str), ", ");

		if(!DorM)
			write_lists_string(root->list, str, false);
	}
	return;
}



//This function invokes ISL with an input string and a character array
//where the output is to be stored
//NB: This is the only function that interacts with ISL
void invokeISL( char input[], char output[] )
{
	unsigned int l;
	char c;
	FILE* fp;

	fp = fopen("input4isl.txt", "w");
	fprintf(fp,"%s", input);
	fclose(fp);

	if(system(PATH_ISL "< input4isl.txt 2>&1 | tee islOutput.txt") == -1)
	{
		printf("\nSystem command could not create child process.\nExiting System.\n");
		exit(0);
	}

	//Now we need to determine whether the input given to ISL was correct or not
	//This is done by searching for the word "error" (which basically belongs to "syntax error")
	//in the file which contains the output of ISL
	if(system("grep -n error islOutput.txt > grepOutput.txt") == -1)
	{
		printf("\nSystem command could not create child process.\nExiting System.\n");
		exit(0);
	}
	//NB: The command "grep -n error" returns the line numbers of those lines which contain the word "error"
	//Hence it suffices to check if the first character in the file "grepOutput.txt" is an integer digit or not
	fp = fopen("grepOutput.txt", "r");
	c = getc(fp);
	if(c >= '0' && c <= '9')
	{
		fclose(fp);
		printf("\nError in input given to ISL.\nExiting System.\n");
		exit(0);
	}
	fclose(fp);

	//Control reaches this line of the function indicates that the output of ISL has been properly generated
	strcpy(output, "M := ");
	l = strlen(output);
	fp = fopen("islOutput.txt", "r");
	c = getc(fp);
	while(c != EOF)
	{
		output[l++] = c;
		c = getc(fp);
	}
	fclose(fp);
	//Add the last character as ';' instead of '\n'
	output[l-1] = ';';
	output[l] = '\0';
}



//This function computes the dependence mapping between definition domain and operand domain
void computeDependenceMapping( char S2L[], char S2R[], unsigned int childId, char M[] )
{
	char input4isl[3*SIZE];

	sprintf(input4isl, "%s\n%s\n", S2L, S2R);
	sprintf(eos(input4isl), "S2LI := S2L^-1;\nM :=  S2R%u( S2LI );\nM;\n", childId);

	invokeISL(input4isl, M);
}



//Insert the string "parameter" into the string "mapping" at the proper place
void insertParameterToMapping( char parameter[], char mapping[] )
{
	int i, j, k;
	char newMapping[SIZE];

	for(i = 0; mapping[i] != '='; i++)
		newMapping[i] = mapping[i];

	newMapping[i] = '=';
	newMapping[i + 1] = '\0';
	strcat(newMapping, parameter);

	//NB: mapping[i + 2] points to '{'
	for(j = strlen(newMapping), k = i + 2; mapping[k]; j++, k++)
		newMapping[j] = mapping[k];

	newMapping[j] = '\0';

	//Copy newMapping into mapping
	strcpy(mapping, newMapping);
}



//This function finds the parameters (variables) in a normalized expression
void addParametersToMappings( char indexVarList[][MAX_NAME],  unsigned int countIndex,
                              char parameterList[][MAX_NAME], unsigned int countParam,
                              char S2L[], char S2R[][SIZE],   unsigned int outDegree )
{
	char parameter[SIZE];
	unsigned int i, j, numParams;
	bool flagAdd;

	#ifdef Debug
	for(i=0; i < countIndex; i++) printf("Index: %s\n", indexVarList[i]);
	for(i=0; i < countParam; i++) printf("Param: %s\n",parameterList[i]);
	#endif

	strcpy(parameter, " [");
	numParams = 0;
	for(i = 0; i < countParam; i++)
	{
		flagAdd = true;
		//Check if some parameter is actually an index variable or not
		for(j = 0; j < countIndex; j++)
		{
			if(strcmp(parameterList[i], indexVarList[j]) == 0)
			{
				flagAdd = false;
				break;
			}
		}//end for j
		if(flagAdd)
		{
			//Check if some parameter has already been covered or not
			for(j = 0; j < i; j++)
			{
				if(strcmp(parameterList[i], parameterList[j]) == 0)
				{
					flagAdd = false;
					break;
				}
			}//end for j
		}
		if(flagAdd)
		{
			//Add the parameter
			sprintf(eos(parameter), " %s,", parameterList[i]);
			numParams++;
		}
	}//end for i

	if(numParams == 0)
		return; //Nothing to do

	//Remove the last comma in the string "parameter"
	parameter[strlen(parameter) - 1] = '\0';
	strcat(parameter, " ] -> ");

	//Insert parameter to mappings S2L and S2R
	insertParameterToMapping(parameter, S2L);
	for(i = 0; i < outDegree; i++)
		insertParameterToMapping(parameter, S2R[i]);
}


//This function extracts the operand indices from a Variable node
void findOperandIndicesVariable( NC *exprn, char S2R[][SIZE] )
{
	sprintf(eos(S2R[G_countOfRhsOperand]), "0 ");
	G_countOfRhsOperand++;

	if(exprn->list != (NC*)NULL)
		findOperandIndicesVariable( exprn->list, S2R );
}


//This function extracts the operand indices from an Array node
void findOperandIndicesArray( NC *exprn, char S2R[][SIZE] )
{
	write_lists_string(exprn->link, S2R[G_countOfRhsOperand], false);
	G_countOfRhsOperand++;

	if(exprn->list != (NC*)NULL)
		findOperandIndicesArray( exprn->list, S2R );
}


//This function extracts the operand indices from a Div or a Mod or a Function node
void findOperandIndicesDivModFunc( NC *exprn, char S2R[][SIZE] )
{
	findOperandIndicesSum( exprn->link, S2R );
}


//This function extracts the operand indices from a Term node
void findOperandIndicesTerm( NC *exprn, char S2R[][SIZE] )
{
	if(exprn->link->type == 'D' || exprn->link->type == 'M' || exprn->link->type == 'f')
		findOperandIndicesDivModFunc( exprn->link, S2R );
	else if(exprn->link->type == 'a')
		findOperandIndicesArray( exprn->link, S2R );
	else //'v'
		findOperandIndicesVariable( exprn->link, S2R );

	if(exprn->list != (NC*)NULL)
		findOperandIndicesTerm( exprn->list, S2R );
}


//This function extracts the operand indices from a Sum node
void findOperandIndicesSum( NC *exprn, char S2R[][SIZE] )
{
	if(exprn->link != (NC*)NULL)
		findOperandIndicesTerm( exprn->link, S2R );
	if(exprn->list != (NC*)NULL)
		findOperandIndicesSum( exprn->list, S2R );
}



//This function computes the mappings corresponding to an OPERATOR vertex in an ADDG
//NB: We expect parameters to exist only in the
// (i) initialization expressions of index variables in the FOR loops, and
//(ii) conditions of the FOR loops and IF statements
//The increment of the index variables should be of the form "i += c", where "c" is a specific integer
//The number of initializations and the number of increments in a FOR loop must be equal, and
//done in the same order of index variables
void computeMappingVertex( VERTEX *node )
{
	unsigned int i, j, k, countIndex;
	SINGSTMT *tempStmt;
	char variable[MAX_NAME], conditions[SIZE], extVariable, exists[SIZE], indexVarList[MAX_INCR][MAX_NAME];
	NC *tempNorm;

	if(node == (VERTEX*)NULL)
		return;

	#ifdef Debug
	printf("In computeMappingVertex: %s\n", node->name);
	#endif

	if(node->type == ARRAY)
	{
		printf("Warning: computeMappingVertex invoked with an ARRAY type vertex: %s\n", node->name);
		return;
	}
	if(node->map==(MAP*)NULL || node->map->stmt->singleStmt==(DATA_TRANS*)NULL)
	{
		printf("Data transformation not found for operator vertex: %s.\nExiting System.\n", node->name);
		exit(0);
	}

	countIndex = 0;

	strcpy(node->map->mappingS2L, "S2L := { [");
	for(i = 0; i < node->outDegree; i++)
		sprintf(node->map->mappingS2R[i], "S2R%u := { [", i);

	tempStmt = node->map->stmt;
	//Find the iterators of the enclosing FOR loops
	for(i = 0; i < tempStmt->count_for; i++)
	{
		for(j = 0; j < tempStmt->forStmt[i]->count_init; j++)
		{
			symbol_for_index(tempStmt->forStmt[i]->init[j]->LHS->link->link->inc, variable);

			//Add the variable to index variable list
			strcpy(indexVarList[countIndex++], variable);

			if(i==(tempStmt->count_for - 1) && j==(tempStmt->forStmt[i]->count_init - 1))
			{
				sprintf(eos(node->map->mappingS2L), "%s] -> [ ", variable);
				for(k = 0; k < node->outDegree; k++)
					sprintf(eos(node->map->mappingS2R[k]), "%s] -> [ ", variable);
			}
			else
			{
				sprintf(eos(node->map->mappingS2L), "%s, ", variable);
				for(k = 0; k < node->outDegree; k++)
					sprintf(eos(node->map->mappingS2R[k]), "%s, ", variable);
			}
		}//end for j
	}//end for i

	//The following code takes care of the case when there is no enclosing FOR loop
	if(tempStmt->count_for == 0)
	{
		sprintf(eos(node->map->mappingS2L), " 0 ] -> [ ");
		for(i = 0; i < node->outDegree; i++)
			sprintf(eos(node->map->mappingS2R[i]), " 0 ] -> [ ");
	}
	G_countOperands = 0;

	//Find the mapping between iteration domain and definition domain
	tempNorm = tempStmt->singleStmt->LHS->link->link->link;
	if(tempNorm != (NC*)NULL)
	{
		//The LHS variable is an array
		write_lists_string(tempNorm, node->map->mappingS2L, false);

		//Find the parameters that may be present in the LHS array index
		extractNamesOfOperands(tempNorm, true, false);
	}
	else
	{
		//The LHS variable is a scalar variable
		strcat(node->map->mappingS2L, "0 ");
	}

	if(tempStmt->count_for != 0 || tempStmt->count_if != 0)
		strcat(node->map->mappingS2L, "] :");
	else
		strcat(node->map->mappingS2L, "]");

	//Find the mapping between iteration domain and each operand domain
	G_countOfRhsOperand = 0;
	if(tempStmt->singleStmt->rhs->link == (NC*)NULL)
	{
		//This case indicates that initialization with constant has occurred, i.e.,
		//fictitious input arrays are involved
		strcat(node->map->mappingS2R[0], "0 ");
		G_countOfRhsOperand = 1;
	}
	else
		findOperandIndicesSum(tempStmt->singleStmt->rhs, node->map->mappingS2R);

	//Find the parameters that may be present in the RHS expression
	extractNamesOfOperands(tempStmt->singleStmt->rhs, true, false);

	if(tempStmt->count_for != 0 || tempStmt->count_if != 0)
	{
		for(i = 0; i < G_countOfRhsOperand; i++)
			strcat(node->map->mappingS2R[i], "] :");
	}
	else
	{
		for(i = 0; i < G_countOfRhsOperand; i++)
			strcat(node->map->mappingS2R[i], "]");
	}

	//Determine the conditions to be added to each mapping
	strcpy(conditions, "");
	//First, we add the conditions involving the enclosing FOR loops
	for(i = 0; i < tempStmt->count_for; i++)
	{
		//1.1: Add the conditions involving the initialization statements
		//of the FOR loop
		if(tempStmt->forStmt[i]->count_init != tempStmt->forStmt[i]->count_incr)
		{
			printf("\nError: The number of initializations and the number of increments in the following FOR loop are not equal.\n" );
			printForStatement(tempStmt->forStmt[i]);
			printf("\nExiting System.\n");
			exit(0);
		}
		for(j = 0; j < tempStmt->forStmt[i]->count_init; j++)
		{
			strcat(conditions, " ( ");
			write_lists_string(tempStmt->forStmt[i]->init[j]->rhs, conditions, false);
			if(tempStmt->forStmt[i]->incr[j]->rhs->link == (NC*)NULL)
			{
				printf("\nError: Incrementation in a FOR loop involves only a constant.\n");
				printForStatement(tempStmt->forStmt[i]);
				printf("\nExiting System.\n");
				exit(0);
			}

			symbol_for_index(tempStmt->forStmt[i]->init[j]->LHS->link->link->inc, variable);
			if(tempStmt->forStmt[i]->incr[j]->rhs->inc > 0)
				sprintf(eos(conditions), " <= %s ) and ", variable);
			else
				sprintf(eos(conditions), " >= %s ) and ", variable);
		}

		//1.2: Add the conditions of the FOR loop
		write_lists_string(tempStmt->forStmt[i]->condition, conditions, false);
		strcat(conditions, " and ");

		//Find the parameters that may be present in the condition
		extractNamesOfOperands(tempStmt->forStmt[i]->condition, true, true);
	}
	//Second, we add the conditions of the enclosing IF statements
	for(i = 0; i < tempStmt->count_if; i++)
	{
		write_lists_string(tempStmt->ifStmt[i]->condition, conditions, false);
		strcat(conditions, " and ");

		//Find the parameters that may be present in the condition
		extractNamesOfOperands(tempStmt->ifStmt[i]->condition, true, true);
	}

	//Add the conditions to each mapping
	if(tempStmt->count_if != 0 && tempStmt->count_for == 0)
	{
		conditions[strlen(conditions) - 5] = '\0'; //To remove the last " and " (5 characters)
		strcat(node->map->mappingS2L, conditions);
		for(i = 0; i < node->outDegree; i++)
			strcat(node->map->mappingS2R[i], conditions);
	}
	else
	{
		strcat(node->map->mappingS2L, conditions);
		for(i = 0; i < node->outDegree; i++)
			strcat(node->map->mappingS2R[i], conditions);
	}

	//Determine the existential expressions to be added to each mapping
	extVariable = 'A'; //initial existential variable
	strcpy(exists, "");
	for(i = 0; i < tempStmt->count_for; i++)
	{
		for(j = 0; j < tempStmt->forStmt[i]->count_init; j++)
		{
			sprintf(eos(exists), "( exists %c: ", extVariable);
			symbol_for_index(tempStmt->forStmt[i]->init[j]->LHS->link->link->inc, variable);
			sprintf(eos(exists), "%s = ", variable);
			write_lists_string(tempStmt->forStmt[i]->init[j]->rhs, exists, false);
			sprintf(eos(exists), " + %d * %c ) and ", tempStmt->forStmt[i]->incr[j]->rhs->inc, extVariable);
			extVariable++;

			//Find the parameters that may be present in the index variable initialization
			extractNamesOfOperands(tempStmt->forStmt[i]->init[j]->rhs, true, true);
		}//end for j
	}//end for i
	exists[strlen(exists) - 5] = '\0'; //To remove the last " and " (5 characters)

	//Add the existential expressions to each mapping
	if(tempStmt->count_for != 0)
	{
		strcat(node->map->mappingS2L, exists);
		for(i = 0; i < node->outDegree; i++)
			strcat(node->map->mappingS2R[i], exists);
	}

	strcat(node->map->mappingS2L, " };");
	for(i = 0; i < node->outDegree; i++)
		strcat(node->map->mappingS2R[i], " };");

	//Add the parameters, if any, to S2L and each S2R
	addParametersToMappings(indexVarList, countIndex, G_namesOfOperands, G_countOperands, node->map->mappingS2L, node->map->mappingS2R, node->outDegree);

	//Find the mapping between definition domain and each operand domain
	for(i = 0; i < node->outDegree; i++)
		computeDependenceMapping(node->map->mappingS2L, node->map->mappingS2R[i], i, node->map->mappingM[i]);

	#ifdef Debug
	printf("Out of computeMappingVertex: %s\n", node->name);
	#endif
}



//This function computes the mappings of each (OPERATOR) vertex in an ADDG
void computeMappingADDG( ADDG *M )
{
	unsigned int i;

	if(M == (ADDG*)NULL)
		return;

	for(i = 0; i < M->numVertices; i++)
	{
		if(M->vertex[i]->type == OPERATOR)
			computeMappingVertex(M->vertex[i]);
	}
}
