#EECE 2560 Sudoku Part B
#Team: Elizabeth Carpenter, Sophie Xie, Vyrakaknyka Sambo, Seth Brown
#Sudoku Part B Carxiesambro Makefile

sudoku: board.cpp d_matrix.h d_except.h
	 g++ -Wall -Wextra board.cpp -o sudoku

clean: 
	 rm -f sudoku
