int main() {
  int a = 2;
  if (a) {
    a = a + 1;
  } else a = 0;  // 在实际写 C/C++ 程序的时候别这样, 建议 if 的分支全部带大括号
  return a;
}
