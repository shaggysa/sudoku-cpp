#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <array>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

using std::string, std::cout, std::cerr, std::ifstream, std::vector, std::stringstream, std::array;

class puzzle_reader {
    public:
        int num_puzzles = 0;

        explicit puzzle_reader(const string& filename) {
            ifstream input{filename};

            if (!input.is_open()) {
                cerr << "Couldn't read file " << filename << "\n";
            }
            bool first_line = true;
            for (string line; std::getline(input, line);) {
                if (first_line) { //skip the header line
                    first_line = false;;
                    continue;
                }

                array<char, 81> unsolved{};
                array<char, 81> solved{};

                for (int i = 0; i < line.length(); i++) {
                    if (i < 81) {
                        unsolved[i] = line[i];
                    }
                    else if (i > 81) { //skip the comma separator
                        solved[i-82] = line[i];
                    }
                }

                this->unsolved_puzzles.push_back(unsolved);
                this->solved_puzzles.push_back(solved);
                ++num_puzzles;

                }
                    }

    array<char, 81> get_unsolved(const int line_numer) const {
        return this->unsolved_puzzles[line_numer-2];
    }

    array<char, 81> get_solved(const int line_numer) const {
        return this->solved_puzzles[line_numer-2];
    }

    private:
    vector<array<char, 81>> unsolved_puzzles;
    vector<array<char, 81>> solved_puzzles;

};


void print_puzzle(const array<char, 81>& puzzle) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            cout << puzzle[i*9+j] << "  ";
        }
        cout << "\n";
    }
}

int main() {
    auto start = std::chrono::system_clock::now();
    puzzle_reader x = puzzle_reader("puzzles.csv");
    auto time = (std::chrono::system_clock::now() - start).count()/1000000.0;
    cout << "Loaded  " <<  x.num_puzzles << " puzzles in " << time << " ms\n";
    print_puzzle(x.get_solved(2));

}

