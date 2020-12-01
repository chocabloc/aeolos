.section .rodata
.global term_font
.type term_font, @object
.align 8

term_font:
    .incbin "dev/term/font.psf"
