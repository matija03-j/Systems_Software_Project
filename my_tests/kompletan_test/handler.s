.section text
.extern isr_software

handler:
    # r2 = 4
    ld  $4, %r2
    # r1 = cause
    csrrd %cause, %r1

    # if (r1 == 4) -> soft
    beq %r1, %r2, soft

    # default: iret
    iret

soft:
    call isr_software          # pool-call preko simbola
    iret
.end
