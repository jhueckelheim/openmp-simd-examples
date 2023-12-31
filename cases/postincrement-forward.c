/// loop-carried forward depdency (vectorized by LLVM)
/// Without -ffast-math, increment and decrement order must not be exchanged.
void func(int n, long A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[i+1]++;
S2:    A[i]--;
     }
}


/*
Dependencies:
  S1(s) -> S2(s+1) over A[s+1] (flow/anti/output, loop-carried, "lexically forward")
    dependence distance:        (1)
    2d+1 dependence distance: (0,1,1) (lexicographically positive)
    Body-only 2d+1 distance:      (1) (lexicographically positive)


// Statement-wise vectorization (valid):
for (int i = 0; i < 2*n; i+=2) {
  load  {A[i+1],   A[i+2]   } -> v
  set   {v[0]+1,   v[1]+1   } -> incv
  store {A[i+1],   A[i+2]   } <- incv

  load  {A[i],     A[i+1]   } -> u
  set   {u[0]-1,   u[1]-1   } -> decu
  store {A[i],     A[i+1]   } <- decu
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
