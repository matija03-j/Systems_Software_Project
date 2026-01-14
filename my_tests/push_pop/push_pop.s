.section text
    ld  $0xF0000100, %r14   # sp init
    ld  $0xDEADBEEF, %r3

    push %r3
    pop  %r4

    halt
.end
