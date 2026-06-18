int factorial(int n) {
  int result = 1;
  while (n > 1) {
    result = result * n;
    n = n - 1;
  }
  return result;
}

print factorial(5);
