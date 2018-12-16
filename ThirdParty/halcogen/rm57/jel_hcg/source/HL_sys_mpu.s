/*-------------------------------------------------------------------------------
 HL_sys_mpu.s

 Copyright (C) 2009-2016 Texas Instruments Incorporated - www.ti.com  
 
 
  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions 
  are met:

    Redistributions of source code must retain the above copyright 
    notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the 
    documentation and/or other materials provided with the   
    distribution.

    Neither the name of Texas Instruments Incorporated nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

--------------------------------------------------------------------------*/


    .section .text 
    .syntax unified
    .cpu cortex-r4
    .arm 



/*-------------------------------------------------------------------------------*/
@ Initialize Mpu

    .weak _mpuInit_    
    .type _mpuInit_, %function	

_mpuInit_:
        @ Disable mpu
        mrc   p15, #0, r0, c1, c0, #0
        bic   r0,  r0, #1
        dsb
        mcr   p15, #0, r0, c1, c0, #0
        isb
        @ Disable background region
        mrc   p15, #0, r0,      c1, c0, #0
        bic   r0,  r0, #0x20000
        mcr   p15, #0, r0,      c1, c0, #0
        @ Setup region 1
        mov   r0,  #0
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r1Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0008
        orr   r0,  r0,    #0x1000
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((1 << 15) + (1 << 14) + (1 << 13) + (1 << 12) + (1 << 11) + (1 << 10) + (1 <<  9) + (1 <<  8) + (0x1F << 1) + (1)) 
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 2
        mov   r0,  #1
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r2Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0002
        orr   r0,  r0,    #0x0600
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x15 << 1) + (1))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region  
        mov   r0,  #2
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r3Base
        mcr   p15, #0,    r0, c6, c1, #0    
        mov   r0,  #0x000B
        orr   r0,  r0,    #0x0300
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x12 << 1) + (1))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 4
        mov   r0,  #3
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r4Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0010
        orr   r0,  r0,    #0x0300
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x16 << 1) + (0))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 5
        mov   r0,  #4
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r5Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0000
        orr   r0,  r0,    #0x0300
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((1 << 15) + (1 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x16 << 1) + (0))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 6
        mov   r0,  #5
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r6Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0006
        orr   r0,  r0,    #0x0300
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x1A << 1) + (0))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 7
        mov   r0,  #6
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r7Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0008
        orr   r0,  r0,    #0x1200
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x16 << 1) + (0))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 8
        mov   r0,  #7
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r8Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0010
        orr   r0,  r0,    #0x1200
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x04 << 1) + (0))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 9
        mov   r0,  #8
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r9Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0006
        orr   r0,  r0,    #0x1200
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x04 << 1) + (0))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 10
        mov   r0,  #9
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r10Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x000C
        orr   r0,  r0,    #0x1300
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x04 << 1) + (0))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 11
        mov   r0,  #10
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r11Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0006
        orr   r0,  r0,    #0x0600
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x04 << 1) + (0))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 12
        mov   r0,  #11
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r12Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0006
        orr   r0,  r0,    #0x1600
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x04 << 1) + (0))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 13
        mov   r0,  #12
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r13Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0006
        orr   r0,  r0,    #0x1600
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x04 << 1) + (0))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 14
        mov   r0,  #13
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r14Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0006
        orr   r0,  r0,    #0x1600
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x04 << 1) + (0))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 15
        mov   r0,  #14
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r15Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0006
        orr   r0,  r0,    #0x1600
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x04 << 1) + (0))
        mcr   p15, #0,    r0, c6, c1, #2
        @ Setup region 16
        mov   r0,  #15
        mcr   p15, #0,    r0, c6, c2, #0
        ldr   r0,  r16Base
        mcr   p15, #0,    r0, c6, c1, #0
        mov   r0,  #0x0010
        orr   r0,  r0,    #0x1200
        mcr   p15, #0,    r0, c6, c1, #4
        movw  r0,  #((0 << 15) + (0 << 14) + (0 << 13) + (0 << 12) + (0 << 11) + (0 << 10) + (0 <<  9) + (0 <<  8) + (0x12 << 1) + (1))
        mcr   p15, #0,    r0, c6, c1, #2

        @ Enable mpu
        mrc   p15, #0, r0, c1, c0, #0
        orr   r0,  r0, #1
        dsb
        mcr   p15, #0, r0, c1, c0, #0
        isb
        bx    lr

r1Base:  .word 0x00000000  
r2Base:  .word 0x00000000  
r3Base:  .word 0x08000000  
r4Base:  .word 0x80000000  
r5Base:  .word 0x80000000  
r6Base:  .word 0x80000000  
r7Base:  .word 0xF0000000  
r8Base:  .word 0x00000000  
r9Base:  .word 0x00000000  
r10Base:  .word 0x00000000  
r11Base:  .word 0x00000000  
r12Base:  .word 0x00000000  
r13Base:  .word 0x00000000  
r14Base:  .word 0x00000000  
r15Base:  .word 0x00000000  
r16Base:  .word 0xFFF80000  


/*-------------------------------------------------------------------------------*/
@ Enable Mpu

    .weak     _mpuEnable_
    .type     _mpuEnable_, %function	
		
_mpuEnable_:

        mrc   p15, #0, r0, c1, c0, #0
        orr   r0,  r0, #1
        dsb
        mcr   p15, #0, r0, c1, c0, #0
        isb
        bx    lr

/*-------------------------------------------------------------------------------*/
@ Disable Mpu

    .weak     _mpuDisable_
    .type     _mpuDisable_, %function	
    
_mpuDisable_:

        mrc   p15, #0, r0, c1, c0, #0
        bic   r0,  r0, #1
        dsb
        mcr   p15, #0, r0, c1, c0, #0
        isb
        bx    lr



/*-------------------------------------------------------------------------------*/
@ Enable Mpu background region

    .weak     _mpuEnableBackgroundRegion_
    .type     _mpuEnableBackgroundRegion_, %function	

_mpuEnableBackgroundRegion_:

        mrc   p15, #0, r0,      c1, c0, #0
        orr   r0,  r0, #0x20000
        mcr   p15, #0, r0,      c1, c0, #0
        bx    lr

/*-------------------------------------------------------------------------------*/
@ Disable Mpu background region

    .weak     _mpuDisableBackgroundRegion_
    .type     _mpuDisableBackgroundRegion_, %function	
	
_mpuDisableBackgroundRegion_:

        mrc   p15, #0, r0,      c1, c0, #0
        bic   r0,  r0, #0x20000
        mcr   p15, #0, r0,      c1, c0, #0
        bx    lr

/*-------------------------------------------------------------------------------*/
@ Returns number of implemented Mpu regions

    .weak     _mpuGetNumberOfRegions_
    .type     _mpuGetNumberOfRegions_, %function	
	
_mpuGetNumberOfRegions_:

        mrc   p15, #0, r0,      c0, c0, #4
        uxtb  r0,  r0, ROR #8
        bx    lr

/*-------------------------------------------------------------------------------*/
@ Returns the type of the implemented mpu regions

    .weak     _mpuAreRegionsSeparate_
    .type     _mpuAreRegionsSeparate_, %function	
	
_mpuAreRegionsSeparate_:

        mrc   p15, #0, r0,      c0, c0, #4
        uxtb  r0,  r0
        bx    lr

/*-------------------------------------------------------------------------------*/
@ Set mpu region number

    .weak     _mpuSetRegion_
    .type     _mpuSetRegion_, %function	
	
_mpuSetRegion_:

        mcr   p15, #0, r0, c6, c2, #0
        bx    lr

/*-------------------------------------------------------------------------------*/
@ Get mpu region number

    .weak     _mpuGetRegion_
    .type     _mpuGetRegion_, %function	
	
_mpuGetRegion_:

        mrc   p15, #0, r0, c6, c2, #0
        bx    lr

/*-------------------------------------------------------------------------------*/
@ Set base address

    .weak     _mpuSetRegionBaseAddress_
    .type     _mpuSetRegionBaseAddress_, %function	
	
_mpuSetRegionBaseAddress_:

        mcr   p15, #0, r0, c6, c1, #0
        bx    lr

/*-------------------------------------------------------------------------------*/
@ Get base address

    .weak     _mpuGetRegionBaseAddress_

_mpuGetRegionBaseAddress_:

        mrc   p15, #0, r0, c6, c1, #0
        bx    lr

/*-------------------------------------------------------------------------------*/
@ Set type and permission

    .weak     _mpuSetRegionTypeAndPermission_
    .type     _mpuSetRegionTypeAndPermission_, %function	    
	
_mpuSetRegionTypeAndPermission_:

        orr   r0,  r0, r1
        mcr   p15, #0, r0, c6, c1, #4
        bx    lr

/*-------------------------------------------------------------------------------*/
@ Get type

    .weak     _mpuGetRegionType_
    .type     _mpuGetRegionType_, %function		
    
_mpuGetRegionType_:

        mrc   p15, #0, r0,     c6, c1, #4
        bic   r0,  r0, #0xFF00
        bx    lr

/*-------------------------------------------------------------------------------*/
@ Get permission

    .weak     _mpuGetRegionPermission_
    .type     _mpuGetRegionPermission_, %function	
    

_mpuGetRegionPermission_:

        mrc   p15, #0, r0,   c6, c1, #4
        bic   r0,  r0, #0xFF
        bx    lr

/*-------------------------------------------------------------------------------*/
@ Set region size register value

    .weak     _mpuSetRegionSizeRegister_
    .type     _mpuSetRegionSizeRegister_, %function	
	
_mpuSetRegionSizeRegister_:

        mcr   p15, #0, r0, c6, c1, #2
        bx    lr

/*-------------------------------------------------------------------------------*/

