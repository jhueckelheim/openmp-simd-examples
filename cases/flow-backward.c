/// Loop with "lexically-backward" loop-carried dependency
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    use(A[i]);
S2:    A[i+1] = gen();
     }
}

/*
Dependencies:
  S2(s) -> S1(s+1) (flow, loop-carried, "lexically backward")
    dependence distance:        (1)
    2d+1 dependence distance: (0,1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (-1) (lexicographically negative)


Statement-wise vectorization (invalid):
for (int i = 0; i < 2*n; i+=2) {
  load  {A[i],   A[i+1]} -> use()
  store {A[i+1], A[i+2]} <- gen()
}
// A[i+1] loads the value from before gen(), only valid for A[0]


Valid vectorization:
for (int i = 0; i < 2*n; i+=2) {
  load (A[i], A[i+1]) -> use()
  store (A[i+1], A[i+2]) <- gen()
}


Statement-wise vectorization possible:
  no

Auto-vectorization possible:
  yes

Has loop-carried dependencies:
  yes

Has "lexically forward"-dependencies:
  no

Has loop-carried non-"lexically forward"-dependencies:
  yes

Requires lexical order information to vectorize:
  n/a

Has defined behaviour according to OpenMP 5.2:
  probably no

Should be supported by OpenMP:
  ?

*/
