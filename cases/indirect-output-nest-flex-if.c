/// Loop with a statement that a naive front-end vectorizer would match up differently.
/// Difference caused by conditional statement.
/// An implementatation could just ensure that the last store survives, but we consider every store even if overwritten as significant.
void func(int P[restrict][2], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2; ++i) {
       for (int j = 0; j < 2; ++j) {
S:       if (j >= i) A[P[i][j]] = gen();
       }
     }
}

/*
Dependencies:
(a) S(0,0) -> S(0,1) over A[k] if k==P[0][0]==P[0][1] (output, intra-body, by definition "lexically backwards" but really "lexically identical")
      dependence distance:        (0)
      2d+1 dependence distance: (0,0,0,1) (lexicographically positive)
      Body-only 2d+1 distance:      (0,1) (lexicographically positive)

(b) S(0,0) -> S(1,1) over A[k] if k==P[0][0]==P[1][1] (output, loop-carried, by definition "lexically backwards" but really "lexically identical")
      dependence distance:        (1)
      2d+1 dependence distance: (0,1,0,1) (lexicographically positive)
      Body-only 2d+1 distance:      (0,1) (lexicographically positive)

(c) S(0,1) -> S(1,1) over A[k] if k==P[0][1]==P[1][1] (output, loop-carried, by definition "lexically backwards" but really "lexically identical")
      dependence distance:        (1)
      2d+1 dependence distance: (0,1,0,0) (lexicographically positive)
      Body-only 2d+1 distance:      (0,0) (lexicographically zero)


Statement-wise vectorization:
for (int i = 0; i < 2; i+=2) { // i = 0
  for (int j = 0; j < 2; ++j) { // j = 0,1
    set           ifmask                     <- {j >= i, j >= i+1}       
    store<ifmask> {A[P[i][j]], A[P[i+1][j]]} <- gen()
  }
}


Unrolled:
store<1,0> {A[P[0][0]], A[P[1][0]]} <- gen()
store<1,1> {A[P[0][1]], A[P[1][1]]} <- gen()

Dependency (c) only preserved if the vector ISA store preserves them.

*/
