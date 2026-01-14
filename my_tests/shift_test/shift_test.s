.section text
    ld  $1, %r1
    ld  $3, %r2        # broj pomeraja

    shl %r1, %r2       # r1 <<= 3 -> 8
    shr %r1, %r2       # r1 >>= 3 -> 1

    halt
.end
