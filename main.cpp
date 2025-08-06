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
        vector<array<char, 81>> unsolved_puzzles;
        vector<array<char, 81>> solved_puzzles;
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
                    break;
                }
            }
            if (!found) {
                p.puzz[spot] = 0;
                p.current_pos[position] = -1;
                --position;
                progressed_forward = false;
            } else {
                progressed_forward = true;
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

    static double solve_and_check(const array<char,81> &puzz, const array<char,81> &solved) {
        auto start = std::chrono::system_clock::now();
        array<char,81> solved_puzz = solve(puzz);
        auto time = (std::chrono::system_clock::now() - start).count()/1000000.0;
        if (solved_puzz != solved) {
            cerr << "solved puzzles do not match!\n";
            exit(-1);
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

    void speedtest_sequential(const puzzle_reader& reader) {
        cout << "Starting Sequential Speedtest:\n---------------\n";
        auto solving_start = std::chrono::system_clock::now();
        vector<double> times {};
        for (int i = 0; i < reader.num_puzzles; i++) {
            double time = puzzle_solver::solve_and_check(reader.get_unsolved(i+2), reader.get_solved(i+2));
            times.push_back(time);
        }

        print_stats(times);
        cout << "Total time elapsed: \n";
        print_time((std::chrono::system_clock::now() - solving_start).count() / 1000000.0);
        print_memory_usage();
    }

    void speedtest_async(puzzle_reader &reader) {
        cout << "Starting Async Speedtest:\n---------------\n";
        auto solving_start = std::chrono::system_clock::now();
        size_t num_threads = std::thread::hardware_concurrency();
        size_t chunk_size = reader.num_puzzles/num_threads;
        vector<double> times(reader.num_puzzles);
        vector<std::thread> threads;

        for (size_t t = 0; t < num_threads; ++t) {
            size_t start = t * chunk_size;
            size_t end = (t == num_threads - 1) ? reader.num_puzzles : start + chunk_size;

            threads.emplace_back([&,start,end]() {
               for (size_t i = start; i < end; i++) {
                   times[i] = puzzle_solver::solve_and_check(reader.get_unsolved(i+2), reader.get_solved(i+2));
               }
            });
        }
            for (auto& thread : threads) {
                thread.join();
            }


        print_stats(times);
        cout << "Total time elapsed: \n";
        print_time((std::chrono::system_clock::now() - solving_start).count() / 1000000.0);
        print_memory_usage();
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
        double read_time = (std::chrono::system_clock::now() - start).count()/1000000.0;
        cout << "Loaded  " <<  x.num_puzzles << " puzzles from " << filename <<" in ";
        print_time(read_time);
        speedtest_async(x);
        speedtest_sequential(x);
    }


