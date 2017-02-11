CC=gcc
CFLAGS=-g -c
INCLUDES=-I ./include/
LEX=flex
LFLAGS=
YACC=bison
YFLAGS=-dv
CUSTOMFLAGS=-DPATH_ISL="\"/home/kunal/Dropbox/currentTools/barvinok-0.36/iscc \"" -DPRINT_MORE #-DDebug #-DUNINTERPRETED #-DALLOW_SUBS
#Explanation of the CUSTOMFLAGS
#PATH_ISL      -- Stores the path to Integer Set Library; NB: Computing the mappings using Omega Calculator is no longer supported
#PRINT_MORE    -- Shows detailed outputs
#Debug         -- Shows much more detailed outputs which can be beneficial for debugging the code
#UNINTERPRETED -- Shows substitution of recurrence arrays by uninterpreted functions
#ALLOW_SUBS    -- Allows substitution of recurrence arrays (to handle cases where the recurrence arrays are named differently in the source and the target programs; this feature requires further update


all: addg

#creates the executable file
addg : addgRecur.o computeDataTransformation.o computeSlices.o computeMapping.o findRecurrenceSubgraphs.o createADDG.o createObjectModel.o createNormExpr.o normalization.o iso2011c.tab.o lex.yy.o
	$(CC) -g -o ./bin/addgEqvChkr ./obj/*.o


#creates the object files
addgRecur.o :
	$(CC) $(CFLAGS) $(CUSTOMFLAGS) -Wall $(INCLUDES) ./src/addgRecur.c -o ./obj/addgRecur.o

computeDataTransformation.o :
	$(CC) $(CFLAGS) $(CUSTOMFLAGS) -Wall $(INCLUDES) ./src/computeDataTransformation.c -o ./obj/computeDataTransformation.o

computeSlices.o :
	$(CC) $(CFLAGS) $(CUSTOMFLAGS) -Wall $(INCLUDES) ./src/computeSlices.c -o ./obj/computeSlices.o

computeMapping.o :
	$(CC) $(CFLAGS) $(CUSTOMFLAGS) -Wall $(INCLUDES) ./src/computeMapping.c -o ./obj/computeMapping.o

findRecurrenceSubgraphs.o :
	$(CC) $(CFLAGS) $(CUSTOMFLAGS) -Wall $(INCLUDES) ./src/findRecurrenceSubgraphs.c -o ./obj/findRecurrenceSubgraphs.o

createADDG.o :
	$(CC) $(CFLAGS) $(CUSTOMFLAGS) -Wall $(INCLUDES) ./src/createADDG.c -o ./obj/createADDG.o

createObjectModel.o :
	$(CC) $(CFLAGS) $(CUSTOMFLAGS) -Wall $(INCLUDES) ./src/createObjectModel.c -o ./obj/createObjectModel.o

createNormExpr.o :
	$(CC) $(CFLAGS) $(CUSTOMFLAGS) -Wall $(INCLUDES) ./src/createNormExpr.c -o ./obj/createNormExpr.o

normalization.o :
	$(CC) $(CFLAGS) $(CUSTOMFLAGS) $(INCLUDES) ./src/normalization.c -o ./obj/normalization.o

iso2011c.tab.o : parser
	$(CC) $(CFLAGS) $(INCLUDES) ./src/iso2011c.tab.c -o ./obj/iso2011c.tab.o

lex.yy.o : parser
	$(CC) $(CFLAGS) $(INCLUDES) ./src/lex.yy.c -o ./obj/lex.yy.o

#calls the parser
parser : lexer
	$(YACC) $(LFLAGS) $(YFLAGS) ./src/iso2011c.y
	mv ./iso2011c.tab.h  ./include/iso2011c.tab.h
	mv ./iso2011c.tab.c  ./src/iso2011c.tab.c
	mv ./iso2011c.output ./obj/iso2011c.output

#calls the lexer
lexer :  
	$(LEX) $(LFLAGS) ./src/iso2011c.l
	mv ./lex.yy.c ./src/lex.yy.c


clean :
	rm -rf ./obj/*.o
	rm -rf ./obj/*.output
	rm -rf ./include/*.tab.h
	rm -rf ./bin/addgEqvChkr
	rm -rf ./src/*.tab.c
	rm -rf ./src/lex.yy.c
	rm -rf ./input4isl.txt
	rm -rf ./islOutput.txt
	rm -rf ./grepOutput.txt
