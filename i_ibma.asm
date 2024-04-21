;
; Copyright (C) 2024 Frenkel Smeijers
;
; This program is free software; you can redistribute it and/or
; modify it under the terms of the GNU General Public License
; as published by the Free Software Foundation; either version 2
; of the License, or (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License
; along with this program. If not, see <https://www.gnu.org/licenses/>.
;

cpu 8086

bits 16

extern source
extern nearcolormap
extern dest

last_pixel_jump_table:
	dw last_pixel0,
	dw last_pixel1,
	dw last_pixel2,
	dw last_pixel3,
	dw last_pixel4,
	dw last_pixel5,
	dw last_pixel6,
	dw last_pixel7,
	dw last_pixel8,
	dw last_pixel9,
	dw last_pixel10,
	dw last_pixel11,
	dw last_pixel12,
	dw last_pixel13,
	dw last_pixel14,
	dw last_pixel15

;
; input:
;   ax = fracstep
;   dx = frac
;   cx = count		1 <= count <= 128	=>	ch = 0
;

global R_DrawColumn2
R_DrawColumn2:
	push si
	push di
	push es
	push bp

	xchg bp, ax						; bp = fracstep

	mov di, [dest]					; ds:di = dest
	les si, [source]				; es:si = source

	mov ah, cl						; ah = count
	shr cl, 1
	shr cl, 1
	shr cl, 1
	shr cl, 1						; 0 <= cl <= 8 && ch = 0

	or cx, cx						; if cx = 0
	jz last_pixels					;  then jump to last_pixels

loop_pixels:
	push cx
	mov cx, nearcolormap

	mov al, dh						; al = hi byte of frac
	shr al, 1						; 0 <= al <= 127
	mov bx, si						; bx = source
	es xlat							; al = source[al]
	mov bx, cx						; bx = nearcolormap
	xlat							; al = nearcolormap[al]
	mov [di+0*VIEWWINDOWWIDTH], al	; write pixel
	add dx, bp						; frac += fracstep

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+1*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+2*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+3*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+4*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+5*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+6*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+7*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+8*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+9*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+10*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+11*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+12*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+13*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+14*VIEWWINDOWWIDTH], al
	add dx, bp

	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di+15*VIEWWINDOWWIDTH], al
	add dx, bp

	add di, 16*VIEWWINDOWWIDTH
	pop cx
	dec cx
	jnz loop_pixels					; if --cx != 0 then jump to loop_pixels


last_pixels:
	mov cx, nearcolormap
	and ah, 15						; 0 <= count <= 15
	xor bh, bh
	mov bl, ah
	shl bx, 1
	cs jmp last_pixel_jump_table[bx]


last_pixel15:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel14:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel13:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel12:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel11:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel10:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel9:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel8:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel7:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel6:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel5:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel4:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel3:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel2:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al
	add di, VIEWWINDOWWIDTH
	add dx, bp

last_pixel1:
	mov al, dh
	shr al, 1
	mov bx, si
	es xlat
	mov bx, cx
	xlat
	mov [di], al

last_pixel0:
	pop bp
	pop es
	pop di
	pop si
	retf


; Divide FFFFFFFFh by a 32-bit number.
; Based on https://github.com/FDOS/kernel/blob/master/kernel/ludivmul.inc
;
; input:
;   dx:ax = divisor
;
; output:
;   dx:ax = quotient of division of FFFFFFFFh by divisor

global FixedReciprocal
FixedReciprocal:
	xchg    bx, ax             ; bx = divisor-lo
	mov     ax, 0ffffh         ; ax = FFFFh

	test    dx, dx             ; divisor > 2^16-1 ?
	jnz     big_divisor        ; yes, divisor > 2^16-1

	mov     cx, ax             ; ax = FFFFh, bx = divisor-lo, cx = FFFFh, dx = 0
	div     bx                 ; quotient-hi in ax
	xchg    ax, cx             ; cx = quotient-hi, ax = FFFFh

	div     bx                 ; ax = quotient-lo
	mov     dx, cx             ; dx = quotient-hi
	retf                       ; return quotient in dx:ax

big_divisor:
	mov     cx, dx             ; cx = divisor-hi
	mov     dx, ax             ; ax = FFFFh, bx = divisor-lo, cx = divisor-hi, dx = FFFFh

	push    si                 ; save temp
	push    di                 ;  variables
	mov     si, bx             ; divisor now in
	mov     di, cx             ;  di:si and cx:bx
shift_loop:
	shr     dx, 1              ; shift both dividend = FFFFFFFFh
	shr     cx, 1              ;  and divisor
	rcr     bx, 1              ;   right by 1 bit
	jnz     shift_loop         ;    loop if di non-zero (rcr does not touch ZF)
	div     bx                 ; compute quotient FFFFh:FFFFh>>x / cx:bx>>x (stored in ax; remainder in dx not used)
	mov     cx, ax             ; save quotient
	mul     di                 ; quotient * divisor hi-word (low only)
	not     ax                 ; dividend high - divisor high * quotient, no overflow (carry/borrow) possible here
	mov     bx, ax             ; save dividend high
	mov     ax, cx             ; ax=quotient
	mul     si                 ; quotient * divisor lo-word
	mov     ax, cx             ; get quotient
	sub     bx, dx             ; subtract (divisor-lo * quot.)-hi from dividend-hi
	sbb     dx, dx             ; 0 if remainder > 0, else FFFFFFFFh
	add     ax, dx             ; correct quotient if necessary           ax += dx
	xor     dx, dx             ; clear hi-word of quot (ax<=FFFFh) dx := 0
	pop     di                 ; restore temp  
	pop     si                 ;  variables
	retf                       ; return quotient in dx:ax


; Divide FFFFFFFFh by a 16-bit number.
;
; input:
;   ax = divisor
;
; output:
;   dx:ax = quotient of division of FFFFFFFFh by divisor

global FixedReciprocalSmall
FixedReciprocalSmall:
	xchg    bx, ax             ; bx = divisor
	mov     ax, 0ffffh         ;
	mov     cx, ax             ;
	xor     dx, dx             ; ax = FFFFh, bx = divisor, cx = FFFFh, dx = 0
	div     bx                 ; quotient-hi in ax
	xchg    ax, cx             ; cx = quotient-hi, ax = FFFFh

	div     bx                 ; ax = quotient-lo
	mov     dx, cx             ; dx = quotient-hi
	retf                       ; return quotient in dx:ax


; Divide FFFFFFFFh by a 32-bit number.
;
; input:
;   dx:ax = divisor, dx != 0
;
; output:
;   ax = quotient of division of FFFFFFFFh by divisor

global FixedReciprocalBig
FixedReciprocalBig:
	xchg    bx, ax             ; bx = divisor-lo
	mov     cx, dx             ; cx = divisor-hi
	mov     ax, 0ffffh         ; ax = FFFFh
	mov     dx, ax             ; ax = FFFFh, bx = divisor-lo, cx = divisor-hi, dx = FFFFh

	push    si                 ; save temp
	push    di                 ;  variables
	mov     si, bx             ; divisor now in
	mov     di, cx             ;  di:si and cx:bx
shift_loop_big:
	shr     dx, 1              ; shift both dividend = FFFFFFFFh
	shr     cx, 1              ;  and divisor
	rcr     bx, 1              ;   right by 1 bit
	jnz     shift_loop_big     ;    loop if di non-zero (rcr does not touch ZF)
	div     bx                 ; compute quotient FFFFh:FFFFh>>x / cx:bx>>x (stored in ax; remainder in dx not used)
	mov     cx, ax             ; save quotient
	mul     di                 ; quotient * divisor hi-word (low only)
	not     ax                 ; dividend high - divisor high * quotient, no overflow (carry/borrow) possible here
	mov     bx, ax             ; save dividend high
	mov     ax, cx             ; ax=quotient
	mul     si                 ; quotient * divisor lo-word
	mov     ax, cx             ; get quotient
	sub     bx, dx             ; subtract (divisor-lo * quot.)-hi from dividend-hi
	sbb     dx, dx             ; 0 if remainder > 0, else FFFFFFFFh
	add     ax, dx             ; correct quotient if necessary           ax += dx
	pop     di                 ; restore temp  
	pop     si                 ;  variables
	retf                       ; return quotient in dx:ax
