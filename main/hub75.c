#include "hub75.h"
#include <string.h>
#include "ASCII.h"
#include "esp_log.h"

#include "HUB_75_FONT_6.h"
#include "HUB75_FONT_11.h"
#include "HUB75_FONT16.h"
#include "HUB75_FONT22.h"
#include "monitor_anime.h"  

static const char *TAG = "SPI";
static spi_device_handle_t spi;

volatile unsigned char row = 0;
unsigned char mon_an_P = 0; // Monitor Animation Pointer

void hub75_init(void){
	gpio_reset_pin(A_ROW);
	gpio_reset_pin(B_ROW);
	gpio_reset_pin(C_ROW);
	gpio_reset_pin(D_ROW);
	gpio_reset_pin(LATCH);
	gpio_reset_pin(OE);

	gpio_config_t io_conf = {
        .pin_bit_mask = ROW_GPIO_OUTPUT_MASK,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

	gpio_config_t io_conf1 = {
        .pin_bit_mask = (1ULL<<LATCH) | (1ULL<<OE),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf1);
}

char rBuff[HEIGHT][WIDTH/8] = {0};
char gBuff[HEIGHT][WIDTH/8] = {0};
char bBuff[HEIGHT][WIDTH/8] = {0};

char row_data[16][48] = {0};

char hub75_buf_1[50] = {0};
char hub75_buf_2[50] = {0};
char OLD_hub75_buf_1[50] = {0};
char OLD_hub75_buf_2[50] = {0};


/**
 * Disable Output
 */
void out_off(void){
	gpio_set_level(OE,1);
}

/**
 * Enable Output
 */
void out_on(void){
	gpio_set_level(OE,0);
}

/**
 * Latch Data(Send Data to Display)
 */
void latch_data(void){
	gpio_set_level(LATCH,1);
	esp_rom_delay_us(200);
	gpio_set_level(LATCH,0);
}

/**
 * @brief Set Pixel
 * @param x X Coordinate
 * @param y Y Coordinate
 * @param buff Buffer
 */
void setPixel(unsigned char x, unsigned char y, char buff[][WIDTH/8]){
	if ( (x >= WIDTH) || (y >= HEIGHT) ) return;
	buff[y][(x / 8) % 8] |= (1 << (7-(x % 8)) );
}

/**
 * @brief Clear Pixel
 * @param x X Coordinate
 * @param y Y Coordinate
 * @param buff Buffer
 */
void ClrPixel(unsigned char x, unsigned char y, char buff[][WIDTH/8]){
	if ( (x >= WIDTH) || (y >= HEIGHT) ) return;
	buff[y][(x / 8) % 8] &= ~(1 << (7-(x % 8)) );
}

/**
 * @brief Draw Byte
 * @param x X Coordinate
 * @param y Y Coordinate
 * @param byte Byte
 * @param buff Buffer
 */
void drawByte(unsigned char x, unsigned char y, char byte, char buff[][WIDTH/8]){
	unsigned char i;
	for(i=0;i<8;i++) {
		if (byte&(1<<(7-i))) setPixel(x + i, y, buff);
	}
}

/**
 * @brief Select Row
 * @param row Row (0-15)
 */
void select_row(unsigned char row){
	if (row > 15) row = 15;
	gpio_set_level(A_ROW, (row >> 0) & 1);
	gpio_set_level(B_ROW, (row >> 1) & 1);
	gpio_set_level(C_ROW, (row >> 2) & 1);
	gpio_set_level(D_ROW, (row >> 3) & 1);
}

/**
 * @brief Draw Byte from PGM
 * @param x X Coordinate
 * @param y Y Coordinate
 * @param byte Byte
 * @param buff Buffer
 */
void drawBytePGM(unsigned char x, unsigned char y, const char *byte, char buff[][WIDTH/8]){
	unsigned char i;
	char data;
	data = *byte;
	
	for(i=0;i<8;i++) {
		if (data&(1<<(i))) setPixel(x + i, y, buff);
	}
}



/**
 * @brief Monitor Animation Timer Callback
 * @param arg Argument
 */
void monitor_animation_timer(void *arg)
{
	if(mon_an_P++ > 4){
		mon_an_P=0; 
		mon_anime_en=1;
	}
}

/**
 * @brief Fill Buffer
 * @param buff Buffer (WIDTH/8 - because each row is 8 bytes)
 * @param val Value
 */
void fillBuff(char buff[][WIDTH/8], unsigned char val){
	memset(buff, val, HEIGHT*WIDTH/8);
}

/**
 * @brief Clear Zone (Red)
 * @param xCord_B X Coordinate Bottom
 * @param xCord_O X Coordinate Top
 * @param yCord_B Y Coordinate Bottom
 * @param yCord_O Y Coordinate Top
 */
void HUB75_ZONE_CLEAR_R(unsigned char xCord_B,unsigned char xCord_O, unsigned char yCord_B, unsigned char yCord_O){
	unsigned char XCord_R;
	unsigned char YCord_R;
	
	for (XCord_R = xCord_B; XCord_R < xCord_O; XCord_R++) {
		for (YCord_R = yCord_B; YCord_R < yCord_O; YCord_R++) {
			ClrPixel(XCord_R, YCord_R, rBuff);
		}
	}
}

/**
 * @brief Clear Zone (Green)
 * @param xCord_B X Coordinate Bottom
 * @param xCord_O X Coordinate Top
 * @param yCord_B Y Coordinate Bottom
 * @param yCord_O Y Coordinate Top
 */
void HUB75_ZONE_CLEAR_G(unsigned char xCord_B,unsigned char xCord_O, unsigned char yCord_B, unsigned char yCord_O){
	unsigned char XCord_R;
	unsigned char YCord_R;
	
	for (XCord_R = xCord_B; XCord_R < xCord_O; XCord_R++) {
		for (YCord_R = yCord_B; YCord_R < yCord_O; YCord_R++) {
			ClrPixel(XCord_R, YCord_R, gBuff);
		}
	}
}

/**
 * @brief Clear Zone (Blue)
 * @param xCord_B X Coordinate Bottom
 * @param xCord_O X Coordinate Top
 * @param yCord_B Y Coordinate Bottom
 * @param yCord_O Y Coordinate Top
 */
void HUB75_ZONE_CLEAR_B(unsigned char xCord_B,unsigned char xCord_O, unsigned char yCord_B, unsigned char yCord_O){
	unsigned char XCord_R;
	unsigned char YCord_R;
	
	for (XCord_R = xCord_B; XCord_R < xCord_O; XCord_R++) {
		for (YCord_R = yCord_B; YCord_R < yCord_O; YCord_R++) {
			ClrPixel(XCord_R, YCord_R, bBuff);
		}
	}
}

/**
 * @brief Clear All Buffers
 */
void HUB75_CLEAR(void){
	fillBuff(rBuff, 0x00);
	fillBuff(gBuff, 0x00);
	fillBuff(bBuff, 0x00);
}

/**
 * @brief Clear Special Buffers (Used for Monitor Animation)
 */
void HUB75_SPECIAL_CLEAR(void){  
	if((strstr(hub75_buf_1,OLD_hub75_buf_1)) || (strstr(hub75_buf_2,OLD_hub75_buf_2)))
	{
		strcpy(OLD_hub75_buf_1,hub75_buf_1);
		strcpy(OLD_hub75_buf_2,hub75_buf_2);
		HUB75_CLEAR();
	}
}

/**
 * @brief Print Character (6x5)
 * @param ch Character
 * @param xCord X Coordinate
 * @param yCord Y Coordinate
 * @param red Red
 * @param green Green
 * @param blue Blue
 */
void printChar6(char ch, unsigned char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue){
	char chBuff[6] = {0};
	unsigned char i, n;
	
	// Copy char to temp buffer
	for (i = 0; i < 6; i++) chBuff[i] = font6[ch-32][i];
	
	// Copy char to display Blue buffer
	if (red) {
		for (i = 0; i < 6; i++) {
			for (n = 0; n < 5; n++){
				if (chBuff[i] & (1<<(n)) ) {
					setPixel(n + xCord, i + yCord, rBuff);
				}
			}
		}
	}
	
	if (green) {
		for (i = 0; i < 6; i++) {
			for (n = 0; n < 5; n++){
				if (chBuff[i] & (1<<(n)) ) {
					setPixel(n + xCord, i + yCord, gBuff);
				}
			}
		}
	}
	
	if (blue) {
		for (i = 0; i < 6; i++) {
			for (n = 0; n < 5; n++){
				if (chBuff[i] & (1<<(n)) ) {
					setPixel(n + xCord, i + yCord, bBuff);
				}
			}
		}
	}
}

/**
 * @brief Print String (6x5)
 * @param str String
 * @param str_len String Length
 * @param xCord X Coordinate
 * @param yCord Y Coordinate
 * @param red Red
 * @param green Green
 * @param blue Blue
 */
void Puts_STR_6 (char * str, unsigned char str_len, char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue){
	
	for(unsigned char strPos = 0; strPos < str_len; strPos++)
	{
		printChar6(str[strPos],xCord,yCord,red,green,blue);
		xCord = xCord+6;
	}
}

/** 
 * @brief Print Character (8x8)
 * @param ch Character
 * @param xCord X Coordinate
 * @param yCord Y Coordinate
 * @param red Red
 * @param green Green
 * @param blue Blue
 */
void printChar8(char ch, unsigned char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue){
	char chBuff[8] = {0};
	unsigned char i, n;
	
	// Copy char to temp buffer
	for (i = 0; i < 8; i++) chBuff[i] = ASCII[(unsigned char)ch][i];
	
	// Copy char to display Blue buffer
	if (red) {
		for (i = 0; i < 8; i++) {
			for (n = 0; n < 8; n++){
				if (chBuff[i] & (1<<(7-n)) ) {
					setPixel(n + xCord, i + yCord, rBuff);
				}
			}
		}
	}
	
	if (green) {
		for (i = 0; i < 8; i++) {
			for (n = 0; n < 8; n++){
				if (chBuff[i] & (1<<(7-n)) ) {
					setPixel(n + xCord, i + yCord, gBuff);
				}
			}
		}
	}
	
	if (blue) {
		for (i = 0; i < 8; i++) {
			for (n = 0; n < 8; n++){
				if (chBuff[i] & (1<<(7-n)) ) {
					setPixel(n + xCord, i + yCord, bBuff);
				}
			}
		}
	}
}

/**
 * @brief Print String (8x8)
 * @param str String
 * @param str_len String Length
 * @param xCord X Coordinate
 * @param yCord Y Coordinate
 * @param red Red
 * @param green Green
 * @param blue Blue
 */
void Puts_STR_8 (char * str, unsigned char str_len, char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue){
		
	for(unsigned char strPos = 0; strPos < str_len; strPos++)
	{
		printChar8(str[strPos],xCord,yCord,red,green,blue);
		xCord = xCord+8;
	}
}

/**
 * @brief Print Character (11x11)
 * @param ch Character
 * @param xCord X Coordinate
 * @param yCord Y Coordinate
 * @param red Red
 * @param green Green
 * @param blue Blue
 */
void printChar11(char ch, unsigned char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue){
	unsigned char i, yStep = yCord, CHA;
	
	CHA = ch - 32;
	
	if (red){
		for (i = 0; i < 11; i++){
			if (i < 10)drawBytePGM(xCord + 8, yStep, &font11[CHA][i * 2 + 1], rBuff);
			drawBytePGM(xCord, yStep, &font11[CHA][i * 2], rBuff);
			yStep++;
		}
	}
	yStep = yCord;
	
	if (green){
		for (i = 0; i < 11; i++){
			if (i < 10)drawBytePGM(xCord + 8, yStep, &font11[CHA][i * 2 + 1], gBuff);
			drawBytePGM(xCord, yStep, &font11[CHA][i * 2], gBuff);
			yStep++;
		}
	}
	yStep = yCord;
	
	if (blue){
		for (i = 0; i < 11; i++){
			if (i < 10)drawBytePGM(xCord + 8, yStep, &font11[CHA][i * 2 + 1], bBuff);
			drawBytePGM(xCord, yStep, &font11[CHA][i * 2], bBuff);
			yStep++;
		}
	}
}

/**
 * @brief Print String (11x11)
 * @param str String
 * @param str_len String Length
 * @param xCord X Coordinate
 * @param yCord Y Coordinate
 * @param red Red
 * @param green Green
 * @param blue Blue
 */
void Puts_STR_11 (char * str, unsigned char str_len, char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue){
	
	for(unsigned char strPos = 0; strPos < str_len; strPos++)
	{
		printChar11(str[strPos],xCord,yCord,red,green,blue);
		xCord = xCord+9;
	}
}

/**
 * @brief Print Character (16x15)
 * @param ch Character
 * @param xCord X Coordinate
 * @param yCord Y Coordinate
 * @param red Red
 * @param green Green
 * @param blue Blue
 */
void printChar16(char ch, unsigned char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue){
	unsigned char i, yStep = yCord, CHA; 
	
	CHA = ch - 32;
	
	if (red){
		for (i = 0; i < 15; i++){
			drawBytePGM(xCord + 8, yStep, &font16[CHA][i * 2 + 1], rBuff);
			drawBytePGM(xCord, yStep, &font16[CHA][i * 2], rBuff);
			yStep++;
		}
	}
	yStep = yCord;
	
	if (green){
		for (i = 0; i < 15; i++){
			drawBytePGM(xCord + 8, yStep, &font16[CHA][i * 2 + 1], gBuff);
			drawBytePGM(xCord, yStep, &font16[CHA][i * 2], gBuff);
			yStep++;
		}
	}
	yStep = yCord;
	
	if (blue){
		for (i = 0; i < 15; i++){
			drawBytePGM(xCord + 8, yStep, &font16[CHA][i * 2 + 1], bBuff);
			drawBytePGM(xCord, yStep, &font16[CHA][i * 2], bBuff);
			yStep++;
		}
	}
}

/**
 * @brief Print String (16x15)
 * @param str String
 * @param str_len String Length
 * @param xCord X Coordinate
 * @param yCord Y Coordinate
 * @param red Red
 * @param green Green
 * @param blue Blue
 */
void Puts_STR_16 (char * str, unsigned char str_len, char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue){
	
	for(unsigned char strPos = 0; strPos < str_len; strPos++)
	{
		printChar16(str[strPos],xCord,yCord,red,green,blue);
	
		xCord = xCord+12;
	}
}

/**
 * @brief Print String (16x15)
 * @param str String
 * @param str_len String Length
 * @param xCord X Coordinate
 * @param yCord Y Coordinate
 * @param red Red
 * @param green Green
 * @param blue Blue
 */
void Putsf_STR_16 (const char * str,uint8_t str_len,uint8_t xCord,uint8_t yCord,uint8_t red,uint8_t green,uint8_t blue){
	while((str)!=0){
	  printChar16(*(str++),xCord,yCord,red,green,blue);
	  xCord = xCord+12;
	};
  }


/**
 * @brief Print Character (21x21)
 * @param ch Character
 * @param xCord X Coordinate
 * @param yCord Y Coordinate
 * @param red Red
 * @param green Green
 * @param blue Blue
 */
void printChar21(char ch, unsigned char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue){
	unsigned char i, yStep = yCord, CHA;
	
	CHA = ch - 32;
	
	if (red){
		for (i = 0; i < 21; i++){
			drawBytePGM(xCord, yStep, &numbers21[CHA][i * 2], rBuff);
			drawBytePGM(xCord + 8, yStep, &numbers21[CHA][i * 2 + 1], rBuff);
			yStep++;
		}
	}
	yStep = yCord;
	
	if (green){
		for (i = 0; i < 21; i++){
			drawBytePGM(xCord, yStep, &numbers21[CHA][i * 2], gBuff);
			drawBytePGM(xCord + 8, yStep, &numbers21[CHA][i * 2 + 1], gBuff);
			yStep++;
		}
	}
	yStep = yCord;
	
	if (blue){
		for (i = 0; i < 21; i++){
			drawBytePGM(xCord, yStep, &numbers21[CHA][i * 2], bBuff);
			drawBytePGM(xCord + 8, yStep, &numbers21[CHA][i * 2 + 1], bBuff);
			yStep++;
		}
	}
}

/**
 * @brief Print String (21x21)
 * @param str String
 * @param str_len String Length
 * @param xCord X Coordinate
 * @param yCord Y Coordinate
 * @param red Red
 * @param green Green
 * @param blue Blue
 */
void Puts_STR_21 (char * str, unsigned char str_len, char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue){
	
	for(unsigned char strPos = 0; strPos < str_len; strPos++)
	{
		printChar21(str[strPos],xCord,yCord,red,green,blue);
		xCord = xCord+16;
	}
}

/**
 * @brief Calculate X Coordinate for Centering
 * @param Xstr_len String Length
 * @param harf_X_bit Half X Bit
 * @return X Coordinate
 */
unsigned char hub75_X_C (char Xstr_len, unsigned char harf_X_bit)
{
	if (harf_X_bit*Xstr_len < WIDTH)
	{
		return (WIDTH-(harf_X_bit*Xstr_len))/2;
	}
	else
	{
		return 0;
	}
}

/**
 * @brief Copy Buffer to Row Data
 */
void HUB75_PAINT_STR_CPY (void)
{
	for (int i = 0; i < HEIGHT/2; i++)
	{
		for (int j = 0; j < (WIDTH/8); j++)
		{
			row_data[i][j] = bBuff[16+i][j];
			row_data[i][8+j] = gBuff[16+i][j];
			row_data[i][16+j] = rBuff[16+i][j];

			row_data[i][24+j] = bBuff[i][j];
			row_data[i][32+j] = gBuff[i][j];
			row_data[i][40+j] = rBuff[i][j];
		}
	}

}

/**
 * @brief Timer Callback (Send Data to Display)
 * @param arg Argument
 */
void IRAM_ATTR timer_callback(void *arg)
{
    static spi_transaction_t trans;

    trans.length = 48 * 8;  // 48 bytes = 384 bits
    trans.tx_buffer = row_data[row];

    out_off();
    select_row(row);

    esp_err_t ret = spi_device_transmit(spi, &trans);
    if (ret != ESP_OK) {
        // Логировать внутри IRAM_ATTR нельзя стандартно, только если UART в IRAM или через специальный буфер
        // Поэтому можно поставить флаг или использовать минимальный способ отладки
        // ESP_EARLY_LOGE — безопасный вариант:
        ESP_EARLY_LOGE(TAG, "SPI transmit error: %s", esp_err_to_name(ret));
    }

    latch_data();
    out_on();

    row++;
    if (row >= (HEIGHT / 2)) {
        row = 0;
    }
}

/**
 * @brief Initialize Timer, SPI, Timer callback (Send Data to Display), Monitor Animation Timer (Used for Monitor Animation)
 */
esp_err_t hub75_timer_init (void)
{
    esp_err_t ret;

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4096,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 20 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 1,
        .flags = SPI_DEVICE_HALFDUPLEX
    };

    ret = spi_bus_initialize(SPI_HOST, &buscfg, DMA_CHAN);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ret = spi_bus_add_device(SPI_HOST, &devcfg, &spi);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    esp_timer_create_args_t timer_args = {
        .callback = timer_callback,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "display_timer"
    };

    esp_timer_create_args_t monitor_anime_timer_args = {
        .callback = monitor_animation_timer,
        .arg = NULL,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "display_animation_timer"
    };

    esp_timer_handle_t timer_handle;
    ret = esp_timer_create(&timer_args, &timer_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create display timer: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ret = esp_timer_start_periodic(timer_handle, 500);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start display timer: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    esp_timer_handle_t monitor_anime_timer_handle;
    ret = esp_timer_create(&monitor_anime_timer_args, &monitor_anime_timer_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create animation timer: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ret = esp_timer_start_periodic(monitor_anime_timer_handle, 16000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start animation timer: %s", esp_err_to_name(ret));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "hub75 timers and SPI initialized successfully");
	return ESP_OK;
}


