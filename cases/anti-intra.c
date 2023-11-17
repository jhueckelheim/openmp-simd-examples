/// Loop with simple anti-dependency
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    use(A[i]);
S2:    A[i] = gen();
     }
}

/*
Dependencies:
  S1(s) -> S2(s) (anti, scalar, intra-body, "lexically forward")
    dependence distance:        (0)
    2d+1 dependence distance: (0,0,1) (lexicographically positive)
    Body-only 2d+1 distance:      (1) (lexicographically positive)


Possible vectorization:
for (int i = 0; i < 2*n; i+=2) {
  load (A[i], A[i+1]) -> use()
  store (A[i], A[i+1]) <- gen()
}


Statement-wise vectorization possible:
  yes

Auto-vectorization possible:
  yes

Has loop-carried dependencies:
  no

Has "lexically forward"-dependencies:
  yes

Has loop-carried non-"lexically forward"-dependencies:
  no

Requires lexical order information to vectorize:
  no, order already determined by C/C++ semantics

Has defined behaviour according to OpenMP 5.2:
  yes

Should be supported by OpenMP:
  yes
*/
