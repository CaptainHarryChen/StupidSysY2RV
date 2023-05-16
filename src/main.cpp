#include <cassert>
#include <fstream>
#include <unordered_map>

using namespace std;

std::unordered_map<char, const char *> generator = {
    {'k', R"(fun @main(): i32 {
%entry:
  ret 0
}
)"},
    {'r', R"(  .text
  .globl main
main:
  li a0, 0
  ret
)"},
    {'p', R"(  .text
  .globl main
main:
  li a0, 0
  ret
)"},
};

int main(int argc, const char *argv[]) {
  assert(argc == 5);
  ofstream ofs(argv[4]);
  ofs << generator[argv[1][1]];
  return 0;
}
