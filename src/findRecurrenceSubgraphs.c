#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "normalization.h"
#include "addg.h"

//The following definition is for those statements which correspond to
//uninterpreted functions introduced in lieu of recurrences
#define FAKE_STMT_NUMBER 1000

extern int Top;
extern VERTEX *STACK[NO_OF_VERTICES];

//The following global variable indicates whether it is the original ADDG
//or the transformed ADDG
unsigned int G_currentAddg;

//The following global variable stores the recurrence subgraphs in an ADDG
SETSGRAPH *G_RecurrenceSubgraph[2];


//This function initializes the subgraphs corresponding to an ADDG
void initializeSubgraphs( unsigned int addgId )
{
	G_currentAddg = addgId;

	G_RecurrenceSubgraph[addgId] = (SETSGRAPH*) malloc (sizeof(SETSGRAPH));
	G_RecurrenceSubgraph[addgId]->numOfRecur = 0;

	Top = -1;
}



//This function checks whether an SCC corresponding to a recurrence array
//is already present or not
unsigned int getSCCid( VERTEX *recurArray )
{
	unsigned int i;

	for(i = 0; i < G_RecurrenceSubgraph[G_currentAddg]->numOfRecur; i++)
	{
		if(G_RecurrenceSubgraph[G_currentAddg]->sgraph[i]->recurrenceArray == recurArray)
			return i;
	}

	//Control reaches here indicates that new SGRAPH has to be created
	G_RecurrenceSubgraph[G_currentAddg]->sgraph[i] = (SGRAPH*) malloc(sizeof(SGRAPH));
	G_RecurrenceSubgraph[G_currentAddg]->sgraph[i]->recurrenceArray = recurArray;
	G_RecurrenceSubgraph[G_currentAddg]->sgraph[i]->numOfVertices = 0;
	G_RecurrenceSubgraph[G_currentAddg]->sgraph[i]->numOfInputs   = 0;
	(G_RecurrenceSubgraph[G_currentAddg]->numOfRecur)++;
	return i;
}



//This function checks whether a vertex is already included in an SGRAPH or not
//It returns "true" if the vertex is not present, "false" otherwise
bool vertexNotPresent( VERTEX *vertex, SGRAPH *sgraph )
{
	unsigned int i;

	for(i = 0; i < sgraph->numOfVertices; i++)
	{
		if(sgraph->vertex[i] == vertex)
			return false;
	}

	return true;
}



//This function finds the strongly connected components in an ADDG
void findSCC( VERTEX *recurArray, ADDG *addg )
{
	int i;
	unsigned int recurId;
	SGRAPH *sgraph;

	if(recurArray == (VERTEX*)NULL || addg == (ADDG*)NULL)
		return;

	//Get the id (number of SCC) in the ADDG
	recurId = getSCCid( recurArray );
	sgraph = G_RecurrenceSubgraph[G_currentAddg]->sgraph[recurId];

	for(i = Top; /* Condition intentionally left blank */; i--)
	{
		if(vertexNotPresent(STACK[i], sgraph))
		{
			sgraph->vertex[sgraph->numOfVertices] = STACK[i];
			(sgraph->numOfVertices)++;
		}
		if(STACK[i] == recurArray)
			break;
	}
}



//This function finds the basis subgraph corresponding to a single SCC in an ADDG
void findBasisSubgraph( SGRAPH *sgraph )
{
	unsigned int i, j;

	for(i = 0; i < sgraph->recurrenceArray->outDegree; i++)
	{
		if(vertexNotPresent(sgraph->recurrenceArray->child[i], sgraph))
		{
			sgraph->vertex[sgraph->numOfVertices] = sgraph->recurrenceArray->child[i];
			(sgraph->numOfVertices)++;

			for(j = 0; j < sgraph->recurrenceArray->child[i]->outDegree; j++)
			{
				if(vertexNotPresent(sgraph->recurrenceArray->child[i]->child[j], sgraph))
				{
					sgraph->vertex[sgraph->numOfVertices] = sgraph->recurrenceArray->child[i]->child[j];
					(sgraph->numOfVertices)++;

					sgraph->inputArray[sgraph->numOfInputs] = sgraph->recurrenceArray->child[i]->child[j];
					(sgraph->numOfInputs)++;
				}
			}//end for j
		}
	}//end for i
}



//This function adds vertices to SGRAPH corresponding to an operator vertex in an SCC
void findInductionSubgraphOperatorVertex( VERTEX *operator, SGRAPH *sgraph )
{
	unsigned int i;

	for(i = 0; i < operator->outDegree; i++)
	{
		if(vertexNotPresent(operator->child[i], sgraph))
		{
			sgraph->vertex[sgraph->numOfVertices] = operator->child[i];
			(sgraph->numOfVertices)++;

			findInductionSubgraphArrayVertex(operator->child[i], sgraph);
		}
	}
}



//This function adds vertices to SGRAPH corresponding to an array vertex in an SCC
void findInductionSubgraphArrayVertex( VERTEX *array, SGRAPH *sgraph )
{
	unsigned int i, j;
	bool isInputArray;

	//Check if the array vertex needs to be designated as an input array or not
	isInputArray = true;
	for(i = 0; i < array->outDegree; i++)
	{
		for(j = 0; j < sgraph->recurrenceArray->outDegree; j++)
		{
			if(array->child[i]->map->stmt->count_for >= sgraph->recurrenceArray->child[j]->map->stmt->count_for &&
			   sgraph->recurrenceArray->child[j]->map->stmt->count_for > 0 &&
			   array->child[i]->map->stmt->forStmt[sgraph->recurrenceArray->child[j]->map->stmt->count_for - 1] ==
			   sgraph->recurrenceArray->child[j]->map->stmt->forStmt[sgraph->recurrenceArray->child[j]->map->stmt->count_for - 1])
			{
				isInputArray = false;
				break;
			}
		}//end for j
		if(!isInputArray)
			break;
	}//end for i
	if(isInputArray)
	{
		for(i = 0; i < sgraph->numOfInputs; i++)
			if(sgraph->inputArray[i] == array)
				return;

		sgraph->inputArray[sgraph->numOfInputs] = array;
		(sgraph->numOfInputs)++;
		return;
	}

	for(i = 0; i < array->outDegree; i++)
	{
		if(vertexNotPresent(array->child[i], sgraph))
		{
			sgraph->vertex[sgraph->numOfVertices] = array->child[i];
			(sgraph->numOfVertices)++;

			for(j = 0; j < array->child[i]->outDegree; j++)
			{
				if(vertexNotPresent(array->child[i]->child[j], sgraph))
				{
					sgraph->vertex[sgraph->numOfVertices] = array->child[i]->child[j];
					(sgraph->numOfVertices)++;

					//sgraph->inputArray[sgraph->numOfInputs] = array->child[i]->child[j];
					//(sgraph->numOfInputs)++;
					findInductionSubgraphArrayVertex(array->child[i]->child[j], sgraph);
				}
			}//end for j
		}
	}//end for i
}



//This function finds the induction subgraph corresponding to a single SCC in an ADDG
void findInductionSubgraph( SGRAPH *sgraph )
{
	unsigned int i;

	for(i = 0; i < sgraph->numOfVertices; i++)
	{
		if(sgraph->vertex[i]->type == OPERATOR)
			findInductionSubgraphOperatorVertex(sgraph->vertex[i], sgraph);
		else
			findInductionSubgraphArrayVertex(sgraph->vertex[i], sgraph);
	}
}



//This function finds the recurrence subgraph corresponding to a single SCC in an ADDG
void findSingleRecurrenceSubgraph( SGRAPH *sgraph )
{
	#ifdef Debug
	unsigned int i;

	for(i = 0; i < sgraph->numOfVertices; i++)
		printf("SCC Member Vertex %u: %s\n", i+1, sgraph->vertex[i]->name);
	#endif

	findBasisSubgraph( sgraph );
	findInductionSubgraph( sgraph );

	#ifdef Debug
	for(i = 0; i < sgraph->numOfVertices; i++)
		printf("RG Member Vertex %u: %s\n", i+1, sgraph->vertex[i]->name);
	#endif
}



//This function finds the recurrence subgraphs corresponding to each SCC in an ADDG
void findRecurrenceSubgraphs( unsigned int addgId )
{
	unsigned int i;

	printf("\nNumber of recurrences in this ADDG: %u.\n", G_RecurrenceSubgraph[addgId]->numOfRecur);

	for(i = 0; i < G_RecurrenceSubgraph[addgId]->numOfRecur; i++)
		findSingleRecurrenceSubgraph( G_RecurrenceSubgraph[addgId]->sgraph[i] );
}



//This function determines whether to retain a vertex in an ADDG or not
bool retainVertexInAddg( VERTEX *vertex, SGRAPH *sgraph )
{
	unsigned int i, j;

	//Check if vertex is contained in sgraph
	for(i = 0; i < sgraph->numOfVertices; i++)
	{
		if(vertex == sgraph->vertex[i])
		{
			//Control has reached here means that vertex is actually contained in sgraph
			//Check if vertex is the recurrence array or one of the input arrays of sgraph
			if(vertex == sgraph->recurrenceArray)
				return true;

			for(j = 0; j < sgraph->numOfInputs; j++)
			{
				if(vertex == sgraph->inputArray[j] && strstr(vertex->name, "__1") == NULL)
					return true;
			}//end for j

			//Control has reached here means that vertex is an ordinary node in sgraph
			return false;
		}
	}//end for i

	return true;
}



//This function creates a new vertex for an uninterpreted function and sets its values
VERTEX* insertUninterpretedFunction( SGRAPH *sgraph )
{
	unsigned int i, j, countValidInputArrays;
	char unintFuncName[MAX_NAME];
	NC  *sum, *term, *func, *fsum, *fterm, *primary, *prevFsum;
	DATA_TRANS *fakeDataTrans;
	SINGSTMT   *fakeStmt;
	VERTEX     *unintFunc;

	#ifdef Debug
	printf("Recurrence Array: %s\n", sgraph->recurrenceArray->name);
	for(i = 0; i < sgraph->numOfInputs; i++)
		printf("Input Array %u: %s\n", i+1, sgraph->inputArray[i]->name);
	#endif

	func = (NC*) malloc (sizeof(NC));
	func->list = (NC*)NULL;
	func->type = 'f';
	func->link = (NC*)NULL;

	countValidInputArrays = 0;
	for(i = 0; i < sgraph->numOfInputs; i++)
	{
		//The following check is for removing the fictitious array vertices
		if(strstr(sgraph->inputArray[i]->name, "__1") != NULL)
			continue;

		primary = (NC*) malloc (sizeof(NC));
		primary->list = (NC*)NULL;
		primary->type = 'v';
		primary->inc  = indexof_symtab( sgraph->inputArray[i]->name );
		primary->link = (NC*)NULL;

		fterm = (NC*) malloc (sizeof(NC));
		fterm->list = (NC*)NULL;
		fterm->type = 'T';
		fterm->inc  =  1;
		fterm->link = primary;

		fsum = (NC*) malloc (sizeof(NC));
		fsum->list = (NC*)NULL;
		fsum->type = 'S';
		fsum->inc  =  0;
		fsum->link = fterm;

		if(i == 0)
			func->link = fsum;
		else
			prevFsum->list = fsum;

		prevFsum = fsum;
		countValidInputArrays++;
	}

	sprintf(unintFuncName, "e%u", countValidInputArrays);
	func->inc = indexof_symtab(unintFuncName);

	term = (NC*) malloc (sizeof(NC));
	term->list = (NC*)NULL;
	term->type = 'T';
	term->inc  =  1;
	term->link = func;

	sum = (NC*) malloc (sizeof(NC));
	sum->list = (NC*)NULL;
	sum->type = 'S';
	sum->inc  =  0;
	sum->link = term;

	fakeDataTrans = (DATA_TRANS*) malloc (sizeof(DATA_TRANS));
	fakeDataTrans->lhs = indexof_symtab( sgraph->recurrenceArray->name );
	fakeDataTrans->LHS = (NC*)NULL;
	fakeDataTrans->rhs = sum;

	fakeStmt = (SINGSTMT*) malloc (sizeof(SINGSTMT));
	fakeStmt->count_for = 0;
	fakeStmt->count_if  = 0;
	fakeStmt->singleStmt = fakeDataTrans;
	fakeStmt->stmtNumber = FAKE_STMT_NUMBER;

	unintFunc = (VERTEX*) malloc (sizeof(VERTEX));
	unintFunc->type = OPERATOR;
	unintFunc->hasParent = true;
	unintFunc->covered = false;
	sprintf(unintFunc->name, "operatorUFe%u", countValidInputArrays);
	unintFunc->map = (MAP*) malloc (sizeof(MAP));
	unintFunc->map->stmt = fakeStmt;
	strcpy(unintFunc->map->mappingS2L, "");
	for(i = 0; i < MAX_CHILD; i++)
	{
		strcpy(unintFunc->map->mappingS2R[i], "");
		strcpy(unintFunc->map->mappingM[i],   "");
		strcpy(unintFunc->map->mappingI2O[i], "");
	}
	unintFunc->outDegree = countValidInputArrays;
	for(i = 0, j = 0; i < sgraph->numOfInputs; i++)
	{
		if(strstr(sgraph->inputArray[i]->name, "__1") == NULL)
		{
			unintFunc->child[j] = sgraph->inputArray[i];
			j++;
		}
	}

	//Set unintFunc as the only child of sgraph->recurrenceArray
	sgraph->recurrenceArray->child[0] = unintFunc;
	sgraph->recurrenceArray->outDegree = 1;

	return unintFunc;
}



//This function replaces all recurrence subgraphs in an ADDG by uninterpreted functions
void introduceSingleUninterpretedFunction( ADDG *addg, SGRAPH *sgraph )
{
	unsigned int i, j, retainedVertex[NO_OF_VERTICES];

	#ifdef Debug
	printf("\n*** In introduceSingleUninterpretedFunction ***\n");
	for(i = 0; i < addg->numVertices; i++)
		printf("ADDG Vertex: %s\n", addg->vertex[i]->name);
	for(i = 0; i < sgraph->numOfVertices; i++)
		printf("SGRAPH Vertex: %s\n", sgraph->vertex[i]->name);
	printf("SGRAPH Recurrence Array: %s\n", sgraph->recurrenceArray->name);
	for(i = 0; i < sgraph->numOfInputs; i++)
		printf("SGRAPH Input Array: %s\n", sgraph->inputArray[i]->name);
	#endif

	//Note that all vertices in the recurrence graph except its recurrence array and
	//input arrays are to be removed from the addg
	j = 0;
	for(i = 0; i < addg->numVertices; i++)
	{
		if(retainVertexInAddg(addg->vertex[i], sgraph))
		{
			retainedVertex[j] = i;
			j++;
		}
	}
	for(i = 0; i < j; i++)
		addg->vertex[i] = addg->vertex[retainedVertex[i]];

	addg->vertex[j] = insertUninterpretedFunction( sgraph );
	addg->numVertices = j + 1;

	#ifdef Debug
	for(i = 0; i < addg->numVertices; i++)
		printf("Modified ADDG Vertex: %s\n", addg->vertex[i]->name);
	printf("\n*** Out of introduceSingleUninterpretedFunction ***\n");
	#endif
}



//This function replaces all recurrence subgraphs in an ADDG by uninterpreted functions
void introduceUninterpretedFunctions( ADDG *addg, unsigned int addgId )
{
	unsigned int i;

	for(i = 0; i < G_RecurrenceSubgraph[addgId]->numOfRecur; i++)
		introduceSingleUninterpretedFunction( addg, G_RecurrenceSubgraph[addgId]->sgraph[i] );
}
