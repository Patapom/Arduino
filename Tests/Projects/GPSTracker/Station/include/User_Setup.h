#define ST7789_DRIVER
//#define ST7789_DRIVER_2

#define TFT_MOSI	23	// SDA Pin on ESP32
#define TFT_SCLK	18	// SCL Pin on ESP32
#define TFT_RST		33
#define TFT_DC		25	// Data Command control pin
#define TFT_CS		26	// Chip select control pin

#define SPI_FREQUENCY  40000000
//#define SPI_FREQUENCY	20000000

#define TFT_WIDTH 	240
#define TFT_HEIGHT	280

//#define CGRAM_OFFSET

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8

//#define USER_SETUP_LOADED
