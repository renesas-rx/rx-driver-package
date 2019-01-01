/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2019 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : r_rx_intrinsic_functions.c
* Description  : Implements functions that apply to all r_bsp boards and MCUs.
***********************************************************************************************************************/
/**********************************************************************************************************************
* History : DD.MM.YYYY Version  Description
*         : xx.xx.xxxx 1.00     First Release
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "r_rx_intrinsic_functions.h"

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables (to be accessed by other files)
***********************************************************************************************************************/

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/


/***********************************************************************************************************************
* Function Name: R_BSP_Max
* Description  : Selects the greater of two input values.
* Arguments    : data1 - Input value 1.
*                data2 - Input value 2.
* Return Value : The greater value of data1 and data2.
***********************************************************************************************************************/
#if defined(__GNUC__)
signed long R_BSP_Max(signed long data1, signed long data2)
{
    return (data1 > data2)? data1 : data2;
}
#endif /* defined(__GNUC__) */

/***********************************************************************************************************************
* Function Name: R_BSP_Min
* Description  : Selects the smaller of two input values.
* Arguments    : data1 - Input value 1.
*                data2 - Input value 2.
* Return Value : The smaller value of data1 and data2.
***********************************************************************************************************************/
#if defined(__GNUC__)
signed long R_BSP_Min(signed long data1, signed long data2)
{
    return (data1 < data2)? data1 : data2;
}
#endif /* defined(__GNUC__) */

/***********************************************************************************************************************
* Function Name: R_BSP_MulAndAccOperation_B
* Description  : Performs a multiply-and-accumulate operation with the initial value specified by init, the number of 
*                multiply-and-accumulate operations specified by count, and the start addresses of values to be 
*                multiplied specified by addr1 and addr2.
* Arguments    : init   - Initial value.
*                count  - Count of multiply-and-accumulate operations.
*                *addr1 - Start address of values 1 to be multiplied.
*                *addr2 - Start address of values 2 to be multiplied.
* Return Value : result - Lower 64 bits of the init + ��(data1[n] * data2[n]) result. (n=0, 1, �c, const-1)
***********************************************************************************************************************/
#if defined(__GNUC__)
long long R_BSP_MulAndAccOperation_B(long long init, unsigned long count, signed char *addr1, signed char *addr2)
{
    long long result = init;
    unsigned long index;
    for(index = 0; index < count; index++)
    {
        result += addr1[index] * addr2[index];
    }
    return result;
}
#endif /* defined(__GNUC__) */

/***********************************************************************************************************************
* Function Name: R_BSP_MulAndAccOperation_W
* Description  : Performs a multiply-and-accumulate operation with the initial value specified by init, the number of
*                multiply-and-accumulate operations specified by count, and the start addresses of values to be 
*                multiplied specified by addr1 and addr2.
* Arguments    : init   - Initial value.
*                count  - Count of multiply-and-accumulate operations.
*                *addr1 - Start address of values 1 to be multiplied.
*                *addr2 - Start address of values 2 to be multiplied.
* Return Value : result - Lower 64 bits of the init + ��(data1[n] * data2[n]) result. (n=0, 1, �c, const-1)
***********************************************************************************************************************/
#if defined(__GNUC__)
long long R_BSP_MulAndAccOperation_W(long long init, unsigned long count, short *addr1, short *addr2)
{
    long long result = init;
    unsigned long index;
    for(index = 0; index < count; index++)
    {
        result += addr1[index] * addr2[index];
    }
    return result;
}
#endif /* defined(__GNUC__) */

/***********************************************************************************************************************
* Function Name: R_BSP_MulAndAccOperation_L
* Description  : Performs a multiply-and-accumulate operation with the initial value specified by init, the number of
*                multiply-and-accumulate operations specified by count, and the start addresses of values to be 
*                multiplied specified by addr1 and addr2.
* Arguments    : init   - Initial value.
*                count  - Count of multiply-and-accumulate operations.
*                *addr1 - Start address of values 1 to be multiplied.
*                *addr2 - Start address of values 2 to be multiplied.
* Return Value : result - Lower 64 bits of the init + ��(data1[n] * data2[n]) result. (n=0, 1, �c, const-1)
***********************************************************************************************************************/
#if defined(__GNUC__)
long long R_BSP_MulAndAccOperation_L(long long init, unsigned long count, long *addr1, long *addr2)
{
    long long result = init;
    unsigned long index;
    for(index = 0; index < count; index++)
    {
        result += addr1[index] * addr2[index];
    }
    return result;
}
#endif /* defined(__GNUC__) */

/***********************************************************************************************************************
* Function Name: R_BSP_RotateLeftWithCarry
* Description  : Rotates data including the C flag to left by one bit. 
*                The bit pushed out of the operand is set to the C flag.
* Arguments    : data - Data to be rotated to left.
* Return Value : data - Result of 1-bit left rotation of data including the C flag.
***********************************************************************************************************************/
#if defined(__GNUC__)
unsigned long R_BSP_RotateLeftWithCarry(unsigned long data)
{
    __asm("rolc %0":"=r"(data) : "r"(data):); 
    return data;
}
#endif /* defined(__GNUC__) */

/***********************************************************************************************************************
* Function Name: R_BSP_RotateRightWithCarry
* Description  : Rotates data including the C flag to right by one bit.
*                The bit pushed out of the operand is set to the C flag.
* Arguments    : data - Data to be rotated to right.
* Return Value : data - Result of 1-bit right rotation of data including the C flag.
***********************************************************************************************************************/
#if defined(__GNUC__)
unsigned long R_BSP_RotateRightWithCarry(unsigned long data)
{
    __asm("rorc %0":"=r"(data) : "r"(data):);
    return data;
}
#endif /* defined(__GNUC__) */

/***********************************************************************************************************************
* Function Name: R_BSP_RotateLeft
* Description  : Rotates data to left by the specified number of bits.
*                The bit pushed out of the operand is set to the C flag.
* Arguments    : data - Data to be rotated to left.
*                num - Number of bits to be rotated.
* Return Value : data - Result of num-bit left rotation of data.
***********************************************************************************************************************/
#if defined(__GNUC__)
unsigned long R_BSP_RotateLeft(unsigned long data, unsigned long num)
{
    __asm("rotl %1, %0":"=r"(data) : "r"(num),"0"(data) :); 
    return data;
}
#endif /* defined(__GNUC__) */

/***********************************************************************************************************************
* Function Name: R_BSP_RotateRight
* Description  : Rotates data to right by the specified number of bits.
*                The bit pushed out of the operand is set to the C flag.
* Arguments    : data - Data to be rotated to right.
*                num - Number of bits to be rotated.
* Return Value : result - Result of num-bit right rotation of data.
***********************************************************************************************************************/
#if defined(__GNUC__)
unsigned long R_BSP_RotateRight(unsigned long data, unsigned long num)
{
    __asm("rotr %1, %0":"=r"(data) : "r"(num),"0"(data) :); 
    return data;
}
#endif /* defined(__GNUC__) */

/***********************************************************************************************************************
* Function Name: R_BSP_SignedMultiplication
* Description  : Performs signed multiplication of significant 64 bits.
* Arguments    : data 1 - Input value 1.
*                data 2 - Input value 2.
* Return Value : Result of signed multiplication. (signed 64-bit value)
***********************************************************************************************************************/
#if defined(__GNUC__) || defined(__ICCRX__)
signed long long R_BSP_SignedMultiplication(signed long data1, signed long data2)
{
    return ((signed long long)data1) * ((signed long long)data2);
}
#endif /* defined(__GNUC__) || defined(__ICCRX__)  */

/***********************************************************************************************************************
* Function Name: R_BSP_UnsignedMultiplication
* Description  : Performs unsigned multiplication of significant 64 bits.
* Arguments    : data 1 - Input value 1.
*                data 2 - Input value 2.
* Return Value : Result of unsigned multiplication. (unsigned 64-bit value)
***********************************************************************************************************************/
#if defined(__GNUC__) || defined(__ICCRX__)
unsigned long long R_BSP_UnsignedMultiplication(unsigned long data1, unsigned long data2)
{
    return ((unsigned long long)data1) * ((unsigned long long)data2);
}
#endif /* defined(__GNUC__) || defined(__ICCRX__)  */

/***********************************************************************************************************************
* Function name: R_BSP_ChangeToUserMode
* Description  : Switches to user mode. The PSW will be changed as following.
*                Before Execution                                       After Execution
*                PSW.PM                 PSW.U                           PSW.PM              PSW.U
*                0 (supervisor mode)    0 (interrupt stack)     -->     1 (user mode)       1 (user stack)
*                0 (supervisor mode)    1 (user stack)          -->     1 (user mode)       1 (user stack)
*                1 (user mode)          1 (user stack)          -->     NO CHANGE
*                1 (user mode)          0 (interrupt stack))    <==     N/A
* Arguments    : dummy1 - Please set to 0.
*                dummy2 - Please set to 0.
* Return value : none
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_ChangeToUserMode)
void R_BSP_ChangeToUserMode(uint8_t dummy1, uint8_t dummy2)
{
    R_BSP_ASM_INTERNAL_USED(dummy1)
    R_BSP_ASM_INTERNAL_USED(dummy2)

    R_BSP_ASM_BEGIN
    R_BSP_ASM(;_R_BSP_Change_PSW_PM_to_UserMode:                                           )
    R_BSP_ASM(    mvfc    psw, r1     ; get the current PSW value                          )
    R_BSP_ASM(    btst    #20, r1     ; check PSW.PM                                       )
    R_BSP_ASM(    bne.b   R_BSP_ASM_LAB_NEXT(0);_psw_pm_is_user_mode                       )
    R_BSP_ASM(;_psw_pm_is_supervisor_mode:                                                 )
    R_BSP_ASM(    pop     r2          ; pop the return address value of caller             )
    R_BSP_ASM(    bset    #20, r1     ; change PM = 0(Supervisor Mode) --> 1(User Mode)    )
    R_BSP_ASM(    push.l  r1          ; push new PSW value which will be changed           )
    R_BSP_ASM(    push.l  r2          ; push return address value                          )
    R_BSP_ASM(    rte                                                                      )
    R_BSP_ASM_LAB(0:;_psw_pm_is_user_mode:                                                 )
    R_BSP_ASM(    ;rts                                                                     )
    R_BSP_ASM_END
} /* End of function Change_PSW_PM_to_UserMode() */

/***********************************************************************************************************************
* Function Name: R_BSP_SetACC
* Description  : Sets a value to ACC.
* Arguments    : data - Value to be set to ACC.
* Return Value : none
***********************************************************************************************************************/
#if defined(__GNUC__) || defined(__ICCRX__)
void R_BSP_SetACC(signed long long data)
{
#if defined(__GNUC__)
    __builtin_rx_mvtachi(data >> 32);
    __builtin_rx_mvtaclo(data & 0xFFFFFFFF);
#elif defined(__ICCRX__)
    int32_t data_hi;
    int32_t data_lo;

    data_hi = (int32_t)(data >> 32);
    data_lo = (int32_t)(data & 0x00000000FFFFFFFF);

    R_BSP_MoveToAccHiLong(data_hi);
    R_BSP_MoveToAccLoLong(data_lo);
#endif /* defined(__GNUC__) || defined(__ICCRX__)  */
}
#endif /* defined(__GNUC__) || defined(__ICCRX__)  */

/***********************************************************************************************************************
* Function Name: R_BSP_GetACC
* Description  : Refers to the ACC value.
* Arguments    : none
* Return Value : result - ACC value.
***********************************************************************************************************************/
#if defined(__GNUC__) || defined(__ICCRX__)
signed long long R_BSP_GetACC(void)
{
#if defined(__GNUC__)
    signed long long result = ((signed long long)__builtin_rx_mvfachi()) << 32;
    result |= (((signed long long)__builtin_rx_mvfacmi()) << 16) & 0xFFFF0000;
    return result;
#elif defined(__ICCRX__)
    int64_t result;

    result = ((int64_t)R_BSP_MoveFromAccHiLong()) << 32;
    result |= (((int64_t)R_BSP_MoveFromAccMiLong()) << 16) & 0xFFFF0000;

    return result;
#endif /* defined(__GNUC__) || defined(__ICCRX__)  */
}
#endif /* defined(__GNUC__) || defined(__ICCRX__)  */

/***********************************************************************************************************************
* Function Name: R_BSP_MulAndAccOperation_2byte
* Description  : Performs a multiply-and-accumulate operation between data of two bytes each and returns the result as
*                four bytes. The multiply-and-accumulate operation is executed with DSP functional instructions (MULLO, 
*                MACLO, and MACHI). Data in the middle of the multiply-and-accumulate operation is retained in ACC as
*                48-bit data. After all multiply-and-accumulate operations have finished, the contents of ACC are 
*                fetched by the MVFACMI instruction and used as the return value of the intrinsic function.
* Arguments    : data1 - Start address of values 1 to be multiplied.
*                data2 - Start address of values 2 to be multiplied.
*                count - Count of multiply-and-accumulate operations.
* Return Value : ��(data1[n] * data2[n]) result.
***********************************************************************************************************************/
#if defined(__GNUC__)
long R_BSP_MulAndAccOperation_2byte(short* data1, short* data2, unsigned long count)
{
    unsigned long index;
    __builtin_rx_mullo(0, 0);
    for(index = 0; index < count; index++)
    {
        __builtin_rx_maclo(data1[index], data2[index]);
        __builtin_rx_machi(data1[index], data2[index]);
    }
    return (long)(R_BSP_GetACC() >> 16);
}
#endif /* defined(__GNUC__) */

/***********************************************************************************************************************
* Function Name: R_BSP_MulAndAccOperation_FixedPoint1
* Description  : Performs a multiply-and-accumulate operation between data of two bytes each and returns the result as
*                two bytes. The multiply-and-accumulate operation is executed with DSP functional instructions (MULLO, 
*                MACLO, and MACHI). Data in the middle of the multiply-and-accumulate operation is retained in ACC as
*                48-bit data. After all multiply-and-accumulate operations have finished, rounding is applied to the 
*                multiply-and-accumulate operation result of ACC.
*                The macw1 function performs rounding with the "RACW #1" instruction.
* Arguments    : data1 - Start address of values 1 to be multiplied.
*                data2 - Start address of values 2 to be multiplied.
*                count - Count of multiply-and-accumulate operations.
* Return Value : Value obtained by rounding the multiply-and-accumulate operation result with the RACW instruction.
***********************************************************************************************************************/
#if defined(__GNUC__)
short R_BSP_MulAndAccOperation_FixedPoint1(short* data1, short* data2, unsigned long count)
{
    unsigned long index;
    __builtin_rx_mullo(0, 0);
    for(index = 0; index < count; index++)
    {
        __builtin_rx_maclo(data1[index], data2[index]);
        __builtin_rx_machi(data1[index], data2[index]);
    }
    __builtin_rx_racw(1);
    return (short)(R_BSP_GetACC() >> 32);
}
#endif /* defined(__GNUC__) */

/***********************************************************************************************************************
* Function Name: R_BSP_MulAndAccOperation_FixedPoint2
* Description  : Performs a multiply-and-accumulate operation between data of two bytes each and returns the result as
*                two bytes. The multiply-and-accumulate operation is executed with DSP functional instructions (MULLO, 
*                MACLO, and MACHI). Data in the middle of the multiply-and-accumulate operation is retained in ACC as
*                48-bit data. After all multiply-and-accumulate operations have finished, rounding is applied to the 
*                multiply-and-accumulate operation result of ACC.
*                the macw2 function performs rounding with the "RACW #2" instruction.
* Arguments    : data1 - Start address of values 1 to be multiplied.
*                data2 - Start address of values 2 to be multiplied.
*                count - Count of multiply-and-accumulate operations.
* Return Value : Value obtained by rounding the multiply-and-accumulate operation result with the RACW instruction.
***********************************************************************************************************************/
#if defined(__GNUC__)
short R_BSP_MulAndAccOperation_FixedPoint2(short* data1, short* data2, unsigned long count)
{
    unsigned long index;
    __builtin_rx_mullo(0, 0);
    for(index = 0; index < count; index++)
    {
        __builtin_rx_maclo(data1[index], data2[index]);
        __builtin_rx_machi(data1[index], data2[index]);
    }
    __builtin_rx_racw(2);
    return (short)(R_BSP_GetACC() >> 32);
}
#endif /* defined(__GNUC__) */

/***********************************************************************************************************************
* Function Name: R_BSP_SetBPSW
* Description  : Sets a value to BPSW.
* Arguments    : data - Value to be set.
* Return Value : none
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_SetBPSW)
void R_BSP_SetBPSW(uint32_t data)
{
    R_BSP_ASM_INTERNAL_USED(data)

    R_BSP_ASM_BEGIN
    R_BSP_ASM(    MVTC    R1, BPSW    )
    R_BSP_ASM_END
} /* End of function R_BSP_SetBPSW() */

/***********************************************************************************************************************
* Function Name: R_BSP_GetBPSW
* Description  : Refers to the BPSW value.
* Arguments    : none
* Return Value : BPSW value.
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_GetBPSW)
uint32_t R_BSP_GetBPSW(void)
{
    R_BSP_ASM_BEGIN
    R_BSP_ASM(    MVFC    BPSW, R1    )
    R_BSP_ASM_END
} /* End of function R_BSP_GetBPSW() */

/***********************************************************************************************************************
* Function Name: R_BSP_SetBPC
* Description  : Sets a value to BPC.
* Arguments    : data - Value to be set.
* Return Value : none
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_SetBPC)
void R_BSP_SetBPC(void *data)
{
    R_BSP_ASM_INTERNAL_USED(data)

    R_BSP_ASM_BEGIN
    R_BSP_ASM(    MVTC    R1, BPC    )
    R_BSP_ASM_END
} /* End of function R_BSP_SetBPC() */

/***********************************************************************************************************************
* Function Name: R_BSP_GetBPC
* Description  : Refers to the BPC value.
* Arguments    : none
* Return Value : BPC value
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_GetBPC)
void *R_BSP_GetBPC(void)
{
    R_BSP_ASM_BEGIN
    R_BSP_ASM(    MVFC    BPC, R1    )
    R_BSP_ASM_END
} /* End of function R_BSP_GetBPC() */

#ifdef BSP_MCU_EXCEPTION_TABLE
/***********************************************************************************************************************
* Function Name: R_BSP_SetEXTB
* Description  : Sets a value for EXTB.
* Arguments    : data - Value to be set.
* Return Value : none
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_SetEXTB)
void R_BSP_SetEXTB(void *data)
{
    R_BSP_ASM_INTERNAL_USED(data)

    R_BSP_ASM_BEGIN
    R_BSP_ASM(    MVTC    R1, EXTB    )
    R_BSP_ASM_END
} /* End of function R_BSP_SetEXTB() */

/***********************************************************************************************************************
* Function Name: R_BSP_GetEXTB
* Description  : Refers to the EXTB value.
* Arguments    : none
* Return Value : EXTB value.
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_GetEXTB)
void *R_BSP_GetEXTB(void)
{
    R_BSP_ASM_BEGIN
    R_BSP_ASM(    MVFC    EXTB, R1    )
    R_BSP_ASM_END
} /* End of function R_BSP_GetEXTB() */
#endif /* BSP_MCU_EXCEPTION_TABLE */

/***********************************************************************************************************************
* Function Name: R_BSP_MoveToAccHiLong
* Description  : This function moves the contents of src to the higher-order 32 bits of the accumulator.
* Arguments    : data - Input value.
* Return Value : none
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_MoveToAccHiLong)
void R_BSP_MoveToAccHiLong(int32_t data)
{
    R_BSP_ASM_INTERNAL_USED(data)

    R_BSP_ASM_BEGIN
    R_BSP_ASM(    MVTACHI    R1    )
    R_BSP_ASM_END
} /* End of function R_BSP_MoveToAccHiLong() */

/***********************************************************************************************************************
* Function Name: R_BSP_MoveToAccLoLong
* Description  : This function moves the contents of src to the lower-order 32 bits of the accumulator.
* Arguments    : data - Input value.
* Return Value : none
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_MoveToAccLoLong)
void R_BSP_MoveToAccLoLong(int32_t data)
{
    R_BSP_ASM_INTERNAL_USED(data)

    R_BSP_ASM_BEGIN
    R_BSP_ASM(    MVTACLO    R1    )
    R_BSP_ASM_END
} /* End of function R_BSP_MoveToAccLoLong() */

/***********************************************************************************************************************
* Function Name: R_BSP_MoveFromAccHiLong
* Description  : This function moves the higher-order 32 bits of the accumulator to dest.
* Arguments    : none
* Return Value : The higher-order 32 bits of the accumulator.
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_MoveFromAccHiLong)
int32_t R_BSP_MoveFromAccHiLong(void)
{
    R_BSP_ASM_BEGIN
    R_BSP_ASM(    MVFACHI    R1    )
    R_BSP_ASM_END
} /* End of function R_BSP_MoveFromAccHiLong() */

/***********************************************************************************************************************
* Function Name: R_BSP_MoveFromAccMiLong
* Description  : This function moves the contents of bits 47 to 16 of the accumulator to dest.
* Arguments    : none
* Return Value : The contents of bits 47 to 16 of the accumulator.
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_MoveFromAccMiLong)
int32_t R_BSP_MoveFromAccMiLong(void)
{
    R_BSP_ASM_BEGIN
    R_BSP_ASM(    MVFACMI    R1    )
    R_BSP_ASM_END
} /* End of function R_BSP_MoveFromAccMiLong() */

/***********************************************************************************************************************
* Function Name: R_BSP_BitSet
* Description  : Sets the specified one bit in the specified 1-byte area to 1.
* Arguments    : data - Address of the target 1-byte area
*                bit  - Position of the bit to be manipulated
* Return Value : none
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_BitSet)
void R_BSP_BitSet(uint8_t *data, uint32_t bit)
{
    R_BSP_ASM_INTERNAL_USED(data)
    R_BSP_ASM_INTERNAL_USED(bit)

    R_BSP_ASM_BEGIN
    R_BSP_ASM(    BSET    R2, [R1]    )
    R_BSP_ASM_END
} /* End of function R_BSP_BitSet() */

/***********************************************************************************************************************
* Function Name: R_BSP_BitClear
* Description  : Sets the specified one bit in the specified 1-byte area to 0.
* Arguments    : data - Address of the target 1-byte area
*                bit  - Position of the bit to be manipulated
* Return Value : none
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_BitClear)
void R_BSP_BitClear(uint8_t *data, uint32_t bit)
{
    R_BSP_ASM_INTERNAL_USED(data)
    R_BSP_ASM_INTERNAL_USED(bit)

    R_BSP_ASM_BEGIN
    R_BSP_ASM(    BCLR    R2, [R1]    )
    R_BSP_ASM_END
} /* End of function R_BSP_BitClear() */

/***********************************************************************************************************************
* Function Name: R_BSP_BitReverse
* Description  : Reverses the value of the specified one bit in the specified 1-byte area.
* Arguments    : data - Address of the target 1-byte area
*                bit  - Position of the bit to be manipulated
* Return Value : none
***********************************************************************************************************************/
R_BSP_PRAGMA_INLINE_ASM(R_BSP_BitReverse)
void R_BSP_BitReverse(uint8_t *data, uint32_t bit)
{
    R_BSP_ASM_INTERNAL_USED(data)
    R_BSP_ASM_INTERNAL_USED(bit)

    R_BSP_ASM_BEGIN
    R_BSP_ASM(    BNOT    R2, [R1]    )
    R_BSP_ASM_END
} /* End of function R_BSP_BitReverse() */
