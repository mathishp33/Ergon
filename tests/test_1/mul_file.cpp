#include <iostream>

#include "../../Talos/include/virtual_machine/environment_manager.h"

#include <string>
#include <chrono>

int main() {
    std::string main = R"(
.section .text
  .extern convert_digit
  .extern scan
  .extern print
  .global main
  main:
    leab r1, buffer
    ldw r2, buffer_size
    call scan
    ldb r5, buffer_size
    call convert_digit
    call print

    movi r0, 0
    syscall
    halt

  .entry main

.section .rodata
  buffer:
    .byte 1
.section .data
  buffer_size:
    .word 1
)";

    std::string module = R"(
.section .text
  .global convert_digit
  convert_digit:
    subi r5, r5, 48
    ret
  .global scan
  scan:
    movi r0, 2
    syscall
    ret
  .global print
  print:
    movi r0, 1
    syscall
    ret
)";

    auto env_m = EnvironmentManager(0XFFFF);

    std::cout << "\n---------- BUILD RESULT ----------\n";
    std::vector<std::pair<std::string, std::string>> files = { { "main", main }, { "module", module } };
    std::cout << env_m.build(files) << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    env_m.start();
    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::microseconds>(stop - start);

    std::cout << "\n---------- RUN RESULT ----------\n";
    std::cout << "RUN DURATION: " << duration.count() << " micro_sec" << std::endl;
    std::cout << "EXIT CODE: " << env_m.exit_code << std::endl;

    return 0;
}