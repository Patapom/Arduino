	.file	"pulseTrainASM.cpp"
__SP_H__ = 0x3e
__SP_L__ = 0x3d
__SREG__ = 0x3f
__tmp_reg__ = 0
__zero_reg__ = 1
 
.global	pulseTrainASM
	.type	pulseTrainASM, @function
pulseTrainASM:
	.stabn	68,0,1,.LM0-.LFBB1
.LM0:
.LFBB1:
	push r4	 ; 	 ;  168	pushqi1/1	[length = 1]
	push r5	 ; 	 ;  169	pushqi1/1	[length = 1]
	push r6	 ; 	 ;  170	pushqi1/1	[length = 1]
	push r7	 ; 	 ;  171	pushqi1/1	[length = 1]
	push r10	 ; 	 ;  172	pushqi1/1	[length = 1]
	push r11	 ; 	 ;  173	pushqi1/1	[length = 1]
	push r12	 ; 	 ;  174	pushqi1/1	[length = 1]
	push r13	 ; 	 ;  175	pushqi1/1	[length = 1]
	push r14	 ; 	 ;  176	pushqi1/1	[length = 1]
	push r15	 ; 	 ;  177	pushqi1/1	[length = 1]
	push r16	 ; 	 ;  178	pushqi1/1	[length = 1]
	push r17	 ; 	 ;  179	pushqi1/1	[length = 1]
	push r28	 ; 	 ;  180	pushqi1/1	[length = 1]
	push r29	 ; 	 ;  181	pushqi1/1	[length = 1]
/* prologue: function */
/* frame size = 0 */
/* stack size = 14 */
.L__stack_usage = 14
	mov r30,r24	 ;  port, port	 ;  21	*movhi/1	[length = 2]
	mov r31,r25	 ;  port, port
	mov r26,r18	 ;  _maxPulses, _maxPulses	 ;  24	*movhi/1	[length = 2]
	mov r27,r19	 ;  _maxPulses, _maxPulses
	mov r4,r14	 ;  _endTrainTimeOutLoopsCount, _endTrainTimeOutLoopsCount	 ;  25	*movsi/1	[length = 4]
	mov r5,r15	 ;  _endTrainTimeOutLoopsCount, _endTrainTimeOutLoopsCount
	mov r6,r16	 ;  _endTrainTimeOutLoopsCount, _endTrainTimeOutLoopsCount
	mov r7,r17	 ;  _endTrainTimeOutLoopsCount, _endTrainTimeOutLoopsCount
	mov r19,r13	 ;  _timeOutLoopsCount, _timeOutLoopsCount	 ;  26	*movsi/1	[length = 4]
	mov r18,r12	 ;  _timeOutLoopsCount, _timeOutLoopsCount
	mov r17,r11	 ;  _timeOutLoopsCount, _timeOutLoopsCount
	mov r16,r10	 ;  _timeOutLoopsCount, _timeOutLoopsCount
.LBB2:
	.stabn	68,0,10,.LM1-.LFBB1
.LM1:
	sbiw r26,0	 ;  _maxPulses,	 ;  33	*cmphi/1	[length = 1]
	brne .+2	 ; 	 ;  34	branch	[length = 2]
	rjmp .L6	 ; 
	mov r28,r20	 ; ,	 ;  161	movqi_insn/1	[length = 1]
	mov r29,r21	 ; ,	 ;  162	movqi_insn/1	[length = 1]
	mov r20,r26	 ;  _maxPulses, _maxPulses	 ;  28	*movhi/1	[length = 2]
	mov r21,r27	 ;  _maxPulses, _maxPulses
	rjmp .L4	 ; 	 ;  212	jump	[length = 1]
.L5:
	.stabn	68,0,13,.LM2-.LFBB1
.LM2:
	subi r16,1	 ;  _timeOutLoopsCount,	 ;  39	addsi3/2	[length = 4]
	sbc r17,__zero_reg__	 ;  _timeOutLoopsCount
	sbc r18,__zero_reg__	 ;  _timeOutLoopsCount
	sbc r19,__zero_reg__	 ;  _timeOutLoopsCount
	brne .+2	 ; 	 ;  41	branch	[length = 2]
	rjmp .L3	 ; 
.L4:
	.stabn	68,0,12,.LM3-.LFBB1
.LM3:
	ld r25,Z	 ;  D.1988, *port_15(D)	 ;  51	movqi_insn/4	[length = 1]
	and r25,r22	 ;  D.1988, _bit	 ;  53	andqi3/1	[length = 1]
	brne .L5	 ; ,	 ;  55	branch	[length = 1]
	.stabn	68,0,17,.LM4-.LFBB1
.LM4:
	cp r16,__zero_reg__	 ;  _timeOutLoopsCount	 ;  57	*cmpsi/1	[length = 4]
	cpc r17,__zero_reg__	 ;  _timeOutLoopsCount
	cpc r18,__zero_reg__	 ;  _timeOutLoopsCount
	cpc r19,__zero_reg__	 ;  _timeOutLoopsCount
	breq .L3	 ; ,	 ;  58	branch	[length = 1]
	.stabn	68,0,22,.LM5-.LFBB1
.LM5:
	mov r15,r7	 ; , _endTrainTimeOutLoopsCount	 ;  163	*movsi/1	[length = 4]
	mov r14,r6	 ; , _endTrainTimeOutLoopsCount
	mov r13,r5	 ; , _endTrainTimeOutLoopsCount
	mov r12,r4	 ; , _endTrainTimeOutLoopsCount
	sub r12,r16	 ; , _timeOutLoopsCount	 ;  61	subsi3/1	[length = 4]
	sbc r13,r17	 ; , _timeOutLoopsCount
	sbc r14,r18	 ; , _timeOutLoopsCount
	sbc r15,r19	 ; , _timeOutLoopsCount
	st Y,r12	 ;  MEM[base: _42, offset: 0B], D.1989	 ;  62	*movsi/4	[length = 4]
	std Y+1,r13	 ;  MEM[base: _42, offset: 0B], D.1989
	std Y+2,r14	 ;  MEM[base: _42, offset: 0B], D.1989
	std Y+3,r15	 ;  MEM[base: _42, offset: 0B], D.1989
	.stabn	68,0,25,.LM6-.LFBB1
.LM6:
	cpi r20,1	 ;  _maxPulses,	 ;  64	*cmphi/6	[length = 2]
	cpc r21,__zero_reg__	 ;  _maxPulses
	breq .L6	 ; ,	 ;  65	branch	[length = 1]
	mov r19,r7	 ;  _endTrainTimeOutLoopsCount, _endTrainTimeOutLoopsCount	 ;  132	*movsi/1	[length = 4]
	mov r18,r6	 ;  _endTrainTimeOutLoopsCount, _endTrainTimeOutLoopsCount
	mov r17,r5	 ;  _endTrainTimeOutLoopsCount, _endTrainTimeOutLoopsCount
	mov r16,r4	 ;  _endTrainTimeOutLoopsCount, _endTrainTimeOutLoopsCount
	rjmp .L9	 ; 	 ;  214	jump	[length = 1]
.L10:
	.stabn	68,0,30,.LM7-.LFBB1
.LM7:
	subi r16,1	 ;  _endTrainTimeOutLoopsCount,	 ;  70	addsi3/2	[length = 4]
	sbc r17,__zero_reg__	 ;  _endTrainTimeOutLoopsCount
	sbc r18,__zero_reg__	 ;  _endTrainTimeOutLoopsCount
	sbc r19,__zero_reg__	 ;  _endTrainTimeOutLoopsCount
	breq .L8	 ; ,	 ;  72	branch	[length = 1]
.L9:
	.stabn	68,0,29,.LM8-.LFBB1
.LM8:
	ld r25,Z	 ;  D.1988, *port_15(D)	 ;  80	movqi_insn/4	[length = 1]
	and r25,r22	 ;  D.1988, _bit	 ;  82	andqi3/1	[length = 1]
	breq .L10	 ; ,	 ;  84	branch	[length = 1]
.L8:
	.stabn	68,0,39,.LM9-.LFBB1
.LM9:
	mov r15,r7	 ; , _endTrainTimeOutLoopsCount	 ;  165	*movsi/1	[length = 4]
	mov r14,r6	 ; , _endTrainTimeOutLoopsCount
	mov r13,r5	 ; , _endTrainTimeOutLoopsCount
	mov r12,r4	 ; , _endTrainTimeOutLoopsCount
	sub r12,r16	 ; , _endTrainTimeOutLoopsCount	 ;  87	subsi3/1	[length = 4]
	sbc r13,r17	 ; , _endTrainTimeOutLoopsCount
	sbc r14,r18	 ; , _endTrainTimeOutLoopsCount
	sbc r15,r19	 ; , _endTrainTimeOutLoopsCount
	std Y+4,r12	 ;  MEM[base: _42, offset: 4B], D.1989	 ;  88	*movsi/4	[length = 4]
	std Y+5,r13	 ;  MEM[base: _42, offset: 4B], D.1989
	std Y+6,r14	 ;  MEM[base: _42, offset: 4B], D.1989
	std Y+7,r15	 ;  MEM[base: _42, offset: 4B], D.1989
	.stabn	68,0,41,.LM10-.LFBB1
.LM10:
	subi r20,2	 ;  _maxPulses,	 ;  89	addhi3_clobber/2	[length = 2]
	sbc r21,__zero_reg__	 ;  _maxPulses
	adiw r28,8	 ;  ivtmp.26,	 ;  90	addhi3_clobber/1	[length = 1]
	.stabn	68,0,40,.LM11-.LFBB1
.LM11:
	mov r19,r7	 ;  _timeOutLoopsCount, _endTrainTimeOutLoopsCount	 ;  133	*movsi/1	[length = 4]
	mov r18,r6	 ;  _timeOutLoopsCount, _endTrainTimeOutLoopsCount
	mov r17,r5	 ;  _timeOutLoopsCount, _endTrainTimeOutLoopsCount
	mov r16,r4	 ;  _timeOutLoopsCount, _endTrainTimeOutLoopsCount
	.stabn	68,0,10,.LM12-.LFBB1
.LM12:
	cp r20,__zero_reg__	 ;  _maxPulses	 ;  93	*cmphi/2	[length = 2]
	cpc r21,__zero_reg__	 ;  _maxPulses
	breq .+2	 ; 	 ;  94	branch	[length = 2]
	rjmp .L4	 ; 
.L6:
	ldi r20,0	 ;  _maxPulses	 ;  30	*movhi/2	[length = 2]
	ldi r21,0	 ;  _maxPulses
.L3:
.LBE2:
	.stabn	68,0,45,.LM13-.LFBB1
.LM13:
	mov r24,r26	 ; , _maxPulses	 ;  167	*movhi/1	[length = 2]
	mov r25,r27	 ; , _maxPulses
	sub r24,r20	 ; , _maxPulses	 ;  105	subhi3/1	[length = 2]
	sbc r25,r21	 ; , _maxPulses
/* epilogue start */
	pop r29	 ; 	 ;  184	popqi	[length = 1]
	pop r28	 ; 	 ;  185	popqi	[length = 1]
	pop r17	 ; 	 ;  186	popqi	[length = 1]
	pop r16	 ; 	 ;  187	popqi	[length = 1]
	pop r15	 ; 	 ;  188	popqi	[length = 1]
	pop r14	 ; 	 ;  189	popqi	[length = 1]
	pop r13	 ; 	 ;  190	popqi	[length = 1]
	pop r12	 ; 	 ;  191	popqi	[length = 1]
	pop r11	 ; 	 ;  192	popqi	[length = 1]
	pop r10	 ; 	 ;  193	popqi	[length = 1]
	pop r7	 ; 	 ;  194	popqi	[length = 1]
	pop r6	 ; 	 ;  195	popqi	[length = 1]
	pop r5	 ; 	 ;  196	popqi	[length = 1]
	pop r4	 ; 	 ;  197	popqi	[length = 1]
	ret	 ;  198	return_from_epilogue	[length = 1]
	.size	pulseTrainASM, .-pulseTrainASM
.Lscope1:
	.stabs	"",100,0,0,.Letext0
.Letext0:
	.ident	"GCC: (GNU) 4.9.2"
