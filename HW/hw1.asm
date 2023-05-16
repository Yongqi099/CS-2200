main:
    addi $t0, $zero, 0       ! zero out loop counter
    addi $t1, $zero, 28      ! set loop limit
loop:
    beq $t0, $t1, end
    lea $a0, x                ! load the address of x to $a0
    lw $t2, 0x0($a0)          ! get the value of x from address
    add $t2, $t2, $t0
    addi $t0, $t0, 7
    lea $a0, x              ! load address x to $a0
    sw $t2, 0x0($a0)         ! update value of x to its address
    beq $zero, $zero, loop         ! repeat loop
end:
    halt

x: .fill 0                   ! x initially set to 0