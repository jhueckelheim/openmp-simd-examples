/// Loop with "lexically-backward" loop-carried dependency, non-dominating gen() and use() through conditional.
/// There is no semantic execution order of S1 and S2 since they are never both executed in an iteration.
/// While source code order can be used here, there are countless variants that where S2 comes before S1 in the source (e.g. preprocessor, call, inner loop, ...)
/// Would need to explicitly define
///   1. For the purpose of if-conversion/vectorization, the if-part is considered to executed before the else part.
///   2. An OpenMP program that depends on the order of then then and else parts of a condition is not conformant (-> can break dependencies that result from any order)
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       if (i%2 == 1) {
S1:      use(A[i]);
       } else {
S2:      A[i+1] = gen();
       }
     }
}

/*
Dependencies:
  S2(s) -> S1(s+1) if s%2==1 over A[s+1] (flow, loop-carried)
    dependence distance:        (1)
    2d+1 dependence distance: (0,1,-1) (lexicographically positive)
    Body-only 2d+1 distance:      (-1) (lexicographically negative)


Statement-wise vectorization (invalid):
for (int i = 0; i < 2*n; i+=2) {
  set             {i%2==1, (i+1)%2==1}     -> ifmask
  load <ifmask>   {A[i],   A[i+1]    }     -> use()
  set             {!ifmask[0], !ifmask[1]} -> elsemask
  store<elsemask> (A[i+1], A[i+2])         <- gen()
}
// A[i+1] loads the value from before gen()


Auto-vectorization:
for (int i = 0; i < 2*n; i+=2) {
  use(A[i+1]);
  A[i+1] = gen();
}


Statement-wise vectorization possible:
  no

Auto-vectorization possible:
  yes

Has loop-carried dependencies:
  yes

Requires lexical order information to vectorize:
  no

Has defined behaviour according to OpenMP 5.2:
  ?

Should be supported by OpenMP:
  ?

*/
