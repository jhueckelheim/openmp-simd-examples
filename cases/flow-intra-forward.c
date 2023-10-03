/// Loop without loop-carried dependencies
void func(int n, double A[restrict], double B[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S:    float a = A[i];
T:    B[i] = a;
     }
}

/*
Dependencies:
  S(i) -> T(i) (flow, intra-body, "lexically forward")
    dependence distance:      (0)
    2d+1 dependence distance: (0,0,1)

Possible vectorization:
for (int i = 0; i < 2*n; i+=2) {
    load (A[i], A[i+1]) -> a
    store (B[i], B[i+1]) <- a
}


Statement-wise vectorization possible:
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
