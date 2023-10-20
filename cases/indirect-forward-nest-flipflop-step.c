/// Loop with unknown dependencies due to indirect accesses, execution order switched during execution.
/// Like indirect-forward-nest-flipflop1, but order reversal reached by inversing the loop.
/// First iteration:  for (int j = 0; j < 2; j+=1)
/// Second iteration: for (int j = 2; 0 <= j; j+=-1)
/// This is vectorizable, e.g. by normalizing the loop iteration variable or unrolling the inner loop.
void func(int n, int dir, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2; ++i) {
       for (int j = 2*i; 0 <= j && j < 2; j+=1-2*i) { // Does not need to be an OpenMP canonical loop nest.
S1:      if (j==0) A[P[i]] = gen();
S2:      if (j!=0) use(A[Q[i]]);
       }
     }
}
