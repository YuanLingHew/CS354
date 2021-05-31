#include <stdio.h>
#define N 4
int f0(){
  return 5589516;
}

// gets first number
int f1(){
  int ebp4 = 0x61C3;
  int ebp8 = 0xC882;
  int ebpC = 0x107DC;
  int ebp10 = 0x76C4;
  int ebp14 = 0x31;
  int ebp18 = 0x11;

  if (ebp4 > (ebp8 + ebpC)){
    ebp4 = ebp4 >> 4;
    return ebp4;
  }
  else {
    if (ebp10 < (ebp8 * ebp4)){
      ebpC = ebpC % ebp14;
      return ebpC;
    }
    else{
      ebp10 = ebp10 / ebp18;
      return ebp10;
    }
  }
}

// gets second number
int f2(int i){
  int ebp4 = 0;
  int ebp8 = 0;

  while (ebp4 <= 27){
    ebp8 = ebp8 + (ebp4 * ebp4) + i;
    ebp4 = ebp4 + 1;
  }

  return ebp8;
}

// helper to get third number
void f3_helper(int *x){
  int ebp4 = 3;
  *x = *x + ebp4;
}

// gets third number
void f3(int a, int *b){
  f3_helper(&a);
  f3_helper(&a);
  *b += a << 3;
}

// checks if code matches
int Is_Number_Correct(int a, int b, int c){
  if ( a == b ){
    printf ("Number %d: Correct. Nice Job\n", c);
    return 1;
  }
  else{
    printf ("Number %d: Incorrect.\n", c);
    return 0;
  }
}

int main() {
    int i;
    int n[N];
    int a[N];
    int c = 0;

    // get numbers from user
    printf("Enter four numbers: ");
    for (i = 0; i<N; i++) scanf("%d", &n[i]);
    printf("\nYou have entered: %d, %d, %d, %d\n",n[0], n[1], n[2], n[3]);
    
    // get answers
    a[0] = f0(); 
    a[1] = f1();
    a[2] = f2(i);
    a[3] = 4046;  
    f3(a[3], &a[3]);

    // print answers
    // for (i = 0; i< N; i++) printf("answers #%d = %d\n",i,a[i]);

    // test 
    c += Is_Number_Correct(n[0], a[0], 0);
    c += Is_Number_Correct(n[1], a[1], 1);
    c += Is_Number_Correct(n[2], a[2], 2);
    c += Is_Number_Correct(n[3], a[3], 3);

    // report results
    if (c==0) printf("You didn't get any correct numbers. Please try again.\n");
    if (c > 0 && c < N) printf("You got %d correct numbers.  Please try again.\n", c);
    if (c==N) printf("All numbers are correct! Nice work!\n");

    return 0;
}
