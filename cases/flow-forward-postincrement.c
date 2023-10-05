/// Order of evaluation unsequenced in C/C++.
void func(int n, long A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[i+1]++;
S2:    A[i]--;
     }
}


/*
Dependencies:
  S1(i) -> S2(i+1) over A[i+1] (flow/anti/output, loop-carried)
    dependence distance:        (1)
    2d+1 dependence distance: (0,1,1) (lexicographically positive)
    Body-only 2d+1 distance:      (1) (lexicographically positive)


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


// Auto-vectorization
for (int i = 0; i < 2*n; i+=2) {
  load  {A[i],     A[i+1]   } -> u
  load  {A[i+1],   A[i+2]   } -> v

  set   {u[0]+1,   u[1]+1   } -> incu
  set   {incu[1],  v[1]     } -> v    // shuffle instruction
  set   {v[0]-1,   v[1]-1   } -> decv

  store {A[i],     A[i+1]   } <- incu
  store {A[i+1],   A[i+2]   } <- decv

  set   {u[0]+v[0],u[1]+v[1]} -> uv
  store {B[i],     B[i+1]   } <- uv
}

*/
