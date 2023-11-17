/// Nested foreach-loop with non-numeric loop iteration "identifier".
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
   std::unordered_set<std::string> rope[2] = { {"mark", "my"}, {"my", "words"} };

#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (std::string s : rope[i]) {
S1:      A[hash(s)] = gen(s); // A[hash("my")] = gen("my"); the same statement for both occurances of "my". 
       }
S2:    use(A[Q[i]]);
     }
}
