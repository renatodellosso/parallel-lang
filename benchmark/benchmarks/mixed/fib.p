int n = 10;
int a = 0;
int b = 1;
int count = 0;
while (count < n) {
  int temp = a + b;
  a = b;
  b = temp;
  count = count + 1;
}
print b;
