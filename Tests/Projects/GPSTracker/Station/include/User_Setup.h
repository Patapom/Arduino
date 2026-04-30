#define ST7789_DRIVER
//#define ST7789_DRIVER_2

#define TFT_MOSI	23
#define TFT_SCLK	18
//#define TFT_RST		14	// Conflict SPI
#define TFT_RST		-1		// No hardware reset
#define TFT_DC		25
#define TFT_CS		26

//#define SPI_FREQUENCY  40000000
#define SPI_FREQUENCY	20000000

#define TFT_WIDTH 	240
#define TFT_HEIGHT	280

#define CGRAM_OFFSET

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8

#define USER_SETUP_LOADED
