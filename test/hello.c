void init(int arr[][10][10]) {
  arr[1][2][3] = 3;
  // int i = 0;
  // while (i < 10) {
  //   int j = 0;
  //   while (j < 10) {
  //     int k = 0;
  //     while (k < 10) {
  //       arr[i][j][k] = i * 100 + j * 10 + k;
  //       k = k + 1;
  //     }
  //     j = j + 1;
  //   }
  //   i = i + 1;
  // }
}
int main() {
  int arr[10][10][10], sum = 0;
  init(arr);
  putint(sum);
  putch(10);
  return 0;
}
