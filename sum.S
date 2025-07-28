; Código Assembly 16-bit (NASM)

section .text

sums():
    ; ignorado: 
    cmp a_sums(), 10
    jne if_end_0
    ; ... (bloco do if)
    if_end_0:
    call imprimir();
    mov ax, a;
    ret

imprimir():
    ; ignorado: 
    mov ax, 0;
    ret

section .data

a_sums(): dw 10
msg_sums(): db 'Olá!', 0
imprimir();:
    ; chamada indefinida
    ret

