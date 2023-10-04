/// Loop with unknown dependencies due to indirect accesses, lexically reversed using loop nest.
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < 2; ++j) {
S1:      if (j==1) use(A[Q[i]]);
S2:      if (j==0) A[P[i]] = gen();
       }
     }
}

/*
Maximal assumed dependencies:
(a) S1(i,1) -> S2(i,0) (flow, intra-body)
    dependence distance:        (0,  1)
    2d+1 dependence distance: (0,0,0,1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,1,-1) (lexicographically positive)

(b) S2(i,1) -> S1(j,0) when j > i (flow, loop-carried)
    dependence distance:        (+,  1)
    2d+1 dependence distance: (0,+,0,1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,1,-1) (lexicographically positive)

(c) S1(i,0) -> S2(j,1) when j > i (anti, loop-carried)
    dependence distance:        (+,  -1)
    2d+1 dependence distance: (0,+,0,-1,1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,-1,1) (lexicographically negative)

(d) S2(i,1) -> S2(j,1) when j > i (output, loop-carried)
    dependence distance:        (+,  0)
    2d+1 dependence distance: (0,+,0,0,0) (lexicographically positive)
    Body-only 2d+1 distance:      (0,0,0) (lexicographically zero)


Statement-wise vectorization:
for (int i = 0; i < 2*n; i+=2) {
  for (int j = 0; j < 2; ++j) {
    if (j==1) {
      load  {Q[i],         Q[i+1]}       -> ldaddr
      load  {A[ldaddr[0]], A[ldaddr[1]]} -> use()
    }
    if (j==0) {
      load  {P[i],         P[i+1]}       -> staddr
      store {A[staddr[0]], A[staddr[1]]} <- gen()
    }
  }
}
// Dependencies preserved: (a), (b), (d)
// Dependencies violated within a simd chunk: (c)

*/
