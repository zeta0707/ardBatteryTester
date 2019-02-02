//아래 LED핀 연결은 변경이 가능하다.
#define newLED 2  
//녹색 LED는 2번핀에 꽂습니다
//Use green LED for fresh level
#define okLED 4   
//파란색 LED는 4번핀에 꽂습니다
//Use blue LED for good level
#define oldLED 6  
//빨간색 LED는 6번핀에 꽂습니다.
//Use red LED for weak level

//include the library code:
#include <LiquidCrystal.h>

//각자 LCD keypad 쉴드에 맞는 핀으로 변경
//initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

//LCD keypad shied information, need to change from your own shield
//Mine:
//Top side 8 HOLE   : D13, D12, D11, D3, D2, TX, RX, GND
//Bottom side 6 HOLE: GND, A1, A2, A3, A4, A5

//현재 ADC 읽은 값
int analogValue = 0;
//이전 ADC 읽은 값
int lastAnalogValue = 0;
//비슷한 값이 읽힌 수
int countSameVale = 0;
//ADC값을 전압으로 변경
float voltage = 0;
//LED display시간, 즉 2초
int ledDelay = 2000;
//LCD에 표시할 문자
static char voltStr[8];

#define __DUE__ 0						//1: Due(=3.3V), 0: Uno, Mega(=5V)

void setup(){
	Serial.begin(9600);
	//serial monitor 9600bps로 설정
	pinMode(newLED, OUTPUT);
	pinMode(okLED, OUTPUT);
	pinMode(oldLED, OUTPUT);
	//LED의 핀모드를 OUTPUT으로 설정해줍니다
	//Set LED pins to OUTPUT

	//set up the LCD's number of columns and rows: 
	lcd.begin(16, 2);
	//Print a message to the LCD.
	//LCD 첫 줄에 "Battery Tester!" 출력
	lcd.print("Battery Tester!");
	//LCD 둘째 줄에 "VOLT:" 출력
	lcd.setCursor(0, 1);
	lcd.print("VOLT:"); 	
}

void loop(){
	//ADC1 read
	analogValue = analogRead(1);
	//analog 1번핀에 들어오는 값을 측정하여 이 값을 전압값으로 바꿔줍니다.
	//ADC1 -> voltage, 0~1023이 0V ~5V 의미
#if __DUE__
    voltage = (((float)analogValue) / 1024.0 ) * 3.3; 
#else  
	voltage = (((float)analogValue) / 1024.0 ) * 5.0;
#endif
	//디버깅용으로 serial monitor로 출력
	Serial.print(analogValue); Serial.print(","); Serial.print(voltage); 
	Serial.print(",");Serial.println(countSameVale);
	
	//For getting stable voltage, increase counter if ADC value is +/-10
	//ADC1 값이 이전에서 +/- 10이 차이가 나지 않은 경우 확인
	if (lastAnalogValue > (analogValue + 10)) 
		countSameVale = countSameVale+1;
	else if (lastAnalogValue > (analogValue - 10))
		countSameVale = countSameVale+1;	
	else 
		countSameVale = 0;
	//lastAnalogValue를 현재 읽은 값으로 저장
	lastAnalogValue = analogValue;
	
	//filtering out unstable value, ie. display voltage if 5 times same value
	// counter가 5이상일 경우, 즉 동일한 전압이 측정되고 있다고 판단되는 경우
	if(countSameVale > 5) {
		countSameVale = 0;
		
		//Made dtostrf2 since dtostrf doesn't work with Due
		//voltage를 voltStr에 4.2f 형식으로 문자열로 만들어 줌
		//그래야만 LCD에 출력할 수 있음
#if __DUE__        
		dtostrf2(voltage,4, 2, voltStr);
#else 
        dtostrf(voltage,4, 2, voltStr);
#endif
		//set the cursor to column 0, line 1
		//(note: line 1 is the second row, since counting begins with 0):
		//LCD 깨끗이 지우고 첫 번째, 두 번째 줄에 고정 문자열 출력
		lcd.clear();
		lcd.setCursor(0, 0);
		// Print a message to the LCD.
		lcd.print("Battery Tester!");
		lcd.setCursor(0, 1);
		lcd.print("VOLT:"); 
		//둘째 줄 "VOLT:"뒤에 측정한 전압 출력
		lcd.print(voltStr); 

		if(voltage >= 1.6){ 
			//전압값이 1.6이상일 경우 새 건전지
			//over 1.6V, fresh battery
			digitalWrite(newLED, HIGH);
			//녹색LED on
			lcd.print(",Fresh");
			//LCD 두 번째 줄 전압출력 뒤에 ",Fresh"출력
			delay(ledDelay);
			//2초 시간 지연
			digitalWrite(newLED, LOW); 
			//녹색 LED off
		}
		else if(voltage < 1.6 && voltage > 1.4){ 
			//1.6미만 1.4초과일 경우 사용가능한 건전지
			//1.4 ~ 1.6V, good level battery
			digitalWrite(okLED, HIGH);
			//녹색LED on 
			lcd.print(",GOOD");
			//LCD 두 번째 줄 전압출력 뒤에 ",Good"출력 
			delay(ledDelay);
			//2초 시간 지연
			digitalWrite(okLED, LOW); 
			//파란색 LED를 2초간 켜줍니다.
		}
		else if(voltage <= 1.4){ 
			//1.4이하일 경우 다 쓴 건전지
			//battery level is low, need to be replaced
			digitalWrite(oldLED, HIGH);
			//빨간색 LED on
			lcd.print(",Fail");
			//LCD 두 번째 줄 전압출력 뒤에 ",Fail"출력
			delay(ledDelay);
			//2초 시간 지연 
			digitalWrite(oldLED, LOW); 
			//빨간색 LED를 2초간 켜줍니다.
		}
	}
	delay(ledDelay / 16);
	//다음 측정까지 시간 지연
} 

char *dtostrf2(double val, signed char width, unsigned char prec, char *sout) {
	char fmt[8];
	sprintf(fmt, "%%%d.%df", width, prec);
	sprintf(sout, fmt, val);
	return sout;
}
