/// Loop with simple loop-carried dependency
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[i+1] = gen();
S2:    use(A[i]);
     }
}

/*
Dependencies:
  S1(i) -> S2(i+1) (flow, loop-carried, "lexically forward")
      dependence distance:        (1)
      2d+1 dependence distance: (0,1,1)
      Body-only 2d+1 distance:      (1) (lexicographically positive)


// Statement-wise vectorization (valid):
for (int i = 0; i < 2*n; i+=2) {
  store {A[i+1], A[i+2]} <- gen()
  load  {A[i],   A[i+1]} -> use()
}


Statement-wise vectorization possible:
  yes

Auto-vectorization possible:
  yes

Has loop-carried dependencies:
  yes

Has "lexically forward"-dependencies:
  yes

Has loop-carried non-"lexically forward"-dependencies:
  yes

Requires lexical order information to vectorize:
  no, order already determined by C/C++ semantics

Has defined behaviour according to OpenMP 5.2:
  probably yes

Should be supported by OpenMP:
  ?

*/
