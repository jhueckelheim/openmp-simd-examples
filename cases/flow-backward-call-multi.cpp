/// Call the same function multiple times.
/// The same lexical statement used for different dependencies.
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       auto call = [&](int j) [[gnu::always_inline]] {
a:       A[j] = gen();
b:       use(A[j+1]);
       };
S1:    call(i);
S2:    call(i+1);
     }
}

/*
Effective body statements:
  S1a: A[i] = gen();
  S1b: use(A[i+1]);
  S2a: A[i+1] = get();
  S2b: use(A[j+1]);
*/
