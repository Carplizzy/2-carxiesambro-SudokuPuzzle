# 2-carxiesambro-SudokuPuzzle Part B
## Team: Elizabeth Carpenter, Sophie Xie, Vyrakaknyka Sambo, Seth Brown

## Sudoku Puzzle Part B Project Description
This project builds a basic Sudoku puzzle solver by implementing a recursive backtracking algorithm. 
The program reads Sudoku puzzles from an input file, stores the puzzle in a 9x9 board, and maintains 
row, column and 3x3 square conflict information using the Improved Conflict Counts Approach. It 
prints the board and conflict data, locates blank cells, and checks legal digit placement. The 
solver uses recursive backtracking to fill in all empty cells, counts every recursive call made 
during the solving process, prints each fully solved puzzle, and reports both the total and average 
recursion counts across all puzzles. The solver is designed to find exactly one valid solution for 
each puzzle without using exit.

## Build Instructions 
Compile the project using: 
```bash
make
```
To remove object files:
```bash
make clean
```
## Run Instructions 
The program requires the Sudoku input filename as a command-line argument. 
To run the program with sudoku.txt: 

```bash
./sudoku sudoku.txt
```
Other input files can be used in the same way: 

./sudoku sudoku2.txt
./sudoku sudoku3.txt

## Notes
The files d_matrix.h and d_except.h were provided by the course instructor and were not
modified by our team. 

