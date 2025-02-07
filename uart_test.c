#include <lpc21xx.h>

#define pclk 15000000
#define baudrate 9600
#define divisor (pclk/(baudrate*16))

#define rs 1<<16
#define e 1<<17

void delay_ms(unsigned int ms)
{
	unsigned int i,j;
	for(i=0;i<ms;i++)
	{
		for(j=0;j<12000;j++);
	}
}

void transmit(char ch)
{
	U0THR=ch;
	while(((U0LSR>>5)&1)==0);
}

char receive(void)
{
	while(((U0LSR>>0)&1)==0);
	return U0RBR;
}

void uart_init(void)
{
	PINSEL0|=0x05;
	U0LCR=0x83;
	U0DLL=(divisor&0xff);
	U0DLM=((divisor>>8)&0xff);
	U0LCR=0x03;
}

void transmit_string(char *str)
{
	while(*str)
	{
		transmit(*str++);
	}
}

void ascii_2_int(int num)
{
	char buff[16];
	int i=0,j;
	char temp;
	while(num>0)
	{
		buff[i++]=(num%10)+'0';
		num=num/10;
	}
	buff[i]='\0';
	for(j=0;j<i/2;j++)
	{
		temp=buff[j];
		buff[j]=buff[i-j-1];
		buff[i-j-1]=temp;
	}
	transmit_string(buff);
}

void ascii_2_float(float num)
{
	int int_part,frac_part;
	int_part=(int)num;
	frac_part=(int)((num-int_part)*1000);
	ascii_2_int(int_part);
	transmit('.');
	ascii_2_int(frac_part);
}

void ascii_2_hexa(int num)
{
	int i;
	char hex_lut[]="0123456789ABCDEF";
	char buff[5];
	buff[0]='0';
	buff[1]='x';
	for(i=0;i<2;i++)
	{
		buff[3-i]=hex_lut[num&0xf];
		num=num>>4;
	}
	buff[4]='\0';
	transmit_string(buff);
}

void cmd(char cmd)
{
	IOCLR0=0XFF<<8;
	IOSET0=cmd<<8;
	IOCLR0=rs;
	delay_ms(100);
	IOSET0=e;
	delay_ms(2);
	IOCLR0=e;
	delay_ms(2);
}

void data(char data)
{
	IOCLR0=0XFF<<8;
	IOSET0=data<<8;
	IOSET0=rs;
	//delay_ms(100);
	IOSET0=e;
	delay_ms(2);
	IOCLR0=e;
	delay_ms(2);
}

void lcd_init(void)
{
	IODIR0|=0xff<<8;
	IODIR0|=(rs|e);
	cmd(0x38);//8 bit
	cmd(0x01);//clear lcd
	cmd(0x0c);//display on cursor off
	cmd(0x06);//increment
}

void lcd_string(char *str)
{
	while(*str)
	{
		data(*str++);
	}
}

void ascii_2_int_lcd(int num)
{
	char buff[16];
	int i=0,j;
	char temp;
	while(num>0)
	{
		buff[i++]=(num%10)+'0';
		num=num/10;
	}
	buff[i]='\0';
	for(j=0;j<i/2;j++)
	{
		temp=buff[j];
		buff[j]=buff[i-j-1];
		buff[i-j-1]=temp;
	}
	lcd_string(buff);
}

void ascii_2_float_lcd(float num)
{
	int int_part,frac_part;
	int_part=(int)num;
	frac_part=(int)((num-int_part)*1000);
	ascii_2_int_lcd(int_part);
	data('.');
	ascii_2_int_lcd(frac_part);
}

void ascii_2_hexa_lcd(int num)
{
	int i;
	char hex_lut[]="0123456789ABCDEF";
	char buff[5];
	buff[0]='0';
	buff[1]='x';
	for(i=0;i<2;i++)
	{
		buff[3-i]=hex_lut[num&0xf];
		num=num>>4;
	}
	buff[4]='\0';
	lcd_string(buff);
}

int main()
{
	uart_init();
	lcd_init();
	transmit_string("character: ");
	transmit('A');
	lcd_string("character: ");
	data('A');
	delay_ms(100);
	cmd(0x01);
	transmit_string("\r\n");
	transmit_string("NUMBER: ");
	ascii_2_int(65);
	lcd_string("NUMBER: ");
	ascii_2_int_lcd(65);
	delay_ms(100);
	cmd(0x01);
	transmit_string("\r\n");
	transmit_string("FLOAT: ");
	ascii_2_float(6.666);
	lcd_string("FLOAT: ");
	ascii_2_float_lcd(6.666);
	transmit_string("\r\n");
	delay_ms(100);
	cmd(0x01);
	transmit_string("HEXA: ");
	ascii_2_hexa(65);
	lcd_string("HEXA: ");
	ascii_2_hexa_lcd(65);
}
