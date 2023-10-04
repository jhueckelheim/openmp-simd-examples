/// Loop with "lexically-backward" loop-carried dependency, non-dominating gen() and use() through conditional
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       if (i%2 == 1) {
S1:      use(A[i]);
       } else {
S2:      A[i+1] = gen();
       }
     }
}

/*
Dependencies:
  S2(i) -> S1(i+1) when i%2==1 (flow, loop-carried)
    dependence distance:        (1)
    2d+1 dependence distance: (0,1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (-1) (lexicographically negative)


Statement-wise vectorization (invalid):
for (int i = 0; i < 2*n; i+=2) {
  ifmask = {i%2==1, (i+1)%2==1}
  load<ifmask> (A[i], A[i+1]) -> use()
  elsemask = {!ifmask[0], !ifmask[1]}
  store<elsemask> (A[i+1], A[i+2]) <- gen()
}
// A[i+1] loads the value from before gen()


Auto-"vectorization":
for (int i = 0; i < 2*n; i+=2) {
  use(A[i+1]);
  A[i+1] = gen();
}


Statement-wise vectorization possible:
  no

Auto-vectorization possible:
  yes

Has loop-carried dependencies:
  yes

Requires lexical order information to vectorize:
  no

Has defined behaviour according to OpenMP 5.2:
  ?

Should be supported by OpenMP:
  ?

*/
