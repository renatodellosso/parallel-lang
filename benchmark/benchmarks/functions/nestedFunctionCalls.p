int multiplyByTwo(int x) {
  return x * 2;
}

int add(int a, int b) {
  return a + b;
}

int result = add(multiplyByTwo(3), multiplyByTwo(4));
print result;
