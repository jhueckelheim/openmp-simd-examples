/// Same as flow-carried-backward, but inversed statement order by nesting into another loop
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < 2; ++j) {
S1:      if (j == 1) A[i+1] = gen();
S2:      if (j == 0) use(A[i]);
       }
     }
}

/*
Dependencies:
  S1(i,1) -> S2(i+1,0) (flow, loop-carried, "lexically forward")
    dependence distance:      (1)
    2d+1 dependence distance: (0,1,0,-1,1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,-1,1) (lexicographically negative)


Statement-wise vectorization (invalid):
for (int i = 0; i < 2*n; i+=2) {
  for (int j = 0; j < 2; ++j) {
    if (j == 1) store (A[i+1], A[i+2]) <- gen()
    if (j == 0) load (A[i], A[i+1]) -> use()
  }
}
// A[i+1] loads the value from before the function call, only valid for A[0]


Valid vectorization (same as flow-carried-backward):
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
  yes

Has loop-carried non-"lexically forward"-dependencies:
  no

Requires lexical order information to vectorize:
  n/a

Has defined behaviour according to OpenMP 5.2:
  yes (letter-of-the-spec) and no (spirit-of-the-spec)

Should be supported by OpenMP:
  ?

*/
