%macro if 1
	
    %push if
    j%-1  %$ifnot

%endmacro

%macro else 0

  %ifctx if
        %repl   else
        jmp     %$ifend
        %$ifnot:
  %else
        %error  "expected `if' before `else'"
  %endif

%endmacro

%macro endif 0

  %ifctx if
        %$ifnot:
        %pop
  %elifctx      else
        %$ifend:
        %pop
  %else
        %error  "expected `if' or `else' before `endif'"
  %endif

%endmacro

%macro printc 1
	push edx
	mov edx, tmp
	mov byte [edx], %1
	call printc_
	pop edx
%endmacro

%macro print 1+
%push printing
	; push eax
	jmp .aa
	%%str db %1, 0
.aa:
	push 0
	push %%str
	call printf
%pop
%endmacro

%macro return 1
	mov eax,1
    mov ebx, %1
    int 0x80
%endmacro
