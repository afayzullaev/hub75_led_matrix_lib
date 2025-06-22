#ifndef __HUB75_H__
#define __HUB75_H__

#include "esp_timer.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"

/*SPI*/
#define SPI_HOST                                SPI2_HOST        // SPI2
#define DMA_CHAN                                SPI_DMA_CH_AUTO  // DMA Channel Auto

/*GPIO*/
#define PIN_NUM_CLK                             12        // CLK
#define PIN_NUM_MOSI                            11       // MOSI
#define PIN_NUM_MISO                            (-1)     // MISO
#define LATCH                                   13       // LATCH
#define OE                                      14       // OE

/*Row*/
#define A_ROW                                   1        // A Row
#define B_ROW                                   2        // B Row
#define C_ROW                                   9        // C Row
#define D_ROW                                   10       // D Row

#define GPIO_OUTPUT_MASK                        ((1ULL<<A_ROW) | (1ULL<<B_ROW) | (1ULL<<C_ROW) | (1ULL<<D_ROW) | (1ULL<<LATCH) | (1ULL<<OE)) // GPIO Output Mask (A, B, C, D, LATCH, OE)
#define ROW_GPIO_OUTPUT_MASK                    ((1ULL<<A_ROW) | (1ULL<<B_ROW) | (1ULL<<C_ROW) | (1ULL<<D_ROW)) // Row GPIO Output Mask (A, B, C, D)

/*Width and Height*/
#define WIDTH                                   64       // Width
#define HEIGHT                                  32      // Height

/*Buffer*/
extern char rBuff[HEIGHT][WIDTH/8];// = {0}; // Red Buffer
extern char gBuff[HEIGHT][WIDTH/8];// = {0}; // Green Buffer
extern char bBuff[HEIGHT][WIDTH/8];// = {0}; // Blue Buffer

/*Buffer for Monitor Animation*/
extern char hub75_buf_1[50];
extern char hub75_buf_2[50];
extern char OLD_hub75_buf_1[50];
extern char OLD_hub75_buf_2[50];

/*Initialize*/
void hub75_init(void);
esp_err_t hub75_timer_init (void);
void latch_data(void);
void out_on(void);
void out_off(void);
void select_row(unsigned char row);

/*Set Pixel*/
void setPixel(unsigned char x, unsigned char y, char buff[][WIDTH/8]);
void ClrPixel(unsigned char x, unsigned char y, char buff[][WIDTH/8]);
void drawByte(unsigned char x, unsigned char y, char byte, char buff[][WIDTH/8]);
void drawBytePGM(unsigned char x, unsigned char y, const char *byte, char buff[][WIDTH/8]);

void fillBuff(char buff[][WIDTH/8], unsigned char val);

/*Clear Zone*/
void HUB75_ZONE_CLEAR_R(unsigned char xCord_B,unsigned char xCord_O, unsigned char yCord_B, unsigned char yCord_O);
void HUB75_ZONE_CLEAR_G(unsigned char xCord_B,unsigned char xCord_O, unsigned char yCord_B, unsigned char yCord_O);
void HUB75_ZONE_CLEAR_B(unsigned char xCord_B,unsigned char xCord_O, unsigned char yCord_B, unsigned char yCord_O);
void HUB75_CLEAR(void);
void HUB75_SPECIAL_CLEAR(void);

/*Paint String*/
void HUB75_PAINT_STR_CPY (void);

/*Print Character*/
void printChar6(char ch, unsigned char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue);
void printChar8(char ch, unsigned char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue);
void printChar11(char ch, unsigned char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue);
void printChar16(char ch, unsigned char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue);
void printChar21(char ch, unsigned char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue);
/*Put String*/
void Puts_STR_6 (char * str, unsigned char str_len, char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue);
void Puts_STR_8 (char * str, unsigned char str_len, char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue);
void Puts_STR_11 (char * str, unsigned char str_len, char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue);
void Puts_STR_16(char * str, unsigned char str_len, char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue);
void Puts_STR_21(char * str, unsigned char str_len, char xCord, unsigned char yCord, unsigned char red,  unsigned char green,  unsigned char blue);





#endif