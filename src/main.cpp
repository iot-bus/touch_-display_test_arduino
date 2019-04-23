#include <Arduino.h>
#include <SPI.h>

#define SPI_SHIFT_DATA_8(data, len) (data<<(8-len))
#define SPI_SHIFT_DATA_16(data, len) __builtin_bswap16(data<<(16-len))
#define SPI_SHIFT_DATA_32(data, len) __builtin_bswap32(data<<(32-len))

#define SPI_GET_DATA_8(data, len) (data>>(8-len))
#define SPI_GET_DATA_16(data, len) (__builtin_bswap16(data)>>(16-len))
#define SPI_GET_DATA_32(data, len) (__builtin_bswap32(data)>>(32-len))


#define csPin 16
#define SPI_SETTING     SPISettings(2000000, MSBFIRST, SPI_MODE0)

// bit 7 - start bit
// bit 6-4 A2-A0 - Channel select bits
// bit 3 - mode (low for 12bit, high for 8bit)
// bit 2 - single ended / differential reference
// bit 1-0 - power down mode (00 - enabled, 01 ref off, adc on, 10 ref on, adc off, 11-always on)
#define CTRLZ1 0b10110011 		// 0xB3 = 179				// 10110011
#define CTRLZ2 0b11000011 		// 0xC3 = 195				// 11000011
#define CTRLY  0b10010011 		// 0x93 = 147				// 10010011
#define CTRLX  0b11010011 		// 0xD3 = 211				// 11010011
#define CTRL_RESET 0b11010100	// 0xD4 = 212		        // 11010100

uint16_t readValue1();
uint16_t readValue2();
uint16_t readValue3();

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(csPin, OUTPUT);
  digitalWrite(csPin, HIGH);
  SPI.begin(18, 19, 23, 16);
}

void loop() {
  // put your main code here, to run repeatedly:

  uint16_t x;
  
  // All three functions return a value between xmin ~ 300 to xmax ~3500 for x

  x = readValue1();
  Serial.print("Read 1, x: ");Serial.print(x);

  x = readValue2();
  Serial.print(", Read 2, x: ");Serial.print(x);

  x = readValue3();
  Serial.print(", Read 3, x: ");Serial.println(x);
  
  delay(500);
}

uint16_t readValue1(){

  uint8_t data[3];
  data[0] = CTRLX;
  data[1] = data[2] = 0;

  SPI.beginTransaction(SPI_SETTING);
	digitalWrite(csPin, LOW);

  // this works - single 24 bit transfer
  SPI.transfer((uint8_t*)&data, sizeof(data));

  digitalWrite(csPin, HIGH);
  SPI.endTransaction();

  uint16_t x = (data[1] << 8 | data[2]) >> 3;
  //printf("data[1]: %d, data[2]: %d\n", data[1], data[2]);
  return x;
}

uint16_t readValue2(){

    union u {
     uint8_t x1;
     uint8_t x2;
     uint16_t x;
  } u;

  SPI.beginTransaction(SPI_SETTING);
	digitalWrite(csPin, LOW);

  //this works - send cmd followed by read
  uint8_t  cmd = SPI.transfer(CTRLX);
	u.x = SPI.transfer16(0x00 /* noop */) >> 3;

  uint16_t x = u.x;

  digitalWrite(csPin, HIGH);
  SPI.endTransaction();

  //printf("data[1]: %d, data[2]: %d\n", (uint8_t)u.x1, u.x2);
  return x;
}  

uint16_t readValue3(){
  
  SPI.beginTransaction(SPI_SETTING);
	digitalWrite(csPin, LOW);

  // this works - three separate transfers
  uint8_t cmd = SPI.transfer(CTRLX);
	uint8_t val1 = SPI.transfer(0x00 /* noop */);
  uint8_t val2 = SPI.transfer(0x00 /* noop */);
  uint16_t x = (val1 << 8 | val2) >> 3;

  digitalWrite(csPin, HIGH);
  SPI.endTransaction();

  //printf("val1: %d, val2: %d\n", val1, val2);
  return x;
}  


