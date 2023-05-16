! Spring 2023 Revisions by Calvin Khiddee-Wu & Andrej Vrtanoski

! This program executes pow as a test program using the LC 2200-pipe calling convention
! Check your registers ($v0) and memory to see if it is consistent with this program

main:	lea $sp, initsp                         ! initialize the stack pointer
        lw $sp, 0($sp)                          ! finish initialization
	
						
	addi $t0, $zero, 0			! ### pipeline hazards test case ###
	addi $t0, $zero, 2200
	addi $t1, $zero, 42
	blt $t0, $t1, END			! raw / waw hazard check

	lea $t2, magic
	lw $t2, 0($t2)			
	blt $t1, $t2, END			! use-after-load check
	blt $t2, $t1, END

	blt $zero, $t1, START			! branch misprediction check
	halt

START: 

        lea $a0, BASE                           ! load base for pow
        lw $a0, 0($a0)
        lea $a1, EXP                            ! load power for pow
        lw $a1, 0($a1)
        lea $at, POW                            ! load address of pow
        jalr $ra, $at                           ! run pow
        lea $a0, ANS                            ! load base for pow
        sw $v0, 0($a0)

END:
        halt                                    ! stop the program here
        addi $v0, $zero, -1                     ! load a bad value on failure to halt

BASE:   .fill 2
EXP:    .fill 8
ANS:	.fill 0                                 ! should come out to 256 (BASE^EXP)

POW:    
        addi $sp, $sp, -1                       ! saves the old frame pointer
        sw $fp, 0($sp)

        addi $fp, $sp, 0                        ! set new frame pointer

        blt $zero, $a1, BASECHK                 ! check if $a1 is zero
        br RET1                                 ! if the exponent is 0, return 1
        
BASECHK:
        blt $zero, $a0, WORK
        br RET0

WORK:
        addi $a1, $a1, -1                       ! decrement the power

        lea $at, POW                            ! load the address of POW
        addi $sp, $sp, -1                       ! saves return address onto stack
        sw $ra, 0($sp)
        addi $sp, $sp, -1                       ! saves arg 0 onto stack
        sw $a0, 0($sp)
        jalr $ra, $at                           ! recursively call POW
        add $a1, $v0, $zero                     ! store return value in arg 1
        lw $a0, -2($fp)                         ! load the base into arg 0
        lea $at, MULT                           ! load the address of MULT
        jalr $ra, $at                           ! multiply arg 0 (base) and arg 1 (running product)
        lw $a0, 0($sp)
        addi $sp, $sp, 1
        lw $ra, 0($sp)
        addi $sp, $sp, 1

        br FIN                                  ! unconditional branch to FIN

RET1:   add $v0, $zero, $zero                   ! return a value of 0
	addi $v0, $v0, 1                        ! increment and return 1
        br FIN                                  ! unconditional branch to FIN

RET0:   add $v0, $zero, $zero                   ! return a value of 0

FIN:	lw $fp, 0($sp)                          ! restore old frame pointer
        addi $sp, $sp, 1
        jalr $zero, $ra

MULT:   add $v0, $zero, $zero                   ! return value = 0
        addi $t0, $zero, 0                      ! sentinel = 0
AGAIN:  add $v0, $v0, $a0                       ! return value += argument0
        addi $t0, $t0, 1                        ! increment sentinel
        blt $t0, $a1, AGAIN                     ! while sentinel < argument, loop again
        jalr $zero, $ra                         ! return from mult

initsp: .fill 0xA000
magic:  .fill 0x002A
