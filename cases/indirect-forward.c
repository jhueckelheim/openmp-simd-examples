/// Loop with unknown dependencies due to indirect accesses.
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[P[i]] = gen();
S2:    use(A[Q[i]]);
     }
}

/*
Maximal assumed dependencies:
(a) S1(i) -> S2(i) (flow, intra-body, "lexically forward")
    dependence distance:        (0)
    2d+1 dependence distance: (0,0,1) (lexicographically positive)
    Body-only 2d+1 distance:      (1) (lexicographically positive)

(b) S1(i) -> S2(j) when j > i (flow, loop-carried, "lexically forward")
    dependence distance:        (+)
    2d+1 dependence distance: (0,+,1) (lexicographically positive)
    Body-only 2d+1 distance:      (1) (lexicographically positive)

(c) S2(i) -> S1(j) when j > i (anti, loop-carried, "lexically backward")
    dependence distance:        (+)
    2d+1 dependence distance: (0,+,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (-1) (lexicographically negative)

(d) S1(i) -> S1(j) when j > i (output, loop-carried, "lexically backward")
    dependence distance:        (+)
    2d+1 dependence distance: (0,+,0) (lexicographically positive)
    Body-only 2d+1 distance:      (0) (lexicographically zero)


Statement-wise vectorization (invalid without additional assumptions):
for (int i = 0; i < 2*n; i+=2) {
  load  {P[i],         P[i+1]}       -> staddr
  store {A[staddr[0]], A[staddr[1]]} <- gen()
  load  {Q[i],         Q[i+1]}       -> ldaddr
  load  {A[ldaddr[0]], A[ldaddr[1]]} -> use()
}
// Preserved dependencies: (a), (b), (d)
// Dependencies violated within a simd chunk: (c)

*/
