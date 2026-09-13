#include <iostream>
#include <fstream>
#include <sstream>

#include "../../Talos/include/environment_manager.h"

#include <string>
#include <chrono>



struct ProgramExecutor {
    std::string program;
    bool running;
    std::shared_ptr<EnvironmentManager> env_m;

    ProgramExecutor() {
        std::cout << "\n---------- IDE ----------\n";
        std::cout << "--> Enter help if you are new to this program. \n\n";

        running = true;
        while (running) {
            try {
                handle_input();
            }
            catch (...) {
                std::cout << "--> Catched an exception while capturing input... \n";
            }
        }
    }

    void handle_input() {
        std::cout << ">>> ";
        std::string input;
        std::getline(std::cin, input);

        size_t pos = input.find(' ');
        std::string command = input.substr(0, pos);
        std::string arg;
        if (pos != std::string::npos)
            arg = input.substr(pos + 1);

        if (command == "help") {
            get_help();
        }
        else if (command == "exit") {
            exit();
        }
        else if (command == "load") {
            load(arg);
        }
        else if (command == "run" || input == "execute") {
            execute();
        }
        else if (command == "init") {
            init(parse_expr(arg, { }, { }).second);
        }
        else if (command == "get_reg") {
            get_reg(arg);
        }
        else if (command == "get_ram") {
            get_ram();
        }
    }

    void exit() {
        running = false;
    }
    void load(const std::string& path) {
        std::ifstream file(path);

        if (!file) {
            std::cout << "--> ERROR: Could not open the file... \n";
            return;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();

        program = buffer.str();
    }
    void execute() {
        if (!env_m) {
            std::cout << "--> ERROR: Environment manager not initialized... \n";
            return;
        }

        std::cout << "\n---------- BUILD RESULT ----------\n";
        std::cout << env_m->build({ { "main", program } }) << std::endl;

        auto start = std::chrono::high_resolution_clock::now();
        env_m->start();
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = duration_cast<std::chrono::microseconds>(stop - start);

        std::cout << "\n---------- RUN RESULT ----------\n";
        std::cout << "RUN DURATION: " << duration.count() << " micro seconds" << std::endl;
        std::cout << "EXIT CODE:  " << env_m->exit_code << std::endl;

        std::cout << "\n";
    }
    void init(size_t ram_size) {
        env_m = std::make_shared<EnvironmentManager>(ram_size);
    }
    void get_reg(const std::string& reg_name) {
        if (env_m)
            std::cout << "r" << reg_name << " = " << (int)env_m->get_from_reg(reg_name) << std::endl;
    }
    void get_ram() {
        if (env_m) {
            std::cout << "---------- RAM ---------- \n";
            for (size_t i = 0; i < env_m->mb.ram.size(); i++) {
                std::cout << "i=" << (int)i << "," << (int)env_m->mb.ram[i] << "  ";
                if (i && i % 10 == 0) std::cout << "\n";
            }
            std::cout << "\n";
        }
    }
    void get_help() {
        std::cout << "---------- HELP ---------- \n";
        std::cout << "type \"exit\" to exit. \n";
        std::cout << "type \"load <path>\" to load a program from a text file. \n";
        std::cout << "type \"execute\" to execute the loaded program. \n";
        std::cout << "type \"init <ram_size>\" to initialize the environment. \n";
        std::cout << "type \"get_reg <name>\" to get the content of a register. \n";
        std::cout << "type \"get_ram\" to get the content of the ram. \n";
    }

};



int main() {
    ProgramExecutor();

    return 0;
}