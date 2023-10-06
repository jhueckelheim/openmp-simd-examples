/// Loop with unknown dependencies due to indirect accesses and a nested loop with variable trip count.
/// Shows that a linearization of a 2d+1 coordinate is not feasible -- S2 would get indices that match other (or non-existing) source statements in other iterations.
/// The j-loop has redundant iterations, but imagine it being full of other uses; don't want the example to become more complicated than necessary.
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < i+1; ++j) {
S1:      A[P[i]] = gen(j);
       }
S2:    use(A[Q[i]]);
     }
}

/*
Maximal assumed dependencies:
(a) S1(s,t) -> S1(s,u) if t<u<=s over A[P[i]] (output, intra-body)
    dependence distance:        (0,  +)
    2d+1 dependence distance: (0,0,0,+,1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,+,1) (lexicographically positive)

(b) S1(s,*) -> S2(s) over A[P[i]],A[Q[i]] if overlapping (flow, intra-body)
    dependence distance:        (0,  ...)
    2d+1 dependence distance: (0,0,1,...) (lexicographically positive)
    Body-only 2d+1 distance:      (1,...) (lexicographically positive)

(c) S1(s,*) -> S2(t) when t > s (flow, loop-carried)
    dependence distance:        (t-s,  ...)
    2d+1 dependence distance: (0,t-s,1,...) (lexicographically positive)
    Body-only 2d+1 distance:        (1,...) (lexicographically positive)

(d) S2(s) -> S1(t,*) when t > s (anti, loop-carried)
    dependence distance:        (t-s,  ...)
    2d+1 dependence distance: (0,t-s,-1,...) (lexicographically positive)
    Body-only 2d+1 distance:        (-1,...) (lexicographically negative)

(e) S1(s,*) -> S1(t,*) when t > s (output, loop-carried)
    dependence distance:        (t-s)
    2d+1 dependence distance: (0,t-s,0,*,0) (lexicographically positive)
    Body-only 2d+1 distance:        (0,*,0) (lexicographically anything)


Statement-wise vectorization:
for (int i = 0; i < 2*n; i+=2) {
  for (int j = 0; j < i+2; ++j) {
      set         {j<i+1,        j+1<i+1}      -> mask
      load <mask> {P[i],         P[i+1]}       -> staddr
      store<mask> {A[staddr[0]], A[staddr[1]]} <- gen()
    }
    load {Q[i],         Q[i+1]}       -> ldaddr
    load {A[ldaddr[0]], A[ldaddr[1]]} -> use()
}

*/
