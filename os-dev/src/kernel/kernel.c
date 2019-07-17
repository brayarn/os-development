#include <stddef.h>
#include <stdint.h>

static inline void mmio_write(uint32_t reg, uint32_t data){
    *(volatile uint32_t*)reg = data;
}

static inline uint32_t mmio_read(uint32_t reg){
    return *(volatile uint32_t*) reg;
}

// Loop <delay> times in a way that the compiler won't optimize away
static inline void delay(int32_t count){
    asm volatile("__delay_%=: subs %[count], %[count], #1; bne __delay_%=\n"
            : "=r"(count): [count]"0"(count) : "cc");
}

enum {
    // The GPIO registers base address.
    GPIO_BASE = 0x3F200000, // for raspi2 & 3, 0x20200000 for raspi1

    GPPUD = (GPIO_BASE + 0x94),
    GPPUDCLKO = (GPIO_BASE + 0x98)

    // The base address for UART
    UARTO_BASE = 0x3F201000, // for raspi1 & 3, 0x20201000 for raspi1

    UARTO_DR      = (UARTO_BASE + 0x00),
    UARTO_RSRECR  = (UARTO_BASE + 0x04),
    UARTO_FR      = (UARTO_BASE + 0x18),
    UARTO_ILPR    = (UARTO_BASE + 0x20),
    UARTO_IBRD    = (UARTO_BASE + 0x24),
    UARTO_FBRD    = (UARTO_BASE + 0x28),
    UARTO_LCRH    = (UARTO_BASE + 0x2C),
    UARTO_CR      = (UARTO_BASE + 0x30),
    UARTO_IFLS    = (UARTO_BASE + 0x34),
    UARTO_IMSC    = (UARTO_BASE + 0x38),
    UARTO_RIS     = (UARTO_BASE + 0x3C),
    UARTO_MIS     = (UARTO_BASE + 0x40),
    UARTO_ICR     = (UARTO_BASE + 0x44),
    UARTO_DMACR   = (UARTO_BASE + 0x48),
    UARTO_ITCR    = (UARTO_BASE + 0x80),
    UARTO_ITIP    = (UARTO_BASE + 0x84),
    UARTO_ITOP    = (UARTO_BASE + 0x88),
    UARTO_TDR     = (UARTO_BASE + 0x8C),
};

void uart_init(){
    mmio_write(UARTO_CR, 0x00000000);

    mmio_write(GPPUD, 0x00000000);
    delay(150);

    mmio_write(GPPUDCLKO, (1 << 14) | (1 << 15));
    delay(150);

    mmio_write(GPPUDCKLO, 0x00000000);

    mmio_write(UARTO_ICR, 0x7FF);

    mmio_write(UARTO_IBRD, 1);
    mmio_write(UARTO_FBRD, 40);

    mmio_write(UARTO_LCRH, (1 << 4) | (1 << 5) | (1 << 6));

    mmio_write(UARTO_IMSC, (1 << 1) | (1 << 4) | (1 << 5) | (1 <<6) |
            (1 << 7) | (1 << 8) | (1 << 9) | (1 << 10));

    mmio_write(UARTO_CR, (1 << 0) | (1 << 9));
}

void uart_putc(unsigned char c){
    
    while( mmio_read(UARTO_FR) & (1 << 5) ) { }
    mmio_write(UARTO_DR, c);
}

unsigned char uart_getc(){

    while ( mmio_read(UARTO_FR) & (1 << 4)) { }
    return mmio_read(UARTO_DR);
}

void uart_puts(const char* str) {
    for (size_t i = 0; str[i] != '\0'; i++)
        uart_putc((unsigned char)str[i]);
}

void kernel_main(uint32_t r0, uint32_t r1, uint32_t atags) {
    (void) r0;
    (void) r1;
    (void) atags;

    uart_init();
    uart_puts("Houston THE kernel is up!\r\n");

    while(1) {
        uart_putc(uart_getc());
        uart_putc('\n');
    }

}