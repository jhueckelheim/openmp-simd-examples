/// Recursion with non-constant call depth.
/// This is a tail-recursion but is shouldn't matter.
/// Does not even work with 2d+1 cordinates, d is unbounded.
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       auto call = [](int j) [[gnu::always_inline]] {
         A[i+1] = gen(i+j);
         if (j > 1) call(j-1);
       };
       call(i);
       use(A[j]);
     }
}
