#define F_CPU  14745600UL
#define BAUD   9600

#define DELAY_A 6
#define DELAY_B 64
#define DELAY_C 60
#define DELAY_D 10
#define DELAY_E 9
#define DELAY_F 55
#define DELAY_G 0
#define DELAY_H 480
#define DELAY_I 70
#define DELAY_J 410
#define RELEASE_BUS DDRD = 0
#define DRIVE_BUS_LOW DDRD |= 0x40; PORTD &= 0x3F;

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
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

// Temperature sensor functions
uint8_t onewire_reset() {
    uint8_t presence;
    DRIVE_BUS_LOW;
    _delay_us(DELAY_H);
    RELEASE_BUS;
    _delay_us(DELAY_I);
    presence = PIND &= 0x40;
    _delay_us(DELAY_J);
    return presence;
}

void onewire_write_byte(uint8_t x) {
    uint8_t i = 8;
    while(i) {
  if(x & 0x01) {
            DRIVE_BUS_LOW;
            _delay_us(DELAY_A);
            RELEASE_BUS;
            _delay_us(DELAY_B);
  } else {
            DRIVE_BUS_LOW;
            _delay_us(DELAY_C);
            RELEASE_BUS;
            _delay_us(DELAY_D);
  }
        x >>= 1;
        i--;
    }
}

uint8_t onewire_read_byte() {
    uint8_t x, tmp;
    x = 0;
    uint8_t i = 8;
    while(i) {
        x >>= 1;
        DRIVE_BUS_LOW;
        _delay_us(DELAY_A);
  RELEASE_BUS;
        _delay_us(DELAY_E);
  tmp = PIND &= 0x40;
        _delay_us(DELAY_F);
  if(tmp) {
      x |= 0x80;
  }
        i--;
    }
    return x;
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
    uint8_t high, low;
    cli();
    init_uart();
    init_multiplexer();
    init_timers();
    sei();
    while(1) {
        if(int_flag != 0) {
            int_flag = 0;
            // Read results of previous temperature conversion
            onewire_reset();
            onewire_write_byte(0xCC); // SKIP ROM
            onewire_write_byte(0xBE); // READ SCRATCHPAD
            low = onewire_read_byte();
            high = onewire_read_byte();
            // Send measurements
            uart_send_byte(capture_tcnt1 & 0xFF);
            uart_send_byte((capture_tcnt1 >> 8) & 0xFF);
            uart_send_byte(capture_overflows);
            uart_send_byte(low);
            uart_send_byte(high);
            // Initiate next temperature conversion
            onewire_reset();
            onewire_write_byte(0xCC); // SKIP ROM
            onewire_write_byte(0x44); // CONVERT T
        }
    }
    return 0; 
}
