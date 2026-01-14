.section text
.extern handler
.extern bump_value2
.extern value1, value2, value3

start:
    # 1) SP init (veliki literal -> pool LD)
    ld  $0xF0001100, %sp

    # 2) Postavi handler CSR (pool LD + CSRWR)
    ld  $handler, %r1
    csrwr %r1, %handler

    # 3) Software interrupt -> handler -> isr_software -> iret
    
    int

    # 4) Pozovi "biblioteku" koja value2 = value1 + 1
    call bump_value2

    # 5) Kratko grananje (disp12 fituje):
    #    Ako je value1 == 0xABCD, preskoči patch value3
    ld  value1, %r1
    ld  $0x0000ABCD, %r2
    beq %r1, %r2, ok_value1

    # (fallback) – inače value3 = 0xDEAD
    ld  $0x0000DEAD, %r3
    st  %r3, value3

ok_value1:
    # 6) “Pool” jmp na simbol (uvek MOD4) -> skok na etiketirani epilog
    jmp end_marker

    # 7) Ako “jmp” ne radi – nikad ne bi trebalo do ovde:
    ld $0xBAD, %r4
    st %r4, value3

end_marker:
    # 8) Učitaj rezultate da ih emulator ispiše
    ld  value1, %r1     # očekujemo 0xABCD
    ld  value2, %r2     # očekujemo 0xABCE
    ld  value3, %r3     # očekujemo 0x00000000 ako BEQ i JMP rade

    halt
.end
