/// Nested loop with data-dependent (and possibly infinite) exit condition.
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       while (A[P[i]] >= epsilon) { 
S1:      A[P[i]] = gen(); 
       }
S2:    use(A[Q[i]]);
     }
}
