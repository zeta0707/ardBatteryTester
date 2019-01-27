#define newLED 2  
// 녹색 LED는 2번핀에 꽂습니다
// Use green LED for fresh level
#define okLED 4   
// 파란색 LED는 4번핀에 꽂습니다
// Use blue LED for good level
#define oldLED 6  
// 빨간색 LED는 6번핀에 꽂습니다.
// Use red LED for weak level

// include the library code:
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// LCD keypad shied information, need to change from your own shield
// Mine
// Top side 8 HOLE   : D13, D12, D11, D3, D2, TX, RX, GND
// Bottom side 6 HOLE: GND, A1, A2, A3, A4, A5

int analogValue = 0;
int lastAnalogValue = 0;
int countSameVale = 0;
float voltage = 0;
int ledDelay = 2000;
static char outstr[15];

#define __DUE__ 0						//1: Due(=3.3V), 0: Uno, Mega(=5V)

void setup(){
	Serial.begin(9600);
	pinMode(newLED, OUTPUT);
	pinMode(okLED, OUTPUT);
	pinMode(oldLED, OUTPUT);
	// LED의 핀모드를 OUTPUT으로 설정해줍니다
	// Set LED pins to OUTPUT

	// set up the LCD's number of columns and rows: 
	lcd.begin(16, 2);
	// Print a message to the LCD.
	lcd.print("Battery Tester!");
	lcd.setCursor(0, 1);
	lcd.print("VOLT:"); 	
}

void loop(){
	analogValue = analogRead(1);
	// analog 1번핀에 들어오는 값을 측정하여 이 값을 전압값으로 바꿔줍니다.
	#if __MEGA2560__	
	voltage = (((float)analogValue) / 1024.0 ) * 3.3; 
	#else  
	voltage = (((float)analogValue) / 1024.0 ) * 5.0;
	#endif

	Serial.print(analogValue); Serial.print(","); Serial.print(voltage); 
	Serial.print(",");Serial.println(countSameVale);

	// For getting stable voltage, increase counter if ADC value is +/-10
	if (lastAnalogValue > (analogValue + 10)) 
		countSameVale = countSameVale+1;
	else if (lastAnalogValue > (analogValue - 10))
		countSameVale = countSameVale+1;	
	else 
		countSameVale = 0;

	lastAnalogValue = analogValue;
	
	//filtering out unstable value, ie. display voltage if 5 times same value
	if(countSameVale > 5) {
		countSameVale = 0;
		
		//Made dtostrf2 since dtostrf doesn't work with Due
		dtostrf2(voltage,4, 2, outstr);
		// set the cursor to column 0, line 1
		// (note: line 1 is the second row, since counting begins with 0):
		lcd.clear();
		lcd.setCursor(0, 0);
		// Print a message to the LCD.
		lcd.print("Battery Tester!");
		lcd.setCursor(0, 1);
		lcd.print("VOLT:"); 
		lcd.print(outstr); 

		if(voltage >= 1.6){ 
			// 전압값이 1.6이상일 경우 새 건전지
			// over 1.6V, fresh battery
			digitalWrite(newLED, HIGH);
			lcd.print(",Fresh");
			delay(ledDelay);
			digitalWrite(newLED, LOW); 
			// 녹색 LED를 2초간 켜줍니다.
		}
		else if(voltage < 1.6 && voltage > 1.4){ 
			// 1.6미만 1.4초과일 경우 사용가능한 건전지
			// 1.4 ~ 1.6V, good level battery
			digitalWrite(okLED, HIGH);
			lcd.print(",GOOD");
			delay(ledDelay);
			digitalWrite(okLED, LOW); 
			// 파란색 LED를 2초간 켜줍니다.
		}
		else if(voltage <= 1.4){ 
			// 1.4이하일 경우 다 쓴 건전지
			// battery level is low, need to be replaced
			digitalWrite(oldLED, HIGH);
			lcd.print(",Fail");
			delay(ledDelay);
			digitalWrite(oldLED, LOW); 
			// 빨간색 LED를 2초간 켜줍니다.
		}
	}
	delay(ledDelay / 16);
} 
char *dtostrf2(double val, signed char width, unsigned char prec, char *sout) {
	char fmt[20];
	sprintf(fmt, "%%%d.%df", width, prec);
	sprintf(sout, fmt, val);
	return sout;
}
