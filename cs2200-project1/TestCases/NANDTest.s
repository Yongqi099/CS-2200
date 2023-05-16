! Written by Jackson McGill
! NAND Test 
main:   lea $sp, initsp                 ! initialize stack pointer
        lw $sp, 0($sp)                  ! finish initialization

        lea $a0, ONE                    ! load first number address
        lw $a0, 0($a0)                  ! load first number

        lea $a1, TWO                    ! load second number address
        lw $a1, 0($a1)                  ! load second number

	lea $a2, TEN                   	! load third number address
        lw $a2, 0($a2)                  ! load third 

        nand $t0, $a0, $a1              ! $t0 should be -1
        halt

ONE:    .fill 1
TWO:    .fill 2
TEN:    .fill 10

initsp: .fill 0xA000