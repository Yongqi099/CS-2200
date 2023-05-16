! Written by Jackson McGill
! AdderTest
main:   lea $sp, initsp                 ! initialize stack pointer
        lw $sp, 0($sp)                  ! finish initialization

        lea $a0, TWO                    ! load first number address
        lw $a0, 0($a0)                  ! load first number

        lea $a1, FIVE                   ! load second number address
        lw $a1, 0($a1)                  ! load second number

        add $t0, $a0, $a1               ! add registers
        addi $t1, $a0, -1               ! add immediate values

        halt

TWO:    .fill 2
FIVE:   .fill 5
TEN:    .fill 10

initsp: .fill 0xA000