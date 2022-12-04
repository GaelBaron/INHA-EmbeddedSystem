		AREA sum69, CODE 
		EXPORT __main 
		ALIGN 
		ENTRY 

__main	PROC	 
		MOVS r0,#0x06 
		MOVS r1,#0x05 
		MOVS r3,#0x04 
		MOVS r2,#0x03 
		PUSH {r0,r1} 
		MOVS r1,#0x02 
		MOVS r0,#0x01		 
		
		MOVS r6, #0x00
		ADD r6, r0, r6
		ADD r6, r1, r6
		ADD r6, r2, r6
		ADD r6, r3, r6
stop	B    stop 
		ENDP

Reset_Handler   PROC
		EXPORT Reset_Handler [Weak]
		LDR R0, =__main
		BX  R0
		ENDP

		END