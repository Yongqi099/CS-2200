! Spring 2023 Revisions by Bijan Nikain

! This program executes pow as a test program using the LC 5500 calling convention
! Check your registers ($v0) and memory to see if it is consistent with this program

main:	lea $sp, initsp                         ! initialize the stack pointer
        lw $sp, 0($sp)                          ! finish initialization

        addi $t0, $zero, 2200                   ! set $t0 for nand check
        nand $t0, $t0, $t0

        lea $a0, BASE                           ! load base for pow
        lw $a0, 0($a0)
        lea $a1, EXP                            ! load power for pow
        lw $a1, 0($a1)
        lea $at, POW                            ! load address of pow
        jalr $ra, $at                           ! run pow
        lea $a0, ANS                            ! load base for pow
        sw $v0, 0($a0)

        halt                                    ! stop the program here
        addi $v0, $zero, -1                     ! load a bad value on failure to halt

BASE:   .fill 2
EXP:    .fill 8
ANS:	.fill 0                                 ! should come out to 256 (BASE^EXP)

POW:    push $fp                                ! saves the old frame pointer

        addi $fp, $sp, 0                        ! set new frame pointer

        blt $zero, $a1, BASECHK                 ! check if $a1 is zero
        br RET1                                 ! if the exponent is 0, return 1
        
BASECHK:
        blt $zero, $a0, WORK
        br RET0

WORK:
        addi $a1, $a1, -1                       ! decrement the power

        lea $at, POW                            ! load the address of POW
        push $ra                                ! saves return address onto stack
        push $a0                                ! saves arg 0 onto stack
        jalr $ra, $at                           ! recursively call POW
        add $a1, $v0, $zero                     ! store return value in arg 1
        lw $a0, -2($fp)                         ! load the base into arg 0
        lea $at, MULT                           ! load the address of MULT
        jalr $ra, $at                           ! multiply arg 0 (base) and arg 1 (running product)
        pop $a0
        pop $ra

        br FIN                                  ! unconditional branch to FIN

RET1:   add $v0, $zero, $zero                   ! return a value of 0
	addi $v0, $v0, 1                        ! increment and return 1
        br FIN                                  ! unconditional branch to FIN

RET0:   add $v0, $zero, $zero                   ! return a value of 0

FIN:	pop $fp                                 ! restore old frame pointer
        jalr $zero, $ra

MULT:   add $v0, $zero, $zero                   ! return value = 0
        addi $t0, $zero, 0                      ! sentinel = 0
AGAIN:  add $v0, $v0, $a0                       ! return value += argument0
        addi $t0, $t0, 1                        ! increment sentinel
        blt $t0, $a1, AGAIN                     ! while sentinel < argument, loop again
        jalr $zero, $ra                         ! return from mult

initsp: .fill 0xA000
