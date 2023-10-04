/// Loop with unknown dependencies due to indirect accesses, execution order selected by runtime variable.
/// Equivalently, imagine `dir` changing the direction of the j-loop to counting downwards.
void func(int n, int dir, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < 2; ++j) {
S1:      if (j==!!dir) A[P[i]] = gen();
S2:      if (j==!dir)  use(A[Q[i]]);
       }
     }
}

/*
Maximal assumed dependencies:
(a1) S1(i,0) -> S2(i,1) when dir==0 (flow, intra-body)
    dependence distance:        (0,  1)
    2d+1 dependence distance: (0,0,0,1,1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,1,1) (lexicographically positive)

(a2) S2(i,1) -> S1(i,0) when dir!=0 (anti, intra-body)
    dependence distance:        (0,  1)
    2d+1 dependence distance: (0,0,0,1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,1,-1) (lexicographically positive)

(b1) S1(i,0) -> S2(j,1) when j > i and dir==0 (flow, loop-carried)
    dependence distance:        (+,  1)
    2d+1 dependence distance: (0,+,0,1,1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,1,1) (lexicographically positive)

(b2) S2(i,0) -> S1(j,1) when j > i and dir!=0 (anti, loop-carried)
    dependence distance:        (+,  1)
    2d+1 dependence distance: (0,+,0,1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,1,-1) (lexicographically positive)

(c1) S2(i,1) -> S1(j,0) when j > i and dir==0 (anti, loop-carried)
    dependence distance:        (+,  -1)
    2d+1 dependence distance: (0,+,0,-1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,-1,-1) (lexicographically negative)

(c2) S1(i,1) -> S2(j,0) when j > i and dir!=0 (flow, loop-carried)
    dependence distance:        (+,  -1)
    2d+1 dependence distance: (0,+,0,-1,1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,-1,1) (lexicographically negative)

(d1) S2(i,0) -> S2(j,0) when j > i and dir==0 (output, loop-carried)
    dependence distance:        (+,  0)
    2d+1 dependence distance: (0,+,0,0,0) (lexicographically positive)
    Body-only 2d+1 distance:      (0,0,0) (lexicographically zero)

(d2) S2(i,1) -> S2(j,1) when j > i and dir!=0 (output, loop-carried)
    dependence distance:        (+,  0)
    2d+1 dependence distance: (0,+,0,0,0) (lexicographically positive)
    Body-only 2d+1 distance:      (0,0,0) (lexicographically zero)


Statement-wise vectorization:
for (int i = 0; i < 2*n; i+=2) {
  for (int j = 0; j < 2; ++j) {
    if (j==!!dir) {
      load  {P[i],         P[i+1]}       -> staddr
      store {A[staddr[0]], A[staddr[1]]} <- gen()
    }
    if (j==!dir) {
      load  {Q[i],         Q[i+1]}       -> ldaddr
      load  {A[ldaddr[0]], A[ldaddr[1]]} -> use()
    }
  }
}
// Dependencies preserved: (a1), (a2), (b1), (b2), (d1), (d2)
// Dependencies violated within a simd chunk: (c1), (c2)

*/
