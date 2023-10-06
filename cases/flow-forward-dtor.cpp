#include <memory>
static char irrelevant;

/// Reserse lexical order through dtors.
/// Dtors executed in reverse order.
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
        std::unique_ptr<char, void(*)(char*)> a(&irrelevant, [&](char* ptr) {
S1:            use(A[i]);
        });
        std::unique_ptr<char, void(*)(char*)> p(&irrelevant, [&](char* ptr) {
S2:            A[i+1] = gen();
        });
     }
}

/*
Dependencies:
  S2(s) -> S1(s+1) (flow, loop-carried)
      dependence distance:        (1)
      2d+1 dependence distance: (0,1,1) (lexicographically positive)
      Body-only 2d+1 distance:      (1) (lexicographically positive)


// Statement-wise vectorization (valid):
for (int i = 0; i < 2*n; i+=2) {
        std::unique_ptr<char, void(*)(char*)> a(&irrelevant, [](char* ptr) {
            load  {A[i],   A[i+1]} -> use()
        });
        std::unique_ptr<char, void(*)(char*)> p(&irrelevant, [](char* ptr) {
            store {A[i+1], A[i+2]} <- gen()
        });
     }
}

*/
