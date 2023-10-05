/// Order of evaluation unsequenced in C/C++.
void func(int n, long A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1a:    long a = A[i+1];
S1b:    A[i+1] = a + 1;

S2a:    long b = A[i];
S2b:    A[i] = a - 1;
     }
}


/*
Dependencies:
  S1a(s) -> S1b(s+1) over a (flow, intra-body)
    dependence distance:        (0)
    2d+1 dependence distance: (0,0,1) (lexicographically positive)
    Body-only 2d+1 distance:      (1) (lexicographically positive)

  S2a(i) -> S2b(i+1) over b (flow, intra-body)
    dependence distance:        (0)
    2d+1 dependence distance: (0,0,1) (lexicographically positive)
    Body-only 2d+1 distance:      (1) (lexicographically positive)

  S1b(s) -> S2a(s+1) over A[s+1] (flow, loop-carried)
    dependence distance:        (1)
    2d+1 dependence distance: (0,1,1) (lexicographically positive)
    Body-only 2d+1 distance:      (1) (lexicographically positive)

  S1b(s) -> S2b(s+1) over A[s+1] (output, loop-carried)
    dependence distance:        (1)
    2d+1 dependence distance: (0,1,2) (lexicographically positive)
    Body-only 2d+1 distance:      (2) (lexicographically positive)

  S1a(s) -> S2b(s+1) over A[s+1] (anti, loop-carried)
    dependence distance:        (1)
    2d+1 dependence distance: (0,1,3) (lexicographically positive)
    Body-only 2d+1 distance:      (3) (lexicographically positive)


// Statement-wise vectorization (invalid):
for (int i = 0; i < 2*n; i+=2) {
  load  {A[i+1],   A[i+2]   } -> v     // A[i+1] supposed to be decremented from last iteration but hasen't yet
  set   {v[0]-1,   v[1]-1   } -> decv
  store {A[i+1],   A[i+2]   } <- decv

  load  {A[i],     A[i+1]   } -> u
  set   {u[0]+1,   u[1]+1   } -> incu
  store {A[i],     A[i+1]   } <- incu

  set   {u[0]+v[0],u[1]+v[1]} -> uv
  store {B[i],     B[i+1]   } <- uv
}


// Auto-vectorization (LLVM)
set {poison, A[0]} -> recur
for (int i = 0; i < 2*n; i+=2) {
  load  {A[i+1],   A[i+2]   } -> v
  set   {v[0]+1,   v[1]+1   } -> incv
  set   {recur[1], incv[0]  } -> shfv // shuffle
  store {A[i+1],   A[i+2]   } <- incv

  set   {shfv[0]-1,shfv[1]-1} -> decu
  store {A[i],     A[i+1]   } <- decu

  set incv                    -> recur
}

*/
