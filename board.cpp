// EECE 2560: Sudoku Puzzle Part B
// Team: Elizabeth Carpenter, Sophie Xie, Vyrakaknyka Sambo, Seth Brown
// Implementation file for the Sudoku board class. Read Sudoku puzzles from an
// input file, stores the board, maintains row, column and 3x3 square conflict
// information using the Improved Conflict Counts Approach, prints the board
// and conflict data. It implements a recursive backtracking solver that uses
// recursive backtracking to locate blank cells and check legal digit placement
// using conflict matrices. The solver counts all recursive calls, prints each
// solved puzzle, and reports the total and average recursion counts across all
// puzzles in the input file.
// The solver finds exactly one solution and does not use exit. 

// Declarations and functions for project #4

#include "d_except.h"
#include "d_matrix.h"
#include <chrono>
#include <fstream>
#include <iostream>
#include <limits.h>
#include <list>

using namespace std;

// Global Types and Variables

typedef int ValueType; // The type of the value in a cell

const int SquareSize = 3; //  The number of cells in a small square
                           //  (usually 3).  The board has
                           //  SquareSize^2 rows and SquareSize^2
                           //  columns.

class board
// Stores the entire Sudoku board
{
 public:
   board(int);
   void clear();
   void initialize(ifstream& fin);
   void print();
   bool isBlank(int, int);
   ValueType getCell(int, int);

   void setCell(int, int, ValueType);
   void clearCell(int, int);
   void printConflicts();

   bool findNextBlank(int &i, int &j);
   bool isLegal(int i, int j, ValueType val);
   bool solve();
   long long getRecursiveCalls() const;

 private:
   // static options
   static const int Blank = -1;  // Indicates that a cell is blank

   static const int MinValue = 1;
   static const int MaxValue = 9;
   // The following matrices go from 1 to BoardSize in each
   // dimension, i.e., they are each (BoardSize+1) * (BoardSize+1)

   int BoardSize;
   matrix<ValueType> value;
   matrix<bool> rowConflicts;
   matrix<bool> colConflicts;
   matrix<bool> squareConflicts;

   long long recursiveCalls = 0; // Counts number of recursions
};

board::board(int sqSize)
    : BoardSize(sqSize * sqSize),
      value(BoardSize + 1, BoardSize + 1),
      rowConflicts(BoardSize + 1, MaxValue + 1, false),
      colConflicts(BoardSize + 1, MaxValue + 1, false),
      squareConflicts(BoardSize + 1, MaxValue + 1, false)
// Board constructor
{
   clear();
}

void board::clear()
// Mark all possible values as legal for each board entry
{
   for (int i = 1; i <= BoardSize; i++)
      for (int j = 1; j <= BoardSize; j++)
      {
         value[i][j] = Blank;
      }

   for (int i = 1; i <= BoardSize; i++)
      for (int v = 1; v <= MaxValue; v++)
      {
         rowConflicts[i][v] = false;
         colConflicts[i][v] = false;
         squareConflicts[i][v] = false;
      }
  recursiveCalls = 0; // Reset recursion counter
}

void board::initialize(ifstream& fin)
// Read a Sudoku board from the input file.
{
   char ch;

   clear();

   for (int i = 1; i <= BoardSize; i++)
      for (int j = 1; j <= BoardSize; j++)
      {
         fin >> ch;

         // If the read char is not Blank
         if (ch != '.')
            setCell(i, j, ch - '0'); // Convert char to int
      }
}

int squareNumber(int i, int j)
// Return the square number of cell i,j (counting from left to right,
// top to bottom.  Note that i and j each go from 1 to BoardSize
{
   // Note that (int) i/SquareSize and (int) j/SquareSize are the x-y
   // coordinates of the square that i,j is in.

   return SquareSize * ((i - 1) / SquareSize) + (j - 1) / SquareSize + 1;
}

ostream& operator<<(ostream& ostr, vector<int>& v)
// Overloaded output operator for vector class.
{
   for (size_t i = 0; i < v.size(); i++)
      ostr << v[i] << " ";
   ostr << endl;
   return ostr;
}

ValueType board::getCell(int i, int j)
// Returns the value stored in a cell.  Throws an exception
// if bad values are passed.
{
   if (i >= 1 && i <= BoardSize && j >= 1 && j <= BoardSize)
      return value[i][j];
   else
      throw rangeError("bad value in getCell");
}

bool board::isBlank(int i, int j)
// Returns true if cell i,j is blank, and false otherwise.
{
   if (i < 1 || i > BoardSize || j < 1 || j > BoardSize)
      throw rangeError("bad value in isBlank");

   return (getCell(i, j) == Blank);
}

void board::print()
// Prints the current board.
{
   for (int i = 1; i <= BoardSize; i++)
   {
      if ((i - 1) % SquareSize == 0)
      {
         cout << " -";
         for (int j = 1; j <= BoardSize; j++)
            cout << "---";
         cout << "-";
         cout << endl;
      }
      for (int j = 1; j <= BoardSize; j++)
      {
         if ((j - 1) % SquareSize == 0)
            cout << "|";
         if (!isBlank(i, j))
            cout << " " << getCell(i, j) << " ";
         else
            cout << "   ";
      }
      cout << "|";
      cout << endl;
   }

   cout << " -";
   for (int j = 1; j <= BoardSize; j++)
      cout << "---";
   cout << "-";
   cout << endl;
}

void board::setCell(int i, int j, ValueType val)
{
   // checks i and j are in range
   if (i < 1 || i > BoardSize || j < 1 || j > BoardSize)
      throw rangeError("bad value in setCell");

   // checks if input is in range (1-9)
   if (val < MinValue || val > MaxValue)
      throw rangeError("bad value in setCell");
   // if the cell is not blank clear it
   if (!isBlank(i, j))
      clearCell(i, j);

   // store value and decrement cell
   value[i][j] = val;
   rowConflicts[i][val] = true;
   colConflicts[j][val] = true;
   squareConflicts[squareNumber(i, j)][val] = true;
}

void board::clearCell(int i, int j)
{
   // if not in board range then throw error
   if (i < 1 || i > BoardSize || j < 1 || j > BoardSize)
      throw rangeError("bad value in clearCell");

   // if already blank return
   if (isBlank(i, j))
      return;

   // store value and decrements each row col and square
   ValueType val = value[i][j];

   // remove the value from the row, column and square
   rowConflicts[i][val] = false;
   colConflicts[j][val] = false;
   squareConflicts[squareNumber(i, j)][val] = false;

   // erase the cell
   value[i][j] = Blank;
}

void board::printConflicts()
{
   // print the conflict information
   cout << "Row Conflicts: " << endl;

   for (int i = 1; i <= BoardSize; i++)
   {
      // print the current row number
      cout << "Row " << i << ": ";

      // print all digits that have been placed in this row
      for (int v = MinValue; v <= MaxValue; v++)
      {
         if (rowConflicts[i][v])
            cout << v << " ";
      }

      cout << endl;
   }

   cout << "Column Conflicts: " << endl;

   for (int j = 1; j <= BoardSize; j++)
   {
      // print the current column nuumber
      cout << "column " << j << ": ";

      // print all digits that have been placed in this column
      for (int v = MinValue; v <= MaxValue; v++)
      {
         if (colConflicts[j][v])
            cout << v << " ";
      }

      cout << endl;
   }

   // print the conflict information for each 3x3 square
   cout << "Square Conflicts: " << endl;

   for (int s = 1; s <= BoardSize; s++)
   {
      // print the current square number
      cout << "Square " << s << ": ";

      // print all digits that have been placed in this square
      for (int v = MinValue; v <= MaxValue; v++)
      {
         if (squareConflicts[s][v])
            cout << v << " ";
      }
      cout << endl;
   }
}

bool board::findNextBlank(int &i, int &j)
// Find the cell with the least legal canidates to reduce recursive calls
{
   //used to track the best cell found
   int bestI = -1, bestJ = -1;
   //track the number possible canidates at the least constrained cell found
   int bestCount = MaxValue + 1;

   //Scan every cell on the board, row by row the through each column
   for (int r = 1; r <= BoardSize; r++)
   {
      for (int c = 1; c <= BoardSize; c++)
      {
         //skip cell that already are filled
         if (!isBlank(r, c))
            continue;

         //count number of legal canidated for the cell   
         int count = 0;
         for (int val = MinValue; val <= MaxValue; val++)
            if (isLegal(r, c, val))
               count++;
         //if there are 0 legal canidated fail
         if (count == 0)
         {
            i = r;
            j = c;
            return true; // dead end — report immediately
         }

         //if the cell has the least legal options set as next cell
         if (count < bestCount)
         {
            bestCount = count;
            bestI = r; bestJ = c;
            //if only one legal option immediatly choose, otherwise continue looking for least constrained cell
            if (bestCount == 1)
            {
               i = bestI; j = bestJ;
               return true; // can't do better than 1 candidate
            }
         }
      } //column end
   } //row end
   //finished going through each cell

   if (bestI == -1)
      return false; // no blanks left — solved

   i = bestI;
   j = bestJ;
   return true;
}

bool board::isLegal(int i, int j, ValueType val)
{
   // checks that the digit doesn't conflict in the same row, 
   // column, or square
   return !rowConflicts[i][val] && 
          !colConflicts[j][val] && 
          !squareConflicts[squareNumber(i, j)][val];
}

//
bool board::solve()
{
   recursiveCalls++; // starts the recursion count each time it calls itself
   int i, j;

   // if no blanks remain, puzzle is solved
   if(!findNextBlank(i, j))
   {
      return true;
   }

   // go through all digits (1-9) for each box
   for(int val = MinValue; val <= MaxValue; val++)
   {
      // check if placement is allowed
      if(isLegal(i, j, val))
      {
         setCell(i, j, val); // place digit

         // recurse if puzzle not solved
         // if the puzzle is solved, stop and return true
         if(solve())
         {
            return true;
         }
        
         clearCell(i, j);    // undo placement
      }
   }

   // no valid digit here, so backtrack
   return false;
}

long long board::getRecursiveCalls() const
{
   // return total recursion count
   return recursiveCalls;
}

// For this program we'll use c-style arguments to pass sudoku board file name
int main(int argc, char* argv[])
{
   // corrected requirement for filename argument
   if (argc < 2) 
   {
      cerr << "Must Supply a File Path" << endl;
      exit(1);
   }
   // Pull the filepath from the first argument supplied
   const char* FILE_NAME = argv[1];

   ifstream fin;

   // Read the sample grid from the file.
   string fileName = FILE_NAME;

   fin.open(fileName.c_str());

   if (!fin)
   {
      cerr << "Cannot open " << fileName << endl;
      exit(1);
   }

   // Process all boards in the file: read one, show it,
   // solve it, print solution, track recursive calls,
   // then move to the next until 'Z' is reached
   try
   {
      board b1(SquareSize);

      long long totalCalls = 0;
      int boardCount = 0;
      
      // 'Z' is termination character in txt file
      while (fin && fin.peek() != 'Z')
      {
         cout << "\nReading next board....\n";
         b1.initialize(fin);

         cout << "\nInitial Board:\n";
         b1.print();

         cout << "\nSolving...\n";

         auto start_time = std::chrono::high_resolution_clock::now();

         bool solved = b1.solve();

         auto end_time = std::chrono::high_resolution_clock::now();
         double elapsed_time = std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count();
         cout << "Sudoku Solver Execution took " << elapsed_time << " seconds" << std::endl;

         if(solved)
         {
            cout << "\nSolved Board:\n";
            b1.print();
         }
         else
         {
            cout << "\nNo solution found.\n";
         }

         long long calls = b1.getRecursiveCalls();   // get recursive count
         cout << "Recursive calls for this board: " << calls << endl;

         totalCalls += calls;   // add up recursion totals
         boardCount++;          // increment number of boards
      }

      // find the average number of calls
      double averageCalls;

      if (boardCount > 0)
          averageCalls = static_cast<double>(totalCalls) / boardCount;
      else
          averageCalls = 0.0; 
      cout << "\nTotal boards solved: " << boardCount << endl;
      cout << "Total recursive calls: " << totalCalls << endl;
      cout << "Average recursive calls: " << averageCalls << endl;  
   }
   catch (indexRangeError& ex)
   {
      cout << ex.what() << endl;
      exit(1);
   }
}
