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
	//int data = 5;
	int data_arr[16];

	for (int addr = 1; addr < 9 ;addr++){

		data_arr[12] = all_data[addr-1]&0xFF;
		data_arr[13] = (all_data[addr-1] >> 8)&0xFF;
		data_arr[14] = (all_data[addr-1] >> 16)&0xFF;
		data_arr[15] = (all_data[addr-1] >> 24)&0xFF;

		data_arr[8] = all_data[addr+7]&0xFF;
		data_arr[9] = (all_data[addr+7] >> 8)&0xFF;
		data_arr[10] = (all_data[addr+7] >> 16)&0xFF;
		data_arr[11] = (all_data[addr+7] >> 24)&0xFF;

		data_arr[4] = all_data[addr+15]&0xFF;
		data_arr[5] = (all_data[addr+15] >> 8)&0xFF;
		data_arr[6] = (all_data[addr+15] >> 16)&0xFF;
		data_arr[7] = (all_data[addr+15] >> 24)&0xFF;

		data_arr[0] = all_data[addr+23]&0xFF;
		data_arr[1] = (all_data[addr+23] >> 8)&0xFF;
		data_arr[2] = (all_data[addr+23] >> 16)&0xFF;
		data_arr[3] = (all_data[addr+23] >> 24)&0xFF;
		/*
		data_arr[0] = all_data[addr-1] & 0xFF;
		data_arr[1] = 2;
		data_arr[2] = 3;
		data_arr[3] = 4;
		data_arr[4] = 5;
		data_arr[5] = 6;
		data_arr[6] = 7;
		data_arr[7] = 8;
		data_arr[8] = 9;
		data_arr[9] = 10;
		data_arr[10] = 11;
		data_arr[11] = 12;
		data_arr[12] = 13;
		data_arr[13] = 14;
		data_arr[14] = 15;
		data_arr[15] = 16;
		*/
		set_gpio(gpio, CLK);
		reset_gpio(gpio, CS);
		for (int _tt = 0;_tt<16;_tt++){
			int data = data_arr[_tt];
			for (int i=0;i<4;i++) send_7seg_onebit(gpio, DIN, CLK, 0);
			for (int i=0;i<4;i++){
				int t = (addr >> (3-i))&0x1;
				if (t == 1) send_7seg_onebit(gpio, DIN, CLK, 1);
				else send_7seg_onebit(gpio, DIN, CLK, 0);
			}
			for (int i=0;i<8;i++){
				int t = (data >> (i))&0x1;
				if (t == 1) send_7seg_onebit(gpio, DIN, CLK, 1);
				else send_7seg_onebit(gpio, DIN, CLK, 0);
			}

		}

		reset_gpio(gpio, CLK);
		set_gpio(gpio, CS);
		set_gpio(gpio, CLK);
		delay_without_interrupt(10);
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
	send_7seg16(gpio, DIN, CS, CLK, SEG_ADDRESS_DECODE_MODE, 0x00);
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
	int show_data[32];
	show_data[7]  = 0b00000000000000000000000000001111;
	show_data[6]  = 0b00000000000000000000000000000000;
	show_data[5]  = 0b00000000000000000000000000000000;
	show_data[4]  = 0b11111111111111111111111111111111;
	show_data[3]  = 0b00000000000000000000000000000000;
	show_data[2]  = 0b00000000000000000000000000000000;
	show_data[1]  = 0b00000000000000000000000000000000;
	show_data[0]  = 0b11111111111111111111111111111111;
	show_data[15] = 0b00000000000000000000000000000000;
	show_data[14] = 0b00000000000000000000000000000000;
	show_data[13] = 0b00000000000000000000000000000000;
	show_data[12] = 0b00011111111111111111111111111111;
	show_data[11] = 0b00000000000000000000000000000000;
	show_data[10] = 0b00000000000000000000000000000000;
	show_data[9]  = 0b00000000000000000000000000000000;
	show_data[8]  = 0b00000000000000000000000000000000;
	show_data[23] = 0b00000000000000000000000000000000;
	show_data[22] = 0b00000000000000000000000000000000;
	show_data[21] = 0b00000000000000000000000000000000;
	show_data[20] = 0b00011111111111111111111111111111;
	show_data[19] = 0b00000000000000000000000000000000;
	show_data[18] = 0b00000000000000000000000000000000;
	show_data[17] = 0b00000000000000000000000000000000;
	show_data[16] = 0b00000000000000000000000000000000;
	show_data[31] = 0b00000000000000000000000000000000;
	show_data[30] = 0b00000000000000000000000000000000;
	show_data[29] = 0b00000000000000000000000000000000;
	show_data[28] = 0b00011111111111111111111111111111;
	show_data[27] = 0b00000000000000000000000000000000;
	show_data[26] = 0b00000000000000000000000000000000;
	show_data[25] = 0b00000000000000000000000000000000;
	show_data[24] = 0b00000000000000000000000000000000;
	int t = 0;
	while(1){
		//A.send_7seg16(SEGgpio, SEGdin, SEGcs, SEGclk, 5, t);
		delay_without_interrupt(500);
		A.send_7seg(SEGgpio, SEGdin, SEGcs, SEGclk, show_data);
		for (int i=0;i<5;i++){
			for (int j=0;j<32;j++){
					show_data[j] <<= 1;

				}
			delay_without_interrupt(10);
			A.send_7seg(SEGgpio, SEGdin, SEGcs, SEGclk, show_data);
			for (int j=0;j<32;j++){
				show_data[j] >>= 1;

			}
			delay_without_interrupt(10);
			A.send_7seg(SEGgpio, SEGdin, SEGcs, SEGclk, show_data);
		}




	}


	return 0;
}
