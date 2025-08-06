# Sudoku-C++
A c++ program to solve sudoku puzzles.
To use, compile the program into an executable and run it in the command line with an argument pointing to the file containing the puzzles you want to solved.
It should be a CSV file in the format "puzzle,solution". 

For example, to solve the puzzles in the included puzzles.csv file, run ./sudoku puzzles.csv  

Note that the time elapsed in the async speedtest is *less* than the total solving time by a significant amount.
The time elapsed represents the time you (the user) spent waiting on the solver. 
The total solving time is the *total time spent by the CPU across all threads*.