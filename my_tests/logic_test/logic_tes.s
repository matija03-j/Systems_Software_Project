.section text
    ld  $0x00FF00FF, %r1
    ld  $0x0F0F0F0F, %r2

    not %r1            # r1 = ~0x00FF00FF = 0xFF00FF00
    and %r1, %r2       # r1 = r1 & r2 = 0x0F000F00
    or  %r1, %r2       # r1 = r1 | r2 = 0x0F0F0F0F
    xor %r1, %r2       # r1 = 0x00000000

    halt
.end
