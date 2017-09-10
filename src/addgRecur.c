#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "normalization.h"
#include "addg.h"


extern void callParser( char* );

//The following variable is used to indicate the Parser which FSMD is to be generated
//false implies ADDG-1
//true  implies ADDG-2
bool flagVar_List;

/* V0 is the list of variables which are present in FSMD M0, V1 is the list of
 * variables which are present in FSMD M1, and V0_V1 is the list of variables
 * which are present both in V0 and V1.
 */
var_list V0, V1, V0_V1, V1_minus_V0_V1;

SYMTAB  stab;


//A global variable that stores the pointers to the single statements
//in a program; an ADDG is later constructed out of it
SINGSTMT *G_PtrStmt[MAX_SINGSTMT];

//The following global variable counts the number of statements in a program
unsigned int G_countStmt;

//The following global variable counts the total number of slices in an ADDG
unsigned int G_countSlice;

//The following global variable keeps count of recurrence vertices
//which have a back edge to itself
unsigned int G_countRecurrence;
char G_namesOfRecurrenceArray[MAX_CHILD][MAX_NAME];


//Main Function
int main( int argc, char* argv[] )
{
	bool  decision;
	ADDG *addg1, *addg2;
	ADDG *slice1[MAX_SLICE], *slice2[MAX_SLICE], *sliceClass1[MAX_SLICE], *sliceClass2[MAX_SLICE];
	unsigned int i, validSliceIndex[MAX_SLICE], countValidSliceIndex, countSliceClass1, countSliceClass2;
	#ifdef ALLOW_SUBS
	unsigned int countRecur1, countRecur2;
	char namesOfRecur1[MAX_CHILD][MAX_NAME], namesOfRecur2[MAX_CHILD][MAX_NAME];
	#endif

	if(argc != 3)
	{
		printf("\nInadequate number of parameters provided.\nExiting System.\n");
		return 0;
	}

	stab.numsymbols = 0;

	G_countStmt = 0;

	flagVar_List = false;
	callParser(argv[1]);

	#ifdef PRINT_MORE
	printProgram();
	#endif

	addg1 = createADDG();
	#ifdef PRINT_MORE
	printf("\n***** Original ADDG *****\n");
	printADDG(addg1);
	#endif
	#ifdef DOTFILE
	createDotFromAddg(addg1, "addg1.dot");
	#endif

	#ifdef UNINTERPRETED
	initializeSubgraphs(0);
	#endif

	decycleADDG(addg1);
	#ifdef DOTFILE
	createDotFromAddg(addg1, "addg1_decycled.dot");
	#endif

	#ifdef UNINTERPRETED
	findRecurrenceSubgraphs(0);
	#endif

	#ifdef ALLOW_SUBS
	countRecur1 = G_countRecurrence;
	for(i = 0; i < countRecur1; i++)
		strcpy(namesOfRecur1[i], G_namesOfRecurrenceArray[i]);
	#endif

	#ifdef PRINT_MORE
	printf("\n***** ADDG without cycles *****\n");
	printADDG(addg1);
	#endif

	computeMappingADDG(addg1);
	#ifdef PRINT_MORE
	printf("\n***** ADDG with mappings computed *****\n");
	printADDG(addg1);
	#endif

	computeSlices(addg1, slice1);
	#ifdef PRINT_MORE
	for(i = 0; i < G_countSlice; i++)
	{
		printf("\n**** Slice %u ****\n", i+1);
		printADDG(slice1[i]);
	}
	#endif

	countValidSliceIndex = 0;
	for(i = 0; i < G_countSlice; i++)
	{
		if(isValidSlice(slice1[i]))
		{
			validSliceIndex[countValidSliceIndex] = i;
			countValidSliceIndex++;
		}
		#ifdef PRINT_MORE
		else
			printf("\nSlice %u is found to be invalid.\n", i+1);
		#endif
	}

	//Henceforth consider only the valid slices
	for(i = 0; i < countValidSliceIndex; i++)
		slice1[i] = slice1[validSliceIndex[i]];

	G_countSlice = countValidSliceIndex;

	for(i = 0; i < G_countSlice; i++)
		computeCharacteristicsOfSlice(slice1[i]);

	#ifdef PRINT_MORE
	printf("\n***** Valid slices with transitive mappings computed *****\n");
	for(i = 0; i < G_countSlice; i++)
	{
		printf("\n**** Slice %u****\n", i+1);
		printADDG(slice1[i]);
	}
	#endif

	//Note that there may be some further invalid slices having non-overlapping index domains
	countValidSliceIndex = 0;
	for(i = 0; i < G_countSlice; i++)
	{
		if(containsOverlappingDomains(slice1[i]))
		{
			validSliceIndex[countValidSliceIndex] = i;
			countValidSliceIndex++;
		}
		#ifdef PRINT_MORE
		else
			printf("\nSlice %u has non-overlapping index domains and hence it is invalid.\n", i+1);
		#endif
	}

	//Henceforth consider only the valid slices with overlapping index domains
	for(i = 0; i < countValidSliceIndex; i++)
		slice1[i] = slice1[validSliceIndex[i]];

	G_countSlice = countValidSliceIndex;
	for(i = 0; i < G_countSlice; i++)

	for(i = 0; i < G_countSlice; i++)
		computeDataTransformation(slice1[i]);

	#ifdef PRINT_MORE
	printf("\n*** Slices with their data transformations computed ***\n");
	for(i = 0; i < G_countSlice; i++)
	{
		printf("\n**** Slice %u ****\n", i+1);
		printADDG(slice1[i]);
	}
	#endif

	countSliceClass1 = mergeSlices(slice1, G_countSlice, sliceClass1);
	#ifdef PRINT_MORE
	printf("\nNumber of slice classes in ADDG-1: %u\n", countSliceClass1);
	printf("\n*** Slice Classes ***\n");
	for(i = 0; i < countSliceClass1; i++)
	{
		printf("\n*** Slice Class %u ***\n", i+1);
		printADDG(sliceClass1[i]);
	}
	#endif

	//////////// End of ADDG-1 ////////////

	G_countStmt = 0;

	flagVar_List = true;
	callParser(argv[2]);

	#ifdef PRINT_MORE
	printProgram();
	#endif

	addg2 = createADDG();
	#ifdef PRINT_MORE
	printf("\n***** Original ADDG *****\n");
	printADDG(addg2);
	#endif
	#ifdef DOTFILE
	createDotFromAddg(addg2, "addg2.dot");
	#endif

	#ifdef UNINTERPRETED
	initializeSubgraphs(1);
	#endif

	decycleADDG(addg2);
	#ifdef DOTFILE
	createDotFromAddg(addg2, "addg2_decycled.dot");
	#endif

	#ifdef UNINTERPRETED
	findRecurrenceSubgraphs(1);
	#endif

	#ifdef ALLOW_SUBS
	countRecur2 = G_countRecurrence;
	for(i = 0; i < countRecur2; i++)
		strcpy(namesOfRecur2[i], G_namesOfRecurrenceArray[i]);
	#endif
	#ifdef PRINT_MORE
	printf("\n***** ADDG without cycles *****\n");
	printADDG(addg2);
	#endif

	computeMappingADDG(addg2);
	#ifdef PRINT_MORE
	printf("\n***** ADDG with mappings computed *****\n");
	printADDG(addg2);
	#endif

	computeSlices(addg2, slice2);
	#ifdef PRINT_MORE
	for(i = 0; i < G_countSlice; i++)
	{
		printf("\n**** Slice %u ****\n", i+1);
		printADDG(slice2[i]);
	}
	#endif

	countValidSliceIndex = 0;
	for(i = 0; i < G_countSlice; i++)
	{
		if(isValidSlice(slice2[i]))
		{
			validSliceIndex[countValidSliceIndex] = i;
			countValidSliceIndex++;
		}
		#ifdef PRINT_MORE
		else
			printf("\nSlice %u is found to be invalid.\n", i+1);
		#endif
	}

	//Henceforth consider only the valid slices
	for(i = 0; i < countValidSliceIndex; i++)
		slice2[i] = slice2[validSliceIndex[i]];

	G_countSlice = countValidSliceIndex;

	for(i = 0; i < G_countSlice; i++)
		computeCharacteristicsOfSlice(slice2[i]);

	#ifdef PRINT_MORE
	printf("\n***** Valid slices with transitive mappings computed *****\n");
	for(i = 0; i < G_countSlice; i++)
	{
		printf("\n**** Slice %u****\n", i+1);
		printADDG(slice2[i]);
	}
	#endif

	//Note that there may be some further invalid slices having non-overlapping index domains
	countValidSliceIndex = 0;
	for(i = 0; i < G_countSlice; i++)
	{
		if(containsOverlappingDomains(slice2[i]))
		{
			validSliceIndex[countValidSliceIndex] = i;
			countValidSliceIndex++;
		}
		#ifdef PRINT_MORE
		else
			printf("\nSlice %u has non-overlapping index domains and hence it is invalid.\n", i+1);
		#endif
	}

	//Henceforth consider only the valid slices with overlapping index domains
	for(i = 0; i < countValidSliceIndex; i++)
		slice2[i] = slice2[validSliceIndex[i]];

	G_countSlice = countValidSliceIndex;

	for(i = 0; i < G_countSlice; i++)
		computeDataTransformation(slice2[i]);

	#ifdef PRINT_MORE
	printf("\n*** Slices with their data transformations computed ***\n");
	for(i = 0; i < G_countSlice; i++)
	{
		printf("\n**** Slice %u ****\n", i+1);
		printADDG(slice2[i]);
	}
	#endif

	countSliceClass2 = mergeSlices(slice2, G_countSlice, sliceClass2);
	#ifdef PRINT_MORE
	printf("\nNumber of slice classes in ADDG-2: %u\n", countSliceClass2);
	printf("\n*** Slice Classes ***\n");
	for(i = 0; i < countSliceClass2; i++)
	{
		printf("\n*** Slice Class %u ***\n", i+1);
		printADDG(sliceClass2[i]);
	}
	#endif

	if(countSliceClass1 != countSliceClass2)
		printf("\nMismatch in the number of slice classes in the two ADDGs is detected.\nTHE PROGRAMS MAY NOT BE EQUIVALENT.\n");
	else
	{
		#ifdef ALLOW_SUBS
		decision = checkEquivalenceWithSubstitutions(sliceClass1, countSliceClass1, sliceClass2, countSliceClass2, namesOfRecur1, countRecur1, namesOfRecur2, countRecur2);
		#else
		decision = checkEquivalence(sliceClass1, countSliceClass1, sliceClass2, countSliceClass2);
		#endif

		#ifdef UNINTERPRETED
		introduceUninterpretedFunctions(addg1, 0);
		printADDG(addg1);
		introduceUninterpretedFunctions(addg2, 1);
		printADDG(addg2);
		#endif

		if(decision)
			printf("\nTHE PROGRAMS ARE EQUIVALENT.\n");
		else
			printf("\nTHE PROGRAMS MAY NOT BE EQUIVALENT.\n");
	}

	return 0;
}
