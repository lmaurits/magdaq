#define F_CPU	14745600UL
#define BAUD	9600

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/setbaud.h>

// Global vars

volatile uint8_t int_flag = 0;
volatile uint8_t overflows = 0;
volatile uint8_t capture_overflows = 0;
volatile uint16_t capture_tcnt1 = 0;

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

// Multiplexer function
void init_multiplexer() {
    // Set PB0 and PB1 as outputs, and set them low
    DDRB |= 3;
    PORTB = 0;
}

// Timer functions
// Timer 0 is the 8 bit timer connected to the 10Hz output of the TCXO
// Timer 1 is the 16 bit timer connected to the magnetometer(s)
void init_timers() {
    /* Timer 0 */
    // Set clock source to T0 pin
    TCCR0B |= (1<<CS01)|(1<<CS02);
    // Fire once per second
    OCR0A = 10;
    // Clear on compare
    TCCR0A |= (1<<WGM01);

    /* Timer 1 */
    // Set clock source to T1 pin
    TCCR1B |= (1<<CS11)|(1<<CS12);

    // Enable interrupts
    TIMSK |= (1<<OCIE0A)|(1<<TOIE1);
}

void send_datapoint() {
    
    uart_send_byte(capture_tcnt1 & 0xFF);
    uart_send_byte((capture_tcnt1 >> 8) & 0xFF);
    uart_send_byte(capture_overflows);
    
    uart_send_byte(0xAA);
    uart_send_byte(0xBB);
    uart_send_byte(0xCC);
    uart_send_byte(0xDD);
    uart_send_byte(0xEE);
}

// Interrupts

ISR(TIMER0_COMPA_vect) {
    capture_tcnt1 = TCNT1;
    TCNT1 = 0;
    capture_overflows = overflows;
    overflows = 0;
    int_flag = 1;
}

ISR(TIMER1_OVF_vect) {
    overflows++;
}

// Main

int main() {
    cli();
    init_uart();
    init_multiplexer();
    init_timers();
    sei();
    while(1) {
       if(int_flag != 0) {
           send_datapoint();
           int_flag = 0;
       }
    }

    return 0; 
}

