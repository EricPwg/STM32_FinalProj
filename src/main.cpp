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


class Dot_array{
	private:
		GPIO_TypeDef* data_gpio;
		int DIN_pin;
		int CS_pin;
		int CLK_pin;
	public:
		int init(GPIO_TypeDef* gpio, int DIN, int CS, int CLK);
		void send_7seg16(int addr, int data);
		void send_7seg(int *all_data);
		void show_flappy_bird(int x, int y, int state);
		void get_xy_coordinate(int _x, int _y, int *shift, int *y);
		bool check_if_end(int x, int y, int state);
};




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

#define PIPLISTLEN 9

class Flappy_bird{
	private:
		Dot_array *A;
		int bird_y;
		int cod_x;
		int state;
		int pipe_list_len = PIPLISTLEN;
		int flappy_bird_list[PIPLISTLEN][3] = {
				{10, 5, 20},
				{15, 5, 20},
				{30, 5, 15},
				{45, 20, 30},
				{60, 5, 20},
				{80, 5, 13},
				{100, 20, 30},
				{110, 20, 31},
				{121, 1, 10}
		}; //x座標, 上柱下緣, 下柱上緣
		int num[10][7] = {
				{0b0110,0b1001,0b1001,0b1001,0b1001,0b1001,0b0110},
				{0b1110,0b0100,0b0100,0b0100,0b0100,0b1100,0b0100},
				{0b1111,0b1000,0b0100,0b0010,0b0001,0b1001,0b0110},
				{0b0110,0b1001,0b0001,0b0010,0b0001,0b1001,0b0110},
				{0b0001,0b0001,0b1111,0b1001,0b0101,0b0011,0b0001},
				{0b0110,0b1001,0b0001,0b0001,0b1110,0b1000,0b1111},
				{0b0110,0b1001,0b1001,0b1110,0b1000,0b1000,0b0110},
				{0b1000,0b1000,0b0100,0b0010,0b0001,0b0001,0b1111},
				{0b0110,0b1001,0b1001,0b0110,0b1001,0b1001,0b0110},
				{0b0110,0b0001,0b0001,0b0111,0b1001,0b1001,0b0110}
		};
	public:
		Flappy_bird(Dot_array *_A);
		void init();
		int run();
		void show_flappy_bird(int x, int y, int state);
		void get_xy_coordinate(int _x, int _y, int *shift, int *y);
		bool check_if_end(int x, int y, int state, int *pass_pipe);
		void end(int point);
		void win();
};

void Flappy_bird::win(){
	int win[32];
	win[7] = 0b00000000000000000000000000000000;
	win[6] = 0b00001000001000111100010000100000;
	win[5] = 0b00000100010001000010010000100000;
	win[4] = 0b00000011100001000010010000100000;
	win[3] = 0b00000001000001000010010000100000;
	win[2] = 0b00000001000001000010010000100000;
	win[1] = 0b00000001000001000010010000100000;
	win[0] = 0b00000001000001000010010000100000;
	win[15] =0b00000001000000111100001111000000;
	win[14] =0b00000000000000000000000000000000;
	win[13] =0b00000000000000000000000000000000;
	win[12] =0b00001000001001111100100000010000;
	win[11] =0b00001000001000010000110000010000;
	win[10] =0b00001000001000010000101000010000;
	win[9] = 0b00001000001000010000100100010000;
	win[8] = 0b00001000001000010000100010010000;
	win[23] =0b00001001001000010000100001010000;
	win[22] =0b00001001001000010000100000110000;
	win[21] =0b00000110110001111100100000010000;
	win[20] =0b00000000000000000000000000000000;
	win[19] =0b00000000000000000000000000000000;
	win[18] =0b00000000000000000000000000000000;
	win[17] =0b00000000000000000000000000000000;
	win[16] =0b00000000000000000000000000000000;
	win[31] =0b00000000000000000000000000000000;
	win[30] =0b00000000000000000000000000000000;
	win[29] =0b00000000000000000000000000000000;
	win[28] =0b00000000000000000000000000000000;
	win[27] =0b00000000000000000000000000000000;
	win[26] =0b00000000000000000000000000000000;
	win[25] =0b00000000000000000000000000000000;
	win[24] =0b00000000000000000000000000000000;
	int point = PIPLISTLEN;
	if (point == 0){
		for (int i=25;i<32;i++){
			win[i] = (num[0][i-25] << 8);
		}
	}
	else{
		int n = 1;
		while(point){
			int digit = point%10;
			for (int i=25;i<32;i++){
				win[i] |= (num[digit][i-25] << 6*n);
			}
			n++;
			point /= 10;
		}
	}
	A->send_7seg(win);
	delay_without_interrupt(1000);
	while(1){
		int count = 0;
		for (int i=0;i<100;i++){
			count += read_gpio(BUTTON_gpio, BUTTON_pin);
		}
		if (count < 70) break;
	}
}

void Flappy_bird::end(int point){
	int game_over[32];
	game_over[7] = 0b00000000000000000000000000000000;
	game_over[6] = 0b00000000000000000000000000000000;
	game_over[5] = 0b00000000000000000000000000000000;
	game_over[4] = 0b00000000000000000000000000000000;
	game_over[3] = 0b00001111100111100100010111110000;
	game_over[2] = 0b00010000001000010110110100000000;
	game_over[1] = 0b00010000001000010101010100000000;
	game_over[0] = 0b00010011101111110100010111110000;
	game_over[15] =0b00010000101000010100010100000000;
	game_over[14] =0b00010000101000010100010100000000;
	game_over[13] =0b00001111101000010100010111110000;
	game_over[12] =0b00000000000000000000000000000000;
	game_over[11] =0b00000000000000000000000000000000;
	game_over[10] =0b00000000000000000000000000000000;
	game_over[9] = 0b00001111001000101111101111100000;
	game_over[8] = 0b00010000101000101000001000010000;
	game_over[23] =0b00010000101000101000001000010000;
	game_over[22] =0b00010000101000101111101111100000;
	game_over[21] =0b00010000101000101000001011000000;
	game_over[20] =0b00010000100101001000001000100000;
	game_over[19] =0b00001111000010001111101000010000;
	game_over[18] =0b00000000000000000000000000000000;
	game_over[17] =0b00000000000000000000000000000000;
	game_over[16] =0b00000000000000000000000000000000;
	game_over[31] =0b00000000000000000000000000000000;
	game_over[30] =0b00000000000000000000000000000000;
	game_over[29] =0b00000000000000000000000000000000;
	game_over[28] =0b00000000000000000000000000000000;
	game_over[27] =0b00000000000000000000000000000000;
	game_over[26] =0b00000000000000000000000000000000;
	game_over[25] =0b00000000000000000000000000000000;
	game_over[24] =0b00000000000000000000000000000000;


	if (point == 0){
		for (int i=25;i<32;i++){
			game_over[i] = (num[0][i-25] << 8);
		}
	}
	else{
		int n = 1;
		while(point){
			int digit = point%10;
			for (int i=25;i<32;i++){
				game_over[i] |= (num[digit][i-25] << 6*n);
			}
			n++;
			point /= 10;
		}
	}
	A->send_7seg(game_over);
	delay_without_interrupt(1000);
	while(1){
		int count = 0;
		for (int i=0;i<100;i++){
			count += read_gpio(BUTTON_gpio, BUTTON_pin);
		}
		if (count < 70) break;
	}

}

Flappy_bird::Flappy_bird(Dot_array *_A){
	A = _A;
	bird_y = 10;
	cod_x = 0;
	state = 0;
}

void Flappy_bird::init(){
	int flappy_bird[32];
	flappy_bird[7] = 0b00000000000000000000000000000000;
	flappy_bird[6] = 0b01111010000011001110011100100010;
	flappy_bird[5] = 0b01000010000100101001010010010100;
	flappy_bird[4] = 0b01000010000100101001010010001000;
	flappy_bird[3] = 0b01111010000111101110011100001000;
	flappy_bird[2] = 0b01000010000100101000010000001000;
	flappy_bird[1] = 0b01000010000100101000010000001000;
	flappy_bird[0] = 0b01000011110100101000010000001000;
	flappy_bird[15] =0b00000000000000000000000000000000;
	flappy_bird[14] =0b00001111001111101111001111000000;
	flappy_bird[13] =0b00001000100010001000101000100000;
	flappy_bird[12] =0b00001000100010001000101000100000;
	flappy_bird[11] =0b00001111000010001111001000100000;
	flappy_bird[10] =0b00001000100010001110001000100000;
	flappy_bird[9] = 0b00001000100010001001001000100000;
	flappy_bird[8] = 0b00001111001111101000101111000000;
	flappy_bird[23] =0b00000000000000000000000000000000;
	flappy_bird[22] =0b00000001111111111111111100000000;
	flappy_bird[21] =0b00000010000000000000000010000000;
	flappy_bird[20] =0b00000010111010000100101010000000;
	flappy_bird[19] =0b00000010101010001010010010000000;
	flappy_bird[18] =0b00000010111010001010010010000000;
	flappy_bird[17] =0b00000010100010001110010010000000;
	flappy_bird[16] =0b00000010100011101010010010000000;
	flappy_bird[31] =0b00000010000000000000000010000000;
	flappy_bird[30] =0b00000001111111111111111100000000;
	flappy_bird[29] =0b00000000000000000000000001111000;
	flappy_bird[28] =0b00000000000000000000000001110000;
	flappy_bird[27] =0b00000000000000000000000001111000;
	flappy_bird[26] =0b00000000000000000000000001011100;
	flappy_bird[25] =0b00000000000000000000000000001110;
	flappy_bird[24] =0b00000000000000000000000000000110;
	bird_y = 10;
	cod_x = 0;
	state = 0;
	A->send_7seg(flappy_bird);
	delay_without_interrupt(1000);
	while(1){
		int count = 0;
		for (int i=0;i<100;i++){
			count += read_gpio(BUTTON_gpio, BUTTON_pin);
		}
		if (count < 70) break;
	}

}

void Flappy_bird::get_xy_coordinate(int _x, int _y, int *shift, int *y){
	if (_y <= 7) *y = 7-_y;
	else if(_y <= 15) *y = 23-_y;
	else if(_y <= 23) *y = 39-_y;
	else *y = 55-_y;
	*shift = 31-_x;
}

bool Flappy_bird::check_if_end(int x, int y, int state, int *pass_pipe){
	if (x > flappy_bird_list[PIPLISTLEN-1][0]){
		*pass_pipe = PIPLISTLEN;
		return true;
	}
	for (int i=0;i<pipe_list_len;i++){
		int pipe_x = flappy_bird_list[i][0]-x;
		if (pipe_x > 5) break;
		if ((pipe_x <= 5 && pipe_x >=1) && (y<=flappy_bird_list[i][1] || y>=flappy_bird_list[i][2])){
			*pass_pipe = i;
			return true;
		}
	}
	return false;
}

void Flappy_bird::show_flappy_bird(int x, int y, int state){
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
	A->send_7seg(show_data);
}

int Flappy_bird::run(){
	int end_point;
	while(1){
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
		show_flappy_bird(cod_x/4, bird_y, state);
		if(check_if_end(cod_x/4, bird_y, state, &end_point)) break;
	}
	return end_point;
}

int main(){
	init_button(BUTTON_gpio, BUTTON_pin);
	Dot_array A;
	A.init(SEGgpio, SEGdin, SEGcs, SEGclk);
	Flappy_bird B(&A);
	//return 0;
	delay_without_interrupt(100);

	while(1){
		B.init();
		int point = B.run();
		for (int i=0;i<5;i++){
			A.send_7seg16(SEG_ADDRESS_ITENSITY, 0x00);
			delay_without_interrupt(100);
			A.send_7seg16(SEG_ADDRESS_ITENSITY, 0x09);
			delay_without_interrupt(100);
		}
		if (point == PIPLISTLEN) B.win();
		else B.end(point);
	}
	for (int i=0;i<5;i++){
		A.send_7seg16(SEG_ADDRESS_ITENSITY, 0x00);
		delay_without_interrupt(100);
		A.send_7seg16(SEG_ADDRESS_ITENSITY, 0x09);
		delay_without_interrupt(100);
	}
	return 0;
}
