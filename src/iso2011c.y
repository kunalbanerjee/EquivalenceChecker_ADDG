/* 2011 ISO C Parser */

%{

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <stdbool.h>
 #include "normalization.h"
 #include "addg.h"

 //The following variable counts the statements (ending with a semicolon)
 //in the program
 unsigned int G_CountStmtNumber;
 extern bool G_FlagDeclare;

%}

%union{
	char*  string;
	struct normalized_cell* norm;
	struct Assign*          dataTrans;
	struct StatementBlock*  stmtblk;
	}

%token <string>	IDENTIFIER I_CONSTANT F_CONSTANT STRING_LITERAL
%token FUNC_NAME SIZEOF
%token PTR_OP INC_OP DEC_OP LEFT_OP RIGHT_OP
%token LE_OP GE_OP EQ_OP NE_OP LT_OP GT_OP
%token AND_OP OR_OP NOT_OP TERNARY
%token EQUAL MUL_ASSIGN DIV_ASSIGN MOD_ASSIGN ADD_ASSIGN SUB_ASSIGN
%token LEFT_ASSIGN RIGHT_ASSIGN AND_ASSIGN
%token XOR_ASSIGN OR_ASSIGN
%token TYPEDEF_NAME ENUMERATION_CONSTANT

%token TYPEDEF EXTERN STATIC AUTO REGISTER INLINE
%token CONST RESTRICT VOLATILE
%token BOOL CHAR SHORT INT LONG SIGNED UNSIGNED FLOAT DOUBLE VOID
%token COMPLEX IMAGINARY
%token STRUCT UNION ENUM ELLIPSIS

%token CASE DEFAULT IF ELSE SWITCH WHILE DO FOR GOTO CONTINUE BREAK RETURN

%token ALIGNAS ALIGNOF ATOMIC GENERIC NORETURN STATIC_ASSERT THREAD_LOCAL

%token SEMIC COMMA COLON DOT
%token LEFT_BR RIGHT_BR LEFT_SQBR RIGHT_SQBR LEFT_CURBR RIGHT_CURBR
%token MINUS PLUS MULT DIV MOD
%token BIT_AND BIT_OR BIT_XOR BIT_NOT

%type <string> constant unary_operator assignment_operator
%type <norm> primary_expression postfix_expression unary_expression
%type <norm> cast_expression multiplicative_expression additive_expression
%type <norm> shift_expression relational_expression equality_expression
%type <norm> and_expression exclusive_or_expression inclusive_or_expression
%type <norm> logical_and_expression logical_or_expression conditional_expression
%type <norm> argument_expression_list
%type <dataTrans> assignment_expression
%type <stmtblk> expression expression_statement statement compound_statement
%type <stmtblk> selection_statement iteration_statement block_item block_item_list


%expect 2

%start translation_unit
%%

primary_expression
	: IDENTIFIER                  { $$ = createVariable($1); }
	| constant                    { $$ = createConstant($1); }
	| string                      { }
	| LEFT_BR expression RIGHT_BR { $$ = $2->statement[0]->rhs; }
	| generic_selection           { }
	;

constant
	: I_CONSTANT           { strcpy($$,$1); }
	| F_CONSTANT           { }
	| ENUMERATION_CONSTANT { }
	;

enumeration_constant
	: IDENTIFIER
	;

string
	: STRING_LITERAL
	| FUNC_NAME
	;

generic_selection
	: GENERIC LEFT_BR assignment_expression COMMA generic_assoc_list RIGHT_BR
	;

generic_assoc_list
	: generic_association
	| generic_assoc_list COMMA generic_association
	;

generic_association
	: type_name COLON assignment_expression
	| DEFAULT COLON assignment_expression
	;

postfix_expression
	: primary_expression                                           { $$ = $1; }
	| postfix_expression LEFT_SQBR expression RIGHT_SQBR           { $$ = createArray($1, $3->statement[0]->rhs); }
	| postfix_expression LEFT_BR RIGHT_BR                          { $$ = createFunction($1, NULL); }
	| postfix_expression LEFT_BR argument_expression_list RIGHT_BR { $$ = createFunction($1, $3);   }
	| postfix_expression DOT IDENTIFIER
	| postfix_expression PTR_OP IDENTIFIER
	| postfix_expression INC_OP                                                { $$ = Add_Sums($1, createConstant("1"),  $$); }
	| postfix_expression DEC_OP                                                { $$ = Add_Sums($1, createConstant("-1"), $$); }
	| LEFT_BR type_name RIGHT_BR LEFT_CURBR initializer_list RIGHT_CURBR       { }
	| LEFT_BR type_name RIGHT_BR LEFT_CURBR initializer_list COMMA RIGHT_CURBR { }
	;

argument_expression_list
	: assignment_expression                                { $$ = $1->rhs; }
	| argument_expression_list COMMA assignment_expression { $$ = addArgument($1, $3->rhs); }
	;

unary_expression
	: postfix_expression                 { $$= $1; }
	| INC_OP unary_expression            { $$ = Add_Sums($2, createConstant("1"),  $$); }
	| DEC_OP unary_expression            { $$ = Add_Sums($2, createConstant("-1"), $$); }
	| unary_operator cast_expression     { if (strcmp($1,"-")==0) {$$ = negateExpression($2);} }
	| SIZEOF unary_expression            { }
	| SIZEOF LEFT_BR type_name RIGHT_BR  { }
	| ALIGNOF LEFT_BR type_name RIGHT_BR { }
	;

unary_operator
	: BIT_AND { }
	| MULT    { }
	| PLUS    { }
	| MINUS   { $$ = strcpy($$,"-"); }
	| BIT_NOT { }
	| NOT_OP  { }
	;

cast_expression
	: unary_expression                           { $$ = $1; }
	| LEFT_BR type_name RIGHT_BR cast_expression { }
	;

multiplicative_expression
	: cast_expression                                { $$ = $1; }
	| multiplicative_expression MULT cast_expression { $$ = Mult_Sum_With_Sum($1, $3, $$); }
	| multiplicative_expression DIV cast_expression  { $$ = createExpression_Mod_Div($1, $3, '/'); }
	| multiplicative_expression MOD cast_expression  { $$ = createExpression_Mod_Div($1, $3, '%'); }
	;

additive_expression
	: multiplicative_expression                           { $$ = $1; }
	| additive_expression PLUS multiplicative_expression  { $$ = Add_Sums($1, $3, $$); }
	| additive_expression MINUS multiplicative_expression { $$ = Add_Sums($1, negateExpression($3), $$); }
	;

shift_expression
	: additive_expression                           { $$ = $1; }
	| shift_expression LEFT_OP additive_expression  { }
	| shift_expression RIGHT_OP additive_expression { }
	;

relational_expression
	: shift_expression                             { $$ = $1; }
	| relational_expression LT_OP shift_expression { $$ = createConditionalExpression($1, "<",  $3); }
	| relational_expression GT_OP shift_expression { $$ = createConditionalExpression($1, ">",  $3); }
	| relational_expression LE_OP shift_expression { $$ = createConditionalExpression($1, "<=", $3); }
	| relational_expression GE_OP shift_expression { $$ = createConditionalExpression($1, ">=", $3); }
	;

equality_expression
	: relational_expression                           { $$ = $1; }
	| equality_expression EQ_OP relational_expression { $$ = createConditionalExpression($1, "==", $3); }
	| equality_expression NE_OP relational_expression { $$ = createConditionalExpression($1, "!=", $3); }
	;

and_expression
	: equality_expression                             { $$ = $1; }
	| and_expression BIT_AND equality_expression
	;

exclusive_or_expression
	: and_expression                                  { $$ = $1; }
	| exclusive_or_expression BIT_XOR and_expression
	;

inclusive_or_expression
	: exclusive_or_expression                         { $$ = $1; }
	| inclusive_or_expression BIT_OR exclusive_or_expression
	;

logical_and_expression
	: inclusive_or_expression                               { $$ = $1; }
	| logical_and_expression AND_OP inclusive_or_expression { $$ = createAndExpression($1, $3); }
	;

logical_or_expression
	: logical_and_expression                                { $$ = $1; }
	| logical_or_expression OR_OP logical_and_expression    { $$ = createOrExpression($1, $3);  }
	;

conditional_expression
	: logical_or_expression                                 { $$ = $1; }
	| logical_or_expression TERNARY expression COLON conditional_expression { }
	;

assignment_expression
	: conditional_expression                                     { $$ = createDataTrans($1, NULL, NULL, false); }
	| unary_expression assignment_operator assignment_expression { $$ = createDataTrans($1, $2, $3, true); }
	;

assignment_operator
	: EQUAL         { strcpy($$, "="); }
	| MUL_ASSIGN    { strcpy($$,"*="); }
	| DIV_ASSIGN    { strcpy($$,"/="); }
	| MOD_ASSIGN    { strcpy($$,"%="); }
	| ADD_ASSIGN    { strcpy($$,"+="); }
	| SUB_ASSIGN    { strcpy($$,"-="); }
	| LEFT_ASSIGN   { /* The following operations are not supported */ }
	| RIGHT_ASSIGN  { }
	| AND_ASSIGN    { }
	| XOR_ASSIGN    { }
	| OR_ASSIGN     { }
	;

expression
	: assignment_expression                  { $$ = (STMTBLK*)malloc(sizeof(STMTBLK)); $$->statement[0] = $1; $$->count_stmts = 1; }
	| expression COMMA assignment_expression { $$->statement[$$->count_stmts] = $3; ($$->count_stmts)++; }
	;

constant_expression
	: conditional_expression	/* with constraints */
	;

declaration
	: declaration_specifiers SEMIC
	| declaration_specifiers init_declarator_list SEMIC
	| static_assert_declaration
	;

declaration_specifiers
	: storage_class_specifier declaration_specifiers
	| storage_class_specifier
	| type_specifier declaration_specifiers
	| type_specifier
	| type_qualifier declaration_specifiers
	| type_qualifier
	| function_specifier declaration_specifiers
	| function_specifier
	| alignment_specifier declaration_specifiers
	| alignment_specifier
	;

init_declarator_list
	: init_declarator
	| init_declarator_list COMMA init_declarator
	;

init_declarator
	: declarator EQUAL initializer
	| declarator
	;

storage_class_specifier
	: TYPEDEF	/* identifiers must be flagged as TYPEDEF_NAME */
	| EXTERN
	| STATIC
	| THREAD_LOCAL
	| AUTO
	| REGISTER
	;

type_specifier
	: VOID
	| CHAR
	| SHORT
	| INT
	| LONG
	| FLOAT
	| DOUBLE
	| SIGNED
	| UNSIGNED
	| BOOL
	| COMPLEX
	| IMAGINARY	  	/* non-mandated extension */
	| atomic_type_specifier
	| struct_or_union_specifier
	| enum_specifier
	| TYPEDEF_NAME		/* after it has been defined as such */
	;

struct_or_union_specifier
	: struct_or_union LEFT_CURBR struct_declaration_list RIGHT_CURBR
	| struct_or_union IDENTIFIER LEFT_CURBR struct_declaration_list RIGHT_CURBR
	| struct_or_union IDENTIFIER
	;

struct_or_union
	: STRUCT
	| UNION
	;

struct_declaration_list
	: struct_declaration
	| struct_declaration_list struct_declaration
	;

struct_declaration
	: specifier_qualifier_list SEMIC	/* for anonymous struct/union */
	| specifier_qualifier_list struct_declarator_list SEMIC
	| static_assert_declaration
	;

specifier_qualifier_list
	: type_specifier specifier_qualifier_list
	| type_specifier
	| type_qualifier specifier_qualifier_list
	| type_qualifier
	;

struct_declarator_list
	: struct_declarator
	| struct_declarator_list COMMA struct_declarator
	;

struct_declarator
	: COLON constant_expression
	| declarator COLON constant_expression
	| declarator
	;

enum_specifier
	: ENUM LEFT_CURBR enumerator_list RIGHT_CURBR
	| ENUM LEFT_CURBR enumerator_list COMMA RIGHT_CURBR
	| ENUM IDENTIFIER LEFT_CURBR enumerator_list RIGHT_CURBR
	| ENUM IDENTIFIER LEFT_CURBR enumerator_list COMMA RIGHT_CURBR
	| ENUM IDENTIFIER
	;

enumerator_list
	: enumerator
	| enumerator_list COMMA enumerator
	;

enumerator	/* identifiers must be flagged as ENUMERATION_CONSTANT */
	: enumeration_constant EQUAL constant_expression
	| enumeration_constant
	;

atomic_type_specifier
	: ATOMIC LEFT_BR type_name RIGHT_BR
	;

type_qualifier
	: CONST
	| RESTRICT
	| VOLATILE
	| ATOMIC
	;

function_specifier
	: INLINE
	| NORETURN
	;

alignment_specifier
	: ALIGNAS LEFT_BR type_name RIGHT_BR
	| ALIGNAS LEFT_BR constant_expression RIGHT_BR
	;

declarator
	: pointer direct_declarator
	| direct_declarator
	;

direct_declarator
	: IDENTIFIER
	| LEFT_BR declarator RIGHT_BR
	| direct_declarator LEFT_SQBR RIGHT_SQBR
	| direct_declarator LEFT_SQBR MULT RIGHT_SQBR
	| direct_declarator LEFT_SQBR STATIC type_qualifier_list assignment_expression RIGHT_SQBR
	| direct_declarator LEFT_SQBR STATIC assignment_expression RIGHT_SQBR
	| direct_declarator LEFT_SQBR type_qualifier_list MULT RIGHT_SQBR
	| direct_declarator LEFT_SQBR type_qualifier_list STATIC assignment_expression RIGHT_SQBR
	| direct_declarator LEFT_SQBR type_qualifier_list assignment_expression RIGHT_SQBR
	| direct_declarator LEFT_SQBR type_qualifier_list RIGHT_SQBR
	| direct_declarator LEFT_SQBR assignment_expression RIGHT_SQBR
	| direct_declarator LEFT_BR parameter_type_list RIGHT_BR
	| direct_declarator LEFT_BR RIGHT_BR
	| direct_declarator LEFT_BR identifier_list RIGHT_BR
	;

pointer
	: MULT type_qualifier_list pointer
	| MULT type_qualifier_list
	| MULT pointer
	| MULT
	;

type_qualifier_list
	: type_qualifier
	| type_qualifier_list type_qualifier
	;


parameter_type_list
	: parameter_list COMMA ELLIPSIS
	| parameter_list
	;

parameter_list
	: parameter_declaration
	| parameter_list COMMA parameter_declaration
	;

parameter_declaration
	: declaration_specifiers declarator
	| declaration_specifiers abstract_declarator
	| declaration_specifiers
	;

identifier_list
	: IDENTIFIER
	| identifier_list COMMA IDENTIFIER
	;

type_name
	: specifier_qualifier_list abstract_declarator
	| specifier_qualifier_list
	;

abstract_declarator
	: pointer direct_abstract_declarator
	| pointer
	| direct_abstract_declarator
	;

direct_abstract_declarator
	: LEFT_BR abstract_declarator RIGHT_BR
	| LEFT_SQBR RIGHT_SQBR
	| LEFT_SQBR MULT RIGHT_SQBR
	| LEFT_SQBR STATIC type_qualifier_list assignment_expression RIGHT_SQBR
	| LEFT_SQBR STATIC assignment_expression RIGHT_SQBR
	| LEFT_SQBR type_qualifier_list STATIC assignment_expression RIGHT_SQBR
	| LEFT_SQBR type_qualifier_list assignment_expression RIGHT_SQBR
	| LEFT_SQBR type_qualifier_list RIGHT_SQBR
	| LEFT_SQBR assignment_expression RIGHT_SQBR
	| direct_abstract_declarator LEFT_SQBR RIGHT_SQBR
	| direct_abstract_declarator LEFT_SQBR MULT RIGHT_SQBR
	| direct_abstract_declarator LEFT_SQBR STATIC type_qualifier_list assignment_expression RIGHT_SQBR
	| direct_abstract_declarator LEFT_SQBR STATIC assignment_expression RIGHT_SQBR
	| direct_abstract_declarator LEFT_SQBR type_qualifier_list assignment_expression RIGHT_SQBR
	| direct_abstract_declarator LEFT_SQBR type_qualifier_list STATIC assignment_expression RIGHT_SQBR
	| direct_abstract_declarator LEFT_SQBR type_qualifier_list RIGHT_SQBR
	| direct_abstract_declarator LEFT_SQBR assignment_expression RIGHT_SQBR
	| LEFT_BR RIGHT_BR
	| LEFT_BR parameter_type_list RIGHT_BR
	| direct_abstract_declarator LEFT_BR RIGHT_BR
	| direct_abstract_declarator LEFT_BR parameter_type_list RIGHT_BR
	;

initializer
	: LEFT_CURBR initializer_list RIGHT_CURBR
	| LEFT_CURBR initializer_list COMMA RIGHT_CURBR
	| assignment_expression
	;

initializer_list
	: designation initializer
	| initializer
	| initializer_list COMMA designation initializer
	| initializer_list COMMA initializer
	;

designation
	: designator_list EQUAL
	;

designator_list
	: designator
	| designator_list designator
	;

designator
	: LEFT_SQBR constant_expression RIGHT_SQBR
	| DOT IDENTIFIER
	;

static_assert_declaration
	: STATIC_ASSERT LEFT_BR constant_expression COMMA STRING_LITERAL RIGHT_BR SEMIC
	;

statement
	: labeled_statement    { }
	| compound_statement   { $$ = $1; }
	| expression_statement { $$ = $1; }
	| selection_statement  { $$ = $1; }
	| iteration_statement  { $$ = $1; }
	| jump_statement       { }
	;

labeled_statement
	: IDENTIFIER COLON statement
	| CASE constant_expression COLON statement
	| DEFAULT COLON statement
	;

compound_statement
	: LEFT_CURBR RIGHT_CURBR                  { $$->count_stmts = 0; }
	| LEFT_CURBR  block_item_list RIGHT_CURBR { $$ = $2; }
	;

block_item_list
	: block_item                 { updateStatementBlock($1, NULL, NULL, 2); $$ = (STMTBLK*)malloc(sizeof(STMTBLK)); $$ = $1; }
	| block_item_list block_item { updateStatementBlock($2, NULL, NULL, 2); $$ = mergeStatementBlocks($1, $2); }
	;

block_item
	: declaration { }
	| statement   { $$ = $1; }
	;

expression_statement
	: SEMIC            { $$->count_stmts = 0; }
	| expression SEMIC { $$ = $1; $$->stmtNumber[$$->count_stmts-1] = G_CountStmtNumber++; }
	;

selection_statement
	: IF LEFT_BR expression RIGHT_BR statement ELSE statement { $$ = assignIfStatement($3, $5, $7); }
	| IF LEFT_BR expression RIGHT_BR statement                { $$ = assignIfStatement($3, $5, NULL); }
	| SWITCH LEFT_BR expression RIGHT_BR statement            { }
	;

iteration_statement
	: WHILE LEFT_BR expression RIGHT_BR statement          { }
	| DO statement WHILE LEFT_BR expression RIGHT_BR SEMIC { }
	| FOR LEFT_BR expression_statement expression_statement RIGHT_BR statement            { /* This construct is not supported because the iterator incrementation values must be supplied */ }
	| FOR LEFT_BR expression_statement expression_statement expression RIGHT_BR statement { $$ = assignForStatement($3, $4, $5, $7); }
	| FOR LEFT_BR declaration expression_statement RIGHT_BR statement            { }
	| FOR LEFT_BR declaration expression_statement expression RIGHT_BR statement { }
	;

jump_statement
	: GOTO IDENTIFIER SEMIC
	| CONTINUE SEMIC
	| BREAK SEMIC
	| RETURN SEMIC
	| RETURN expression SEMIC
	;

translation_unit
	: external_declaration
	| translation_unit external_declaration
	;

external_declaration
	: function_definition
	| declaration
	;

function_definition
	: declaration_specifiers declarator declaration_list compound_statement
	| declaration_specifiers declarator compound_statement

declaration_list
	: declaration
	| declaration_list declaration
	;

%%

extern char yytext[];
extern int column;

extern FILE *yyin;


int yyerror(const char *s)
{
	fflush(stdout);
	printf("\n%*s\n%*s\n", column, "^", column, s);
}


void callParser(char *fileName)
{

 	printf("\nCalling Parser....");
 	printf("\nFile opening: %s\n", fileName);

 	G_FlagDeclare = false;

 	yyin = fopen(fileName, "r");

 	if(yyin == NULL)
	{
 		printf("\nFile open error\n");
		exit(0);
	}

	G_CountStmtNumber = 0;

	yyparse();
}
