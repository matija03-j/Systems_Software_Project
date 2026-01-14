.section text
    # test jmp literal (PC-relativno)
    ld  $1, %r1
    jmp L1               # MOD4 (pool) u tvom asembleru za simbol
    ld  $2, %r2          # preskače se
L1:
    # sad test jmp literal mali – +4 bajta
    jmp 4
    ld  $3, %r3          # preskače se
    ld  $4, %r5          # izvršava se

    halt
.end
