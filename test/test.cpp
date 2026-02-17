#include <iostream>

#include "../Talos/include/env_manager.h"

#include <string>
#include <chrono>

int main() {
    std::cout << "---------- TEST PROGRAM ----------\n";
    std::string program = "test_var db 0      ; variable RAM\n"
                          "rmax dd 2_000_000_000   ; nombre d'itérations (à augmenter pour tester la vitesse)\n"
                          ""
                          "movi r0 0      ; compteur = 0\n"
                          "movi r1 0      ; valeur temporaire = 0\n"
                          "movi r2 0      ; résultat final = 0\n"
                          "ldb r3 test_var ; charger valeur initiale\n"
                          "loop_start:\n"
                          " addi r0 r0 1    ; incrémenter compteur\n"
                          " addi r1 r1 2    ; incrémenter valeur temporaire\n"
                          " add r2 r2 r1    ; accumuler résultat\n"
                          " ldb r3 test_var ; relire la RAM\n"
                          " addi r3 r3 1    ; incrémenter valeur\n"
                          " stb r3 test_var ; écrire dans la RAM\n"
                          " ldw r3 rmax     ; \n"
                          " cmpu r0 r3     ; comparer compteur avec rmax\n"
                          " jl loop_start   ; si compteur < rmax, revenir au début\n"
                          ""
                          "halt\n";

    std::cout << program << std::endl;

    auto env_m = EnvironmentManager<0x10000, 0xFFF>();

    env_m.build(program);

    std::cout << "\n---------- BUILD RESULT ----------\n";

    std::cout << "ERROR: " << env_m.get_error_msg() << std::endl;

    //running the program
    env_m.set_mod(RunMode::AUTO);

    auto start = std::chrono::high_resolution_clock::now();
    env_m.start();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "\n---------- RUN RESULT ----------\n";
    std::cout << "RUN DURATION: " << duration.count() << " micro_sec" << std::endl;
    return 0;
}