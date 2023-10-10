/// Loop where counter and logical iteration number are different.
/// This should still be a simple loop-carried dependency
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 2*n-1; i >= 0; --i) {
S1:    A[i-1] = gen();
S2:    use(A[i]);
     }
}

/*
// Statement-wise vectorization (valid):
for (int i = 2*n-1; i >= 0; i-=2) {
  store {A[i-1], A[i-2]} <- gen()
  load  {A[i],   A[i-1]} -> use()
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
