/*=================================================================
 * E32-TTL-100----- Arduino UNO or esp8266
 * M0         ----- 3.3v (To config) GND (To send) 
 * M1         ----- 3.3v (To config) GND (To send)
 * TX         ----- RX PIN 2 
 * RX         ----- TX PIN 3 
 * AUX        ----- Not connected 
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 */
 // Khai báo thư viện để sử dụng.
#include "Arduino.h"
#include "LoRa_E32.h"
#include <MPU6050_tockn.h>
#include <SoftwareSerial.h> // Thư viện để tạo các cổng kết nối ảo của Arduino.
#include <Wire.h>
SoftwareSerial mySerial(2, 3); 
MPU6050 mpu6050(Wire);
#define BUZZER 10 // Khai báo chân của còi.
//Khai báo các biến đọc cảm biến và các biến so sánh.
int goc_X,goc_Y,goc_Y2,goc_Z;
String TrangThai;
int AnToan =60;
int NguyHiem=80;
int RatNguyHiem=90;
unsigned long thoigian; //Thời gian là kiểu số nguyên

LoRa_E32 e32ttl100(2, 3); // Khai báo chân cho Module E32 Lora  E32 TX - E32 RX
// 2 hàm in ra trạng thái cấu hình của Module Lora
void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);

void setup() {
	Serial.begin(9600);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  Wire.begin();
  mpu6050.begin();
  // Hàm mpu6050.calcGyroOffsets(true) này để set vị trí gốc cho module.
  mpu6050.calcGyroOffsets(true);
  Serial.println("Done!\n");
  Serial.println("=============================");
	// Hàm mở cổng kết nối cho module E32
	e32ttl100.begin();
  // After set configuration comment set M0 and M1 to low
  // and reboot if you directly set HIGH M0 and M1 to program
  // In ra các trạng thái cấu hình.
	ResponseStructContainer c;
	c = e32ttl100.getConfiguration();
	// It's important get configuration pointer before all other operation
	Configuration configuration = *(Configuration*) c.data;
	Serial.println(c.status.getResponseDescription());
	Serial.println(c.status.code);
	printParameters(configuration);
 // Cấu hình cho Module E32Lora.
 // Các địa chỉ để truyền dữ liệu.
	configuration.ADDL = 0x02;
  configuration.ADDH = 0x02;
  configuration.CHAN = 0x02; // Bắt buộc phải cùng kênh với module nhận.

	configuration.OPTION.fec = FEC_0_OFF;
	configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
	configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
	configuration.OPTION.transmissionPower = POWER_20;
	configuration.OPTION.wirelessWakeupTime = WAKE_UP_750;

	configuration.SPED.airDataRate = AIR_DATA_RATE_100_96;
	configuration.SPED.uartBaudRate = UART_BPS_9600;
	configuration.SPED.uartParity = MODE_00_8N1;

	// Set configuration changed and set to not hold the configuration
	ResponseStatus rs = e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
	Serial.println(rs.getResponseDescription());
	Serial.println(rs.code);
	printParameters(configuration);
	c.close();
}
void loop() {
mpu6050.update();
// Đọc tín hiệu từ MPU6050.
goc_Y2=mpu6050.getAngleY(); // Tính góc nghiêng.
  Serial.print("angleX : ");
  Serial.print(mpu6050.getAngleX());
  Serial.print("\tangleY : ");
  Serial.print(mpu6050.getAngleY());
  Serial.print("\tangleZ : ");
  Serial.println(mpu6050.getAngleZ());
  Serial.println("===========");
  Serial.println(goc_Y2);
// So sánh giá trị để hiện ra cảnh báo.
if (goc_Y2 < 60){
  TrangThai="AN TOAN";
  digitalWrite(BUZZER, LOW);
  }
if (goc_Y2 > 60 && goc_Y2 < 80){
  TrangThai="NGUY HIEM";
  digitalWrite(BUZZER, HIGH);
  delay(2000);
  }
if (goc_Y2 > 80){
  TrangThai="RAT NGUY HIEM";
  digitalWrite(BUZZER, HIGH);
  }
  //Hàm send Data là hàm gửi chuỗi tín hiệu xử lý từ Arduino qua ESP.
sendData(String (goc_Y2), TrangThai);
}
// Config LORA.
void printParameters(struct Configuration configuration) {
	Serial.println("----------------------------------------");

	Serial.print(F("HEAD : "));  Serial.print(configuration.HEAD, BIN);Serial.print(" ");Serial.print(configuration.HEAD, DEC);Serial.print(" ");Serial.println(configuration.HEAD, HEX);
	Serial.println(F(" "));
	Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, BIN);
	Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, BIN);
	Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
	Serial.println(F(" "));
	Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
	Serial.print(F("SpeedUARTDatte  : "));  Serial.print(configuration.SPED.uartBaudRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRate());
	Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRate());

	Serial.print(F("OptionTrans        : "));  Serial.print(configuration.OPTION.fixedTransmission, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFixedTransmissionDescription());
	Serial.print(F("OptionPullup       : "));  Serial.print(configuration.OPTION.ioDriveMode, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getIODroveModeDescription());
	Serial.print(F("OptionWakeup       : "));  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
	Serial.print(F("OptionFEC          : "));  Serial.print(configuration.OPTION.fec, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFECDescription());
	Serial.print(F("OptionPower        : "));  Serial.print(configuration.OPTION.transmissionPower, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());

	Serial.println("----------------------------------------");
}
void printModuleInformation(struct ModuleInformation moduleInformation) {
  Serial.println("----------------------------------------");
  Serial.print(F("HEAD BIN: "));  Serial.print(moduleInformation.HEAD, BIN);Serial.print(" ");Serial.print(moduleInformation.HEAD, DEC);Serial.print(" ");Serial.println(moduleInformation.HEAD, HEX);

  Serial.print(F("Freq.: "));  Serial.println(moduleInformation.frequency, HEX);
  Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
  Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
  Serial.println("----------------------------------------");

}
// Ham nhận.
String revData()
{
  String res = " ";
  // Hàm e32ttl100.available() sẽ kiểm tra xem có tín hiệu được gửi hay không.
  if(e32ttl100.available()>1)
{
  ResponseContainer rs = e32ttl100.receiveMessage();
  res=rs.data;
  Serial.println(rs.status.getResponseDescription());
  Serial.println(res);
}
return res;
}
// Ham gui.
void sendData(String message, String TrangThai)
{
  String s = "GOC = " + message + "°" + "\n" + TrangThai;
  ResponseStatus rs = e32ttl100.sendFixedMessage(0x01,0x01,0x02,s);
  //Hàm sendFixedMessage ( Dia chi gui ) là hàm gửi chuỗi "s" qua bên esp32. Các địa chỉ phải trùng với bên nhận.
  Serial.println(s);
  Serial.println(rs.getResponseDescription());
  delay(1000);
  }
