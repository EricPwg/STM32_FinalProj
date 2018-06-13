#include "../inc/stm32l476xx.h"
#include "../inc/7seg.h"

#define SEGgpio GPIOA
#define SEGdin 5
#define SEGcs 6
#define SEGclk 7

void delay_without_interrupt(int msec){
	int loop_cnt = 700*msec;
	while(loop_cnt){
		loop_cnt--;
	}
	return;
}

void set_gpio(GPIO_TypeDef* gpio, int pin)
{
		gpio->BSRR |=( 1<< pin);
}
void reset_gpio(GPIO_TypeDef* gpio, int pin)
{
		gpio->BRR |=( 1<< pin);
}

void send_7seg_onebit(GPIO_TypeDef* gpio, int DIN, int CLK, int data){
	reset_gpio(gpio, CLK);
	if (data == 1) set_gpio(gpio, DIN);
	else reset_gpio(gpio, DIN);
	set_gpio(gpio, CLK);

}

void send_7seg(GPIO_TypeDef* gpio, int DIN, int CS, int CLK, int addr, int data){
	set_gpio(gpio, CLK);
	reset_gpio(gpio, CS);
	for (int i=0;i<4;i++) send_7seg_onebit(gpio, DIN, CLK, 0);
	for (int i=0;i<4;i++){
		int t = (addr >> (3-i))&0x1;
		if (t == 1) send_7seg_onebit(gpio, DIN, CLK, 1);
		else send_7seg_onebit(gpio, DIN, CLK, 0);
	}
	for (int i=0;i<8;i++){
		int t = (data >> (7-i))&0x1;
		if (t == 1) send_7seg_onebit(gpio, DIN, CLK, 1);
		else send_7seg_onebit(gpio, DIN, CLK, 0);
	}
	reset_gpio(gpio, CLK);
	set_gpio(gpio, CS);
	set_gpio(gpio, CLK);
}

int init_7seg(GPIO_TypeDef* gpio, int DIN, int CS, int CLK){
	if(gpio==GPIOA){
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	}
	else if(gpio==GPIOB){
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	}
	else return -1;

	gpio->MODER &= ~(0b11 << (2*DIN));
	gpio->MODER |= (0b01 << (2*DIN));
	gpio->MODER &= ~(0b11 << (2*CS));
	gpio->MODER |= (0b01 << (2*CS));
	gpio->MODER &= ~(0b11 << (2*CLK));
	gpio->MODER |= (0b01 << (2*CLK));

	send_7seg(gpio, DIN, CS, CLK, SEG_ADDRESS_DISPLAY_TEST, 0x00);
	return 0;
}

class Dot_array{
	private:

	public:
		int init(GPIO_TypeDef* gpio, int DIN, int CS, int CLK);
		void send_7seg16(GPIO_TypeDef* gpio, int DIN, int CS, int CLK, int addr, int data);
		void send_7seg(GPIO_TypeDef* gpio, int DIN, int CS, int CLK, int *all_data);
};

void Dot_array::send_7seg(GPIO_TypeDef* gpio, int DIN, int CS, int CLK, int *all_data){
	int data = 5;
	for (int a = 1;a<4;a++){
		int addr = a;

		set_gpio(gpio, CLK);
		reset_gpio(gpio, CS);

		for (int _tt;_tt<16;_tt++){
			for (int i=0;i<4;i++) send_7seg_onebit(gpio, DIN, CLK, 0);
			for (int i=0;i<4;i++){
				int t = (addr >> (3-i))&0x1;
				if (t == 1) send_7seg_onebit(gpio, DIN, CLK, 1);
				else send_7seg_onebit(gpio, DIN, CLK, 0);
			}
			for (int i=0;i<8;i++){
				int t = (data >> (7-i))&0x1;
				if (t == 1) send_7seg_onebit(gpio, DIN, CLK, 1);
				else send_7seg_onebit(gpio, DIN, CLK, 0);
			}

		}

		reset_gpio(gpio, CLK);
		set_gpio(gpio, CS);
		set_gpio(gpio, CLK);
		//delay_without_interrupt(1);
	}

}


void Dot_array::send_7seg16(GPIO_TypeDef* gpio, int DIN, int CS, int CLK, int addr, int data){
	set_gpio(gpio, CLK);
	reset_gpio(gpio, CS);

	for (int _t = 0;_t<16;_t++){
		for (int i=0;i<4;i++) send_7seg_onebit(gpio, DIN, CLK, 0);

		for (int i=0;i<4;i++){
			int t = (addr >> (3-i))&0x1;
			if (t == 1) send_7seg_onebit(gpio, DIN, CLK, 1);
			else send_7seg_onebit(gpio, DIN, CLK, 0);
		}
		for (int i=0;i<8;i++){
			int t = (data >> (7-i))&0x1;
			if (t == 1) send_7seg_onebit(gpio, DIN, CLK, 1);
			else send_7seg_onebit(gpio, DIN, CLK, 0);
		}
	}


	reset_gpio(gpio, CLK);
	set_gpio(gpio, CS);
	set_gpio(gpio, CLK);

}

int Dot_array::init(GPIO_TypeDef* gpio, int DIN, int CS, int CLK){
	//Enable AHB2 Clock
	if(gpio == GPIOA)
	{
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN;
	}
	else if(gpio == GPIOB)
	{
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOBEN;
	}
	else if(gpio == GPIOC)
	{
		RCC->AHB2ENR |= RCC_AHB2ENR_GPIOCEN;
	}
	else
	{
		//Error! Add other cases to suit other GPIO pins
		return -1;
	}
	//Set GPIO pins to output mode (01)
	//First Clear bits(&) then set bits (|)
	gpio->MODER &= ~(0b11 << (2*DIN));
	gpio->MODER |= (0b01 << (2*DIN));
	gpio->MODER &= ~(0b11 << (2*CS));
	gpio->MODER |= (0b01 << (2*CS));
	gpio->MODER &= ~(0b11 << (2*CLK));
	gpio->MODER |= (0b01 << (2*CLK));

	//Close display test
	send_7seg16(gpio, DIN, CS, CLK, SEG_ADDRESS_DISPLAY_TEST, 0x00);
	send_7seg16(gpio, DIN, CS, CLK, SEG_ADDRESS_SCAN_LIMIT, 0x07);
	send_7seg16(gpio, DIN, CS, CLK, SEG_ADDRESS_SHUTDOWN, 0x01);
	send_7seg16(gpio, DIN, CS, CLK, SEG_ADDRESS_ITENSITY, 0x05);
	return 0;
}

int main(){
	Dot_array A;
	A.init(SEGgpio, SEGdin, SEGcs, SEGclk);
	delay_without_interrupt(1000);
	for (int i=1;i<9;i++){
		A.send_7seg16(SEGgpio, SEGdin, SEGcs, SEGclk, i, 0);
	}
	//return 0;
	delay_without_interrupt(100);
	int show_data[16];
	show_data[0]  = 0b00000000000000000000000000000000;
	show_data[1]  = 0b00000000000000000000000000000000;
	show_data[2]  = 0b00000000000000000000000000000000;
	show_data[3]  = 0b11111111111111111111111111111111;

	show_data[4]  = 0b00000000000000000000000000000000;
	show_data[5]  = 0b00000000000000000000000000000000;
	show_data[6]  = 0b00000000000000000000000000000000;
	show_data[7]  = 0b11111111111111111111111111111111;

	show_data[8]  = 0b00000000000000000000000000000000;
	show_data[9]  = 0b00000000000000000000000000000000;
	show_data[10] = 0b00000000000000000000000000000000;
	show_data[11] = 0b00000000000000000000000000000000;

	show_data[12] = 0b00000000000000000000000000000000;
	show_data[13] = 0b00000000000000000000000000000000;
	show_data[14] = 0b00000000000000000000000000000000;
	show_data[15] = 0b00000000000000000000000000000000;
	int t = 0;
	while(1){
		A.send_7seg16(SEGgpio, SEGdin, SEGcs, SEGclk, 5, t);
		delay_without_interrupt(500);
		A.send_7seg(SEGgpio, SEGdin, SEGcs, SEGclk, show_data);
		delay_without_interrupt(500);
		t++;
	}


	return 0;
}
