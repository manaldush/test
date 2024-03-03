    bits 64
    extern malloc, puts, printf, fflush, abort, free
    global main

    section   .data
empty_str: db 0x0
int_format: db "%ld ", 0x0
data: dq 4, 8, 15, 16, 23, 42
data_length: equ ($-data) / 8

    section   .text
;;; print_int proc
print_int:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    mov rsi, rdi
    mov rdi, int_format
    xor rax, rax
    call printf

    xor rdi, rdi
    call fflush

    mov rsp, rbp
    pop rbp
    ret

;;; free_list proc
free_list:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    push r12
    push rdi

repeat_free:
    test rdi, rdi
    jz outfree
    mov r12, [rdi+8]
    mov rcx, rdi
    call free
    mov rdi, r12
    jmp repeat_free

outfree:

    pop rdi
    pop r12

    mov rsp, rbp
    pop rbp
    ret

;;; p proc
p:
    mov rax, rdi
    and rax, 1
    ret

;;; add_element proc
add_element:
    push rbp
    push rbx
    push r14
    mov rbp, rsp
    sub rsp, 16

    mov r14, rdi
    mov rbx, rsi

    mov rdi, 16
    call malloc
    test rax, rax
    jz abort

    mov [rax], r14
    mov [rax + 8], rbx

    mov rsp, rbp
    pop r14
    pop rbx
    pop rbp
    ret

;;; m proc
m:
    push rbp
    mov rbp, rsp
    sub rsp, 16

    test rdi, rdi
    jz outm

    push rbp
    push rbx
# заменяем call на jmp, экономим размер стека + jmp эффективнее
repeatm:
    mov rbx, rdi
    mov rbp, rsi
    mov rdi, [rdi]
    call rsi

    mov rdi, [rbx + 8]
    mov rsi, rbp
    test rdi, rdi
    jnz repeatm

    pop rbx
    pop rbp

outm:
    mov rsp, rbp
    pop rbp
    ret

;;; f proc
f:
    mov rax, rsi

    test rdi, rdi
    jz outf

    push rbx
    push r12
    push r13

repeatf:
    mov rbx, rdi
    mov r12, rsi
    mov r13, rdx

    mov rdi, [rdi]
    call rdx
    test rax, rax
    jz z

    mov rdi, [rbx]
    mov rsi, r12
    call add_element
    mov rsi, rax
    jmp ff

z:
    mov rsi, r12

ff:
    mov rdi, [rbx + 8]
    mov rdx, r13
    test rdi, rdi
# заменяем call на jmp, экономим размер стека + jmp эффективнее
    jnz repeatf

    mov rax, rsi
    pop r13
    pop r12
    pop rbx

outf:
    ret

;;; main proc
main:
    push rbx

    xor rax, rax
    mov rbx, data_length
adding_loop:
    mov rdi, [data - 8 + rbx * 8]
    mov rsi, rax
    call add_element
    dec rbx
    jnz adding_loop

    push rax
    mov rbx, rax

    mov rdi, rax
    mov rsi, print_int
    call m

    mov rdi, empty_str
    call puts

    mov rdx, p
    xor rsi, rsi
    mov rdi, rbx
    call f

    push rax
    mov rdi, rax
    mov rsi, print_int
    call m

    mov rdi, empty_str
    call puts

#memory leak fix
    pop rdi
    call free_list
    pop rdi
    call free_list

    pop rbx

    xor rax, rax
    ret
