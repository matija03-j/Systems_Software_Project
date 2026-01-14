.section text
.extern value1

# isr_software:
#   value1 = 0xABCD
#   ret
isr_software:
    ld  $0x0000ABCD, %r1       # pool-imm test
    st  %r1, value1            # mem dir + relokacija
    ret
.end
