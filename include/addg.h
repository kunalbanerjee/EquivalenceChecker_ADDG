#ifndef ADDG_HEADER
#define ADDG_HEADER


//The following parameter defines the maximum depth of nesting
//for a statement (within IF blocks and FOR blocks)
#define MAX_DEPTH 10

//The following parameter defines the maximum number of initializations
//and incrementation/decrementation in a FOR statement
#define MAX_INCR  10

//The following parameter defines the maximum number of single statements
//allowed in a block
#define MAX_BLOCK 100

//The following parameter defines the maximum number of single statements
//allowed in a program
#define MAX_SINGSTMT 100

//The following parameter defines the maximum number of vertices allowed
//in an ADDG
#define NO_OF_VERTICES 100

//The following parameter sets the maximum character length permitted as
//a name of a node in an ADDG
#define MAX_NAME 30

//The following parameter sets the maximum number of outgoing edges permitted
//from a node in an ADDG
#define MAX_CHILD 10

//The following parameter sets the maximum possible string length
//to store domains/mappings that may be fed to OMEGA Library/ISL
#define SIZE 1000

//The following parameter gives the maximum number of slices that may
//be present in an ADDG
#define MAX_SLICE 20

//The following definition is required to append a string at the end of
//another string while using the function "sprintf
#define eos(s) ((s)+strlen(s))

//The following enumerator defines the types of nodes supported in an ADDG
enum ADDG_NODE { ARRAY, OPERATOR };


//This structure stores an IF statement
typedef struct IfStatement{
	NC *condition;
}IFSTMT;


//This structure stores a FOR statement
typedef struct ForStatement{
	unsigned int  count_init;
	DATA_TRANS   *init[MAX_INCR];
	NC           *condition;
	unsigned int  count_incr;
	DATA_TRANS   *incr[MAX_INCR];
}FORSTMT;


//A single statement is stored in DATA_TRANS
//This structure also stores the IF and FOR statements
//encompassing this statement
typedef struct SingleStatement{
	unsigned int  count_for;
	FORSTMT      *forStmt[MAX_DEPTH];
	unsigned int  count_if;
	IFSTMT       *ifStmt[MAX_DEPTH];
	DATA_TRANS   *singleStmt;
	unsigned int  stmtNumber;
}SINGSTMT;


//This structure stores a block of statements
typedef struct StatementBlock{
	unsigned int  count_stmts;
	DATA_TRANS   *statement[MAX_BLOCK];
	//The following variable is used to uniquely identify the statement number
	//in the program from which the corresponding Statement is derived
	unsigned int  stmtNumber[MAX_BLOCK];
}STMTBLK;


//This structure stores the mapping corresponding to a vertex in an ADDG
typedef struct{
	SINGSTMT *stmt;
	//The following strings are required to communicate with ISL
	char mappingS2L[SIZE];            //Mapping between iteration domain and definition domain
	char mappingS2R[MAX_CHILD][SIZE]; //Mapping between iteration domain and each operand domain
	char mappingM[MAX_CHILD][SIZE];   //Mapping between definition domain and each operand domain
	char mappingI2O[MAX_CHILD][SIZE]; //Transitive mapping between two definition domains
}MAP;


//This structure stores the details of a vertex in an ADDG
typedef struct nodeOfAddg{
	bool type;      //array or operator type -- 0 for array, 1 for operator
	bool hasParent; //if hasParent is false, it means that this node is an output array
	bool covered;   //helps in determining slices of ADDG
	char name[MAX_NAME]; //array or operator name
	MAP *map;       //map contains Mapping for operator node, and NULL for array node
	unsigned int  outDegree; //outDegree 0 implies that this node is an input array
	struct nodeOfAddg* child[MAX_CHILD]; //pointers to the children nodes
}VERTEX;


//This structure stores the characteristics of a slice
typedef struct CharacteristicsOfSlice{
	DATA_TRANS  *dataTransformation;
	unsigned int numOfInputs;
	char nameOfInput[MAX_CHILD][MAX_NAME];
	char mappingIO[MAX_CHILD][SIZE];
}CSLICE;


//This structure stores the details of an ADDG
typedef struct ArrayDataDependenceGraph{
	unsigned int numVertices;
	VERTEX* vertex[NO_OF_VERTICES];
	CSLICE* cslice;
}ADDG;


//This structure stores the details of a recurrence subgraph
typedef struct Subgraph{
	VERTEX *recurrenceArray;
	unsigned int numOfVertices;
	VERTEX *vertex[NO_OF_VERTICES];
	unsigned int numOfInputs;
	VERTEX *inputArray[MAX_CHILD];
}SGRAPH;


//This structure stores the details of all the subgraphs in an ADDG
typedef struct SetOfSubgraphs{
	unsigned int numOfRecur;
	SGRAPH *sgraph[MAX_SLICE];
}SETSGRAPH;


/**********************************************************************/

/*** Functions in createObjectModel ***/

DATA_TRANS* createDataTrans( NC*, char*, DATA_TRANS*, bool );
void printDataTrans( DATA_TRANS* );
DATA_TRANS* copyDataTrans( DATA_TRANS* );
void printSingleStatement( SINGSTMT* );
STMTBLK* assignIfStatement( STMTBLK*, STMTBLK*, STMTBLK* );
void printIfStatement( IFSTMT* );
STMTBLK* assignForStatement( STMTBLK*, STMTBLK*, STMTBLK*, STMTBLK* );
void printForStatement( FORSTMT* );
void printProgram( void );

SINGSTMT* existSingleStatement( DATA_TRANS*, unsigned int );
void updateStatementBlock( STMTBLK*, FORSTMT*, IFSTMT*, int );
void printStatementBlock( STMTBLK* );
STMTBLK* mergeStatementBlocks( STMTBLK*, STMTBLK* );


/*** Functions in createADDG ***/
void extractNamesFromVariable  ( NC*, bool, bool );
void extractNamesFromArray     ( NC*, bool, bool );
void extractNamesFromDivModFunc( NC*, bool, bool );
void extractNamesFromTerm      ( NC*, bool, bool );
void extractNamesFromSum       ( NC*, bool, bool );
void extractNamesFromRelation  ( NC*, bool, bool );
void extractNamesFromOr        ( NC*, bool, bool );
void extractNamesFromAnd       ( NC*, bool, bool );
void extractNamesOfOperands    ( NC*, bool, bool );
unsigned int existInADDG( char[], ADDG* );
unsigned int createVertexForArray   ( char[], ADDG*, bool );
unsigned int createVertexForOperator( SINGSTMT*, ADDG*    );
void  createVertices( SINGSTMT*, ADDG* );
ADDG* createADDG ( void    );
void  printVertex( VERTEX* );
void  printADDG  ( ADDG*   );

VERTEX* insertNewNode( VERTEX*, ADDG* );
void dfsADDG( VERTEX*, ADDG* );
void decycleADDG( ADDG* );


/*** Functions in computeMapping ***/
void write_lists_string( NC*, char[], bool );
void invokeISL( char[], char[] );
void computeDependenceMapping( char[], char[], unsigned int, char[] );
void insertParameterToMapping( char[], char[] );
void addParametersToMappings ( char[][MAX_NAME],  unsigned int, char[][MAX_NAME], unsigned int,
                               char[], char[][SIZE], unsigned int );
void findOperandIndicesVariable  ( NC*, char[][SIZE] );
void findOperandIndicesArray     ( NC*, char[][SIZE] );
void findOperandIndicesDivModFunc( NC*, char[][SIZE] );
void findOperandIndicesTerm      ( NC*, char[][SIZE] );
void findOperandIndicesSum       ( NC*, char[][SIZE] );
void computeMappingVertex( VERTEX* );
void computeMappingADDG  ( ADDG*   );


/*** Functions in computeSlice ***/
VERTEX* copyVertex( VERTEX* );
void recursiveCopySlice ( VERTEX*, VERTEX*, unsigned int, ADDG* );
ADDG* copySlice( ADDG* );
void continueOrBifurcate( VERTEX*, VERTEX*, ADDG*[], unsigned int );
void createSingleSlice  ( VERTEX*, ADDG*[], ADDG* );
void computeSlices( ADDG*, ADDG*[] );

void insertMappingToSlice( char[], char[], ADDG* );
void computeTransitiveCharacteristics( VERTEX*, ADDG* );
void computeCharacteristicsOfSlice( ADDG* );

void computeParameters( char[][MAX_NAME], unsigned int, char[] );
bool isValidSlice( ADDG* );


/*** Functions in computeDataTransformation ***/
bool containsOverlappingDomains( ADDG* );
void convertArraysToScalars    ( NC*   );
void computeCumulativeDataTransformation( VERTEX* );
void computeDataTransformation ( ADDG* );

unsigned int mergeSlices( ADDG*[], unsigned int, ADDG*[] );
bool checkEquivalence( ADDG*[], unsigned int, ADDG*[], unsigned int );
bool checkEquivalenceWithSubstitutions( ADDG*[], unsigned int, ADDG*[], unsigned int,
                                        char[][MAX_NAME], unsigned int, char[][MAX_NAME], unsigned int );


/*** Functions in findRecurrenceSubgraphs ***/
void initializeSubgraphs( unsigned int );
unsigned int getSCCid( VERTEX* );
bool vertexNotPresent( VERTEX*, SGRAPH* );
void findSCC         ( VERTEX*, ADDG*   );
void findBasisSubgraph           ( SGRAPH* );
void findInductionSubgraphOperatorVertex( VERTEX*, SGRAPH* );
void findInductionSubgraphArrayVertex   ( VERTEX*, SGRAPH* );
void findInductionSubgraph       ( SGRAPH* );
void findSingleRecurrenceSubgraph( SGRAPH* );
void findRecurrenceSubgraphs( unsigned int );

bool retainVertexInAddg ( VERTEX*, SGRAPH* );
VERTEX* insertUninterpretedFunction( SGRAPH* );
void introduceSingleUninterpretedFunction( ADDG*, SGRAPH* );
void introduceUninterpretedFunctions( ADDG*, unsigned int );


/*** Functions in creteDotFromAddg ***/
void createDotFromAddg( ADDG*, char* );

#endif
