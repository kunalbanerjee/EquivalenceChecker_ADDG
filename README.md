# EquivalenceChecker_ADDG
### Kunal Banerjee
#### Feb 11, 2016

This tool is a product of our works reported below:
* [Translation Validation of Loop and Arithmetic Transformations in the Presence of Recurrences, LCTES 2016](http://dl.acm.org/citation.cfm?doid=2907950.2907954)
* [Verification of Loop and Arithmetic Transformations of Array-Intensive Behaviors, TCAD 2013](http://ieeexplore.ieee.org/document/6634544/?tp=&arnumber=6634544)
* [Equivalence Checking of Array-Intensive Programs, ISVLSI 2011](http://ieeexplore.ieee.org/document/5992498/)



Pre-requisites:
* gcc 4.7.2
* flex 2.5.35
* bison 2.5
* [barvinok-0.36](http://barvinok.gforge.inria.fr/) (Integer Set Library)


Please execute the following commands to install and run ADDG.

/***********************************************************************/

make                         <-- This will install ADDG Equivalence Checker.

cd bin

./eqChkAddg file1.c file2.c  <-- To run ADDG Equivalence Checker.

/***********************************************************************/

NB: You are required to modify the path to ISL as defined by the variable
"PATH_ISL" on line 8 of Makefile.


Some benchmarks are provided in the folder "Benchmarks".

Please note the following restrictions on the input C files:
* Should have a single function.
* Should not have bitwise operators, pointers, user-defined data structures like struct, enum, etc.
* Should not have while, do..while loops, i.e., only "for" loops are allowed.


For more information (and on discovery of bugs), please contact:
kunal.banerjee.cse@gmail.com
