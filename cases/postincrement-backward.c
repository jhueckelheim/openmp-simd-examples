/// loop-carried backward dependency (not vectorized LLVM)
/// Without -ffast-math, increment and decrement order must not be exchanged.
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[i]--;
S2:    A[i+1]++;
     }
}


/*
Dependencies:
  S2(s) -> S1(s+1) over A[s+1] (flow/anti/output, loop-carried, "lexically backward")
    dependence distance:        (1)
    2d+1 dependence distance: (0,1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (-1) (lexicographically negative)


// Expression-wise vectorization (invalid):
for (int i = 0; i < 2*n; i+=2) {
  load  {A[i],     A[i+1]   } -> u
  set   {u[0]+1,   u[1]+1   } -> incu
  store {A[i],     A[i+1]   } <- incu

  load  {A[i+1],   A[i+2]   } -> v     // A[i+1] has already been incremented which should only happen in the next iteration
  set   {v[0]-1,   v[1]-1   } -> decv
  store {A[i+1],   A[i+2]   } <- decv

  set   {u[0]+v[0],u[1]+v[1]} -> uv
  store {B[i],     B[i+1]   } <- uv
}


*/
