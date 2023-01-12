/*******************************************************************************
  TCC Peripheral Library Interface Source File

  Company
    Microchip Technology Inc.

  File Name
    plib_tcc2.c

  Summary
    TCC2 peripheral library source file.

  Description
    This file implements the interface to the TCC peripheral library.  This
    library provides access to and control of the associated peripheral
    instance.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

/*  This section lists the other files that are included in this file.
*/
#include "interrupts.h"
#include "plib_tcc2.h"



/* Initialize TCC module */
void TCC2_PWMInitialize(void)
{
    /* Reset TCC */
    TCC2_REGS->TCC_CTRLA = TCC_CTRLA_SWRST_Msk;
    while ((TCC2_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_SWRST_Msk) != 0U)
    {
        /* Wait for sync */
    }
    /* Clock prescaler */
    TCC2_REGS->TCC_CTRLA = TCC_CTRLA_PRESCALER_DIV1 ;
    TCC2_REGS->TCC_WEXCTRL = TCC_WEXCTRL_OTMX(0UL);

    TCC2_REGS->TCC_WAVE = TCC_WAVE_WAVEGEN_NPWM;

    /* Configure duty cycle values */
    TCC2_REGS->TCC_CC[0] = 0U;
    TCC2_REGS->TCC_CC[1] = 0U;
    TCC2_REGS->TCC_CC[2] = 0U;
    TCC2_REGS->TCC_PER = 2399U;



    while (TCC2_REGS->TCC_SYNCBUSY != 0U)
    {
        /* Wait for sync */
    }
}


/* Start the PWM generation */
void TCC2_PWMStart(void)
{
    TCC2_REGS->TCC_CTRLA |= TCC_CTRLA_ENABLE_Msk;
    while ((TCC2_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_ENABLE_Msk) != 0U)
    {
        /* Wait for sync */
    }
}

/* Stop the PWM generation */
void TCC2_PWMStop (void)
{
    TCC2_REGS->TCC_CTRLA &= ~TCC_CTRLA_ENABLE_Msk;
    while ((TCC2_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_ENABLE_Msk) != 0U)
    {
        /* Wait for sync */
    }
}

/* Configure PWM period */
bool TCC2_PWM16bitPeriodSet (uint16_t period)
{
    bool status = false;
    if ((TCC2_REGS->TCC_STATUS & (TCC_STATUS_PERBUFV_Msk)) == 0U)
    {
        TCC2_REGS->TCC_PERBUF = period;
        status = true;
    }    
    return status;
}

/* Read TCC period */
uint16_t TCC2_PWM16bitPeriodGet (void)
{
    while ((TCC2_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_PER_Msk) != 0U)
    {
        /* Wait for sync */
    }
    return (uint16_t)TCC2_REGS->TCC_PER;
}



/* Set the counter*/
void TCC2_PWM16bitCounterSet (uint16_t count)
{
    TCC2_REGS->TCC_COUNT = count;
    while ((TCC2_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_COUNT_Msk) != 0U)
    {
        /* Wait for sync */
    }
}

/* Enable forced synchronous update */
void TCC2_PWMForceUpdate(void)
{
    TCC2_REGS->TCC_CTRLBSET |= (uint8_t)TCC_CTRLBCLR_CMD_UPDATE;
    while ((TCC2_REGS->TCC_SYNCBUSY & TCC_SYNCBUSY_CTRLB_Msk) != 0U)
    {
        /* Wait for sync */
    }
}

/* Enable the period interrupt - overflow or underflow interrupt */
void TCC2_PWMPeriodInterruptEnable(void)
{
    TCC2_REGS->TCC_INTENSET = TCC_INTENSET_OVF_Msk;
}

/* Disable the period interrupt - overflow or underflow interrupt */
void TCC2_PWMPeriodInterruptDisable(void)
{
    TCC2_REGS->TCC_INTENCLR = TCC_INTENCLR_OVF_Msk;
}

/* Read interrupt flags */
uint32_t TCC2_PWMInterruptStatusGet(void)
{
    uint32_t interrupt_status;
    interrupt_status = TCC2_REGS->TCC_INTFLAG;
    /* Clear interrupt flags */
    TCC2_REGS->TCC_INTFLAG = interrupt_status;
    (void)TCC2_REGS->TCC_INTFLAG;
    return interrupt_status;
}


/**
 End of File
*/
