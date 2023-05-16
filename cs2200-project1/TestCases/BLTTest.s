! Written by Jackson McGill
! BLT Test
main:   lea $sp, initsp                 ! initialize stack pointer
        lw $sp, 0($sp)                  ! finish initialization

        lea $a0, TWO                    ! load first number address
        lw $a0, 0($a0)                  ! load first number

        lea $a1, FIVE                   ! load second number address
        lw $a1, 0($a1)                  ! load second number

	lea $a2, TEN                   	! load third number address
        lw $a2, 0($a2)                  ! load third number
	blt $a2, $a0, destwo            ! go to destwo if $a0 < $a2
	br desone
        halt

desone:					! should arrive here
	add $t0, $a0, $a1 		
	halt

destwo:                                 
	add $t0, $a0, $a2
	halt

TWO:    .fill 2
FIVE:   .fill 5
TEN:    .fill 10

initsp: .fill 0xA000
