/// Loop with unknown dependencies due to indirect accesses.
void func(int n, int P[restrict], int Q[restrict], long A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[P[i]]++;
S2:    A[Q[i]]--;
     }
}

/// These indices make func() identical to postincrement-forward (i.e. only "lexical forward dependencies").
/// Without -ffast-math, increment and decrement order must not be exchanged.
/// Vectorizing it requires a recurrance variable.
void test(int n, double A[restrict]) {
  int P[2*n], Q[2*n];
  for (int i = 0; i < 2*n; ++i) {
    P[i] = i + 1;
    Q[i] = i;
  }
  func(n, P, Q, A);
}


/*
Maximal assumed dependencies:
(a) S1(s) -> S2(s) over A[P[s]],A[Q[s]] if overlapping (flow/anti/output, intra-body, "lexically forward")
    dependence distance:        (0)
    2d+1 dependence distance: (0,0,1) (lexicographically positive)
    Body-only 2d+1 distance:      (1) (lexicographically positive)

(b) S1(s) -> S2(t) if t > s over A[P[s]],A[Q[t]] if overlapping (flow/anti/output, loop-carried, "lexically forward")
    dependence distance:        (+)
    2d+1 dependence distance: (0,+,1) (lexicographically positive)
    Body-only 2d+1 distance:      (1) (lexicographically positive)

(c) S2(s) -> S1(t) if t > s over A[Q[s]],A[P[t]] if overlapping (flow/anti/ouput, loop-carried, "lexically backward")
    dependence distance:        (+)
    2d+1 dependence distance: (0,+,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (-1) (lexicographically negative)

(d) S1(s) -> S1(t) if t > s over A[P[s]],A[P[t]] if overlapping (flow/anti/output, loop-carried, "lexically backward")
    dependence distance:        (+)
    2d+1 dependence distance: (0,+,0) (lexicographically positive)
    Body-only 2d+1 distance:      (0) (lexicographically zero)

(e) S2(s) -> S2(t) if t > s over A[Q[s]],A[Q[t]] if overlapping (flow/anti/output, loop-carried, "lexically backward")
    dependence distance:        (+)
    2d+1 dependence distance: (0,+,0) (lexicographically positive)
    Body-only 2d+1 distance:      (0) (lexicographically zero)


Statement-wise vectorization (invalid without additional assumptions):
for (int i = 0; i < 2*n; i+=2) {
  load  {P[i],     P[i+1]  } -> uaddr
  load  {uaddr[0], uaddr[1]} -> u
  set   {u[0]+1,   u[1]+1  } -> incu
  store {uaddr[0], uaddr[1]} <- incu

  load  {Q[i],     Q[i+1]  } -> vaddr
  load  {vaddr[0], vaddr[1]} -> v
  set   {v[0]-1,   u[1]-1  } -> decv
  store {vaddr[0], vaddr[1]} <- decv
}
// Dependencies preserved: (a) (d) (e)
// Dependencies violated within a simd chunk: (b) (c)

*/
