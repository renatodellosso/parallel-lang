int power(int base, int exp) {
  int result = 1;
  int count = 0;
  while (count < exp) {
    result = result * base;
    count = count + 1;
  }
  return result;
}

print power(2, 8);
