/// Loop with unknown dependencies due to indirect accesses and a nested loop with variable trip count.
/// Shows that a linearization of a 2d+1 coordinate is not feasible -- S2 would get indices that match other (or non-existing) source statements in other iterations.
/// The j-loop has many empty iterations, but imagine it being full of other uses; don't want the example to become more complicated than necessary.
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < i+1; ++j) {
S1:      if (j==0) A[P[i]] = gen();
S2:      if (j==i) use(A[Q[i]]);
       }
     }
}

/*
Maximal assumed dependencies:
(a) S1(s,0) -> S2(s,s) (flow, intra-body)
    dependence distance:        (0,  s)
    2d+1 dependence distance: (0,0,0,s,1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,s,1) (lexicographically positive)

(b) S1(s,0) -> S2(t,t) when t > s (flow, loop-carried)
    dependence distance:        (t-s,  s)
    2d+1 dependence distance: (0,t-s,0,t-s,1) (lexicographically positive)
    Body-only 2d+1 distance:        (0,t-s,1) (lexicographically positive)

(c) S2(s,s) -> S1(t,0) when t > s (anti, loop-carried)
    dependence distance:        (t-s,  s-t)
    2d+1 dependence distance: (0,t-s,0,s-t,-1) (lexicographically positive)
    Body-only 2d+1 distance:        (0,s-t,-1) (lexicographically negative)

(d) S1(s,0) -> S1(t,0) when t > s (output, loop-carried)
    dependence distance:        (t-s)
    2d+1 dependence distance: (0,t-s,0,0,0) (lexicographically positive)
    Body-only 2d+1 distance:        (0,0,0) (lexicographically zero)


Statement-wise vectorization:
for (int i = 0; i < 2*n; i+=2) {
  for (int j = 0; j < i+2; ++j) {
    if (j==0) {
      load <mask> {P[i],         P[i+1]}       -> staddr
      store<mask> {A[staddr[0]], A[staddr[1]]} <- gen()
    }
    set        {j==i,         j==i+1}       -> mask
    load<mask> {Q[i],         Q[i+1]}       -> ldaddr
    load<mask> {A[ldaddr[0]], A[ldaddr[1]]} -> use()
  }
}
// Dependencies preserved: (a), (b), (d)
// Dependencies violated within a simd chunk: (c)

*/
