Loop:
; assign phase accumulator as r28 L r29 M r30 H
; assign phase adder as  r24 L, r25 M, r26 H
; The r30 is ZL, it is Z register(LPM pointer) 's low 8bit, so it can 
; natually be the ROM phase selector and without judgement on the phase accumlator
; value.
; The output frequency is fs * (phase_adder / (2 ^ phase_adder_bitwidth))
; but according to sampling theorem, the maximum output frequency is fs/2
; or more than fs/2
; if you want to design a more performance filter and let it keep simple.
; please use 40% or less fs bandwidth.
; the whole loop uses 7 clock cycles, which means than if the mcu is running
; at 16MHz, you will get 2.5MS/s(approx) sample rate
; Have fun!
	add ph_acc_l, add_l ;1
	adc ph_acc_m, add_m ;1
	add ph_acc_h, add_h ;1 (ph_acc_h = ZL)
	lpm Z ;2 r0 <- data
	out PORTA, r0 ;1
	rjmp loop ;1
