﻿




//D0 = GPIO16; 
//D1 = GPIO5; 
//D2 = GPIO4;	LED on esp8266
//D3 = GPIO0;can not download when connected to low
//D4 = GPIO2;	
//D5 = GPIO14;  
//D6 = GPIO12;
//D7 = GPIO13;
//D8 = GPIO15;  can not start when high input
//D9 = GPIO3; UART RX
//D10 = GPIO1; UART TX
//LED_BUILTIN = GPIO16 (auxiliary constant for the board LED, not a board pin);


//#define RELAY				D4



#include<ESP8266WiFi.h>
#include<WiFiUdp.h>
#include<ESP8266mDNS.h>
#include<ArduinoOTA.h>
#include<ESP8266WiFiMulti.h>
#include<time.h>
#define timezone 8

#define IIC_DAT				D9
#define IIC_CLK				D10




const char* ssid = "frye";  //Wifi名称
const char* password = "52150337";  //Wifi密码
WiFiUDP m_WiFiUDP;


char *time_str;   
char H1,H2,M1,M2,S1,S2;
char TimeString[9];


#include "Z:\bt\web\datastruct.h"
unsigned char DebugLogIndex = 20;
//tXXXXXXXXData XXXXXXXXData;
unsigned long LastServerUpdate;


#include <Wire.h>     //The DHT12 uses I2C comunication.
//#include "VL53L0X.h"
//VL53L0X DistanceSensor;
uint16_t Distance;
#define DISTANCE_NUMBER 30
uint16_t DistanceArray[DISTANCE_NUMBER];

#include <U8g2lib.h>
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, IIC_CLK, IIC_DAT); //配置构造函数




unsigned long TenthSecondsSinceStart = 0;
void TenthSecondsSinceStartTask();
void OnTenthSecond();
void OnSecond();



void MyPrintf(const char *fmt, ...);


void setup() 
{       


	delay(50);                      
	//Serial.begin(115200);

	//XXXXXXXXData.DataType = 0;


	WiFi.disconnect();
	WiFi.mode(WIFI_STA);//设置模式为STA
	byte mac[6];
	WiFi.softAPmacAddress(mac);
	//printf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	for (byte i=0;i<6;i++)
	{
		//XXXXXXXXData.Mac[i] = mac[i];
	}

	//XXXXXXXXData.Id = 0xFF;

	//for (unsigned char i = 0;i<ROOM_NUMBER;i++)
	//{
	//	if (memcmp(&RoomData.Mac[0],&RoomMacAddress[i][0],sizeof(unsigned long)*6) == 0)
	//	{
	//		XXXXXXXXData.Id = i;
	//		switch(i)
	//		{
	//		case ROOM_SOUTH:
	//			ssid = "frye_iot2";  //Wifi名称
	//			break;
	//		case ROOM_SMALL:
	//			ssid = "frye_iot2";  //Wifi名称
	//			break;
	//		case ROOM_CANTEEN:
	//			ssid = "frye_iot";  //Wifi名称
	//			break;
	//		case ROOM_BALCONY:
	//			ssid = "frye_iot2";  //Wifi名称
	//			break;
	//		}
	//		break;
	//	}
	//}


	//Serial.print("Is connection routing, please wait");  
	WiFi.begin(ssid, password); //Wifi接入到网络
	//Serial.println("\nConnecting to WiFi");
	//如果Wifi状态不是WL_CONNECTED，则表示连接失败
	unsigned char WiFiTimeOut = 0;
	while (WiFi.status() != WL_CONNECTED) {  
		//Serial.print("."); 
		delay(1000);    //延时等待接入网络
		WiFiTimeOut++;
		if (WiFiTimeOut>10)
		{
			break;
			//Serial.println("\nConnecting to WiFi Failed");
		}
	}



	//设置时间格式以及时间服务器的网址
	configTime(timezone * 3600, 0, "pool.ntp.org", "time.nist.gov");
	//Serial.println("\nWaiting for time");
	while (!time(nullptr)) {
		//Serial.print(".");
		delay(1000);    
	}
	//Serial.println("");


	m_WiFiUDP.begin(5050); 

	MyPrintf("macAddress 0x%02X:0x%02X:0x%02X:0x%02X:0x%02X:0x%02X AP:%s\r\n",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],ssid);

	ArduinoOTA.onStart([]() {
		String type;
		if (ArduinoOTA.getCommand() == U_FLASH) {
			type = "sketch";
		} else { // U_SPIFFS
			type = "filesystem";
		}

		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
		//Serial.println("Start updating " + type);
	});
	ArduinoOTA.onEnd([]() {
		//Serial.println("\nEnd");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
		//Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
	});
	ArduinoOTA.onError([](ota_error_t error) {
		//Serial.printf("Error[%u]: ", error);
		if (error == OTA_AUTH_ERROR) {
			//Serial.println("Auth Failed");
		} else if (error == OTA_BEGIN_ERROR) {
			//Serial.println("Begin Failed");
		} else if (error == OTA_CONNECT_ERROR) {
			//Serial.println("Connect Failed");
		} else if (error == OTA_RECEIVE_ERROR) {
			//Serial.println("Receive Failed");
		} else if (error == OTA_END_ERROR) {
			//Serial.println("End Failed");
		}
	});
	ArduinoOTA.begin();
	//Serial.println("Ready");
	//Serial.print("IP address: ");
	//Serial.println(WiFi.localIP());





	u8g2.begin(); //启动u8g2驱动程序 / 可按需更改u8x8驱动程序


	Wire.begin(IIC_DAT,IIC_CLK);
	//DistanceSensor.setTimeout(100);
	//if (!DistanceSensor.init())
	//{
	//	printf("Failed to detect and initialize sensor!\r\n");
	//}
	//DistanceSensor.startContinuous();
}

void loop() 
{
	ArduinoOTA.handle();

	TenthSecondsSinceStartTask();



	//m_WiFiUDP.parsePacket(); 
	//unsigned int UdpAvailable = m_WiFiUDP.available();
	//if (UdpAvailable == sizeof(tXXXXXXXXCommand))
	//{
	//	//MyPrintf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
	//	//printf("m_WiFiUDP.available() = %d\r\n",UdpAvailable);
	//	tXXXXXXXXCommand tempXXXXXXXXCommand;
	//	m_WiFiUDP.read((char *)&tempXXXXXXXXCommand,sizeof(tXXXXXXXXCommand));
	//	LastServerUpdate = 0;
	//}
}

unsigned long LastMillis = 0;
void TenthSecondsSinceStartTask()
{
	unsigned long CurrentMillis = millis();
	if (abs(CurrentMillis - LastMillis) > 100)
	{
		LastMillis = CurrentMillis;
		TenthSecondsSinceStart++;
		OnTenthSecond();
		//MyPrintf("TenthSecondsSinceStart = %d \r\n",TenthSecondsSinceStart);
		//MyPrintf("TenthSecondsSinceStart = %d \r\n",TenthSecondsSinceStart);
	}
}

void OnSecond()
{
	time_t now = time(nullptr); //获取当前时间
	time_str = ctime(&now);
	H1 = time_str[11];
	H2 = time_str[12];
	M1 = time_str[14];
	M2 = time_str[15];
	S1 = time_str[17];
	S2 = time_str[18];

	sprintf(TimeString, "%c%c:%c%c:%c%c\n", H1, H2, M1, M2, S1, S2);


	//OLED Displayer
	char DistanceString[16];
	sprintf(DistanceString, "%u", Distance);
	u8g2.clearBuffer();
	//u8g2.setFont(u8g2_font_wqy16_t_gb2312b);
	//u8g2.drawUTF8(0, 15, TimeString);
	
	u8g2.setFont(u8g2_font_logisoso26_tf);
	u8g2.drawStr(0, 41, TimeString);

	//u8g2.drawUTF8(64, 15, DistanceString);
	//u8g2.updateDisplayArea(0, 0, 8, 2);// One tile is a 8x8 pixel area
	u8g2.sendBuffer();




	//printf("LastServerUpdate = %d\r\n",LastServerUpdate);


	//LastServerUpdate++;
	//if (LastServerUpdate > 30)
	//{
	//	printf("Re connection routing!\n");  
	//	WiFi.disconnect();
	//	WiFi.mode(WIFI_STA);//设置模式为STA
	//	WiFi.begin(ssid, password); //Wifi接入到网络

	//	//printf("reset!\n");  
	//	//ESP.reset();

	//	LastServerUpdate = 0;
	//}

	//m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	//m_WiFiUDP.write((const char*)&XXXXXXXXData, sizeof(tXXXXXXXXData));
	//m_WiFiUDP.endPacket(); 
}

void OnTenthSecond()
{


	static long LastTenthSecondsForOnSecond = 0;
	if (TenthSecondsSinceStart - LastTenthSecondsForOnSecond >9)
	{
		OnSecond();
		LastTenthSecondsForOnSecond = TenthSecondsSinceStart;
	}

	//Distance = DistanceSensor.readRangeContinuousMillimeters();

	//for (byte i = 0; i < DISTANCE_NUMBER-1; i++)
	//{
	//	DistanceArray[DISTANCE_NUMBER-i-1] = DistanceArray[DISTANCE_NUMBER-i-2];
	//}
	//DistanceArray[0] = Distance;

	////Open or Close
	//static bool LastUnder100 = false;
	//static bool LastOver790 = false;
	//char OutputString[DISTANCE_NUMBER * 10 + 10];

	//if ((Distance < 100)&&(!LastUnder100))
	//{
	//	LastUnder100 = true;
	//	LastOver790 = false;
	//	MyPrintf("Sliding door Open finished\r\n");

	//	for (byte i = 0; i < DISTANCE_NUMBER; i++)
	//	{
	//		sprintf(OutputString + i * 10, "\n%04d %04d", DistanceArray[i],DistanceArray[i+1] - DistanceArray[i]);
	//	}
	//	MyPrintf("Distance = %s\r\n", OutputString);
	//}



	//if ((Distance > 790) && (!LastOver790))
	//{
	//	LastOver790 = true;
	//	LastUnder100 = false;
	//	MyPrintf("Sliding door Close finished\r\n");

	//	for (byte i = 0; i < DISTANCE_NUMBER; i++)
	//	{
	//		sprintf(OutputString + i * 10, "\n%04d %04d", DistanceArray[i],DistanceArray[i] - DistanceArray[i+1]);
	//	}
	//	MyPrintf("Distance = %s\r\n", OutputString);
	//}







	//printf("Distance = %u\r\n", Distance);




}



void MyPrintf(const char *fmt, ...)
{
	static char send_buf[1024];
	char * sprint_buf = send_buf+sizeof(tDebugData);
	tDebugData * pDebugData = (tDebugData*)send_buf;

	int n;
	va_list args;

	va_start(args,fmt);
	n = vsprintf(sprint_buf, fmt, args);
	va_end(args);

	//printf(sprint_buf);

	pDebugData->DataType = 3;
	pDebugData->RoomId = DebugLogIndex;
	pDebugData->Length = n;

	m_WiFiUDP.beginPacket("192.168.0.17", 5050);
	m_WiFiUDP.write((const char*)send_buf, sizeof(tDebugData)+n);
	m_WiFiUDP.endPacket(); 

}






