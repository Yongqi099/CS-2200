! SW Test
main:
    addi $t0, $zero, 8
    lea $a0, MEM
    sw $t0, 0($a0)
    halt
MEM: .fill 0
