/*=================================================================
 * E32-TTL-100----- Arduino UNO or esp8266
 * M0         ----- 3.3v (To config) GND (To send)
 * M1         ----- 3.3v (To config) GND (To send) 
 * TX         ----- RX PIN 2 (PullUP)
 * RX         ----- TX PIN 3 (PullUP & Voltage divider)
 * AUX        ----- Not connected (5 if you connect)
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 */
 // Khai báo các thư viện Blynk để cấu hình,
#define BLYNK_PRINT Serial

#define BLYNK_TEMPLATE_ID "TMPL67tkda9lP"
#define BLYNK_TEMPLATE_NAME "TrangThai"
#define BLYNK_AUTH_TOKEN "blE892MTvhAmYkhxpCtZXPFcUcppFOuM"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "Arduino.h"
#include "LoRa_E32.h"
#include <Wire.h>
// Hàm cho màn hình LCD1602
#include <LiquidCrystal_I2C.h>
// Hàm này để cấu hình màn hình LCD
LiquidCrystal_I2C lcd(0x27,16,2); 
// Hàm để tạo cổng Serial ảo.
#include <SoftwareSerial.h>
// Hàm khai báo chân sử dụng Lora E32.
LoRa_E32 e32ttl100(&Serial2, 18, 19, 5); //  RX AUX M0 M1. Có nghĩa là sử dụng 2 chân UART2.

void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);
// Khai báo các biến.
String GOC_NGHIENG;
String CANH_BAO;

//Khai báo tài khoản và mật khẩu Wifi để kết nối. Wifi này phải là 2.4GHz.
char ssid[] = "Xom tro 81"; // Tài Khoản WIFI.
char pass[] = "66668888"; // Mật Khẩu WIFI.
char auth[] = BLYNK_AUTH_TOKEN;

void setup() {
Serial.begin(9600);
// Kết nối Blynk theo TK, MK đã khai báo ở trên.
Blynk.begin(auth, ssid, pass);
delay(500);
// Hàm bắt đầu cho LCD.
  lcd.begin();
  lcd.backlight();
	// Hàm mở kết nối UART qua LORA E32
  e32ttl100.begin();
  // After set configuration comment set M0 and M1 to low
  // and reboot if you directly set HIGH M0 and M1 to program
  // Hàm in ra cấu hình và trạng thái cấu hình.
	ResponseStructContainer c;
	c = e32ttl100.getConfiguration();
	// It's important get configuration pointer before all other operation
	Configuration configuration = *(Configuration*) c.data;
	Serial.println(c.status.getResponseDescription());
	Serial.println(c.status.code);

	printParameters(configuration);
 
  // Các địa chỉ để nhận dữ liệu.
	configuration.ADDL = 0x01;
  configuration.ADDH = 0x01;
  configuration.CHAN = 0x02;// Bắt buộc phải cùng kênh với module truyền.

	configuration.OPTION.fec = FEC_0_OFF;
	configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION;
	configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
	configuration.OPTION.transmissionPower = POWER_20;
	configuration.OPTION.wirelessWakeupTime = WAKE_UP_750;

	configuration.SPED.airDataRate = AIR_DATA_RATE_100_96;
	configuration.SPED.uartBaudRate = UART_BPS_9600;
	configuration.SPED.uartParity = MODE_00_8N1;

	// Set configuration changed and set to not hold the configuration
  // Hàm này sẽ cấu hình và lưu cấu hình ở trên xong rồi in ra trạng thái.
	ResponseStatus rs = e32ttl100.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
	Serial.println(rs.getResponseDescription());
	Serial.println(rs.code);
	printParameters(configuration);
	c.close();
  Serial.println("Start listening!");
}
void loop() {
// Mở cổng kết nối với Blynk.
Blynk.run();
//sendData("H E L L O");
String x = revData(); // biến x là biến nhận giá trị từ Arduino gửi qua LORA sang.
String blynk = x;   
// Bởi vì cái giá trị x = GÓC = 9 * \n Trang Thai;
// Cắt chuỗi để hiển thị.
char targetChar = '\n'; // Khai báo kí tự tìm kiếm là "\n" là ký tự xuống dòng.
int targetIndex = -1; // Vị trí của kí tự cần tìm.
// Tìm kiếm giá trị
for (int i = 0; i < blynk.length(); i++) {
    char currentChar = blynk[i];
     if (currentChar == targetChar) {
      targetIndex = i;
      break;
      }
}
//Hàm substring là hàm cắt chuỗi để phân ra 1 nửa là Góc và 1 nửa là trạng thái.
String GOC_NGHIENG = blynk.substring(0, targetIndex);
String CANH_BAO = blynk.substring(targetIndex +1);
//Hiển thị ra LCD.
lcd.setCursor(0,0);
lcd.print(GOC_NGHIENG);
lcd.setCursor(0,1);
lcd.print(CANH_BAO);
//Hàm này là hàm gửi giá trị lên Blynk.
Blynk.virtualWrite(V3,GOC_NGHIENG); // Gửi dữ liệu GOC_NGHIENG đến pin V3 của Blynk (thay đổi nếu cần).
Blynk.virtualWrite(V2,CANH_BAO); // Gửi dữ liệu CANH_BAO đến pin V2 của Blynk (thay đổi nếu cần).
}
// CONFIG LORA.
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
// Ham nhan.
String revData()
{
  String res = " ";
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
void sendData(String message)
{
  String s = message +":" ;
  
  ResponseStatus rs = e32ttl100.sendFixedMessage(0x01,0x01,0x02,s);
  Serial.println(s);
  Serial.println(rs.getResponseDescription());
  delay(1000);
 }
