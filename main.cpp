#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <array>
#include <chrono>
#include <ctime>
#include <set>
#include <unordered_map>
#include <bits/stdc++.h>
#include <sys/resource.h>
#include <unistd.h>

namespace fs = std::filesystem;

using std::string, std::cout, std::cerr, std::ifstream, std::vector, std::stringstream, std::array;

struct puzzle {
    array <char,81> puzz;
    vector<int> blank_positions;
    std::unordered_map<int, vector<char>> possibilities;
    std::unordered_map<int, int> current_pos;
    bool solved = false;
};

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
                        unsolved[i] = (line[i] - '0');
                    }
                    else if (i > 81) { //skip the comma separator
                        solved[i-82] = line[i] - '0';
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

class puzzle_solver {
public:
    static bool no_repeats(const char list[9]) {
        bool seen[10] = {false};
        for (int i = 0; i < 9; i++) {
            if (list[i] == 0) {
                continue;
            }
            if (seen[list[i]]) {
                return false;
            }
            seen[list[i]] = true;
        }
        return true;
    }

    static bool position_valid(const array<char,81> &puzzle, const int position) {
        const int row = position / 9;
        const int col = position % 9;

        const int root_row = (row/3) * 3;
        const int root_col = (col/3) * 3;
        const int root_pos = (root_row * 9) + root_col;

        char col_to_check[9];
        char row_to_check[9];
        const char square_to_check[] {puzzle[root_pos], puzzle[root_pos + 1], puzzle[root_pos + 2], puzzle[root_pos + 9], puzzle[root_pos + 10], puzzle[root_pos + 11], puzzle[root_pos + 18], puzzle[root_pos + 19], puzzle[root_pos + 20]};

        for (int i = 0; i < 9; i++) {
            col_to_check[i] = puzzle[i + (row * 9)];
            row_to_check[i] = puzzle[col + (i * 9)];
        }
        if (no_repeats(col_to_check) && no_repeats(row_to_check) && no_repeats(square_to_check)) {
            return true;

        }
        return false;
    }

    static vector<char> get_possibilities(array<char, 81> &puzzle, const int position) {
        const int row = position / 9;
        const int col = position % 9;

        const int root_row = (row/3) * 3;
        const int root_col = (col/3) * 3;
        const int root_pos = (root_row * 9) + root_col;

        array<char, 26> items {puzzle[root_pos], puzzle[root_pos + 1], puzzle[root_pos + 2], puzzle[root_pos + 9], puzzle[root_pos + 10], puzzle[root_pos + 11], puzzle[root_pos + 18], puzzle[root_pos + 19], puzzle[root_pos + 20]};
        for (int i = 0; i < 9; i++) {
            items[i + 8] = puzzle[i + (row * 9)];
            items[i + 17] = puzzle[col + (i * 9)];
        }

        bool seen[10] = {false};

        for (char item:items) {
            if (item == 0) {
                continue;
            }
            seen[item] = true;
        }
        vector<char> possibilities;
        for (char i = 1; i <= 9; i++) {
            if (!seen[i]) {
                possibilities.push_back(i);
            }
        }
        return possibilities;
    }


    static puzzle solver_pre_init(const array<char, 81> &puzzle) {
        struct puzzle p = {puzzle};
        for (int i = 0; i < 81; ++i) {
            if (puzzle.at(i) == 0) {
                p.blank_positions.push_back(i);
            }
        }

        vector<int> to_remove{};

        for (int item:p.blank_positions) {
            p.possibilities[item] = get_possibilities(p.puzz, item);
            if (p.possibilities[item].size() == 1) {
                p.puzz[item] = p.possibilities[item][0];
                to_remove.push_back(item);
            } else {
                p.current_pos[item] = -1;
            }
        }
        if (to_remove.empty()) {
            p.solved = true;
        } else {
            for (int item : to_remove) {
                remove(p.blank_positions.begin(), p.blank_positions.end(), item);
                p.blank_positions.pop_back();
            }
        }

    return p;
}



        static array<char,81> unwrapped_solve(puzzle &p, const int position) {
            if (position < 0) {
                cerr << "puzzle is unsolvabe";
                p.puzz.fill(0);
                return p.puzz;
            }
            else if (position == p.blank_positions.size()) {
                p.solved = true;
                return p.puzz;
            }

            const int spot = p.blank_positions[position];
            const int max = p.possibilities[spot].size() - 1;
            while (p.current_pos[spot] < max) {
                ++p.current_pos[spot];
                p.puzz[spot] = p.possibilities[spot][p.current_pos[spot]];
                if (position_valid(p.puzz, spot)) {
                    return unwrapped_solve(p, position + 1);
                }
            }

            p.puzz[spot] = 0;
            p.current_pos[spot] = -1;
            return unwrapped_solve(p, position - 1);
        }

        static array<char, 81> solve(array<char, 81> puzz) {
            puzzle p = solver_pre_init(puzz);
            if (p.solved)
                return p.puzz;
            return unwrapped_solve(p, 0);
        }
};

void print_puzzle(const array<char,81> &puzzle) {
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            cout << static_cast<int>(puzzle[i*9+j])
            << "  ";
        }
        cout << "\n";
    }
}

double solve_and_check(const puzzle_reader &puzzles, const int line_number) {
    cout << "Unsolved:\n";
    print_puzzle(puzzles.get_unsolved(line_number));
    auto start = std::chrono::system_clock::now();
    array<char,81> solved = puzzle_solver::solve(puzzles.get_unsolved(line_number));
    auto time = (std::chrono::system_clock::now() - start).count()/1000000.0;
    cout << "\nSolved:\n";
    print_puzzle(solved);
    if (!(solved == puzzles.get_solved(line_number))) {
        cerr << "Solve failed\n";
    }
    return time;
}

bool comp (double a, double b) {
    return a > b;
}

void print_stats (vector<double> &times) {
    double min = times[0];
    double max = times[0];
    double total = 0;
    double median;

    std::ranges::sort(times, comp);

    if (times.size() % 2 == 0) {
        median = (times[times.size()/2] + times[times.size()/2 - 1]) / 2.0;
    } else {
        median = times[times.size()/2];
    }
    for (double const time : times) {
        if (time < min) {
            min = time;
        } else if (time > max) {
            max = time;
        }
        total += time;
    }
    cout << "Solving Stats (" << times.size() << " puzzles):\n";
    cout << "total: " << total << " ms\n"
    << "mean: " << total / times.size() << " ms\n"
    << "median: " << median << "\n"
    << "min: " << min << " ms\n"
    << "max: " << max << " ms\n"
    << "median: " << median << " ms\n";

}

void print_memory_usage() {
    std::ifstream status ("/proc/self/status");
    std::string line;
    std::getline(status, line);
    while (std::getline(status, line)) {
        if (line.substr(0, 6) == "VmRSS:") {
            cout << line << "\n";
        }
    }
}

int main(const int argc, char *argv[]) {
    string filename;
    if (argc != 2) {
        cerr << "Please provide a csv containing the puzzles you want solved.";
    } else {
        filename = argv[1];
    }
    //Increase stack size
    struct rlimit rl;
    rl.rlim_cur = 16 * 1024 * 1024;
    rl.rlim_max = 16 * 1024 * 1024;
    setrlimit(RLIMIT_STACK, &rl);

    auto start = std::chrono::system_clock::now();
    puzzle_reader x = puzzle_reader(filename);
    auto solving_start = std::chrono::system_clock::now();
    double read_time = (std::chrono::system_clock::now() - start).count()/1000000.0;
    vector<double> times {};
    for (int i = 2; i < x.num_puzzles + 2; ++i) {
        cout << "puzzle " << i << ":\n";
        double time = solve_and_check(x, i);
        times.push_back(time);
    }

    cout << "Loaded  " <<  x.num_puzzles << " puzzles from " << filename <<" in " << read_time << " ms" << std::endl;
    print_stats(times);
    cout << "Total solving time including printing overhead: " << (std::chrono::system_clock::now() - solving_start).count() / 1000000.0 << " ms" << std::endl;
    print_memory_usage();

}

