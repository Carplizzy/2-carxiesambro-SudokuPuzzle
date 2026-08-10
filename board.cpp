// EECE 2560: Sudoku Puzzle Part B
// Team: Elizabeth Carpenter, Sophie Xie, Vyrakaknyka Sambo, Seth Brown
// Implementation file for the Sudoku board class. Read Sudoku puzzles from an
// input file, stores the board, maintains row, column and 3x3 square conflict
// information using the Improved Conflict Counts Approach, prints the board
// and conflicts, and checks whether the board is solved. It implements a
// recursive backtracking solver that uses recursive backtracking to locate
// blank cells and check legal digit placement using conflict matrices. The
// solver counts all recursive calls, prints each solved puzzle, and reports
// the total and average recursion counts across all puzzles in the input file.
// The solver finds exactly one solution and does not use exit. 

// Declarations and functions for project #4

#include "d_except.h"
#include "d_matrix.h"
#include <fstream>
#include <iostream>
#include <limits.h>
#include <list>

using namespace std;

// Global Types and Variables

typedef int ValueType; // The type of the value in a cell
int numSolutions = 0;

const int SquareSize = 3; //  The number of cells in a small square
                           //  (usually 3).  The board has
                           //  SquareSize^2 rows and SquareSize^2
                           //  columns.
int numsolutions = 0;      // Keep track of number of solutions per board

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

bool board::isSolved()
{
   // Check every cell on the board
   for (int i = 1; i <= BoardSize; i++)
   {
      for (int j = 1; j <= BoardSize; j++)
      {
         // if any cell is blank, the board is not solved
         if (isBlank(i, j))
         {
            cout << "Board is NOT solved." << endl;
            return false;
         }
      }
   }

   // all cells have been filled
   cout << "Board is solved." << endl;
   return true;
}

// For this program we'll use c-style arguments to pass sudoku board file name
int main(int argc, char* argv[])
{
   if (argc == 0) {
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

   // Catches file-read exceptions in order to display to console
   try
   {
      board b1(SquareSize);
      
      // 'Z' is termination character in txt file
      while (fin && fin.peek() != 'Z')
      {
         b1.initialize(fin);
         b1.print();
         b1.printConflicts();
         b1.isSolved();
      }
   }
   catch (indexRangeError& ex)
   {
      cout << ex.what() << endl;
      exit(1);
   }
}


