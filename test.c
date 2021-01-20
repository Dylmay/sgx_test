#include <stdio.h>
#include <time.h>

#define tick clock();
#define tock(start_clock) measure_ticks(start_clock, clock());

double measure_ticks(clock_t, clock_t);

int main() {

  clock_t start = tick;

  printf("Hello world\n");
  for (long i = 0; i < 999999999; i++);

  float end = tock(start);

  printf("clock cycles taken by CPU: %f\n", end);
  printf("time taken by CPU: %f\n", end / CLOCKS_PER_SEC);

  return 0;
}

double measure_ticks(clock_t start, clock_t end) {
  return (double) (end - start);
}
