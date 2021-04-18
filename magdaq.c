#define F_CPU	14745600UL
#define BAUD	9600

#include <avr/io.h>
#include <util/setbaud.h>

// UART functions

void init_uart() {
    // Set baud rate
    UBRRH = UBRRH_VALUE;
    UBRRL = UBRRL_VALUE;
    // Set frame format to 8 data bits, no parity, 1 stop bit
    UCSRC |= (1<<UCSZ1)|(1<<UCSZ0);
    // Enable receiver and transmitter
    UCSRB = (1<<RXEN)|(1<<TXEN);
}

uint8_t uart_read_byte() {
    while(!(UCSRA & (1<<RXC)));
    return UDR;
}

void uart_send_byte(uint8_t out) {
    while(!(UCSRA & (1<<UDRE)));
    UDR = out;
}

void send_datapoint() {
    // Just say "DEAD BEEF\n" for now
    uart_send_byte(0x44);
    uart_send_byte(0x45);
    uart_send_byte(0x41);
    uart_send_byte(0x44);

    uart_send_byte(0x20);
    
    uart_send_byte(0x42);
    uart_send_byte(0x45);
    uart_send_byte(0x45);
    uart_send_byte(0x46);
    
    uart_send_byte(0x0A);
    uart_send_byte(0x0D);
}

// Main

int main() {
    init_uart();
    send_datapoint();
    return 0; 
}

