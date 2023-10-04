/// Like flow-carried-forward, but lexically inverted using a nested loop
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < 2; ++j) {
S1:      if (j == 1) use(A[i]);
S2:      if (j == 0) A[i+1] = gen();
       }
     }
}

/*
Dependencies:
  S2(i,0) -> S1(i+1,1) (flow, loop-carried, "lexically backward")
    dependence distance:      (1,1)
    2d+1 dependence distance: (0,1,0,1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (0,1,-1) (lexicographically positive)


// Statement-wise vectorization (valid):
for (int i = 0; i < 2*n; i+=2) {
  for (int j = 0; j < 2; ++j) {
    if (j == 1) load (A[i], A[i+1]) -> use()
    if (j == 0) store (A[i+1], A[i+2]) <- gen()
  }
}


// Body-only semantic vectorization (valid):
for (int i = 0; i < 2*n; i+=2) {
  store (A[i+1], A[i+2]) <- gen()
  load (A[i], A[i+1]) -> use()
}


Has defined behaviour according to OpenMP 5.2:
  no, "lexically backward" dependency does not need to be preserved

*/
