section .data
	; Null terminated string to show end of program
	; This is 'Hello, World!'
	; prog db "++++++++[>+++++++++>+++++++++++++>++++++>++++>+++++++++++<<<<<-]>.>---.+++++++..+++.>----.>.>-.<<<.+++.------.--------.>>+.", 0
	; 1 MiB Should be enough to load most files
	prog times 1000000 db 0
	; prog_file db "files/hanoi.b", 0
	; prog_file db "files/hello_world.b", 0
	prog_file db "files/bf-standard-compliance-test.bf", 0
	; but not for LostKingdom.b, it needs at least 3 Mib
	; prog times 3000000 db 0
	; prog_file db "files/LostKingdom.b", 0
	prog_file_mode db "r", 0
	prog_file_handle dd 0
	prog_file_idx dd 0
	prog_idx dd 0

	; The optimized program
	prog_opt times 1000000 db 0
	; prog_repeat times 1000000 dw 0
	; The 30kib of program memory
	prog_mem times 30000 db 0
	; The current memory index
	prog_ptr dd 0
	; A pointer to the current cell
	curr_cell dd 0
	tmp times 100 dd 0
	scanf_str db "%c", 0

	tok_inc equ 1
	tok_dec equ 2
	tok_next equ 3
	tok_prev equ 4
	tok_print equ 5
	tok_read equ 6
	tok_loop_f equ 7
	tok_loop_b equ 8

section .text
global main
extern putchar
extern fopen
extern fgetc
extern fclose
extern scanf
extern printf

%include 'macros.asm'

main:
	call load_file
	; call main_loop
	ret

load_file:
	%push fopen
	push prog_file_mode
	push prog_file
	; eax should now be the file handle
	call fopen
	call fopen
	%pop

	cmp eax, 0
	if e
	    print "file does not exist", 0x0a, ":(", 0x0a
	    return 1
	endif

	mov [prog_file_handle], eax

	; The index of the prog_file
	mov dword [prog_file_idx], 0
	jmp read_file_loop
read_file_loop:
	push dword [prog_file_handle]
	call fgetc
	pop ebx
	cmp eax, -1
	je read_file_end
	mov edx, prog
	add edx, dword [prog_file_idx]
	mov [edx], byte al
	inc dword [prog_file_idx]
	jmp read_file_loop


read_file_end:
	push dword [prog_file_handle]
	call fclose
	mov ebx, 0
	mov ecx, 0
	call optimize
	ret

optimize:
	mov eax, prog
	add eax, ebx

	cmp byte [eax], 0
	je main_loop

	; The main switch case
	cmp byte [eax], '+'
	je .tok_add
	cmp byte [eax], '-'
	je .tok_dec
	cmp byte [eax], '>'
	je .tok_next
	cmp byte [eax], '<'
	je .tok_prev
	cmp byte [eax], '.'
	je .tok_print
	cmp byte [eax], ','
	je .tok_read
	cmp byte [eax], '['
	je .tok_loop_f
	cmp byte [eax], ']'
	je .tok_loop_b

	; Else, this ignores the char as it is probably a comment
	dec ecx
	jmp .optimize_fin

.tok_add:
	mov byte [prog_opt + ecx], tok_inc
	jmp .optimize_fin
.tok_dec:
	mov byte [prog_opt + ecx], tok_dec
	jmp .optimize_fin
.tok_next:
	mov byte [prog_opt + ecx], tok_next
	jmp .optimize_fin
.tok_prev:
	mov byte [prog_opt + ecx], tok_prev
	jmp .optimize_fin
.tok_print:
	mov byte [prog_opt + ecx], tok_print
	jmp .optimize_fin
.tok_read:
	mov byte [prog_opt + ecx], tok_read
	jmp .optimize_fin
.tok_loop_f:
	mov byte [prog_opt + ecx], tok_loop_f
	jmp .optimize_fin
.tok_loop_b:
	mov byte [prog_opt + ecx], tok_loop_b
	jmp .optimize_fin

.optimize_fin:
	inc ebx
	inc ecx
	jmp optimize

main_loop:
	; Load the current program instruction into eax
	mov eax, prog_opt
	add eax, [prog_idx]

	; Load the current cell at prog_ptr to ebx
	call load_cell

	; The main switch case
	cmp byte [eax], tok_inc ; '+'
	je .inst_add
	cmp byte [eax], tok_dec; '-'
	je .inst_dec
	cmp byte [eax], tok_next; '>'
	je .inst_next
	cmp byte [eax], tok_prev; '<'
	je .inst_prev
	cmp byte [eax], tok_print; '.'
	je .inst_print
	cmp byte [eax], tok_read; ','
	je .inst_read
	cmp byte [eax], tok_loop_f; '['
	je .inst_loop_f
	cmp byte [eax], tok_loop_b; ']'
	je .inst_loop_b
	; Else, TODO error as this is out of bounds memory
	jmp .fin
.inst_add:
	; TODO check overflow & underflow and see if I need to manually handle it
	call inc_curr_cell
	jmp .fin
.inst_dec:
	; dec byte [curr_cell]
	call dec_curr_cell
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
	; TODO handle wraparound to 30k
	dec dword [prog_ptr]
	jmp .fin
.inst_print:
	mov edx, curr_cell
	call printc_
	jmp .fin
.inst_read:
	; This is a mess
	push tmp + 10
	push scanf_str
	call scanf
	mov ebx, prog_mem
	add ebx, [prog_ptr]
	mov eax, tmp
	add eax, 10
	mov ecx, [eax]
	mov [ebx], ecx
	; Remove the pushed arguments
	pop ebx
	pop ebx
	jmp .fin
.inst_loop_f:
	mov ebx, 1
	cmp byte [curr_cell], 0
	je .goto_next_loop_f
	jmp .fin
.goto_next_loop_f:
	cmp ebx, 0
	je .fin
	inc dword [prog_idx]
	mov eax, prog_opt
	add eax, [prog_idx]
	cmp byte [eax], tok_loop_b
	je .goto_next_loop_f_close
	cmp byte [eax], tok_loop_f
	je .goto_next_loop_f_open
	jmp .goto_next_loop_f
.goto_next_loop_f_close:
	dec ebx
	jmp .goto_next_loop_f
.goto_next_loop_f_open:
	inc ebx
	jmp .goto_next_loop_f

.inst_loop_b:
	mov ebx, 1
	cmp byte [curr_cell], 0
	jne .goto_prev_loop_b
	jmp .fin

.goto_prev_loop_b:
	cmp dword ebx, 0
	je .fin
	dec dword [prog_idx]
	mov eax, prog_opt
	add eax, [prog_idx]
	cmp byte [eax], tok_loop_b
	je .goto_prev_loop_b_close
	cmp byte [eax], tok_loop_f
	je .goto_prev_loop_b_open
	jmp .goto_prev_loop_b
.goto_prev_loop_b_close:
	inc ebx
	jmp .goto_prev_loop_b
.goto_prev_loop_b_open:
	dec ebx
	jmp .goto_prev_loop_b

.fin:
	inc dword [prog_idx]
	mov eax, prog_opt
	add eax, [prog_idx]
	; Check if the current instruction is 0, the end of the program
	cmp byte [eax], 0
	; If it isnt, repeat the loop
	jne main_loop
	; If it is, print a newline to flush the print buffer, then exit with code 0
	printc 0x0a

	return 0

	; Prints from eax
printc_:
	push word [edx]
	call putchar
	; Remove the value of edx that I pushed
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

; TODO replace both of these with a curr_cell variable that, after the switch statement is what prog_mem+[prog_ptr] is set to
inc_curr_cell:
	mov ebx, prog_mem
	; Add the offset to the current cell
	add ebx, [prog_ptr]
	inc byte [ebx]
	ret

dec_curr_cell:
	mov ebx, prog_mem
	; Add the offset to the current cell
	add ebx, [prog_ptr]
	dec byte [ebx]
	ret
