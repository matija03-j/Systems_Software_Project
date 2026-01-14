.section text
    ld  $0xF0000000, %r0
    csrwr %r0, %handler

    ld  $0x11111111, %r3

    int                 # ulaz u prekid

    ld  $0x22222222, %r3    # mora da se izvrši posle IRET-a
    halt

.section isr
ISR:
    ld  $0x33333333, %r4    # bilo šta, samo da vidimo da smo bili ovde
    iret
.end
