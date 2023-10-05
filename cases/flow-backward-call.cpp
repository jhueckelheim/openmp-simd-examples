/// Same as flow-carried-backward, but inversed statement order by calling an inlined function
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       auto call = []() [[gnu::always_inline]] {
S1:      A[i+1] = gen();
       };
S2:    use(A[i]);
S3:    call();
     }
}

/*
Dependencies:
  S1(i) -> S2(i+1) (flow, loop-carried)
    dependence distance:        (1)
    2d+1 dependence distance: (0,1,-1,*) (lexicographically positive)
    Body-only 2d+1 distance:      (-1,*) (lexicographically negative)


Statement-wise vectorization (invalid):
for (int i = 0; i < 2*n; i+=2) {
    auto call = []() [[gnu::always_inline]] {
      store (A[i+1], A[i+2]) <- gen()
    }
    load (A[i], A[i+1]) -> use()
  }
}
// A[i+1] loads the value from before the function call, only valid for A[0]


Auto-vectorization after inlining (same as flow-carried-backward):
for (int i = 0; i < 2*n; i+=2) {
  load (A[i], A[i+1]) -> use()
  store (A[i+1], A[i+2]) <- gen()
}

*/
