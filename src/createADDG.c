#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "normalization.h"
#include "addg.h"


extern SINGSTMT *G_PtrStmt[MAX_SINGSTMT];
extern unsigned int G_countStmt;

extern unsigned int G_countRecurrence;
extern char G_namesOfRecurrenceArray[MAX_CHILD][MAX_NAME];

//The following global variables are required to find the operands
//in a normalized expression
unsigned int G_countOperands;
char G_namesOfOperands[3 * MAX_INCR][MAX_NAME];

//The following global variable keeps count of fictitious arrays,
//which are created when initialization with some contant value is done
unsigned int G_countFictitious;

//The following global variables are required to maintain a stack of vertices
int Top;
VERTEX *STACK[NO_OF_VERTICES];


//This function extracts the names of the operand from a Variable node
void extractNamesFromVariable( NC *exprn, bool includeIndices, bool includeArrays )
{
	symbol_for_index(exprn->inc, G_namesOfOperands[G_countOperands]);
	G_countOperands++;

	if(exprn->list != (NC*)NULL)
		extractNamesFromVariable( exprn->list, includeIndices, includeArrays );
}


//This function extracts the names of the operands from an Array node
void extractNamesFromArray( NC *exprn, bool includeIndices, bool includeArrays )
{
	if(includeArrays)
	{
		symbol_for_index(exprn->inc, G_namesOfOperands[G_countOperands]);
		G_countOperands++;
	}

	if(includeIndices)
		extractNamesFromSum( exprn->link, includeIndices, includeArrays );

	if(exprn->list != (NC*)NULL)
		extractNamesFromArray( exprn->list, includeIndices, includeArrays );
}


//This function extracts the names of the operands from a Div or a Mod or a Function node
void extractNamesFromDivModFunc( NC *exprn, bool includeIndices, bool includeArrays )
{
	extractNamesFromSum( exprn->link, includeIndices, includeArrays );
}


//This function extracts the names of the operands from a Term node
void extractNamesFromTerm( NC *exprn, bool includeIndices, bool includeArrays )
{
	if(exprn->link->type == 'D' || exprn->link->type == 'M' || exprn->link->type == 'f')
		extractNamesFromDivModFunc( exprn->link, includeIndices, includeArrays );
	else if(exprn->link->type == 'a')
		extractNamesFromArray( exprn->link, includeIndices, includeArrays );
	else //'v'
		extractNamesFromVariable( exprn->link, includeIndices, includeArrays );

	if(exprn->list != (NC*)NULL)
		extractNamesFromTerm( exprn->list, includeIndices, includeArrays );
}


//This function extracts the names of the operands from a Sum node
void extractNamesFromSum( NC *exprn, bool includeIndices, bool includeArrays )
{
	if(exprn->link != (NC*)NULL)
		extractNamesFromTerm( exprn->link, includeIndices, includeArrays );
	if(exprn->list != (NC*)NULL)
		extractNamesFromSum( exprn->list, includeIndices, includeArrays );
}


//This function extracts the names of the operands from a Relation node
void extractNamesFromRelation( NC *exprn, bool includeIndices, bool includeArrays )
{
	extractNamesFromSum( exprn->link, includeIndices, includeArrays );
	if(exprn->list != (NC*)NULL)
		extractNamesFromRelation( exprn->list, includeIndices, includeArrays );
}


//This function extracts the names of the operands from an OR node
void extractNamesFromOr( NC *exprn, bool includeIndices, bool includeArrays )
{
	extractNamesFromRelation( exprn->link, includeIndices, includeArrays );
	if(exprn->list != (NC*)NULL)
		extractNamesFromOr( exprn->list, includeIndices, includeArrays );
}


//This function extracts the names of the operands from an AND node
void extractNamesFromAnd( NC *exprn, bool includeIndices, bool includeArrays )
{
	extractNamesFromOr( exprn->link, includeIndices, includeArrays );
}


//This function extracts the names of the operands from a normalized expression
void extractNamesOfOperands( NC *exprn, bool includeIndices, bool includeArrays )
{
	if(exprn == (NC*)NULL)
		return;

	if(exprn->type == 'A')
		extractNamesFromAnd( exprn, includeIndices, includeArrays );
	else if(exprn->type == 'S')
		extractNamesFromSum( exprn, includeIndices, includeArrays );
	else
		printf("Warning: The function extractNamesOfOperands has been invoked with a normalized expression which is neither And nor Sum.\n");
}



//This function checks (by name) whether an ADDG (Slice) already contains a node
//If yes, then the function returns the corresponding index in the ADDG
//Otherwise, it returns the value of NO_OF_VERTICES (maximum possible index of a
//vertex permitted in an ADDG + 1)
unsigned int existInADDG( char name[], ADDG *addg )
{
	unsigned int i;

	for(i = 0; i < addg->numVertices; i++)
	{
		if(strcmp(name, addg->vertex[i]->name) == 0)
			return i;
	}
	return NO_OF_VERTICES;
}



//This function checks whether an array node already exists corresponding
//to an array variable; if not, it creates a new array node
//flagI0 = false implies lhs array, flagIO = true implies rhs array
//The function returns the id corresponding to a node in an ADDG
unsigned int createVertexForArray( char name[], ADDG *addg, bool flagIO )
{
	unsigned int i;

	//Check whether nodes corrsponding to each operand exists or not
	i = existInADDG(name, addg);

	if(i != NO_OF_VERTICES)
	{
		addg->vertex[i]->hasParent = flagIO;
		return i;
	}
	else
	{
		addg->vertex[addg->numVertices] = (VERTEX*) malloc (sizeof(VERTEX));
		addg->vertex[addg->numVertices]->type = ARRAY;
		addg->vertex[addg->numVertices]->hasParent = flagIO;
		addg->vertex[addg->numVertices]->covered = false;
		strcpy(addg->vertex[addg->numVertices]->name, name);
		addg->vertex[addg->numVertices]->map = (MAP*)NULL;
		addg->vertex[addg->numVertices]->outDegree = 0;

		(addg->numVertices)++;
		return (addg->numVertices - 1);
	}
}



//This function creates a vertex corresponding to an operator in an ADDG
//and returns its corresponding Id
//NB: Every operation contributes towards a new vertex in the ADDG
unsigned int createVertexForOperator( SINGSTMT *stmt, ADDG *addg )
{
	unsigned int i;

	addg->vertex[addg->numVertices] = (VERTEX*) malloc (sizeof(VERTEX));
	addg->vertex[addg->numVertices]->type = OPERATOR;
	addg->vertex[addg->numVertices]->hasParent = true;
	addg->vertex[addg->numVertices]->covered = false;
	sprintf(addg->vertex[addg->numVertices]->name, "operator%u", stmt->stmtNumber);
	addg->vertex[addg->numVertices]->map = (MAP*) malloc (sizeof(MAP));
	addg->vertex[addg->numVertices]->map->stmt = stmt;
	strcpy(addg->vertex[addg->numVertices]->map->mappingS2L, "");
	for(i = 0; i < MAX_CHILD; i++)
	{
		strcpy(addg->vertex[addg->numVertices]->map->mappingS2R[i], "");
		strcpy(addg->vertex[addg->numVertices]->map->mappingM[i],   "");
		strcpy(addg->vertex[addg->numVertices]->map->mappingI2O[i], "");
	}

	(addg->numVertices)++;
	return (addg->numVertices - 1);
}



//This function creates vertices for an ADDG corresponding to a SingleStatement
void createVertices( SINGSTMT *stmt, ADDG *addg )
{
	unsigned int i, parentId, operatorId, childId;

	G_countOperands = 0;
	extractNamesOfOperands(stmt->singleStmt->LHS, false, true);
	if(G_countOperands != 1)
	{
		printf("LHS operand is not equal to one.\nExiting System.\n");
		exit(0);
	}
	parentId = createVertexForArray(G_namesOfOperands[0], addg, false);

	operatorId = createVertexForOperator(stmt, addg);

	G_countOperands = 0;
	extractNamesOfOperands(stmt->singleStmt->rhs, false, true);
	if(G_countOperands == 0)
	{
		if(stmt->singleStmt->rhs == (NC*)NULL)
		{
			printf("No RHS operand found.\nExiting System.\n");
			exit(0);
		}
		//This means initialization has been done with some constant value
		//Hence we need to create a fictitious array
		G_countOperands = 1;
		sprintf(G_namesOfOperands[0], "FICTITIOUS%u", G_countFictitious);
		G_countFictitious++;
	}
	addg->vertex[operatorId]->outDegree = G_countOperands;
	for(i = 0; i < G_countOperands; i++)
	{
		childId = createVertexForArray(G_namesOfOperands[i], addg, true);
		addg->vertex[operatorId]->child[i] = addg->vertex[childId];
	}

	addg->vertex[parentId]->child[addg->vertex[parentId]->outDegree] = addg->vertex[operatorId];
	(addg->vertex[parentId]->outDegree)++;
}



//This function creates an ADDG from G_PtrStmt
ADDG* createADDG( void )
{
	ADDG *addg;
	unsigned int i;

	if(G_countStmt == 0)
		return (ADDG*)NULL;

	G_countFictitious = 0;

	addg = (ADDG*) malloc (sizeof(ADDG));
	addg->numVertices = 0;
	addg->cslice = (CSLICE*)NULL;

	for(i = 0; i < G_countStmt; i++)
		createVertices(G_PtrStmt[i], addg);

	return addg;
}



//This function prints a VERTEX in an ADDG
void printVertex( VERTEX *vertex )
{
	unsigned int j;

	if(vertex == (VERTEX*)NULL)
	{
		printf("Vertex is empty.\n");
		return;
	}

	printf("\nNode: %s\n", vertex->name);
	switch(vertex->type)
	{
		case ARRAY:    printf("ARRAY Node\n");    break;
		case OPERATOR: printf("OPERATOR Node: "); printDataTrans(vertex->map->stmt->singleStmt); printf("\n"); break;
		default:       printf("ADDG Vertex: Type unknown.\nExiting System.\n"); exit(0);
	}
	switch(vertex->hasParent)
	{
		case false: printf("OUTPUT Node\n"); break;
		case true:  break;
	}
	printf("Out Degree: %u\n", vertex->outDegree);
	if(vertex->outDegree == 0)
		printf("INPUT Node\n");
	else
	{
		printf("Names of child nodes:\n");
		for(j = 0; j < vertex->outDegree; j++)
			printf("%s, ", vertex->child[j]->name);
		printf("\n");
	}

	if(vertex->type == OPERATOR)
	{
		if( strcmp(vertex->map->mappingS2L, "") != 0 )
			printf("%s\n", vertex->map->mappingS2L);
		for(j = 0; j < vertex->outDegree; j++)
		{
			if( strcmp(vertex->map->mappingS2R[j], "") != 0 )
				printf("%s\n", vertex->map->mappingS2R[j]);
			if( strcmp(vertex->map->mappingM[j], "") != 0 )
				printf("%s\n", vertex->map->mappingM[j]);
			if( strcmp(vertex->map->mappingI2O[j], "") != 0 )
				printf("%s\n", vertex->map->mappingI2O[j]);
		}
	}//end if OPERATOR
}



//This function prints an ADDG
void printADDG( ADDG *addg )
{
	unsigned int i;

	if(addg == (ADDG*)NULL)
	{
		printf("ADDG is empty.\n");
		return;
	}

	printf("\n***Start of ADDG***\n");
	for(i = 0; i < addg->numVertices; i++)
		printVertex(addg->vertex[i]);

	if(addg->cslice != (CSLICE*)NULL)
	{
		printf("\nCharacteristics of the slice:\n");
		printDataTrans(addg->cslice->dataTransformation);
		printf("\nNumber of input arrays: %u\n", addg->cslice->numOfInputs);
		for(i = 0; i < addg->cslice->numOfInputs; i++)
			printf("%s:: %s\n", addg->cslice->nameOfInput[i], addg->cslice->mappingIO[i]);
	}

	printf("\n***End of ADDG***\n");
}



//This function inserts a new node in an ADDG in order to decycle it
VERTEX* insertNewNode( VERTEX *vertex, ADDG *addg )
{
	char name[MAX_NAME];
	unsigned int i;

	//First check whether a new node corresponding to vertex has already
	//been inserted in addg or not
	sprintf(name, "%s__1", vertex->name);
	for(i = 0; i < addg->numVertices; i++)
	{
		if(strcmp(name, addg->vertex[i]->name) == 0)
			return addg->vertex[i];
	}

	addg->vertex[addg->numVertices] = (VERTEX*) malloc (sizeof(VERTEX));
	addg->vertex[addg->numVertices]->type = vertex->type;
	addg->vertex[addg->numVertices]->hasParent = true;
	addg->vertex[addg->numVertices]->covered = false;
	strcpy(addg->vertex[addg->numVertices]->name, name);
	addg->vertex[addg->numVertices]->map = (MAP*)NULL;
	addg->vertex[addg->numVertices]->outDegree = 0;

	#ifdef ALLOW_SUBS
	//Add the newly created vertex to the list of recurrence arrays
	strcpy(G_namesOfRecurrenceArray[G_countRecurrence], vertex->name);
	G_countRecurrence++;
	#endif

	(addg->numVertices)++;
	return addg->vertex[addg->numVertices - 1];
}



//This function traverses an ADDG in depth-first manner to detect cycles
void dfsADDG( VERTEX *vertex, ADDG *addg )
{
	unsigned int i;

	if(vertex == (VERTEX*)NULL)
		return;

	vertex->covered = true;

	#ifdef UNINTERPRETED
	//Push the vertex into the stack
	STACK[++Top] = vertex;
	#endif

	for(i = 0; i < vertex->outDegree; i++)
	{
		if(vertex->child[i]->covered == false)
		{
			//Continue DFS traversal
			dfsADDG(vertex->child[i], addg);
		}
		else
		{
			#ifdef UNINTERPRETED
			findSCC(vertex->child[i], addg);
			#endif

			//Time to insert a new node (input array)
			vertex->child[i] = insertNewNode(vertex->child[i], addg);
		}
	}
	vertex->covered = false;

	#ifdef UNINTERPRETED
	//Pop the vertex from the stack
	Top--;
	#endif
}



//This function removes the cycles, if any, from an ADDG
void decycleADDG( ADDG *addg )
{
	unsigned int i;

	if(addg == (ADDG*)NULL)
		return;

	#ifdef ALLOW_SUBS
		G_countRecurrence = 0;
	#endif

	//In the general case more than one output nodes is possible
	for(i = 0; i < addg->numVertices; i++)
	{
		if(addg->vertex[i]->hasParent == false)
		{
			dfsADDG(addg->vertex[i], addg);
		}
	}
}
