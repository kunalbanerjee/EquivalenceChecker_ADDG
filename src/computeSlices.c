#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "normalization.h"
#include "addg.h"

//NB: A slice has the same data structure as that of an ADDG

extern unsigned int G_countSlice;
extern unsigned int G_countOperands;
extern char G_namesOfOperands[3 * MAX_INCR][MAX_NAME];


//This function returns a copy of a VERTEX
VERTEX* copyVertex( VERTEX *node )
{
	VERTEX *newNode;
	unsigned int i;

	if(node == (VERTEX*)NULL)
		return (VERTEX*)NULL;

	newNode = (VERTEX*) malloc (sizeof(VERTEX));
	newNode->type = node->type;
	newNode->hasParent = node->hasParent;
	newNode->covered = node->covered;
	strcpy(newNode->name, node->name);
	if(node->map == (MAP*)NULL)
		newNode->map = (MAP*)NULL;
	else
	{
		newNode->map = (MAP*) malloc (sizeof(MAP));
		newNode->map->stmt = node->map->stmt;
		strcpy(newNode->map->mappingS2L, node->map->mappingS2L);
		for(i = 0; i < node->outDegree; i++)
		{
			strcpy(newNode->map->mappingS2R[i], node->map->mappingS2R[i]);
			strcpy(newNode->map->mappingM[i], node->map->mappingM[i]);
		}
	}

	//The following code sets the outDegree of a node to 0
	//NB: The onus of setting of child nodes properly is on
	//the function continueOrBifurcate
	newNode->outDegree = 0;

	return newNode;
}



//This function (recursively) copies a slice
//(unlike copyVertex which only copies a single VERTEX)
void recursiveCopySlice( VERTEX *node, VERTEX *parentNode, unsigned int childId, ADDG *slice )
{
	VERTEX *newNode;
	unsigned int i, j;

	if(node == (VERTEX*)NULL)
		return;

	#ifdef Debug
	printf("In recursiveCopySlice with node: %s\n", node->name);
	#endif

	//Check if the node already exists in the slice
	for(i = 0; i < slice->numVertices; i++)
	{
		if(strcmp(slice->vertex[i]->name, node->name) == 0)
			break;
	}

	if(i != slice->numVertices)
	{
		//No need to create a new node
		if(parentNode != (VERTEX*)NULL)
			parentNode->child[childId] = slice->vertex[i];

		//Since recursiveCopySlice proceeds in a depth-first manner,
		//the current "node" has already been covered implies all its
		//descendant nodes must also have been covered
		//So, there is no need to proceed along this node any furhter
	}
	else
	{
		//New node has to be created
		newNode = (VERTEX*) malloc (sizeof(VERTEX));
		newNode->type = node->type;
		newNode->hasParent = node->hasParent;
		newNode->covered = node->covered;
		strcpy(newNode->name, node->name);
		if(node->map == (MAP*)NULL)
			newNode->map = (MAP*)NULL;
		else
		{
			newNode->map = (MAP*) malloc (sizeof(MAP));
			newNode->map->stmt = node->map->stmt;
			strcpy(newNode->map->mappingS2L, node->map->mappingS2L);
			for(j = 0; j < node->outDegree; j++)
			{
				strcpy(newNode->map->mappingS2R[j], node->map->mappingS2R[j]);
				strcpy(newNode->map->mappingM[j], node->map->mappingM[j]);
			}
		}
		newNode->outDegree = node->outDegree;

		//newNode's child's are set in the following code because otherwise
		//printADDG will generate Segmentation fault while printing newNode
		//Note that these child's of newNode are later reset, if necessary
		for(i = 0; i < node->outDegree; i++)
			newNode->child[i] = node->child[i];

		//Place the newNode in slice
		slice->vertex[slice->numVertices] = newNode;
		(slice->numVertices)++;

		if(parentNode != (VERTEX*)NULL)
			parentNode->child[childId] = newNode;

		for(i = 0; i < node->outDegree; i++)
			recursiveCopySlice(node->child[i], newNode, i, slice);
	}

	#ifdef Debug
	printf("Out of recursiveCopySlice with node: %s\n", node->name);
	#endif
}



//This function makes a copy of a slice (ADDG)
ADDG* copySlice( ADDG *slice )
{
	ADDG *newSlice;
	unsigned int i;

	#ifdef Debug
	printf("In copySlice\n");
	#endif

	newSlice = (ADDG*) malloc (sizeof(ADDG));
	newSlice->numVertices = 0;

	//NB: There should a single OUTPUT node in a slice
	for(i = 0; i < slice->numVertices; i++)
	{
		if(slice->vertex[i]->hasParent == false)
		{
			recursiveCopySlice(slice->vertex[i], NULL, 0, newSlice);
			break;
		}
	}

	if(slice->numVertices != newSlice->numVertices)
	{
		printf("\nMismatch in number of vertices detected while copying a slice.\nExiting System.\n");
		printf("Slice 1\n");
		printADDG(slice);
		printf("Slice 2\n");
		printADDG(newSlice);
		exit(0);
	}

	//Increment the count of G_countSlice by 1
	G_countSlice++;

	#ifdef Debug
	printf("Out of copySlice\n");
	#endif

	return newSlice;
}



//Whenever this function is invoked, parsing starts from an ARRAY node
//of the ADDG
//If there are multiple outgoing edges from an ARRAY node, then multiple
//slices have to be created (Bifurcate), otherwise we continue along the
//same slice (Continue)
void continueOrBifurcate( VERTEX *orgVertex, VERTEX *newVertex, ADDG *slice[], unsigned int currentSliceId )
{
	unsigned int i, j, k, l, m;
	unsigned int count, currentNumSlices;

	#ifdef Debug
	printf("In continueOrBifurcate: %s\n", orgVertex->name);
	#endif

	if(orgVertex->outDegree == 0)
	{
		//End of slice reached -- nothing to do
	}
	else if(orgVertex->outDegree == 1)
	{
		//Continue
		//Make a copy of the child: OPEARATOR Node
		j = existInADDG(orgVertex->child[0]->name, slice[currentSliceId]);
		if(j != NO_OF_VERTICES)
		{
			newVertex->child[0] = slice[currentSliceId]->vertex[j];
		}
		else
		{
			newVertex->child[0] = copyVertex(orgVertex->child[0]);
			newVertex->outDegree = 1;
			slice[currentSliceId]->vertex[slice[currentSliceId]->numVertices] = newVertex->child[0];
			(slice[currentSliceId]->numVertices)++;

			//Make a copy of every grand-child
			for(i = 0; i < orgVertex->child[0]->outDegree; i++)
			{
				j = existInADDG(orgVertex->child[0]->child[i]->name, slice[currentSliceId]);
				if(j == NO_OF_VERTICES)
				{
					newVertex->child[0]->child[i] = copyVertex(orgVertex->child[0]->child[i]);
					slice[currentSliceId]->vertex[slice[currentSliceId]->numVertices] = newVertex->child[0]->child[i];
					(slice[currentSliceId]->numVertices)++;
				}
				else
				{
					newVertex->child[0]->child[i] = slice[currentSliceId]->vertex[j];
				}
			}
			newVertex->child[0]->outDegree = orgVertex->child[0]->outDegree;
			//Make a recursive call to this function for every grand-child
			for(i = 0; i < orgVertex->child[0]->outDegree; i++)
			{
				if(i == 0)
					continueOrBifurcate(orgVertex->child[0]->child[i], newVertex->child[0]->child[i], slice, currentSliceId);
				else
				{
					currentNumSlices = G_countSlice;
					continueOrBifurcate(orgVertex->child[0]->child[i], newVertex->child[0]->child[i], slice, currentSliceId);
					//The previous call to continueOrBifurcate with child[k], k = 0 to i-1,
					//may have spawned new slices (other than slice[currentSliceId]; hence
					//the following calls to continueOrBifurcate has to be done for each
					//such slice including slice[currentSliceId]
					for(k = currentSliceId + 1; k < currentNumSlices; k++)
					{
						j = existInADDG(orgVertex->child[0]->name, slice[k]);
						if(j != NO_OF_VERTICES)
							continueOrBifurcate(orgVertex->child[0]->child[i], slice[k]->vertex[j]->child[i], slice, k);
					}
				}//end if i == 0
			}//end for grand-children
		}//end if j != NO_OF_VERTICES
	}
	else
	{
		//Bifurcate
		count = G_countSlice - 1;
		for(i = 1; i < orgVertex->outDegree; i++)
		{
			//Make a copy of the current slice;
			//(there is no need to create a new slice for i == 0)
			slice[count + i] = copySlice(slice[currentSliceId]);
		}

		//for i == 0
		//Make a copy of the child: OPEARATOR Node
		j = existInADDG(orgVertex->child[0]->name, slice[currentSliceId]);
		if(j != NO_OF_VERTICES)
		{
			newVertex->child[0] = slice[currentSliceId]->vertex[j];
		}
		else
		{
			newVertex->child[0] = copyVertex(orgVertex->child[0]);
			newVertex->outDegree = 1;
			slice[currentSliceId]->vertex[slice[currentSliceId]->numVertices] = newVertex->child[0];
			(slice[currentSliceId]->numVertices)++;

			//Make a copy of every grand-child
			for(i = 0; i < orgVertex->child[0]->outDegree; i++)
			{
				j = existInADDG(orgVertex->child[0]->child[i]->name, slice[currentSliceId]);
				if(j == NO_OF_VERTICES)
				{
					newVertex->child[0]->child[i] = copyVertex(orgVertex->child[0]->child[i]);
					slice[currentSliceId]->vertex[slice[currentSliceId]->numVertices] = newVertex->child[0]->child[i];
					(slice[currentSliceId]->numVertices)++;
				}
				else
				{
					newVertex->child[0]->child[i] = slice[currentSliceId]->vertex[j];
				}
			}
			newVertex->child[0]->outDegree = orgVertex->child[0]->outDegree;
			//Make a recursive call to this function for every grand-child
			for(i = 0; i < orgVertex->child[0]->outDegree; i++)
			{
				if(i == 0)
					continueOrBifurcate(orgVertex->child[0]->child[i], newVertex->child[0]->child[i], slice, currentSliceId);
				else
				{
					currentNumSlices = G_countSlice;
					continueOrBifurcate(orgVertex->child[0]->child[i], newVertex->child[0]->child[i], slice, currentSliceId);
					for(k = currentSliceId + 1; k < currentNumSlices; k++)
					{
						j = existInADDG(orgVertex->child[0]->name, slice[k]);
						if(j != NO_OF_VERTICES)
							continueOrBifurcate(orgVertex->child[0]->child[i], slice[k]->vertex[j]->child[i], slice, k);
					}
				}//end if i == 0
			}

			//for i = 1 to rest
			for(i = 1; i < orgVertex->outDegree; i++)
			{
				//Find the Id of the vertex whose name matches with the present vertex
				j =	existInADDG(orgVertex->name, slice[count + i]);
				if(j == NO_OF_VERTICES)
				{
					printf("\nNo matching name found within a slice.\nExiting System.\n");
					exit(0);
				}
				//Make a copy of the child: OPEARATOR Node
				m = existInADDG(orgVertex->child[i]->name, slice[count + i]);
				if(m == NO_OF_VERTICES)
				{
					slice[count + i]->vertex[j]->child[0] = copyVertex(orgVertex->child[i]);
					slice[count + i]->vertex[j]->outDegree = 1;
					slice[count + i]->vertex[slice[count + i]->numVertices] = slice[count + i]->vertex[j]->child[0];
					(slice[count + i]->numVertices)++;
				}
				else
				{
					slice[count + i]->vertex[j]->child[0] = slice[count + i]->vertex[m];
				}
				//Make a copy of every grand-child
				for(k = 0; k < orgVertex->child[i]->outDegree; k++)
				{
					l = existInADDG(orgVertex->child[i]->child[k]->name, slice[count + i]);
					if(l == NO_OF_VERTICES)
					{
						slice[count + i]->vertex[j]->child[0]->child[k] = copyVertex(orgVertex->child[i]->child[k]);
						slice[count + i]->vertex[slice[count + i]->numVertices] = slice[count + i]->vertex[j]->child[0]->child[k];
						(slice[count + i]->numVertices)++;
					}
					else
					{
						slice[count + i]->vertex[j]->child[0]->child[k] = slice[count + i]->vertex[l];
					}
				}
				slice[count + i]->vertex[j]->child[0]->outDegree = orgVertex->child[i]->outDegree;
				//Make a recursive call to this function for every grand-child
				for(k = 0; k < orgVertex->child[i]->outDegree; k++)
				{
					if(k == 0)
						continueOrBifurcate(orgVertex->child[i]->child[k], slice[count + i]->vertex[j]->child[0]->child[k], slice, count + i);
					else
					{
						currentNumSlices = G_countSlice;
						continueOrBifurcate(orgVertex->child[i]->child[k], slice[count + i]->vertex[j]->child[0]->child[k], slice, count + i);
						for(l = count + i + 1; l < currentNumSlices; l++)
						{
							m = existInADDG(orgVertex->child[i]->name, slice[l]);
							if(m != NO_OF_VERTICES)
								continueOrBifurcate(orgVertex->child[i]->child[k], slice[l]->vertex[m]->child[k], slice, l);
						}
					}//end if k == 0
				}
			}//end for i = 1 to rest
		}//end if j != NO_OF_VERTICES
	}//end if Bifurcate

	#ifdef Debug
	printf("Out of continueOrBifurcate: %s\n", orgVertex->name);
	#endif
}



//This function creates a slice and insert the output array in it
//It also increments the count of G_countSlice by 1
void createSingleSlice( VERTEX* output, ADDG *slice[], ADDG *addg )
{
	slice[G_countSlice] = (ADDG*) malloc (sizeof(ADDG));
	slice[G_countSlice]->numVertices = 1;
	slice[G_countSlice]->vertex[0] = copyVertex(output);

	//Increment the counter for slices
	G_countSlice++;

	//Proceed with this newly created slice
	continueOrBifurcate(output, slice[G_countSlice-1]->vertex[0], slice, G_countSlice-1);
}



//This function computes the slice(s) from an ADDG and returns
//the number of slices found
void computeSlices( ADDG *addg, ADDG *slice[] )
{
	unsigned int i;

	G_countSlice = 0;

	if (addg == (ADDG*)NULL)
		return;

	for(i = 0; i < addg->numVertices; i++)
	{
		if(addg->vertex[i]->hasParent == false)
		{
			createSingleSlice(addg->vertex[i], slice, addg);
			break;
		}
	}
}



//This function inserts a mapping into the characteristics of a slice
void insertMappingToSlice( char inputArrayName[], char mappingIO[], ADDG *slice )
{
	unsigned int i;
	bool notInserted;

	notInserted = true;
	for(i = 0; i < slice->cslice->numOfInputs; i++)
	{
		if( strcmp(inputArrayName, slice->cslice->nameOfInput[i]) == 0)
		{
			strcpy(slice->cslice->mappingIO[i], mappingIO);
			notInserted = false;
		}
	}
	if(notInserted)
	{
		strcpy(slice->cslice->nameOfInput[i], inputArrayName);
		strcpy(slice->cslice->mappingIO[i], mappingIO);
		(slice->cslice->numOfInputs)++;
	}
}



//This function computes the transitive characteristics of a slice
//Note that this function is always visited with "vertex" pointing
//to an ARRAY node
void computeTransitiveCharacteristics( VERTEX *vertex, ADDG *slice )
{
	unsigned int i, j;
	char input4isl[3 * SIZE], initialMapping[SIZE], tempMapping[SIZE];
	VERTEX *child, *parent;

	if(vertex->outDegree > 1)
	{
		printf("\nError: ARRAY vertex in a slice has more than one child.\nExiting System.\n");
		exit(0);
	}
	if(vertex->outDegree == 0)
	{
		//Nothing more to do
		return;
	}

	for(i = 0; i < vertex->child[0]->outDegree; i++)
	{
		if(vertex->child[0]->child[i]->outDegree == 0)
		{
			//This means vertex->child[0]->child[i] is an INPUT node
			insertMappingToSlice(vertex->child[0]->child[i]->name, vertex->child[0]->map->mappingI2O[i], slice);
		}
		else
		{
			child  = vertex->child[0]->child[i]->child[0];
			parent = vertex->child[0];
			for(j = 0; j < child->outDegree; j++)
			{
				strcpy(initialMapping, child->map->mappingM[j]);
				//Change the first character from 'M' to 'S'
				initialMapping[0] = 'S';
				sprintf(input4isl, "%s\n", initialMapping);
				sprintf(eos(input4isl), "%s\n", parent->map->mappingI2O[i]);

				strcat(input4isl, "T := S( R );\nT;\n");
				invokeISL(input4isl, tempMapping);

				if( strcmp(child->map->mappingI2O[j], "")==0 )
				{
					//No union of mappings is to be performed
					//Change the first character from 'M' to 'R'
					tempMapping[0] = 'R';
					strcpy(child->map->mappingI2O[j], tempMapping);
				}
				else
				{
					//Union of mappings is to be performed
					//Change the first character from 'M' to 'N'
					tempMapping[0] = 'N';
					sprintf(input4isl, "%s\n", tempMapping);
					sprintf(eos(input4isl), "%s\n", child->map->mappingI2O[j]);
					strcat(input4isl, "U := N + R;\nU;\n");
					invokeISL(input4isl, tempMapping);

					//Change the first character from 'M' to 'R'
					tempMapping[0] = 'R';
					strcpy(child->map->mappingI2O[j], tempMapping);
				}
			}//end for j
		}//end else if outDegree != 0
	}//end for i

	//Recursive call
	for(i = 0; i < vertex->child[0]->outDegree; i++)
		computeTransitiveCharacteristics(vertex->child[0]->child[i], slice);
}



//This function computes the characteristics of a slice
void computeCharacteristicsOfSlice( ADDG *slice )
{
	unsigned int i, j;

	if (slice == (ADDG*)NULL)
		return;

	slice->cslice = (CSLICE*) malloc (sizeof(CSLICE));

	for(i = 0; i < slice->numVertices; i++)
	{
		if(slice->vertex[i]->hasParent == false)
		{
			for(j = 0; j < slice->vertex[i]->child[0]->outDegree; j++)
			{
				strcpy(slice->vertex[i]->child[0]->map->mappingI2O[j], slice->vertex[i]->child[0]->map->mappingM[j]);
				//Change the first character from 'M' to 'R'
				slice->vertex[i]->child[0]->map->mappingI2O[j][0] = 'R';
			}
			computeTransitiveCharacteristics(slice->vertex[i], slice);
		}
	}//end for i
}



//This function computes the list of parameters required to checking
//validity of a slice
void computeParameters( char parameterList[][MAX_NAME], unsigned int count, char parameters[] )
{
	unsigned int i, j;
	bool isAbsent;
	char tempParameters[SIZE];

	strcpy(tempParameters, "[");
	for(i = 0; i < count; i++)
	{
		isAbsent = true;
		for(j = 0; j < i; j++)
		{
			if( strcmp(parameterList[j], parameterList[i])==0 )
			{
				isAbsent = false;
				break;
			}
		}
		if(isAbsent)
		{
			sprintf(eos(tempParameters), "%s,", parameterList[i]);
		}
	}
	//Replace the last comma by ']'
	tempParameters[strlen(tempParameters) - 1] = ']';

	sprintf(parameters, "%s -> %s : ", tempParameters, tempParameters);
}



//This function determines whether a slice is valid or not by checking
//whether the involved sentences contain conflicting IF conditions
//It returns "true" if the slice is valid, "false" otherwise
bool isValidSlice( ADDG *slice )
{
	unsigned int i, j;
	char checkValidity[2 * SIZE], outputCheckValidity[SIZE], parameters[SIZE], conditions[SIZE];
	bool containsIfStatement;

	strcpy(conditions, "");
	containsIfStatement = false;
	G_countOperands = 0;

	for(i = 0; i < slice->numVertices; i++)
	{
		if(slice->vertex[i]->type == OPERATOR)
		{
			//Add the conditions of the enclosing IF statements
			for(j = 0; j < slice->vertex[i]->map->stmt->count_if; j++)
			{
				containsIfStatement = true;
				write_lists_string(slice->vertex[i]->map->stmt->ifStmt[j]->condition, conditions, false);
				strcat(conditions, " and ");

				//Find the parameters that may be present in the condition
				extractNamesOfOperands(slice->vertex[i]->map->stmt->ifStmt[j]->condition, true, true);
			}//end for j
		}
	}//end for i

	if(containsIfStatement)
	{
		computeParameters(G_namesOfOperands, G_countOperands, parameters);
		conditions[strlen(conditions) - 5] = '\0'; //To remove the last " and " (5 characters)
		sprintf(checkValidity, "C := { %s %s };\nC;\n", parameters, conditions);

		invokeISL(checkValidity, outputCheckValidity); //Input and output both are deliberately set to the same string
		if(strstr(outputCheckValidity, " 1 = 0 ") == NULL)
			return true;
		else
			return false;
	}
	else
		return true;
}
