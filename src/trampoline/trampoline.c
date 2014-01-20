#include <stdio.h>
#include <stdlib.h>

void loop1(int loops) {
  int dummy = 1;

  if(loops < 1) { return; }
  printf("loop1(%d): address of dummy : %lu\n", loops, (long)&dummy);
  loops--;

  loop1(loops);
}

// turning loop(int) into a trampolined version:
// 1. change the argument to a pointer, to allow reduction
// 2. return a pointer to the next function to execute (in this case recursive)
void* loop2(int *loops) {
  int dummy = 1;

  if(*loops < 1) { return NULL; }
  printf("loop2(%d): address of dummy : %lu\n", *loops, (long)&dummy);
  *loops -= 1;

  return loop2;
}

// trampoline function wraps the execution of the provided function in a while
// loop, avoiding a growing stack due to recursion.
// to mimic the simple call signature of loop(int), the same int is accepted
// but it's local address is passed ot the trampolined function.
void trampoline(void*(*f)(int*), int arg) {
  int dummy = 1;
  printf("trampoline: address of dummy : %lu\n", (long)&dummy);
  do {
    f = f(&arg);
  } while(f);
}

#ifndef __clang__
// GCC-only (clang doesn't support it): nested functions that reuse their 
// parents' stack frame
// source: http://www.technovelty.org/c/gcc-trampolines.html
void loop3(int loops) {
  int dummy = 1;
  printf("trampoline: address of dummy : %lu\n", (long)&dummy);

  void* trampolined_loop3(void) {
    int dummy = 1;

    if(loops < 1) { return NULL; }
    printf("loop3(%d): address of dummy : %lu\n", loops, (long)&dummy);
    loops--;

    return trampolined_loop3;
  }
  
  void* (*f)(void) = trampolined_loop3;
  do {
    f = f();
  } while(f);
}
#endif

// comparing the addresses of the dummy variable when called recursively or
// trampolined.
int main(void) {
  printf("recursive\n");
  loop1(5);

  printf("trampolined\n");
  trampoline(loop2, 5);

#ifndef __clang__
  printf("nested functions trampoline\n");
  loop3(5);
#endif
  
  exit(EXIT_SUCCESS);
}

/*

example output:

$ /opt/local/bin/gcc-mp-4.7 trampoline.c 
$ ./a.out 
recursive
loop1(5): address of dummy : 140734531599244
loop1(4): address of dummy : 140734531599196
loop1(3): address of dummy : 140734531599148
loop1(2): address of dummy : 140734531599100
loop1(1): address of dummy : 140734531599052
trampolined
trampoline: address of dummy : 140734531599244
loop2(5): address of dummy : 140734531599196
loop2(4): address of dummy : 140734531599196
loop2(3): address of dummy : 140734531599196
loop2(2): address of dummy : 140734531599196
loop2(1): address of dummy : 140734531599196
nested functions trampoline
trampoline: address of dummy : 140734531599236
loop3(5): address of dummy : 140734531599148
loop3(4): address of dummy : 140734531599148
loop3(3): address of dummy : 140734531599148
loop3(2): address of dummy : 140734531599148
loop3(1): address of dummy : 140734531599148

*/