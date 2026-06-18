int a = 48;
int b = 18;
while (b != 0) {
  while (a >= b) {
    a = a - b;
  }
  int temp = a;
  a = b;
  b = temp;
}
print a;
