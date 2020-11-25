.section .rodata
.global fbcon_font
.type fbcon_font, @object
.align 8

fbcon_font:
    .incbin "drivers/fbcon/fbcon_font.psf"
