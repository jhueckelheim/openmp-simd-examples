




# Intra-body dependences

<details>
<summary><h2>anti-intra</h2></summary>

Dependencies not carried by the loop-to-be-vectorized should be preserved, even if carried in a nested loop.

### Code
```c
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    use(A[i]);
S2:    A[i] = gen();
     }
}
```

### Dependencies

<ol type="a">
  <li><code>S1(s) -> S2(s)</code> (anti, scalar, intra-body, "lexically forward")
<pre>
dependence distance:        (0)
2d+1 dependence distance: (0,0,1) (lexicographically positive)
Body-only 2d+1 distance:      (1) (lexicographically positive)
</pre>
</li>
</ol>


### Expression-wise vectorization
```c
for (int i = 0; i < 2*n; i+=2) {
  load (A[i], A[i+1]) -> use()
  store (A[i], A[i+1]) <- gen()
}
```
</details>
















<details>
<summary><h2>flow-intra</h2></summary>

Dependencies not carried by the loop-to-be-vectorized should be preserved, even if carried in a nested loop.

### Code
```c
void func(int n, double A[restrict], double B[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S:    float a = A[i];
T:    B[i] = a;
     }
}
```
</details>






# Backward dependences

<details>
<summary><h2>flow-backward</h2></summary>

Loop with "lexically-backward" loop-carried dependency

### Code
```c
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    use(A[i]);
S2:    A[i+1] = gen();
     }
}
```

### Expression-wise vectorization (invalid)
```c
for (int i = 0; i < 2*n; i+=2) {
  load  {A[i],   A[i+1]} -> use()
  store {A[i+1], A[i+2]} <- gen()
}
```

### Expression-wise vectorization (valid)
```c
for (int i = 0; i < 2*n; i+=2) {
  load  (A[i]  , A[i+1]) -> use()
  store (A[i+1], A[i+2]) <- gen()
}
```
by instruction reordering within the loop body, possible with any mid-end pass.
</details>


<details>
<summary><h2>flow-backward-nest</h2></summary>
Same as flow-backward, but inversed statement order by nesting into another loop,

### Code
```c
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < 2; ++j) {
S1:      if (j == 1) A[i+1] = gen();
S2:      if (j == 0) use(A[i]);
       }
     }
}
```
</details>




<details>
<summary><h2>flow-backward-call</h2></summary>

Same as flow-carried-backward, but inversed statement order by calling an inlined function.

### Code
```c
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       auto call = [&]() [[gnu::always_inline]] {
S1:      A[i+1] = gen();
       };
S2:    use(A[i]);
S3:    call();
     }
}
```
</details>





<details>
<summary><h2>flow-backward-call-multi</h2></summary>

Call the same function multiple times.
The same lexical statement used for different dependencies.

### Code
```c
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
```
</details>





<details>
<summary><h2>flow-backward-elseif</h2></summary>

 Loop with "lexically-backward" loop-carried dependency, non-dominating gen() and use() and effectively reversed order

### Code
```c
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       if (i%2 == 0) {
S1:      A[i+1] = gen();
       } else {
S2:      use(A[i]);
       }
     }
}
```
</details>






<details>
<summary><h2>flow-backward-ifelse</h2></summary>
Loop with "lexically-backward" loop-carried dependency, non-dominating gen() and use() through conditional.
There is no semantic execution order of S1 and S2 since they are never both executed in an iteration.
While source code order can be used here, there are countless variants that where S2 comes before S1 in the source (e.g. preprocessor, call, inner loop, ...)
Would need to explicitly define

   1. For the purpose of if-conversion/vectorization, the if-part is considered to executed before the else part.
   2. An OpenMP program that depends on the order of then then and else parts of a condition is not conformant (-> can break dependencies that result from any order)

### Code
```c
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       if (i%2 == 0) {
S1:      A[i+1] = gen();
       } else {
S2:      use(A[i]);
       }
     }
}
```
</details>




<details>
<summary><h2>indirect-backward</h2></summary>
Loop with unknown dependencies due to indirect accesses.

### Code
```c
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    use(A[P[i]]);
S2:    A[Q[i]] = gen();
     }
}
```
</details>







# Forward dependences

<details>
<summary><h2>flow-forward</h2></summary>

Loop with simple loop-carried dependency

### Code
```c
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[i+1] = gen();
S2:    use(A[i]);
     }
}
```

### Expression-wise vectorization
```c
for (int i = 0; i < 2*n; i+=2) {
  store {A[i+1], A[i+2]} <- gen()
  load  {A[i],   A[i+1]} -> use()
}
```
</details>



<details>
<summary><h2>flow-forward-call-recursion</h2></summary>

Recursion with non-constant call depth.
This is a tail-recursion but is shouldn't matter.
Does not even work with 2d+1 cordinates, d is unbounded.

### Code
```c
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       auto call = [](int j) [[gnu::always_inline]] {
         A[i+1] = gen(i+j);
         if (j > 1) call(j-1);
       };
       call(i);
       use(A[i]);
     }
}
```
</details>




<details>
<summary><h2>flow-forward-dtor</h2></summary>

Reserse lexical order through dtors.
Dtors executed in reverse order.

### Code
```c
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
```
</details>






<details>
<summary><h2>flow-forward-logical</h2></summary>

Loop where counter and logical iteration number are different.
This should still be a simple loop-carried dependency

### Code
```c
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 2*n-1; i >= 0; --i) {
S1:    A[i-1] = gen();
S2:    use(A[i]);
     }
}
```
</details>





<details>
<summary><h2>flow-forward-nest</h2></summary>

Like flow-carried-forward, but lexically inverted using a nested loop

### Code
```c
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < 2; ++j) {
S1:      if (j == 1) use(A[i]);
S2:      if (j == 0) A[i+1] = gen();
       }
     }
}
```
</details>




# Forward dependencies with indirect accesses

<details>
<summary><h2>indirect-forward</h2></summary>

Loop with unknown dependencies due to indirect accesses.

### Code
```c
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[P[i]] = gen();
S2:    use(A[Q[i]]);
     }
}
```
</details>




<details>
<summary><h2>indirect-forward-ifelse</h2></summary>

Loop with unknown dependencies due to indirect accesses with conditional execution.

### Code
```c
void func(int n, bool C[restrict], int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       if (C[i]) {
S1:      A[P[i]] = gen();
       } else {
S2:      use(A[Q[i]]);
       }
     }
}

```
</details>





<details>
<summary><h2>indirect-forward-nest</h2></summary>

Loop with unknown dependencies due to indirect accesses, lexically reversed using a loop nest.


### Code
```c
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < 2; ++j) {
S1:      if (j==1) use(A[Q[i]]);
S2:      if (j==0) A[P[i]] = gen();
       }
     }
}
```
</details>


<details>
<summary><h2>indirect-forward-nest-dir</h2></summary>

Loop with unknown dependencies due to indirect accesses, execution order selected by runtime variable.
Equivalently, imagine `dir` changing the direction of the j-loop to counting downwards.

### Code
```c
void func(int n, int dir, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < 2; ++j) {
S1:      if (j==!!dir) A[P[i]] = gen();
S2:      if (j==!dir)  use(A[Q[i]]);
       }
     }
}
```
</details>



<details>
<summary><h2>indirect-forward-nest-flex-cond</h2></summary>

Number of executions of S2 varies between loops:
Difference caused by conditional.

### Code
```c
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[P[i]] = gen(i);
       for (int j = 0; j < 2; ++j) {
S2:      if (j >= i) use(A[Q[2*i+j]]);
       }
     }
}

```
</details>


<details>
<summary><h2>indirect-forward-nest-flex-lb</h2></summary>

Number of executions of S2 varies between loops:
Difference caused by dependent lower bound, semantically identical to indirect-forward-nest-flex-cond.

### Code
```c
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[P[i]] = gen(i);
       for (int j = i; j < 2; ++j) {
S2:      use(A[Q[2*i+j]]);
       }
     }
}
```
</details>

<details>
<summary><h2>indirect-forward-nest-flex-ub</h2></summary>

Number of executions of S2 varies between loops:
Difference caused by dependent upper bound, semantically identical to indirect-forward-nest-flex-cond.

### Code
```c
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[P[i]] = gen(i);
       for (int j = 0; j+i < 2; ++j) {
S2:      use(A[Q[2*i+1+j]]);
       }
     }
}
```
</details>



<details>
<summary><h2>indirect-forward-nest-flexlength</h2></summary>

Loop with unknown dependencies due to indirect accesses and a nested loop with variable trip count.
Shows that a linearization of a 2d+1 coordinate is not feasible -- S2 would get indices that match other (or non-existing) source statements in other iterations.
The j-loop has redundant iterations, but imagine it being full of other uses; don't want the example to become more complicated than necessary.

### Code
```c
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < 2*i; ++j) {
S1:      A[P[i]] = gen(j); // Always same address, so it's effectively  `if(i > 0) A[P[i]] = gen(2*i-1)`
       }
S2:    use(A[Q[i]]);
     }
}
```
</details>




<details>
<summary><h2>indirect-forward-nest-flipflop-modulo</h2></summary>

Loop with unknown dependencies due to indirect accesses, execution order switched during execution.
Equivalently, imagine `i%2` changing the direction of the j-loop to counting downwards.

### Code
```c
void func(int n, int dir, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       for (int j = 0; j < 2; ++j) {
S1:      if (j==i%2) A[P[i]] = gen();
S2:      if (j!=i%2) use(A[Q[i]]);
       }
     }
}
```
</details>


<details>
<summary><h2>indirect-forward-nest-flipflop-step</h2></summary>

Like indirect-forward-nest-flipflop1, but order reversal reached by inversing the loop.

 * First iteration:  for (int j = 0; j < 2; j+=1)
 * Second iteration: for (int j = 2; 0 <= j; j+=-1)

This is vectorizable, e.g. by normalizing the loop iteration variable or unrolling the inner loop.

### Code
```c
void func(int n, int dir, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2; ++i) {
       for (int j = 2*i; 0 <= j && j < 2; j+=1-2*i) { // Does not need to be an OpenMP canonical loop nest.
S1:      if (j==0) A[P[i]] = gen();
S2:      if (j!=0) use(A[Q[i]]);
       }
     }
}
```
</details>



<details>
<summary><h2>indirect-forward-nest-foreach</h2></summary>

Nested foreach-loop with non-numeric loop iteration "identifier".

### Code
```c
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
```
</details>



<details>
<summary><h2>indirect-forward-nest-while</h2></summary>

Nested loop with data-dependent (and possibly infinite) exit condition.

### Code
```c
void func(int n, int P[restrict], int Q[restrict], double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       while (A[P[i]] >= epsilon) {
S1:      A[P[i]] = gen();
       }
S2:    use(A[Q[i]]);
     }
}
```
</details>





# Postincrement

<details>
<summary><h2>postincrement-backward</h2></summary>

loop-carried backward dependency (not vectorized LLVM)
Without -ffast-math, increment and decrement order must not be exchanged.

### Code
```c
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[i]--;
S2:    A[i+1]++;
     }
}
```
</details>


<details>
<summary><h2>postincrement-forward</h2></summary>

loop-carried forward depdency (vectorized by LLVM)
Without -ffast-math, increment and decrement order must not be exchanged.

### Code
```c
void func(int n, long A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[i+1]++;
S2:    A[i]--;
     }
}
```
</details>


<details>
<summary><h2>postincrement-forward-detail</h2></summary>

Order of evaluation unsequenced in C/C++.
Without -ffast-math, increment and decrement order must not be exchanged.

### Code
```c
void func(int n, double A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1a:    long a = A[i+1];
S1b:    A[i+1] = a + 1;

S2a:    long b = A[i];
S2b:    A[i] = a - 1;
     }
}
```
</details>


<details>
<summary><h2>postincrement-indirect</h2></summary>

Loop with unknown dependencies due to indirect accesses.

### Code
```c
void func(int n, int P[restrict], int Q[restrict], long A[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
S1:    A[P[i]]++;
S2:    A[Q[i]]--;
     }
}
```

### Possible Calling code

These indices make `func()` identical to postincrement-forward (i.e. only "lexical forward dependencies").
Without -ffast-math, increment and decrement order must not be exchanged.
Vectorizing it requires a recurrance variable.
```c
void test(int n, double A[restrict]) {
  int P[2*n], Q[2*n];
  for (int i = 0; i < 2*n; ++i) {
    P[i] = i + 1;
    Q[i] = i;
  }
  func(n, P, Q, A);
}
```
</details>




<details>
<summary><h2>postincrement-unsequenced</h2></summary>

Order of evaluation unsequenced in C/C++.
Note that the result is well-defined since withing the same statement execution, A[i], A[i+1] and B[i] (by __restrict) are different memory locations.

### Code
```c
void func(int n, long A[restrict], long B[restrict]) {
#pragma omp simd simdlen(2)
     for (int i = 0; i < 2*n; ++i) {
       B[i] = /*S1*/A[i+1]++ + /*S2*/A[i]--;
     }
}
```
</details>



# Other

<details>
<summary><h2>short-circuit</h2></summary>

Fortran allows, but does not mandate short-circuiting
of boolean expressions. `sub` has a loop-carried dependency
if the compiler does not use short-curcuiting, but can be
vectorized if the compiler short-circuits.

Observe this example being vectorized (or not) by gfortran with
gfortran -fopenmp short-circuit.f90 -Ofast -msse2 -ftree-vectorize -fopt-info-vec-missed -c -fno-frontend-optimize
(leave the last option away to enable short-circuiting)

```f90
! A function that always returns true, and has a side-effect
! since it modifies its input variable
function gen_ret_true(a)
    logical :: gen_ret_true
    double precision :: a
    a = 1.0
    gen_ret_true = .true.
end function

! A function that modifies its input variable in a different
! way than gen_ret_true. The return value of this function
! does not matter.
function gen(a)
    logical :: gen
    double precision :: a
    a = 2.0
    gen = .true.
end function

! A function that has a loop that can or cannot be vectorized
! depending on whether the compiler short-circuits the boolean
! expression in the loop body.
subroutine sub(n, a)
    integer :: i, n
    double precision, dimension(n) :: a
    logical :: irrelevant, gen_ret_true, gen

    !$omp simd simdlen(2) private(b)
    do i=1,n-1
        irrelevant = gen_ret_true(A(i)) .or. gen(A(i+1))
    end do
    irrelevant = gen_ret_true(A(n))
end subroutine

program main
    integer, parameter :: n = 4
    double precision, dimension(n) :: a
    a = 0
    call sub(n, a)
    write(*,*) a
end program
```

</details>
