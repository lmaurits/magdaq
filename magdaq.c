#define F_CPU	14745600UL
#define BAUD	9600

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

// Global vars

volatile uint8_t int_flag = 0;

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

// Timer 0 functions
// This is the 8 bit timer connected to the 10Hz output of the TCXO
void init_timer0() {
    // Set clock source to T0 pin
    TCCR0B |= (1<<CS01)|(1<<CS02);
    // Fire once per second
    OCR0A = 10;
    // Clear on compare
    TCCR0A |= (1<<WGM01);
    // Enable interrupts
    TIMSK |= (1<<OCIE0A);
    TIFR |= (1<<OCF0A);
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

// Interrupts

ISR(TIMER0_COMPA_vect) {
    int_flag = 1;
}

// Main

int main() {
    cli();
    init_uart();
    init_timer0();
    sei();
    while(1) {
       if(int_flag != 0) {
           send_datapoint();
           int_flag = 0;
       }
    }

    return 0; 
}

