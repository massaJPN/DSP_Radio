#include <M5Unified.h>
#include <Wire.h>

uint16_t data ;

unsigned char num_byte = 2;
unsigned char DEVICE_ADDRESS = 0x35;
uint8_t REGISTER_ADDRESS;
String FMMODE;
String AM_FM;

//レジスタアドレス
unsigned char CHIP_ID = 0x01;
unsigned char SEEK = 0x02;
unsigned char TUNE = 0x03; //FM Channel
unsigned char VOLUME = 0x04; //Softmute / Mute / Bass Boost Efect / Audio DAC Anti-pop
unsigned char DSPCFGA = 0x05; //Mono
unsigned char LOCFGA = 0x0A;
unsigned char LOCFGC = 0x0C;
unsigned char RXCFG= 0x0F; // Standby mode / Volume Control
unsigned char STATUSA = 0x12; //RSSI
unsigned char STATUSB = 0x13;
unsigned char STATUSC = 0x14; //SNR
unsigned char AMSYSCFG = 0x16; // AM/FM mode Control / Audio Gain Selection
unsigned char AMCHAN = 0x17; // AM Channel Setting
unsigned char AMCALI = 0x18; //On Chip Capacitor for AM
unsigned char GPIOCFG = 0x1D; // Vol Pin Mode Selection, CH Pin Mode Selection
unsigned char AMDSP = 0x22; // AM Channel Bandwidth Selection
unsigned char AMSTATUSA = 0x24; // AM Channel RSSI
unsigned char AMSTATUSB = 0x25;
unsigned char SOFTMUTE = 0x2E; // Softmute Attenuation
unsigned char USERSTARTCH = 0x2F;
unsigned char USERGUARD = 0x30;
unsigned char USERCHANNUM = 0x31;
unsigned char AMCFG = 0x33; //AM Channel Space Selection , Working mode selections when key mode is selected.
unsigned char AMCFG2 = 0x34;
unsigned char VOLGUARD = 0x3A;
unsigned char AFC = 0x3C;
//

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setCursor(80, 0); 
  M5.Display.setFont(&fonts::Font0);
  M5.Display.setTextSize(3);
  M5.Display.print("DSP RADIO");
  Serial.println("KT0913 RADIO");

  Wire.begin();

  fm_mode();
  
  REGISTER_ADDRESS = GPIOCFG; //Dial mode VOL amd CH
  data = 0B0000000000001010; // VOL = 10 / CH = 10
  I2CsendMULTIbyte();
 
}

void I2CsendMULTIbyte(){
  Wire.beginTransmission(DEVICE_ADDRESS);
  Wire.write(REGISTER_ADDRESS);
  Wire.write(highByte(data));
  Wire.write(lowByte(data));
  Wire.endTransmission();
}

void I2CreadMULTIbyte(){
  Wire.beginTransmission(DEVICE_ADDRESS);
  Wire.write(REGISTER_ADDRESS);
  Wire.endTransmission(false);
  Wire.requestFrom(DEVICE_ADDRESS,num_byte);
  while (Wire.available() < num_byte);
  uint8_t Hb = Wire.read();
  uint8_t Lb = Wire.read();
  data = (Hb << 8) | Lb;
}

void fm_mode(){
  REGISTER_ADDRESS = AMSYSCFG;
  I2CreadMULTIbyte();
  uint16_t amsyscfg_mode = data;
  data = 0B0100000000000010;
  I2CsendMULTIbyte();
  uint16_t am_fm = (data & 0B1000000000000000) >> 15;
  if (am_fm == 1) AM_FM ="AM";
  else AM_FM ="FM"; 
  
  M5.Display.setCursor(145, 210);
  M5.Display.setFont(&fonts::Font0);
  M5.Display.setTextSize(3);
  M5.Display.print("AM");
  Serial.printf("AM/FM: %S\n", AM_FM); 

  REGISTER_ADDRESS = STATUSA;
  I2CreadMULTIbyte();
  uint16_t statusa = data;
  uint16_t fmmode = (statusa & 0B000001100000000) >> 8;

  if (fmmode == 3) FMMODE ="STEREO";
  else FMMODE ="MONO"; 

  Serial.printf("FMMODE: %S\n", FMMODE); 

  REGISTER_ADDRESS = AMSYSCFG; 
  I2CreadMULTIbyte();
  uint16_t amsyscfg_band = data;
  data = amsyscfg_band | 0B0100000000000000;
  I2CsendMULTIbyte();
  
  REGISTER_ADDRESS = USERSTARTCH; 
  data = 0B0000010111110000; // 76MHz = 1520 * 50kHZ (1520= 5F0h)
  I2CsendMULTIbyte();

  REGISTER_ADDRESS = USERGUARD; 
  data = 0B0000000000010111; //23ch  2kohm
  I2CsendMULTIbyte();

  REGISTER_ADDRESS = USERCHANNUM; 
  data = 0B0000000010110101; //181ch  76MHz - 94MHz  100KHz/step  8kohm
  I2CsendMULTIbyte();

}

void am_mode(){
  REGISTER_ADDRESS = AMSYSCFG;
  I2CreadMULTIbyte();
  uint16_t amsyscfg_mode = data;
  data = 0B1000000000000010;
  I2CsendMULTIbyte();
  
  uint16_t am_fm = (data & 0B1000000000000000) >> 15;
  if (am_fm == 1) AM_FM ="AM";
  else AM_FM ="FM";

  M5.Display.setCursor(145, 210);
  M5.Display.setFont(&fonts::Font0);
  M5.Display.setTextSize(3);
  M5.Display.print("FM");  // 1NO TOKI AM
  Serial.printf("AM/FM: %S\n", AM_FM); 

  REGISTER_ADDRESS = AMSYSCFG; 
  I2CreadMULTIbyte();
  uint16_t amsyscfg_band = data;
  data = amsyscfg_band | 0B0100000000000000;
  I2CsendMULTIbyte();
  
  REGISTER_ADDRESS = USERSTARTCH; 
  data = 0B0000001000001110; // 526kHz = 526 * 1kHZ (526= 20Eh) 
  I2CsendMULTIbyte();

  REGISTER_ADDRESS = USERGUARD; 
  data = 0B0000000100010010; //274ch  2kohm
  I2CsendMULTIbyte();

  REGISTER_ADDRESS = USERCHANNUM; 
  data = 0B0000010001000111; //1095ch 526kHz ～ 1620kHz    ch_space = 1KHz/step  8kohm
  I2CsendMULTIbyte();  
}

void loop() {
  delay(1);
  M5.update();
  
  REGISTER_ADDRESS = CHIP_ID;
  I2CreadMULTIbyte();
  uint16_t chipID = data;
  Serial.printf("chipID: %x\n", chipID); 

  REGISTER_ADDRESS = STATUSA;
  I2CreadMULTIbyte();
  uint16_t statusa = data;
  uint16_t fmrssi = (statusa & 0B000000011111000) >> 3;
  int16_t FMRSSI = -100+fmrssi*3;
  uint16_t fmmode = (statusa & 0B000001100000000) >> 8;

if (fmmode == 3) FMMODE ="STEREO";
  else FMMODE ="MONO"; 

Serial.printf("RSSI: %d\n", FMRSSI); 
Serial.printf("FMMODE: %S\n", FMMODE); 

REGISTER_ADDRESS = STATUSC;
I2CreadMULTIbyte();
uint16_t statusc = data;
uint16_t snr = (statusc & 0B0001111111000000) >> 6;
Serial.printf("SNR: %d\n", snr); 

REGISTER_ADDRESS = RXCFG;
I2CreadMULTIbyte();
uint16_t rxcfg = data;
uint16_t volume = rxcfg & 0B0000000000011111;

M5.Display.setCursor(205, 50);
M5.Display.setFont(&fonts::Font0);
M5.Display.setTextSize(3);
M5.Display.printf("VOLUME");
M5.Display.setCursor(215, 85);
M5.Display.setFont(&fonts::Font7);
M5.Display.setTextSize(1);
M5.Display.printf("%03d", volume*100 / 31);
M5.Display.setCursor(295, 150);
M5.Display.setTextSize(3);
M5.Display.setFont(&fonts::Font0);
M5.Display.print("%");
M5.Display.setFont(&fonts::Font0);

if (AM_FM == "FM"){
  REGISTER_ADDRESS = TUNE;
  I2CreadMULTIbyte();
  uint16_t tune = data;
  uint16_t fm_freq = tune & 0B0000111111111111;

  M5.Display.setCursor(10, 50);
  M5.Display.setFont(&fonts::Font0);
  M5.Display.setTextSize(3); 
  M5.Display.printf("%S FREQ", AM_FM);
  M5.Display.setCursor(25, 85);
  M5.Display.setTextSize(1);
  M5.Display.setFont(&fonts::Font7);
  M5.Display.printf("%03.1f", (float)fm_freq/1000*50);
  M5.Display.setCursor(82, 150);
  M5.Display.setTextSize(3);
  M5.Display.setFont(&fonts::Font0);
  M5.Display.print("MHz");

  Serial.printf("FM_FREQ: %.1f\n   MHz", (float)fm_freq/1000*50); 
  M5.Display.setFont(&fonts::Font0);
}
else if (AM_FM == "AM"){
  REGISTER_ADDRESS = AMCHAN;
  I2CreadMULTIbyte();
  uint16_t amchan = data;
  uint16_t am_freq = amchan & 0B0000011111111111;
  M5.Display.setCursor(10, 50);
  M5.Display.setFont(&fonts::Font0);
  M5.Display.setTextSize(3);
  M5.Display.printf("%S FREQ", AM_FM);
  M5.Display.setCursor(5, 85);
  M5.Display.setTextSize(1);
  M5.Display.setFont(&fonts::Font7);
  M5.Display.printf("%04d", am_freq);
  M5.Display.setCursor(82, 150);
  M5.Display.setTextSize(3);
  M5.Display.setFont(&fonts::Font0);
  M5.Display.print("KHz");

  Serial.printf("AM_FREQ: %d\n", am_freq); 
}

if (M5.BtnB.wasPressed() && (AM_FM == "FM") ){
  am_mode();
  M5.Display.setCursor(0, 85);
  M5.Display.setTextSize(1);
  M5.Display.setFont(&fonts::Font7);
  M5.Display.print("    ");
}
  else if (M5.BtnB.wasPressed() && (AM_FM == "AM") ){
    fm_mode();
    M5.Display.setCursor(0, 85);
    M5.Display.setTextSize(1);
    M5.Display.setFont(&fonts::Font7);
    M5.Display.print("    ");
  }
}
