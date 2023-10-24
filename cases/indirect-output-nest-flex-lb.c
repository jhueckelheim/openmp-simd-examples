/// Loop with a statement that a naive front-end vectorizer would match up differently.
/// Difference caused by dependent loop lower bound.
void func(int P[restrict][2], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2; ++i) {
       for (int j = i; j < 2; ++j) {
S:       A[P[i][j]] = gen();
       }
     }
}

/*
Dependencies:
(a) S(0,0) -> S(0,1) over A[k] if k==P[0][0]==P[0][1] (output, intra-body, by definition "lexically backwards" but really "lexically identical")
      dependence distance:        (0)
      2d+1 dependence distance: (0,0,0,1) (lexicographically positive)
      Body-only 2d+1 distance:      (0,1) (lexicographically positive)

(b) S(0,0) -> S(1,0) over A[k] if k==P[0][0]==P[1][1] (output, loop-carried, by definition "lexically backwards" but really "lexically identical")
      dependence distance:        (1)
      2d+1 dependence distance: (0,1,0,0) (lexicographically positive)
      Body-only 2d+1 distance:      (0,0) (lexicographically zero)

(c) S(0,1) -> S(1,0) over A[k] if k==P[0][1]==P[1][1] (output, loop-carried, by definition "lexically backwards" but really "lexically identical")
      dependence distance:        (1)
      2d+1 dependence distance: (0,1,0,-1) (lexicographically positive)
      Body-only 2d+1 distance:      (0,-1) (lexicographically negative)


Statement-wise vectorization:
for (int i = 0; i < 2; i+=2) { // i = 0
  for (int j = i; j < 2; ++j) { // j = 0,1
    set              boundmask                  <- {i < 2, i+1 < 2}       
    store<boundmask> {A[P[i][j]], A[P[i+1][j]]} <- gen()
  }
}

Unrolled:
store<1,1> {A[P[0][0]], A[P[1][1]]} <- gen()
store<1,0> {A[P[0][1]], A[P[1][2]]} <- gen()

Dependency (c) NOT preserved; write to A[1][1] occurs before A[0][1].
Dependency (b) only preserved if the vector ISA store preserves them.

*/
