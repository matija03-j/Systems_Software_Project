.section text
    # r1 = 0x11223344  (immediate)
    ld  $0x11223344, %r1

    # r2 = r1 (reg-dir)
    ld  %r1, %r2

    push %r1
    push %r15

    # pripremi adresu u r5 = 0xF0000000
    ld  %r14, %r5

    # mem32[r5] = 0xAABBCCDD (pomoću store testa, ili pretpostavi linker/emulator niz)
    # ovde samo testiramo load reg-ind i reg-ind+disp:
    ld  [%r5], %r3              # r3 = mem32[r5]
    ld  [%r5 + 4], %r4        # r4 = mem32[r5 + 4]

    halt
.end
