int num = 17;
int i = 2;
int isPrime = 1;
while (i * i <= num) {
  int temp = num;
  while (temp >= i) {
    temp = temp - i;
  }
  if (temp == 0) {
    isPrime = 0;
  }
  i = i + 1;
}
print isPrime;
