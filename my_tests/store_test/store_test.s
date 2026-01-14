.section text
    ld  $0xF0000200, %r5     # baza
    ld  $0xCAFEBABE, %r7

    # [r5]   = r7
    st  %r7, [%r5]

    # [r5+4] = r7
    st  %r7, [%r5 + 4]

    # mem-dir: [0xF0000210] = r7    (u tvom asembleru biće emitovan pool + store)
    st  %r7, 0xF0000210

    ld 0xF0000200, %r1
    ld 0xF0000204, %r2
    ld 0xF0000210, %r3

    halt
.end
