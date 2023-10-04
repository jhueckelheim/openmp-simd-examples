/// Loop with unknown dependencies due to indirect accesses with conditional execution.
void func(int n, bool C[restrict], int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       if (C[i]) {
S1:      A[P[i]] = gen();
       } else {
S2:      use(A[Q[i]]);
       }
     }
}

/*
Maximal assumed dependencies:
(b) S1(i) -> S2(j) when j > i (flow, loop-carried)
    dependence distance:        (+)
    2d+1 dependence distance: (0,+,1) (lexicographically positive)
    Body-only 2d+1 distance:      (1) (lexicographically positive)

(c) S2(i) -> S1(j) when j > i (anti, loop-carried)
    dependence distance:        (+)
    2d+1 dependence distance: (0,+,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (-1) (lexicographically negative)

(d) S1(i) -> S1(j) when j > i (output, loop-carried)
    dependence distance:        (+)
    2d+1 dependence distance: (0,+,0) (lexicographically positive)
    Body-only 2d+1 distance:      (0) (lexicographically zero)


Statement-wise vectorization:
for (int i = 0; i < 2*n; i+=2) {
  set             {C[i],         C[i+1]}       -> ifmask
  load<ifmask>    {P[i],         P[i+1]}       -> staddr
  store<ifmask>   {A[staddr[0]], A[staddr[1]]} <- gen()
  set             {!ifmask[0],   !ifmask[1]}   -> elsemask
  load<elsemask>  {Q[i],         Q[i+1]}       -> ldaddr
  load<elsemask>  {A[ldaddr[0]], A[ldaddr[1]]} -> use()
}
// Preserved dependencies: (b), (d)
// Dependencies violated within a simd chunk: (c)

*/
