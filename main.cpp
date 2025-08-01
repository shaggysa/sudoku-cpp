#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <vector>
#include <array>
#include <chrono>
#include <ctime>
#include <bits/stdc++.h>
#include <sys/resource.h>

namespace fs = std::filesystem;

using std::string, std::cout, std::cerr, std::ifstream, std::vector, std::stringstream, std::array;

struct puzzle {
    array <char,81> puzz;
    vector<int> blank_positions;
    vector<vector<char>> possibilities;
    vector<int> current_pos;
    vector<array<bool,10>> cached_possibilities;
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

                for (int i = 0; i < 81; i++) {
                    unsolved[i] = line[i] - '0';
                    solved[i] = line[i+82] - '0'; //skip the comma separator

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
    static vector<char> get_possbililities(const array<char,81> &puzzle, const int position) {
        array<bool,10> seen;
        seen.fill(false);
        vector<char> possibilities;
        const int row = position / 9;
        const int col = position % 9;

        const int root_row = (row/3) * 3;
        const int root_col = (col/3) * 3;
        for (int r = root_row; r < root_row + 3; r++) {
            for (int c = root_col; c < root_col + 3; c++) {
                seen[puzzle[r*9+c]] = true;
            }
        }

        for (int i = 0; i < 9; i++) {
            seen[puzzle[i + (row * 9)]] = true;
            seen[puzzle[col + (i * 9)]] = true;
        }

        for (int i = 1; i < 10; i++) {
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

        while (1) {
            vector<int> to_remove{};
            for (int item:p.blank_positions) {
                vector<char> possibilities = get_possbililities(p.puzz, item);
                if (possibilities.size() == 1) {
                    p.puzz[item] = possibilities[0];
                    to_remove.push_back(item);
                }
            }

            if (to_remove.size() == p.blank_positions.size()) {
                p.solved = true;
                return p;
            } if (to_remove.empty()) {
                for (int i = 0; i < p.blank_positions.size(); i++) {
                    p.possibilities.push_back(get_possbililities(p.puzz, p.blank_positions[i]));
                    p.current_pos.push_back(-1);
                }
                return p;
            }
            for (int item : to_remove) {
                remove(p.blank_positions.begin(), p.blank_positions.end(), item);
                p.blank_positions.pop_back();
            }
        }
    }

    static array<bool,10> get_possbililities_as_array(const array<char,81> &puzzle, const int position) {
        array<bool,10> seen{};
        seen.fill(true);
        const int row = position / 9;
        const int col = position % 9;

        const int root_row = (row/3) * 3;
        const int root_col = (col/3) * 3;
        for (int r = root_row; r < root_row + 3; r++) {
            for (int c = root_col; c < root_col + 3; c++) {
                seen[puzzle[r*9+c]] = false;
            }
        }

        for (int i = 0; i < 9; i++) {
            seen[puzzle[i + (row * 9)]] = false;
            seen[puzzle[col + (i * 9)]] = false;
        }

        return seen;
    }

    static array<char,81> solve(const array<char, 81> &puzz) {
        puzzle p = solver_pre_init(puzz);
        if (p.solved) {
            return p.puzz;
        }

        int position = 0;
        int max_len = p.blank_positions.size();

        bool progressed_forward = true;

        while (position < max_len) {
            bool found = false;
            if (position < 0) {
                cerr << "puzzle is unsolvabe";
                p.puzz.fill(0);
                return p.puzz;
            }
            const int spot = p.blank_positions[position];
            const int max = p.possibilities[position].size() - 1;
            if (progressed_forward) {
                if (position == p.cached_possibilities.size()) {
                    p.cached_possibilities.push_back(get_possbililities_as_array(p.puzz, spot));
                } else {
                    p.cached_possibilities[position] = get_possbililities_as_array(p.puzz, spot);
                }
            }

            while (p.current_pos[position] < max) {
                ++p.current_pos[position];
                if (p.cached_possibilities[position][p.possibilities[position][p.current_pos[position]]]) {
                    p.puzz[spot] = p.possibilities[position][p.current_pos[position]];
                    found = true;
                    ++position;
                    progressed_forward = true;
                    break;
                }
            }
            if (!found) {
                p.puzz[spot] = 0;
                p.current_pos[position] = -1;
                --position;
                progressed_forward = false;
            }

        }
        p.solved = true;
        return p.puzz;
    }


    static void print_puzzle(const array<char,81> &puzzle) {
        for (int i = 0; i < 9; i++) {
            for (int j = 0; j < 9; j++) {
                cout << static_cast<int>(puzzle[i*9+j])
                << "  ";
            }
            cout << "\n";
        }
    }

    static double solve_and_check(const puzzle_reader &puzzles, const int line_number) {
        cout << "Unsolved:\n";
        print_puzzle(puzzles.get_unsolved(line_number));
        auto start = std::chrono::system_clock::now();
        array<char,81> solved = puzzle_solver::solve(puzzles.get_unsolved(line_number));
        auto time = (std::chrono::system_clock::now() - start).count()/1000000.0;
        cout << "\nSolved:\n";
        print_puzzle(solved);
        if (!(solved == puzzles.get_solved(line_number))) {
            cerr << "Solve failed\n";
            throw::std::runtime_error("Solve failed");
        }
        return time;
    }
};
    bool comp (double a, double b) {
        return a > b;
    }

    void print_time(double time) {
        int seconds = time/1000;
        time -= seconds*1000;
        int minutes = seconds/60;
        seconds -= minutes*60;
        cout << minutes << " minutes, " << seconds << " seconds, " << time << " milliseconds\n";
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
        cout << "total: ";
        print_time(total);
        cout << "mean: " << total / times.size() << " ms\n"
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

        auto start = std::chrono::system_clock::now();
        puzzle_reader x = puzzle_reader(filename);
        auto solving_start = std::chrono::system_clock::now();
        double read_time = (std::chrono::system_clock::now() - start).count()/1000000.0;
        vector<double> times {};
        for (int i = 2; i < x.num_puzzles + 2; ++i) {
            cout << "line " << i << ":\n";
            double time = puzzle_solver::solve_and_check(x, i);
            times.push_back(time);
        }

        cout << "Loaded  " <<  x.num_puzzles << " puzzles from " << filename <<" in ";
        print_time(read_time);

        print_stats(times);
        cout << "Total solving time including printing overhead: " << (std::chrono::system_clock::now() - solving_start).count() / 1000000.0 << " ms" << std::endl;
        print_memory_usage();

    }


