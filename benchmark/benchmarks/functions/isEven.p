int isEven(int n) {
  int temp = n;
  while (temp >= 2) {
    temp = temp - 2;
  }
  if (temp == 0) {
    return 1;
  } else {
    return 0;
  }
}

print isEven(4);
print isEven(7);

