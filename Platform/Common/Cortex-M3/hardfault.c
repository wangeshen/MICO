/**
******************************************************************************
* @file    hardfault.c 
* @author  William Xu
* @version V1.0.0
* @date    05-May-2014
* @brief   This file provide debug information in hardfault.
******************************************************************************
*
*  The MIT License
*  Copyright (c) 2014 MXCHIP Inc.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy 
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights 
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is furnished
*  to do so, subject to the following conditions:
*
*  The above copyright notice and this permission notice shall be included in
*  all copies or substantial portions of the Software.
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
*  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR 
*  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************
*/ 

#include "stdio.h"
#include "stm32f2xx.h"

void hard_fault_handler_c (unsigned int * hardfault_args)
{
  unsigned int stacked_r0;
  unsigned int stacked_r1;
  unsigned int stacked_r2;
  unsigned int stacked_r3;
  unsigned int stacked_r12;
  unsigned int stacked_lr;
  unsigned int stacked_pc;
  unsigned int stacked_psr;
 
  stacked_r0 = ((unsigned long) hardfault_args[0]);
  stacked_r1 = ((unsigned long) hardfault_args[1]);
  stacked_r2 = ((unsigned long) hardfault_args[2]);
  stacked_r3 = ((unsigned long) hardfault_args[3]);
 
  stacked_r12 = ((unsigned long) hardfault_args[4]);
  stacked_lr = ((unsigned long) hardfault_args[5]);
  stacked_pc = ((unsigned long) hardfault_args[6]);
  stacked_psr = ((unsigned long) hardfault_args[7]);
 
  printf ("\r\n\r\n[Hard fault handler - all numbers in hex]\r\n");
  printf ("R0 = %x\r\n", stacked_r0);
  printf ("R1 = %x\r\n", stacked_r1);
  printf ("R2 = %x\r\n", stacked_r2);
  printf ("R3 = %x\r\n", stacked_r3);
  printf ("R12 = %x\r\n", stacked_r12);
  printf ("LR [R14] = %x  subroutine call return address\r\n", stacked_lr);
  printf ("PC [R15] = %x  program counter\r\n", stacked_pc);
  printf ("PSR = %x\r\n", stacked_psr);
  printf ("BFAR = %x\r\n", (*((volatile unsigned long *)(0xE000ED38))));
  printf ("CFSR = %x\r\n", (*((volatile unsigned long *)(0xE000ED28))));
  printf ("HFSR = %x\r\n", (*((volatile unsigned long *)(0xE000ED2C))));
  printf ("DFSR = %x\r\n", (*((volatile unsigned long *)(0xE000ED30))));
  printf ("AFSR = %x\r\n", (*((volatile unsigned long *)(0xE000ED3C))));
  printf ("SCB_SHCSR = %x\r\n", SCB->SHCSR);
 
  while (1);
}