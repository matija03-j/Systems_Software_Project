.section text
.extern value1, value2

# bump_value2:
#   r1 = mem32[value1]
#   r1 = r1 + 1
#   mem32[value2] = r1
#   ret
bump_value2:
    ld  value1, %r1            # mem dir + relok
    ld  $1, %r2
    add %r2, %r1               # r1 += r2
    st  %r1, value2
    ret
.end
