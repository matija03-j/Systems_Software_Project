.section text
    ld  $5, %r1
    ld  $5, %r2
    ld  $7, %r3

    # beq kratko (disp12): r1 == r2 pa preskoči sledeći ld
    beq %r1, %r2, 4
    ld  $6, %r3          # ne sme da se izvrši

    # bne kratko: r1 != r2? ne, pa ne skače
    bne %r1, %r2, 4
    ld  $8, %r3          # izvršava se -> r3 = 8

    # bgt sa simbolom: r3 (8) > r2 (5) → skok na label
    bgt %r3, %r2, LEND
    ld  $9, %r3          # ne izvodi se
LEND:
    halt
.end
