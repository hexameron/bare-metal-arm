
//
//  Copyright (c) 2012-2013 Andrew Payne <andy@payne.org>
//

#include "MKL25Z4.h"

// Memory locations defined by the linker
extern uint32_t __heap_start[];
extern uint32_t __StackTop[];
extern uint32_t __data_start__[], __data_end__[];
extern uint32_t __bss_start__[], __bss_end__[];
extern uint32_t __etext[];                // End of code/flash

// From rfm98.c
void rfm98_init(void);
short rfm98_temp(void);
void rfm98_transmit(char* message);

// From spi.c
void spi_init(void);
uint8_t spi_status(void);
uint8_t spi_single(uint8_t addr, uint8_t data);
void spi_write(uint8_t* p, uint8_t size, uint8_t addr);
void spi_read(uint8_t* p, uint8_t size, uint8_t addr);
void SPI0_IRQHandler(void) __attribute__((interrupt("IRQ")));

// From math.c
unsigned short magnitude(short x, short y, short z);
short findArctan(short x, short y, short z);
short upness(short x, short y, short z);
short findArcsin( short scalar, unsigned short mag );

// From uart.c
void UART0_IRQHandler(void) __attribute__((interrupt("IRQ")));
int uart_write(char *p, int len);
int uart_write_err(char *p, int len);
int uart_read(char *p, int len);
void uart_init(int baud_rate);

// From delay.c
void delay(unsigned int ms);

// From accel.c
void accel_init(void);
int16_t accel_x(void);
int16_t accel_y(void);
int16_t accel_z(void);

// From touch.c
int touch_data(int channel);
void touch_init(uint32_t channel_mask);

// From _startup.c
void fault(uint32_t pattern);
#define FAULT_FAST_BLINK 	(0b10101010101010101010101010101010)
#define FAULT_MEDIUM_BLINK 	(0b11110000111100001111000011110000)
#define FAULT_SLOW_BLINK 	(0b11111111000000001111111100000000)

// usb.c
void usb_init(void);
void usb_dump(void);

// Interrupt enabling and disabling
static inline void enable_irq(int n) {
    NVIC_ICPR |= 1 << (n - 16);
    NVIC_ISER |= 1 << (n - 16);			
}
// TODO:  IRQ disable

static inline void __enable_irq(void)	{ asm volatile ("cpsie i"); }
static inline void __disable_irq(void)  { asm volatile ("cpsid i"); }

// ring.c
typedef struct {
    volatile uint16_t head;
    volatile uint16_t tail;
    volatile uint16_t size;
    volatile uint8_t data[];
} RingBuffer;

void buf_reset(RingBuffer *buf, int size);
int buf_len(const RingBuffer *buf);
int buf_isfull(const RingBuffer *buf);
int buf_isempty(const RingBuffer *buf);
uint8_t buf_get_byte(RingBuffer *buf);
void buf_put_byte(RingBuffer *buf, uint8_t val);

// tests.c
void tests(void);
