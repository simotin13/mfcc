section .text
global add
global main

add:
	push rbp
	mov rbp, rsp
	mov rax, [rbp + 0x18]
	push rax
	mov rax, [rbp + 0x10]
	push rax
	pop rbx
	pop rax
	add rax, rbx
	push rax
	pop rax
	mov rsp, rbp
	pop rbp
	ret

main:
	push rbp
	mov rbp, rsp
	sub rsp, 4
	push 2
	push 1
	call add
	mov [rbp - 0x4], rax
	mov rax, [rbp - 0x4]
	push rax
	pop rax
	add rsp, 0x4
	mov rsp, rbp
	pop rbp
	ret

