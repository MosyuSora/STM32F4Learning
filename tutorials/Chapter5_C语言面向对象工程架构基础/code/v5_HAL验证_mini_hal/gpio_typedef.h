#ifndef GPIO_TYPEDEF_H
#define GPIO_TYPEDEF_H
#include <stdint.h>
#include <stdbool.h>
typedef struct {volatile uint32_t MODER;volatile uint32_t OTYPER;volatile uint32_t OSPEEDR;volatile uint32_t PUPDR;volatile uint32_t IDR;volatile uint32_t ODR;volatile uint32_t BSRR;volatile uint32_t LCKR;} GPIO_TypeDef;
#define GPIO_MODE_INPUT 0x00
#define GPIO_MODE_OUTPUT 0x01
#define GPIO_MODE_AF 0x02
#define GPIO_MODE_ANALOG 0x03
#define GPIO_NOPULL 0x00
#define GPIO_PULLUP 0x01
#define GPIO_PULLDOWN 0x02
#define GPIO_PIN_0 0x0001
#define GPIO_PIN_5 0x0020
#define GPIO_PIN_13 0x2000
#define GPIO_PIN_ALL 0xFFFF
typedef struct {uint32_t Pin;uint32_t Mode;uint32_t Pull;} GPIO_InitTypeDef;
#endif