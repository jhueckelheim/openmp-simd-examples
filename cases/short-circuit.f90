! Fortran allows, but does not mandate short-circuiting
! of boolean expressions. `sub` has a loop-carried dependency
! if the compiler does not use short-curcuiting, but can be
! vectorized if the compiler short-circuits.

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
