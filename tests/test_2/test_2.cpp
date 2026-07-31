#include <iostream>

#include "../../Talos/include/environment_manager.h"

#include <string>
#include <chrono>


void stof_test(const std::string& s) {
    std::cout << "test_2: \" " << s << " \" " << std::stof(s) << " | " << string_utils::better_stof(s).second << "\n";
}

void stoi_test(const std::string& s) {
    std::cout << "test_2: \" " << s << " \" " << std::stoi(s) << " | " << string_utils::better_stoi(s).second << "\n";
}

void parser_test(const std::string& s, bool isFloat = false ) {
    std::unordered_map<std::string, Value> c;
    std::unordered_map<std::string, Value> v;
    auto [e, res] = parse_expr(s, c, v);
    std::cout << "\" " << s << " \"" << " | " << std::to_string(isFloat ? std::bit_cast<float>(res) : (float)res) << " | error message: " << e.message << "\n";
}

int main() {

    std::cout << "\n" << "--- Test 2 ---" << "\n";

    std::cout << "\n" << "  stof test: " << "\n";
    stof_test("0");
    stof_test("1.0");
    stof_test("2.24");
    stof_test("99999999999999999999999");

    std::cout << "\n" << "  stoi test: " << "\n";
    stof_test("0");
    stof_test("0x55");
    stof_test("0b01001");
    stof_test("4242");

    std::cout << "\n" << "parser test: " << "\n";
    parser_test("3*3");
    parser_test("3 % 3");
    parser_test("3.6f % 3.5");
    parser_test("3 * 5 + (6 % 4) / 2");
    parser_test("3 > 5 * 2");
    parser_test("3 == 3 * 3 / 3");
    parser_test("3 / 0");
    parser_test("3 % 0");
    parser_test("3.5 >> 5.6");
    parser_test("3.6f > 2");
    parser_test("3.5f == 2");
    parser_test("2.0 == 2");

    parser_test("2 | 3 & 1");        // 3
    parser_test("2 ^ 1 | 4");        // 7
    parser_test("1 << 2 + 1");       // 8
    parser_test("2 ** 3 ** 2");      // 512.0
    parser_test("1 < 2 < 3");        // 1
    parser_test("(1+2)*3");          // 9

    parser_test("-2 ** 2");          // 4.0 (your grammar: (-2)**2, not -(2**2))
    parser_test("~-5");              // 4
    parser_test("--5");              // 5

    parser_test("7/2");              // 3 (int)
    parser_test("7/2.0");            // 3.5 (float)
    parser_test("7%3");              // 1
    parser_test("5.5 % 2");          // 1.5
    parser_test("3 == 3.0");         // 1
    parser_test("3 == 3.5");         // 0

    parser_test("5/0");              // error: int div by zero
    parser_test("5/0.5");            // 10.0
    parser_test("5%0");              // error: mod by zero

    parser_test("3.0 & 1");          // error: bitwise requires int
    parser_test("3 & 1.5");          // error: bitwise requires int
    parser_test("-8 >> 1");          // -4

    //parser_test("x + y");            // 7.5
    //parser_test("x * y");            // 12.5
    parser_test("z + 1");            // error: unknown variable

    parser_test("(1 + 2");           // error: expected ')'
    parser_test("1 + + 2");          //
    parser_test("");                 //


    return 0;
}