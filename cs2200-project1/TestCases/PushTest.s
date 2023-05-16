! Written by Jackson McGill
! PUSH Test -- PASSED
main:   lea $sp, initsp                 ! initialize stack pointer
        lw $sp, 0($sp)                  ! finish initialization

        lea $a0, TWO                    ! load first number address
        lw $a0, 0($a0)                  ! load first number

        lea $a1, FIVE                   ! load second number address
        lw $a1, 0($a1)                  ! load second number

        push $a0                        ! memory location of 0x9FFF should be 2
        push $a1                        ! memory location of 0x9FFE should be 5
        
        halt

TWO:    .fill 2
FIVE:   .fill 5
TEN:    .fill 10

initsp: .fill 0xA000                    ! $sp should be 0x9FFE at the end