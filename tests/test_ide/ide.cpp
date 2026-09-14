#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

#include "../../Talos/include/environment_manager.h"

#include <string>
#include <chrono>

namespace fs = std::filesystem;

struct ProgramExecutor {
    std::string program;
    std::vector<std::pair<std::string, std::string>> programs;
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

        if (input.empty()) {
            std::cout << "\n";
            return;
        }
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
        else if (command == "build") {
            build(true);
        }
        else if (command == "builds") {
            build(false);
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
        else if (command == "info") {
            get_info(true);
        }
        else if (command == "infos") {
            get_info(false);
        }
        else if (command == "load_dir") {
            load_dir(arg);
        }
        else {
            std::cout << "--> ERROR: unknown command. \n";
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
        std::cout << "--> Program loaded successfully. \n";
    }
    void build(bool single_file) {
        if (!env_m) {
            std::cout << "--> ERROR: Environment manager not initialized... \n";
            return;
        }

        std::cout << "\n---------- BUILD RESULT ----------\n";
        if (single_file)
            std::cout << env_m->build_single(program) << std::endl;
        else
            std::cout << env_m->build(programs) << std::endl;

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
        std::cout << "--> Environment created successfully. \n";
    }
    void get_reg(const std::string& reg_name) {
        if (env_m)
            std::cout << reg_name << " = " << (int)env_m->get_from_reg(reg_name) << std::endl;
        else
            std::cout << "--> ERROR: Environment not initialized. \n";
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
        else
            std::cout << "--> ERROR: Environment not initialized. \n";
    }
    void get_info(bool single_file) {
        if (env_m) {
            std::cout << "\n---------- BUILD RESULT ----------\n";
            std::cout << env_m->build({ { "main", program } }) << std::endl;
            std::cout << "---------- PROGRAM ----------\n";

            std::vector<ObjectFile> obj_files = { };
            if (single_file) {
                auto [obj_file, error_info] = env_m->decoder.decode(program);
                if (error_info.code != ErrorCode::OK) std::cout << env_m->handle_error("main", error_info) << "\n";
                obj_files.emplace_back(obj_file);
            }
            else {
                for (const auto& [name, file] : programs) {
                    auto [obj_file, error_info] = env_m->decoder.decode(file);
                    if (error_info.code != ErrorCode::OK) std::cout << env_m->handle_error(name, error_info) << "\n";
                    obj_files.emplace_back(obj_file);
                }
            }

            auto [e, linked_bin] = link(obj_files);
            if (e.code != ErrorCode::OK) std::cout << env_m->handle_error("linked binary", e) << "\n";

            env_m->mb.reset();
            if (env_m->load_ram(linked_bin.data, linked_bin.rodata) != ErrorCode::OK)
                std::cout << env_m->handle_error("linked binary", ErrorInfo(ErrorCode::RAM_OVERFLOW, 0)) << "\n";

            std::cout << "Entry PC is: " << (int)linked_bin.entry_pc << "\n";
            for (int i = 0; i < linked_bin.text.size(); i++) {
                DecodedInstr instr = linked_bin.text[i];
                std::cout << "PC=" << (int)i << ", instr {" << "OpCode=" << (int)instr.opcode <<", rd=" << (int)instr.rd
                << ", rs1=" << (int)instr.rs1 << ", rs2=" << (int)instr.rs2 << ", imm=" << (int)instr.imm << "} \n";
            }
        }
        else
            std::cout << "--> ERROR: Environment not initialized. \n";
    }
    void load_dir(const std::string& dir_path) {
        programs.clear();

        for (const auto& entry : fs::directory_iterator(dir_path)) {
            if (!entry.is_regular_file())
                continue;

            std::ifstream file(entry.path());

            if (!file)
                continue;

            std::stringstream buffer;
            buffer << file.rdbuf();

            programs.emplace_back(entry.path().filename().string(), buffer.str());
        }

        std::cout << "--> Programs loaded successfully. \n";
    }
    static void get_help() {
        std::cout << "---------- HELP ---------- \n";
        std::cout << "type \"exit\" to exit. \n";
        std::cout << "type \"load <path>\" to load a program from a text file. \n";
        std::cout << "type \"build\" to build the single loaded program. \n";
        std::cout << "type \"builds\" to build the loaded programs. \n";
        std::cout << "type \"init <ram_size>\" to initialize the environment. \n";
        std::cout << "type \"get_reg <name>\" to get the content of a register. \n";
        std::cout << "type \"get_ram\" to get the content of the ram. \n";
        std::cout << "type \"info\" to get all infos about the single program. \n";
        std::cout << "type \"infos\" to get all infos about the programs. \n";
        std::cout << "type \"load_dir\" to load multiple files from a directory. \n";
    }

};



int main() {
    ProgramExecutor p_e;

    return 0;
}