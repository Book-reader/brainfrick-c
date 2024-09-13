section .data
	; Null terminated string to show end of program
	; This is 'Hello, World!'
	; prog db "++++++++[>+++++++++>+++++++++++++>++++++>++++>+++++++++++<<<<<-]>.>---.+++++++..+++.>----.>.>-.<<<.+++.------.--------.>>+.", 0
	prog db "+++++++++++++++++++++++++++++++++++++++++.+.>++++++++++++++++++++++++++++++++++++++.", 0
	prog_idx dd 0
	; The current 
	curr_inst db 0
	; The 30kib of program memory
	prog_mem times 30000 db 0
	; The current memory index
	prog_ptr dd 0
	; A pointer to the current cell
	curr_cell dd 0

section .text
global main
extern putchar

main:
	call loop
	ret

loop:
	; Load the current program instruction into eax
	mov eax, prog
	add eax, [prog_idx]
	mov ebx, [eax]
	mov [curr_inst], ebx

	; Print the current instruction
	; call printc

	; Load the current cell at prog_ptr to ebx
	call load_cell

	; The main switch case
	cmp byte [curr_inst], '+'
	je .inst_add
	cmp byte [curr_inst], '-'
	je .inst_dec
	cmp byte [curr_inst], '>'
	je .inst_next
	cmp byte [curr_inst], '<'
	je .inst_prev
	cmp byte [curr_inst], '.'
	je .inst_print
	cmp byte [curr_inst], ','
	je .inst_read
	cmp byte [curr_inst], '['
	je .inst_loop_f
	cmp byte [curr_inst], ']'
	je .inst_loop_b
	; Else, this ignores the char as it is probably a comment
	; TODO: remove printc
	; mov edx, eax
	; call printc
	jmp .fin
.inst_add:
	; TODO check overflow & underflow and see if I need to manually handle it
	inc byte [curr_cell]
	jmp .fin
.inst_dec:
	dec byte [curr_cell]
	jmp .fin
.inst_next:
	; TODO handle reaching 30,000
	cmp dword [prog_ptr], 30000
	je .inst_next_overflow
	inc dword [prog_ptr]
	jmp .fin
.inst_next_overflow:
	mov dword [prog_ptr], 0
	jmp .fin
.inst_prev:
	dec dword [prog_ptr]
	jmp .fin
.inst_print:
	mov edx, curr_cell
	call printc
	jmp .fin
.inst_read:
	; TODO implement
	jmp .fin
.inst_loop_f:
	;cmp dword [curr_cell], 0
	;je .goto_next_loop_b
	jmp .fin
.inst_loop_b:
	jmp .fin
.fin:
	; Increment the prog_idx
	inc dword [prog_idx]
	mov eax, prog
	add eax, [prog_idx]
	; Check if the current instruction is 0, the end of the program
	cmp word [eax], 0
	; If it isnt, repeat the loop
	jne loop
	; If it is, break
	ret

	; Prints from eax
printc:
	push word [edx]
	call putchar
	; Remove the return value of putchar from the stack and store it in dx, it is never used
	; If I don't do this it will add data to the stack and I will have to add 4 to esp to show the stack has had data added
	pop word dx
	ret

	; Load the current program cell
load_cell:
	; Get the start of program memory
	mov ebx, prog_mem
	; Add the offset to the current cell
	add ebx, [prog_ptr]
	; Store the actual pointer to the cell in ecx, Im not entirely sure why I have to do this
	mov ecx, [ebx]
	; Store the pointer to the cell in curr_cell
	mov [curr_cell], ecx
	ret
