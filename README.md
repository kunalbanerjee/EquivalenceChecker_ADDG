#EquivalenceChecker_ADDG
Kunal Banerjee
Jan 21, 2016


Pre-requisites:
1. gcc 4.7.2
2. flex 2.5.35
3. bison 2.5
4. barvinok-0.36 (Integer Set Library, downloadable from "http://barvinok.gforge.inria.fr/")


Please execute the following commands to install and run ADDG.

/***********************************************************************/

In the folder neo_addg:

make                     <-- This will install ADDG Equivalence Checker.

cd bin			

./eqChkAddg file1 file2  <-- To run ADDG Equivalence Checker.

/***********************************************************************/

NB: You are required to modify the path to ISL as defined by the variable
"PATH_ISL" on line 8 of Makefile.


Some benchmarks are provided in the folder "Benchmarks".

Please note the following restrictions on the input C files:
1. Should have a single function.
2. Should not have bitwise operators, pointers, user-defined data structures like struct, enum, etc.
4. Should not have while, do..while loops.


For more information (and on discovery of bugs), please contact:
kunalb@cse.iitkgp.ernet.in, kunal.banerjee.cse@gmail.com
