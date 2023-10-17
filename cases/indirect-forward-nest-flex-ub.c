/// Number of executions of S2 varies between loops:
/// Difference caused by dependent upper bound, semantically identical to indirect-forward-nest-flex-cond. 
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[P[i]] = gen(i);
       for (int j = 0; j+i < 2; ++j) {
S2:      use(A[Q[2*i+1+j]]);
       }
     }
}
