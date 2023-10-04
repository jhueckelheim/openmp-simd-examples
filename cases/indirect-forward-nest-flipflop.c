/// Loop with unknown dependencies due to indirect accesses, execution order switched during execution.
/// Equivalently, imagine `i%2` changing the direction of the j-loop to counting downwards.
void func(int n, int dir, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < 2; ++j) {
S1:      if (j==i%2) A[P[i]] = gen();
S2:      if (j!=i%2) use(A[Q[i]]);
       }
     }
}

/*
Maximal assumed dependencies:
(a1) S1(i,0) -> S2(i,1) when i%2==0 (flow, intra-body)
    dependence distance:        (0,  1)
    2d+1 dependence distance: (0,0,0,1,1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,1,1) (lexicographically positive)

(a2) S2(i,1) -> S1(i,0) when i%2==1 (anti, intra-body)
    dependence distance:        (0,  1)
    2d+1 dependence distance: (0,0,0,1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,1,-1) (lexicographically positive)

(b1) S1(i,0) -> S2(j,1) when j > i and i%2==0 (flow, loop-carried)
    dependence distance:        (+,  1)
    2d+1 dependence distance: (0,+,0,1,1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,1,1) (lexicographically positive)

(b2) S2(i,0) -> S1(j,1) when j > i and i%2==1 (anti, loop-carried)
    dependence distance:        (+,  1)
    2d+1 dependence distance: (0,+,0,1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,1,-1) (lexicographically positive)

(c1) S2(i,1) -> S1(j,0) when j > i and i%2==0 (anti, loop-carried)
    dependence distance:        (+,  -1)
    2d+1 dependence distance: (0,+,0,-1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,-1,-1) (lexicographically negative)

(c2) S1(i,1) -> S2(j,0) when j > i and i%2==1 (flow, loop-carried)
    dependence distance:        (+,  -1)
    2d+1 dependence distance: (0,+,0,-1,1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,-1,1) (lexicographically negative)

(d1) S2(i,0) -> S2(j,0) when j > i and i%2==0 (output, loop-carried)
    dependence distance:        (+,  0)
    2d+1 dependence distance: (0,+,0,0,0) (lexicographically positive)
    Body-only 2d+1 distance:      (0,0,0) (lexicographically zero)

(d2) S2(i,1) -> S2(j,1) when j > i and i%2==1 (output, loop-carried)
    dependence distance:        (+,  0)
    2d+1 dependence distance: (0,+,0,0,0) (lexicographically positive)
    Body-only 2d+1 distance:      (0,0,0) (lexicographically zero)


Statement-wise vectorization:
for (int i = 0; i < 2*n; i+=2) {
  for (int j = 0; j < 2; ++j) {
    set             {i%2==j,       (i+1)%2-=j}   -> ifmask
    load <ifmask>   {P[i],         P[i+1]}       -> staddr
    store<ifmask>   {A[staddr[0]], A[staddr[1]]} <- gen()
    set             {i%2!=j,       (i+1)%2!=j}   -> elsemask
    load <elsemask> {Q[i],         Q[i+1]}       -> ldaddr
    load <elsemask> {A[ldaddr[0]], A[ldaddr[1]]} -> use()
  }
}
// Dependencies preserved: (a1), (a2), (b1), (b2), (d1), (d2)
// Dependencies violated within a simd chunk: (c1), (c2)

*/
