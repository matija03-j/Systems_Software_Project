.section text
    ld  $3, %r1        # r1 = 3
    ld  $4, %r2        # r2 = 4

    add %r1, %r2       # r1 = r1 + r2 = 7
    sub %r1, %r2       # r1 = 7 - 4 = 3
    mul %r1, %r2       # r1 = 3 * 4 = 12
    div %r1, %r2       # r1 = 12 / 4 = 3

    halt
.end
