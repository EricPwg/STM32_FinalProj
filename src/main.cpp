#include "../inc/stm32l476xx.h"
#include "../inc/7seg.h"

#define SEGgpio GPIOA
#define SEGdin 5
#define SEGcs 6
#define SEGclk 7

#define BUTTON_gpio GPIOC
#define BUTTON_pin 13

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

int read_gpio(GPIO_TypeDef* gpio, int pin)
{
	if((gpio->IDR)>>pin & 1)
		return 1; //no touch 傳1
	else
		return 0; //touch 傳0
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

#define PIPLISTLEN 7

class Dot_array{
	private:
		GPIO_TypeDef* data_gpio;
		int DIN_pin;
		int CS_pin;
		int CLK_pin;
		int pipe_list_len = PIPLISTLEN;
		int flappy_bird_list[PIPLISTLEN][3] = {
				{10, 5, 20},
				{15, 5, 20},
				{30, 5, 15},
				{45, 20, 30},
				{60, 5, 20},
				{80, 5, 10},
				{100, 25, 30}
		}; //x座標, 上柱下緣, 下柱上緣
	public:
		int init(GPIO_TypeDef* gpio, int DIN, int CS, int CLK);
		void send_7seg16(int addr, int data);
		void send_7seg(int *all_data);
		void show_flappy_bird(int x, int y, int state);
		void flappy_bird_set();
		void get_xy_coordinate(int _x, int _y, int *shift, int *y);
		bool check_if_end(int x, int y, int state);
};

void Dot_array::flappy_bird_set(){
	return;
}

void Dot_array::get_xy_coordinate(int _x, int _y, int *shift, int *y){
	if (_y <= 7) *y = 7-_y;
	else if(_y <= 15) *y = 23-_y;
	else if(_y <= 23) *y = 39-_y;
	else *y = 55-_y;
	*shift = 31-_x;
}

bool Dot_array::check_if_end(int x, int y, int state){
	for (int i=0;i<pipe_list_len;i++){
		int pipe_x = flappy_bird_list[i][0]-x;
		if (pipe_x > 5) break;
		if ((pipe_x <= 5 && pipe_x >=1) && (y<=flappy_bird_list[i][1] || y>=flappy_bird_list[i][2])) return true;
	}
	return false;
}

void Dot_array::show_flappy_bird(int x, int y, int state){
	int show_data[32];
	show_data[7]  = 0b11111111111111111111111111111111;
	show_data[6]  = 0b00000000000000000000000000000000;
	show_data[5]  = 0b00000000000000000000000000000000;
	show_data[4]  = 0b00000000000000000000000000000000;
	show_data[3]  = 0b00000000000000000000000000000000;
	show_data[2]  = 0b00000000000000000000000000000000;
	show_data[1]  = 0b00000000000000000000000000000000;
	show_data[0]  = 0b00000000000000000000000000000000;
	show_data[15] = 0b00000000000000000000000000000000;
	show_data[14] = 0b00000000000000000000000000000000;
	show_data[13] = 0b00000000000000000000000000000000;
	show_data[12] = 0b00000000000000000000000000000000;
	show_data[11] = 0b00000000000000000000000000000000;
	show_data[10] = 0b00000000000000000000000000000000;
	show_data[9]  = 0b00000000000000000000000000000000;
	show_data[8]  = 0b00000000000000000000000000000000;
	show_data[23] = 0b00000000000000000000000000000000;
	show_data[22] = 0b00000000000000000000000000000000;
	show_data[21] = 0b00000000000000000000000000000000;
	show_data[20] = 0b00000000000000000000000000000000;
	show_data[19] = 0b00000000000000000000000000000000;
	show_data[18] = 0b00000000000000000000000000000000;
	show_data[17] = 0b00000000000000000000000000000000;
	show_data[16] = 0b00000000000000000000000000000000;
	show_data[31] = 0b00000000000000000000000000000000;
	show_data[30] = 0b00000000000000000000000000000000;
	show_data[29] = 0b00000000000000000000000000000000;
	show_data[28] = 0b00000000000000000000000000000000;
	show_data[27] = 0b00000000000000000000000000000000;
	show_data[26] = 0b00000000000000000000000000000000;
	show_data[25] = 0b00000000000000000000000000000000;
	show_data[24] = 0b11111111111111111111111111111111;
	int co_x;
	int co_y;
	get_xy_coordinate(3, y, &co_x, &co_y); //bird
	show_data[co_y] |= (3 << co_x);
	get_xy_coordinate(3, y+state, &co_x, &co_y);
	show_data[co_y] |= (5 << co_x);

	for (int i=0;i<pipe_list_len;i++){
		int pipe_x = flappy_bird_list[i][0]-x;
		if (pipe_x > 32) break;
		if (pipe_x >= 0){
			for (int j = 0;j<32;j++){
				if (j<=flappy_bird_list[i][1] || j>=flappy_bird_list[i][2]){
					get_xy_coordinate(pipe_x, j, &co_x, &co_y);
					show_data[co_y] |= (7 << co_x);
				}

			}
		}
	}
	send_7seg(show_data);
}

void Dot_array::send_7seg(int *all_data){
	int data_arr[16];

	for (int addr = 8; addr > 0 ;addr--){

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

		set_gpio(data_gpio, CLK_pin);
		reset_gpio(data_gpio, CS_pin);
		for (int _tt = 0;_tt<16;_tt++){
			int data = data_arr[_tt];
			for (int i=0;i<4;i++) send_7seg_onebit(data_gpio, DIN_pin, CLK_pin, 0);
			for (int i=0;i<4;i++){
				int t = (addr >> (3-i))&0x1;
				if (t == 1) send_7seg_onebit(data_gpio, DIN_pin, CLK_pin, 1);
				else send_7seg_onebit(data_gpio, DIN_pin, CLK_pin, 0);
			}
			for (int i=0;i<8;i++){
				int t = (data >> (i))&0x1;
				if (t == 1) send_7seg_onebit(data_gpio, DIN_pin, CLK_pin, 1);
				else send_7seg_onebit(data_gpio, DIN_pin, CLK_pin, 0);
			}

		}

		reset_gpio(data_gpio, CLK_pin);
		set_gpio(data_gpio, CS_pin);
		set_gpio(data_gpio, CLK_pin);
		//delay_without_interrupt(10);
	}

}


void Dot_array::send_7seg16(int addr, int data){
	set_gpio(data_gpio, CLK_pin);
	reset_gpio(data_gpio, CS_pin);

	for (int _t = 0;_t<16;_t++){
		for (int i=0;i<4;i++) send_7seg_onebit(data_gpio, DIN_pin, CLK_pin, 0);
		for (int i=0;i<4;i++){
			int t = (addr >> (3-i))&0x1;
			if (t == 1) send_7seg_onebit(data_gpio, DIN_pin, CLK_pin, 1);
			else send_7seg_onebit(data_gpio, DIN_pin, CLK_pin, 0);
		}
		for (int i=0;i<8;i++){
			int t = (data >> (7-i))&0x1;
			if (t == 1) send_7seg_onebit(data_gpio, DIN_pin, CLK_pin, 1);
			else send_7seg_onebit(data_gpio, DIN_pin, CLK_pin, 0);
		}
	}


	reset_gpio(data_gpio, CLK_pin);
	set_gpio(data_gpio, CS_pin);
	set_gpio(data_gpio, CLK_pin);

}

int Dot_array::init(GPIO_TypeDef* gpio, int DIN, int CS, int CLK){
	//Enable AHB2 Clock
	data_gpio = gpio;
	DIN_pin = DIN;
	CS_pin = CS;
	CLK_pin = CLK;

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
	send_7seg16(SEG_ADDRESS_DISPLAY_TEST, 0x00);
	send_7seg16(SEG_ADDRESS_SCAN_LIMIT, 0x07);
	send_7seg16(SEG_ADDRESS_DECODE_MODE, 0x00);
	send_7seg16(SEG_ADDRESS_SHUTDOWN, 0x01);
	send_7seg16(SEG_ADDRESS_ITENSITY, 0x05);
	return 0;
}

int init_button(GPIO_TypeDef* gpio, int button_pin)
{
	//Enable AHB2 Clock
	if(gpio==GPIOC)
	{
		RCC->AHB2ENR |=RCC_AHB2ENR_GPIOCEN;
	}
	else
	{
		//Error! Add other cases to suit other GPIO pins
		return -1;
	}
	//Set GPIO pins to input mode (00)
	//First Clear bits(&) then set bits (|)
	gpio->MODER &= ~(0b11<<(2*button_pin));
	gpio->MODER |= (0b00<<(2*button_pin));

	return 0;
}

int main(){
	init_button(BUTTON_gpio, BUTTON_pin);
	Dot_array A;
	A.init(SEGgpio, SEGdin, SEGcs, SEGclk);
	A.flappy_bird_set();
	delay_without_interrupt(1000);
	for (int i=1;i<9;i++){
		A.send_7seg16(i, 0);
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
	int bird_y = 10;
	int cod_x = 0;
	int state = 0;
	while(1){
		//A.send_7seg16(SEGgpio, SEGdin, SEGcs, SEGclk, 5, t);
		//delay_without_interrupt(100);
		int rb = read_gpio(BUTTON_gpio, BUTTON_pin);
		if(rb){
			bird_y++;
			state = -1;
		}
		else{
			bird_y--;
			state = 1;
		}
		if (bird_y < 1) bird_y = 1;
		if (bird_y > 30) bird_y = 30;
		cod_x++;
		A.show_flappy_bird(cod_x/4, bird_y, state);
		if(A.check_if_end(cod_x/4, bird_y, state)) break;
		//A.show_flappy_bird(3, 8);
		//A.show_flappy_bird(3, 7);
	}
	for (int i=0;i<5;i++){
		A.send_7seg16(SEG_ADDRESS_ITENSITY, 0x00);
		delay_without_interrupt(100);
		A.send_7seg16(SEG_ADDRESS_ITENSITY, 0x09);
		delay_without_interrupt(100);
	}
	return 0;
}
