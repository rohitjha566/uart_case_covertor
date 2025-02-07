#include <lpc21xx.h>
#include <ctype.h>

#define pclk 15000000
#define baurdrate 9600
#define divisor (pclk/(baurdrate*16))

#define rs (1 << 16)
#define e (1 << 17)

void delay_ms(unsigned int ms) {
    int i, j;
    for (i = 0; i < ms; i++) {
        for (j = 0; j < 12000; j++);
    }
}

void cmd(char cmd) {
    IOCLR0 = 0xff << 8;
    IOSET0 = cmd << 8;
    IOCLR0 = rs;
    IOSET0 = e;
    delay_ms(2);
    IOCLR0 = e;
    delay_ms(2);
}

void data(char data) {
    IOCLR0 = 0xFF << 8;
    IOSET0 = data << 8;
    IOSET0 = rs;
    IOSET0 = e;
    delay_ms(2);
    IOCLR0 = e;
    delay_ms(2);
}

void LCD_string(char *str) {
    while (*str) {
        data(*str++);
    }
}

void LCD_init(void) {
    IODIR0 |= 0xff << 8;
    IODIR0 |= (rs | e);
    cmd(0x38); // 8bit 2line 5x7 display
    cmd(0x0C); // Display on, cursor off
    cmd(0x06); // Entry mode cursor increment
    cmd(0x01); // Clear display
}

void uart_init(void) {
    PINSEL0 |= 0x05;
    U0LCR = 0x83;
    U0DLL = (divisor & 0xFF);
    U0DLM = ((divisor >> 8) & 0xFF);
    U0LCR = 0x03;
}

void transmit(char ch) {
    U0THR = ch;
    while (((U0LSR >> 5) & 1) == 0);
}

char receive(void) {
    while (((U0LSR >> 0) & 1) == 0);
    return U0RBR;
}

void uart_string(char *str) {
    while (*str) {
        transmit(*str++);
    }
}

void clear_LCD(void) {
    cmd(0x01); // Clear display
    delay_ms(2);
    cmd(0x80); // Move cursor to the beginning of the first line
}

void receive_string(char *buffer, int maxLength) {
    char ch;
    int i = 0;
	//clear_LCD();
    while (i < maxLength - 1 && (ch = receive()) != '\r' && ch != '\n') { // Consider both '\r' and '\n' as terminators
        transmit(ch); // Echo back the received character
        data(ch); // Display the received character on LCD
        buffer[i++] = ch;
    }
    buffer[i] = '\0';
    uart_string("\r\n");
}

void convert_case(char* str) {
    int i = 0, start = 0;
    while (str[i] != '\0') {
        // Skip spaces
        while (str[i] == ' ') {
            i++;
        }
        start = i;
        // Find the end of the word
        while (str[i] != ' ' && str[i] != '\0') {
            i++;
        }
        // Convert first and last letter to uppercase
        if (start < i) {
            str[start] = toupper(str[start]);
            str[i-1] = toupper(str[i-1]);
        }
    }
}

int main() {
    char buffer[100];  // Adjust size as needed
    uart_init();
    delay_ms(100); // Give some time for the LCD to power up and initialize
    LCD_init();

    while (1) {
        uart_string("Enter string: ");
        receive_string(buffer, 100);  // Receive the string from UART
        convert_case(buffer);         // Convert the first and last letters to uppercase
        uart_string("Modified string: ");
        uart_string(buffer);          // Transmit the modified string
        uart_string("\r\n");
			  cmd(0xc0);
        LCD_string(buffer);           // Display the modified string on LCD
			  delay_ms(400);
			clear_LCD();
    }
}
