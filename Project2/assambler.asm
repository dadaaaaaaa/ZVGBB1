.MODEL FLAT, STDCALL
 OPTION CASEMAP : NONE
 EXTERN ExitProcess@4: PROC
.DATA
a dd 0
b dd 0
const_2_0 dd 2
const_4_0 dd 4

.CODE
MAIN PROC
FILD const_2_0
FISTP a
; Присвоение a
FILD const_4_0
FISTP b
; Присвоение b
WHILE_START_0:
; Вычисление условия while
MOV  EAX, [a] 
MOV  EAX, [b] 
CMP EAX, EBX
JNE WHILE_END_1
JMP WHILE_START_0
WHILE_END_1:
PUSH 0
CALL ExitProcess@4
 MAIN ENDP
END MAIN
