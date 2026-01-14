.section text
    # init SP (ako emulator ne postavlja automatski)

    ld  $10, %r1
    call FUNC
    # posle povratka:
    add %r1, %r1         # r1 = r1 + r1  (ako je r1=11, sada 22)
    halt

FUNC:
    add %r1, %r1         # r1 = 10 + 10 = 20
    sub %r1, %r14        # besmisleno, ali neka je neutralno (opciono ukloni)
    ld  $3, %r1         # postavi r1 = 11
    ret
.end
