.section text
    # inic SP i postavi vektor prekida
    ld  $0xF0001000, %sp
    ld  $0xF0000000, %r1
    csrwr %r1, %handler

    # neka nešto stoji u statusu (nije obavezno)
    ld  $0xA5A5A5A5, %r2
    csrwr %r2, %status

    # softverski prekid
    int

RESUME:
    # ako je iret odradio posao, doći ćemo ovde
    halt

# Prekidna rutina – smesti na 0xF0000000 sa -place opcijom linkera
.section isr
ISR:
    # uradi nešto vidljivo
    ld  $0xDEADCAFE, %r5
    iret
.end
