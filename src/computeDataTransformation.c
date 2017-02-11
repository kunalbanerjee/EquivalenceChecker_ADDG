#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "normalization.h"
#include "addg.h"

extern unsigned int G_countOperands;
extern char G_namesOfOperands[3 * MAX_INCR][MAX_NAME];

//The following is a global variable that stores the cumulative
//data transformation of a slice
NC *G_cumulativeNorm;

//This function checks whether the index domains in a slice are overlapping or not
//It returns "true" if they are overlapping, "false" otherwise
bool containsOverlappingDomains( ADDG *slice )
{
	unsigned int i, j;

	for(i = 0; i < slice->numVertices; i++)
	{
		if(slice->vertex[i]->type == OPERATOR)
		{
			for(j = 0; j < slice->vertex[i]->outDegree; j++)
			{
				//Check if mappingI2O contains the null set indicated by "{  }"
				if(strstr(slice->vertex[i]->map->mappingI2O[j], "{  }") != NULL)
					return false;
			}//end for j
		}
	}//end for i

	return true;
}



//This function converts array variables, if any, in a normalized expression into scalars
void convertArraysToScalars( NC *expr )
{
	if(expr == (NC*)NULL)
		return;

	if(expr->type == 'a')
	{
		expr->type = 'v';
		expr->link = (NC*)NULL;
	}

	if(expr->list != (NC*)NULL)
		convertArraysToScalars( expr->list );
	if(expr->link != (NC*)NULL)
		convertArraysToScalars( expr->link );
}



//This function performs repeated substitution to obtain the final
//data transformation of a slice
void computeCumulativeDataTransformation( VERTEX *node )
{
	unsigned int i;
	NC *primary, *tempNorm;

	if(node->outDegree == 0)
		return;

	if(G_cumulativeNorm == (NC*)NULL)
	{
		G_cumulativeNorm = copylist(node->child[0]->map->stmt->singleStmt->rhs);
		convertArraysToScalars(G_cumulativeNorm); //KB
	}
	else
	{
		tempNorm = copylist(node->child[0]->map->stmt->singleStmt->rhs);
		convertArraysToScalars(tempNorm); //KB

		primary = (NC*) malloc (sizeof(NC));
		primary->list = (NC*)NULL;
		primary->type = 'v';
		primary->inc  = node->child[0]->map->stmt->singleStmt->LHS->link->link->inc;
		primary->link = (NC*)NULL;

		G_cumulativeNorm = Substitute_In_Sum(G_cumulativeNorm, primary, tempNorm, G_cumulativeNorm);
	}

	for(i = 0; i < node->child[0]->outDegree; i++)
		computeCumulativeDataTransformation(node->child[0]->child[i]);
}



//This function computes the data transformation of a slice
void computeDataTransformation( ADDG *slice )
{
	unsigned int i, outputArrayIndex, countRealArray, realArrayIndex[MAX_CHILD];

	G_cumulativeNorm = (NC*)NULL;
	for(i = 0; i < slice->numVertices; i++)
	{
		if(slice->vertex[i]->hasParent == false)
		{
			outputArrayIndex = i;
			computeCumulativeDataTransformation(slice->vertex[i]);
			break;
		}
	}

	slice->cslice->dataTransformation = (DATA_TRANS*) malloc (sizeof(DATA_TRANS));
	slice->cslice->dataTransformation->lhs = slice->vertex[outputArrayIndex]->child[0]->map->stmt->singleStmt->LHS->link->link->inc;
	slice->cslice->dataTransformation->LHS = (NC*)NULL;
	//convertArraysToScalars(G_cumulativeNorm); //KB
	slice->cslice->dataTransformation->rhs = copylist(G_cumulativeNorm);

	//Remove FICTITIOUS arrays (representing constants), if any, from the list of input arrays
	countRealArray = 0;
	for(i = 0; i < slice->cslice->numOfInputs; i++)
	{
		if(strstr(slice->cslice->nameOfInput[i], "FICTITIOUS") == NULL)
		{
			realArrayIndex[countRealArray] = i;
			countRealArray++;
		}
	}

	if(countRealArray == slice->cslice->numOfInputs)
		return;

	for(i = 0; i < countRealArray; i++)
	{
		strcpy(slice->cslice->nameOfInput[i], slice->cslice->nameOfInput[realArrayIndex[i]]);
		strcpy(slice->cslice->mappingIO[i], slice->cslice->mappingIO[realArrayIndex[i]]);
	}
	slice->cslice->numOfInputs = countRealArray;
}



//This function merges those slices which have identical data transformations
//Merging implies union of individual maps
unsigned int mergeSlices( ADDG *slice[], unsigned int countSlice, ADDG *sliceClass[] )
{
	unsigned int i, j, k, l, countSliceClass;
	bool notCovered[MAX_SLICE];
	DATA_TRANS *before, *after;
	char input4isl[2 * SIZE];

	for(i = 0; i < countSlice; i++)
		notCovered[i] = true;

	countSliceClass = 0;
	for(i = 0; i < countSlice; i++)
	{
		if(notCovered[i])
		{
			sliceClass[countSliceClass] = slice[i];
			countSliceClass++;

			//Check if any other slice has the same data transformation or not
			for(j = i+1; j < countSlice; j++)
			{
				before = slice[i]->cslice->dataTransformation;
				after  = slice[j]->cslice->dataTransformation;
				if(before->lhs == after->lhs && compare_trees(before->rhs, after->rhs))
				{
					//Mark slice[j] as covered
					notCovered[j] = false;

					//The two slices have identical data transformation
					for(k = 0; k < slice[i]->cslice->numOfInputs; k++)
					{
						//Ensure that the two mappings point to the same input array
						for(l = 0; l < slice[j]->cslice->numOfInputs; l++)
						{
							if( strcmp(slice[i]->cslice->nameOfInput[k], slice[j]->cslice->nameOfInput[l]) == 0 )
							{
								sprintf(input4isl, "%s\n", slice[i]->cslice->mappingIO[k]);
								//Change the first character from 'R' to 'N'
								slice[j]->cslice->mappingIO[l][0] = 'N';
								sprintf(eos(input4isl), "%s\n", slice[j]->cslice->mappingIO[l]);
								sprintf(eos(input4isl), "U := R + N;\nU;\n");
								invokeISL(input4isl, slice[i]->cslice->mappingIO[k]);
								//Change the first character from 'M' to 'R'
								slice[i]->cslice->mappingIO[k][0] = 'R';
							}
						}//end for l
					}//end for k
				}
			}//end for j
		}
	}//end for i

	return countSliceClass;
}



//This function substitutes "array1" by "array2" in "orig"
NC* Substitute_In_Sum_WithRecurrenceArray( NC *orig, char array1[], char array2[] )
{
	NC *primary, *sum, *finalNorm;

	primary = (NC*) malloc (sizeof(NC));
	primary->list = (NC*)NULL;
	primary->type = 'v';
	primary->inc  = indexof_symtab(array1);
	primary->link = (NC*)NULL;

	sum = (NC*) malloc (sizeof(NC));
	sum->list = (NC*)NULL;
	sum->type = 'S';
	sum->inc  = 0;
	sum->link = (NC*) malloc (sizeof(NC));

	sum->link->list = (NC*)NULL;
	sum->link->type = 'T';
	sum->link->inc  = 1;
	sum->link->link = (NC*) malloc (sizeof(NC));

	sum->link->link->list = (NC*)NULL;
	sum->link->link->type = 'v';
	sum->link->link->inc  = indexof_symtab(array2);
	sum->link->link->link = (NC*)NULL;

	finalNorm = (NC*)NULL;
	finalNorm = Substitute_In_Sum(orig, primary, sum, finalNorm);

	return finalNorm;
}



//This function checks if two slice classes are equivalent on replacement of some recurrence arrays
bool checkEquivalenceOfSliceClassesWithDifferentRecurrenceArrays( ADDG *class1, ADDG *class2,
                                                                  char namesOfRecur1[][MAX_NAME], unsigned int countRecur1,
                                                                  char namesOfRecur2[][MAX_NAME], unsigned int countRecur2 )
{
	//NB: This function assumes that the number of recurrence arrays in
	//the data transformations of the two slice classes are equal.
	//This function also assumes that the maximum number of recurrence arrays
	//in the data transformation of a single slice class is 2 (which suffices
	//for our set of benchmarks).
	//To extend the function to cover the general case where the number of
	//recurrence arrays in class1 is m and in class2 is n, a code similar to
	//string permutation has to be implemented.
	//Eg. Recurrence arrays in class1 = x, y, z; recurrence arrays in
	//class2 = a, b, c, d
	//The solution is given by the following program (including the case of
	//no substituion):
	//str = "xyzabcd"; compute all possible permutations of str and extract
	//the first m (= 3, in this case) letters (arrays).
	unsigned int i, j, recur1, recur2;
	char namesOfRecurArray1[MAX_INCR][MAX_NAME], namesOfRecurArray2[MAX_INCR][MAX_NAME];
	NC *tempNorm, *finalNorm;

	//Find the recurrence arrays, if any, in class1's data transformation
	G_countOperands = 0;
	extractNamesOfOperands(class1->cslice->dataTransformation->rhs, true, true);

	recur1 = 0;
	for(i = 0; i < G_countOperands; i++)
	{
		for(j = 0; j < countRecur1; j++)
		{
			if(strcmp(G_namesOfOperands[i], namesOfRecur1[j]) == 0)
			{
				strcpy(namesOfRecurArray1[recur1], namesOfRecur1[j]);
				recur1++;
			}
		}
	}
	if(recur1 == 0 || recur1 >= 3)
		return false;

	//Find the recurrence arrays, if any, in class2's data transformation
	G_countOperands = 0;
	extractNamesOfOperands(class2->cslice->dataTransformation->rhs, true, true);

	recur2 = 0;
	for(i = 0; i < G_countOperands; i++)
	{
		for(j = 0; j < countRecur2; j++)
		{
			if(strcmp(G_namesOfOperands[i], namesOfRecur2[j]) == 0)
			{
				strcpy(namesOfRecurArray2[recur2], namesOfRecur2[j]);
				recur2++;
			}
		}
	}
	if(recur2 == 0 || recur2 >= 3)
		return false;

	if(recur1 != recur2)
		return false;

	if(recur1 == 1)
	{
		tempNorm = copylist(class1->cslice->dataTransformation->rhs);
		finalNorm = Substitute_In_Sum_WithRecurrenceArray(tempNorm, namesOfRecurArray1[0], namesOfRecurArray2[0]);

		if(compare_trees(finalNorm, class2->cslice->dataTransformation->rhs))
		{
			for(i = 0; i < class2->cslice->numOfInputs; i++)
			{
				if(strcmp(class2->cslice->nameOfInput[i], namesOfRecurArray2[0]) == 0)
				{
					strcpy(class2->cslice->nameOfInput[i], namesOfRecurArray1[0]);
					break;
				}
			}//end for i
		}
		else
			return false;
	}
	else //recur1 == 2
	{
		//One substitution has to be done -- Four cases may occur
		//Case 1
		tempNorm = copylist(class1->cslice->dataTransformation->rhs);
		finalNorm = Substitute_In_Sum_WithRecurrenceArray(tempNorm, namesOfRecurArray1[0], namesOfRecurArray2[0]);

		if(compare_trees(finalNorm, class2->cslice->dataTransformation->rhs))
		{
			for(i = 0; i < class2->cslice->numOfInputs; i++)
			{
				if(strcmp(class2->cslice->nameOfInput[i], namesOfRecurArray2[0]) == 0)
				{
					strcpy(class2->cslice->nameOfInput[i], namesOfRecurArray1[0]);
					break;
				}
			}//end for i
		}

		//Case 2
		tempNorm = copylist(class1->cslice->dataTransformation->rhs);
		finalNorm = Substitute_In_Sum_WithRecurrenceArray(tempNorm, namesOfRecurArray1[1], namesOfRecurArray2[0]);

		if(compare_trees(finalNorm, class2->cslice->dataTransformation->rhs))
		{
			for(i = 0; i < class2->cslice->numOfInputs; i++)
			{
				if(strcmp(class2->cslice->nameOfInput[i], namesOfRecurArray2[0]) == 0)
				{
					strcpy(class2->cslice->nameOfInput[i], namesOfRecurArray1[1]);
					break;
				}
			}//end for i
		}

		//Case 3
		tempNorm = copylist(class1->cslice->dataTransformation->rhs);
		finalNorm = Substitute_In_Sum_WithRecurrenceArray(tempNorm, namesOfRecurArray1[0], namesOfRecurArray2[1]);

		if(compare_trees(finalNorm, class2->cslice->dataTransformation->rhs))
		{
			for(i = 0; i < class2->cslice->numOfInputs; i++)
			{
				if(strcmp(class2->cslice->nameOfInput[i], namesOfRecurArray2[1]) == 0)
				{
					strcpy(class2->cslice->nameOfInput[i], namesOfRecurArray1[0]);
					break;
				}
			}//end for i
		}

		//Case 4
		tempNorm = copylist(class1->cslice->dataTransformation->rhs);
		finalNorm = Substitute_In_Sum_WithRecurrenceArray(tempNorm, namesOfRecurArray1[1], namesOfRecurArray2[1]);

		if(compare_trees(finalNorm, class2->cslice->dataTransformation->rhs))
		{
			for(i = 0; i < class2->cslice->numOfInputs; i++)
			{
				if(strcmp(class2->cslice->nameOfInput[i], namesOfRecurArray2[1]) == 0)
				{
					strcpy(class2->cslice->nameOfInput[i], namesOfRecurArray1[1]);
					break;
				}
			}//end for i
		}

		//Two substitutions have to be done -- Two cases may occur
		//Case 1
		tempNorm = copylist(class1->cslice->dataTransformation->rhs);
		finalNorm = Substitute_In_Sum_WithRecurrenceArray(tempNorm,  namesOfRecurArray1[0], namesOfRecurArray2[0]);
		finalNorm = Substitute_In_Sum_WithRecurrenceArray(finalNorm, namesOfRecurArray1[1], namesOfRecurArray2[1]);

		if(compare_trees(finalNorm, class2->cslice->dataTransformation->rhs))
		{
			for(i = 0; i < class2->cslice->numOfInputs; i++)
			{
				if(strcmp(class2->cslice->nameOfInput[i], namesOfRecurArray2[0]) == 0)
					strcpy(class2->cslice->nameOfInput[i], namesOfRecurArray1[0]);
				if(strcmp(class2->cslice->nameOfInput[i], namesOfRecurArray2[1]) == 0)
					strcpy(class2->cslice->nameOfInput[i], namesOfRecurArray1[1]);
			}//end for i
		}

		//Case 2
		tempNorm = copylist(class1->cslice->dataTransformation->rhs);
		finalNorm = Substitute_In_Sum_WithRecurrenceArray(tempNorm,  namesOfRecurArray1[0], namesOfRecurArray2[1]);
		finalNorm = Substitute_In_Sum_WithRecurrenceArray(finalNorm, namesOfRecurArray1[1], namesOfRecurArray2[0]);

		if(compare_trees(finalNorm, class2->cslice->dataTransformation->rhs))
		{
			for(i = 0; i < class2->cslice->numOfInputs; i++)
			{
				if(strcmp(class2->cslice->nameOfInput[i], namesOfRecurArray2[0]) == 0)
					strcpy(class2->cslice->nameOfInput[i], namesOfRecurArray1[1]);
				if(strcmp(class2->cslice->nameOfInput[i], namesOfRecurArray2[1]) == 0)
					strcpy(class2->cslice->nameOfInput[i], namesOfRecurArray1[0]);
			}//end for i
		}
		else
			return false;
	}

	return true;
}



//This function checks equivalence of two ADDGs
bool checkEquivalence( ADDG *class1[], unsigned int count1, ADDG *class2[], unsigned int count2 )
{
	unsigned int i, j, k, l;
	DATA_TRANS *source, *target;
	char input4isl[2 * SIZE], result[SIZE];

	for(i = 0; i < count1; i++)
	{
		for(j = 0; j < count2; j++)
		{
			source = class1[i]->cslice->dataTransformation;
			target = class2[j]->cslice->dataTransformation;
			if(source->lhs == target->lhs && compare_trees(source->rhs, target->rhs))
			{
				//Data transformations match; now we have to check equivalence of mappings
				for(k = 0; k < class1[i]->cslice->numOfInputs; k++)
				{
					//Ensure that the two mappings point to the same input array
					for(l = 0; l < class2[j]->cslice->numOfInputs; l++)
					{
						if( strcmp(class1[i]->cslice->nameOfInput[k], class2[j]->cslice->nameOfInput[l]) == 0 )
						{
							//Change the first character from 'R' to 'S'
							class1[i]->cslice->mappingIO[k][0] = 'S';
							sprintf(input4isl, "%s\n", class1[i]->cslice->mappingIO[k]);
							//Change the first character from 'R' to 'T'
							class2[j]->cslice->mappingIO[l][0] = 'T';
							sprintf(eos(input4isl), "%s\n", class2[j]->cslice->mappingIO[l]);
							sprintf(eos(input4isl), "S = T;\n");
							invokeISL(input4isl, result);
							if(strstr(result, "True") == NULL)
							{
								printf("Mappings mismatch for the following slice classes for the input array \"%s\".\n", class1[i]->cslice->nameOfInput[k]);
								printf("Slice class in source ADDG:\n");
								printADDG(class1[i]);
								printf("Slice class in transformed ADDG:\n");
								printADDG(class2[j]);

								return false;
							}
						}
					}//end for l
				}//end for k
				break;
			}
		}//end for j

		if(j == count2)
		{
			printf("No match found for the following slice class in the source ADDG.\n");
			printADDG(class1[i]);
			return false;
		}
	}//end for i

	return true;
}



//This function checks equivalence of two ADDGs while accommodating substitutions of recurrence arrays
bool checkEquivalenceWithSubstitutions( ADDG *class1[], unsigned int count1, ADDG *class2[], unsigned int count2,
                                        char namesOfRecur1[][MAX_NAME], unsigned int countRecur1, char namesOfRecur2[][MAX_NAME], unsigned int countRecur2 )
{
	unsigned int i, j, k, l;
	DATA_TRANS *source, *target;
	char input4isl[2 * SIZE], result[SIZE];

	for(i = 0; i < count1; i++)
	{
		for(j = 0; j < count2; j++)
		{
			source = class1[i]->cslice->dataTransformation;
			target = class2[j]->cslice->dataTransformation;
			if(source->lhs == target->lhs && compare_trees(source->rhs, target->rhs))
			{
				//Data transformations match; now we have to check equivalence of mappings
				for(k = 0; k < class1[i]->cslice->numOfInputs; k++)
				{
					//Ensure that the two mappings point to the same input array
					for(l = 0; l < class2[j]->cslice->numOfInputs; l++)
					{
						if( strcmp(class1[i]->cslice->nameOfInput[k], class2[j]->cslice->nameOfInput[l]) == 0 )
						{
							//Change the first character from 'R' to 'S'
							class1[i]->cslice->mappingIO[k][0] = 'S';
							sprintf(input4isl, "%s\n", class1[i]->cslice->mappingIO[k]);
							//Change the first character from 'R' to 'T'
							class2[j]->cslice->mappingIO[l][0] = 'T';
							sprintf(eos(input4isl), "%s\n", class2[j]->cslice->mappingIO[l]);
							sprintf(eos(input4isl), "S = T;\n");
							invokeISL(input4isl, result);
							if(strstr(result, "True") == NULL)
							{
								printf("Mappings mismatch for the following slice classes for the input array \"%s\".\n", class1[i]->cslice->nameOfInput[k]);
								printf("Slice class in source ADDG:\n");
								printADDG(class1[i]);
								printf("Slice class in transformed ADDG:\n");
								printADDG(class2[j]);

								return false;
							}
						}
					}//end for l
				}//end for k
				break;
			}
		}//end for j

		//Allow substitutions
		if(j == count2) //Originally no match found for class1[i]
		{
			//Check if the mismatch in data transformation is due to difference in recurrence array names
			for(j = 0; j < count2; j++)
			{
				if(class1[i]->cslice->numOfInputs == class2[j]->cslice->numOfInputs && class1[i]->cslice->dataTransformation->lhs == class2[j]->cslice->dataTransformation->lhs)
				{
					if(checkEquivalenceOfSliceClassesWithDifferentRecurrenceArrays(class1[i], class2[j], namesOfRecur1, countRecur1, namesOfRecur2, countRecur2))
					{
						//Data transformations match; now we have to check equivalence of mappings
						for(k = 0; k < class1[i]->cslice->numOfInputs; k++)
						{
							//Ensure that the two mappings point to the same input array
							for(l = 0; l < class2[j]->cslice->numOfInputs; l++)
							{
								if( strcmp(class1[i]->cslice->nameOfInput[k], class2[j]->cslice->nameOfInput[l]) == 0 )
								{
									//Change the first character from 'R' to 'S'
									class1[i]->cslice->mappingIO[k][0] = 'S';
									sprintf(input4isl, "%s\n", class1[i]->cslice->mappingIO[k]);
									//Change the first character from 'R' to 'T'
									class2[j]->cslice->mappingIO[l][0] = 'T';
									sprintf(eos(input4isl), "%s\n", class2[j]->cslice->mappingIO[l]);
									sprintf(eos(input4isl), "S = T;\n");
									invokeISL(input4isl, result);
									if(strstr(result, "True") == NULL)
									{
										printf("Mappings mismatch for the following slice classes for the input array \"%s\".\n", class1[i]->cslice->nameOfInput[k]);
										printf("Slice class in source ADDG:\n");
										printADDG(class1[i]);
										printf("Slice class in transformed ADDG:\n");
										printADDG(class2[j]);

										return false;
									}
								}
							}//end for l
						}//end for k
						break;
					}
				}
			}//end for j
		}//end if j == count2

		if(j == count2)
		{
			printf("No match found for the following slice class in the source ADDG.\n");
			printADDG(class1[i]);
			return false;
		}
	}//end for i

	return true;
}
