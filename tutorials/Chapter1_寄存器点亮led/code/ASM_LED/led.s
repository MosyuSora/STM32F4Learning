  .syntax unified
  .cpu cortex-m4
  .fpu softvfp
  .thumb

  .global g_pfnVectors
  .global Reset_Handler

  .section .isr_vector,"a",%progbits
  .type g_pfnVectors, %object
g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word Default_Handler
  .word Default_Handler
  .word Default_Handler
  .word Default_Handler
  .word Default_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word Default_Handler
  .word Default_Handler
  .word 0
  .word Default_Handler
  .word Default_Handler
  .rept 82
    .word Default_Handler
  .endr

  .section .text.Reset_Handler,"ax",%progbits
  .type Reset_Handler, %function
Reset_Handler:
  bl main
reset_hang:
  b reset_hang

  .section .text.Default_Handler,"ax",%progbits
  .type Default_Handler, %function
Default_Handler:
default_hang:
  b default_hang

  .equ RCC_AHB1ENR,   0x40023830
  .equ GPIOF_MODER,   0x40021400
  .equ GPIOF_OTYPER,  0x40021404
  .equ GPIOF_OSPEEDR, 0x40021408
  .equ GPIOF_PUPDR,   0x4002140C
  .equ GPIOF_BSRR,    0x40021418

  .equ GPIOF_MODER_MASK,   0x0003F000
  .equ GPIOF_MODE_OUTPUT,  0x00015000
  .equ GPIOF_OTYPER_MASK,  0x000001C0
  .equ GPIOF_OSPEEDR_MASK, 0x0003F000
  .equ GPIOF_PUPDR_MASK,   0x0003F000
  .equ GPIOF_PUPDR_UP,     0x00015000

  .equ LED_RED_BSRR,   0x00400180
  .equ LED_GREEN_BSRR, 0x00800140
  .equ LED_BLUE_BSRR,  0x010000C0
  .equ DELAY_COUNT,    2000000

  .type main, %function

main:
  ldr r0, =RCC_AHB1ENR
  ldr r1, [r0]
  orr r1, r1, #(1 << 5)
  str r1, [r0]

  ldr r0, =GPIOF_MODER
  ldr r1, [r0]
  ldr r2, =GPIOF_MODER_MASK
  bic r1, r1, r2
  ldr r2, =GPIOF_MODE_OUTPUT
  orr r1, r1, r2
  str r1, [r0]

  ldr r0, =GPIOF_OTYPER
  ldr r1, [r0]
  ldr r2, =GPIOF_OTYPER_MASK
  bic r1, r1, r2
  str r1, [r0]

  ldr r0, =GPIOF_OSPEEDR
  ldr r1, [r0]
  ldr r2, =GPIOF_OSPEEDR_MASK
  bic r1, r1, r2
  str r1, [r0]

  ldr r0, =GPIOF_PUPDR
  ldr r1, [r0]
  ldr r2, =GPIOF_PUPDR_MASK
  bic r1, r1, r2
  ldr r2, =GPIOF_PUPDR_UP
  orr r1, r1, r2
  str r1, [r0]

loop:
  ldr r0, =GPIOF_BSRR
  ldr r1, =LED_RED_BSRR
  str r1, [r0]
  bl delay

  ldr r1, =LED_GREEN_BSRR
  str r1, [r0]
  bl delay

  ldr r1, =LED_BLUE_BSRR
  str r1, [r0]
  bl delay

  b loop

  .type delay, %function
delay:
  ldr r2, =DELAY_COUNT
delay_loop:
  subs r2, r2, #1
  bne delay_loop
  bx lr
