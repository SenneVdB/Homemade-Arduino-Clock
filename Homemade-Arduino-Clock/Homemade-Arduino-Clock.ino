#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

const int ledDown = 1;			// LED 1
const int ledRight = 5;			// LED 2
const int ledLeft = 3;			// LED 3
const int ledTop = 7;			// LED 4
const int buttonDown = 2;		// Select
const int buttonRight = 6;		// Right
const int buttonLeft = 4;		// Left
const int buttonTop = 8;		// Menu
/**
 * Button layout based on a keyboard numpad:
 * 		  8
 * 		4   6
 * 		  2
 */

RTC_DS1307 rtc;						// RTC Object which holds the time
LiquidCrystal_I2C lcd(0x20,16,2);	// LCD Display initializer
boolean menuActive;					// Whether or not a menu is active, used to get out of every menu at once
DateTime timeNow; 					// Object used to check the time, must be updated in order to keep track of time
String tempStr;						// Reuseable temp string object 
int h;								// Global hour variable
int m;								// Global minute variable
int s;								// Global second variable
long alarmTiming;					// Variable for alarm timings

byte bell[8] = { // Custom bell character
	B00000,
	B00100,
	B01110,
	B01110,
	B01110,
	B11111,
	B00100,
	B00000
};

enum alarmFrequency {
	daily,
	weekdays,
	weekends,
	once
};

class Alarm { // Alarm class which contains all needed info on an alarm
	public:		
		Alarm () { // Constructor
			alarmStatus = false;
			alarmTriggered = false;
			t = DateTime(timeNow.year(), timeNow.month(), timeNow.day(), 0, 0, 0);
		}
		boolean alarmStatus;
		DateTime t;
		alarmFrequency frequency;
		bool alarmTriggered;
		
		void setTime(int h, int m, int s) {
			t = DateTime(t.year(), t.month(), t.day(), h, m, s);
		}
		void setTime(int h, int m) {
			t = DateTime(t.year(), t.month(), t.day(), h, m, 0);
		}
};

Alarm alarms[4];		// Array of 4 alarms

void setup () {
	if (!rtc.begin();) {			// Check for the existance of an RTC
		Serial.begin(9600);
		Serial.println("RTC could not be found");
	}

	if (!rtc.isrunning()) {			// If the RTC is already running, do not change it's date and time
		rtc.adjust(DateTime(2020,12,10,0,0,0));
	}
	
	menuActive = false;
	
	pinMode(ledDown, OUTPUT);		// LED 1
	pinMode(ledRight, OUTPUT);		// LED 2
	pinMode(ledLeft, OUTPUT);		// LED 3
	pinMode(ledTop, OUTPUT);		// LED 4
	pinMode(buttonDown, INPUT);		// Select
	pinMode(buttonRight, INPUT);	// Right
	pinMode(buttonLeft, INPUT);		// Left
	pinMode(buttonTop, INPUT);		// Menu

	lcd.init();
	lcd.noCursor();
    lcd.clear();
    lcd.display();
    lcd.backlight();
	lcd.home();
	lcd.createChar(0, bell);
}

void loop() {
	updateDisplay();
	lcd.home();
	for (int i = 0; i < 4; i++) {
		if (alarms[i].alarmStatus) alarms[i].alarmStatus = checkAlarm(alarms[i]);
	}
	switch (button()) { // More cases then needed for potential to add features in the future
		case buttonDown:
		case buttonLeft:
		case buttonRight:
		case buttonTop:
			menu(0);
			menuActive = false;
			lcd.clear();
	}
	delay(20);
}

void updateDisplay() {
	/**
	 * Function to display the time and date on the main screen.
	 */
	lcd.setCursor(0,0);
	timeNow = rtc.now();
	int h = timeNow.hour();
	if (h < 10) {
		lcd.print("0" + String(h) + ":");
	} else {
		lcd.print(String(h) + ":");
	}
	int m = timeNow.minute();
	if (m < 10) {
		lcd.print("0" + String(m));
	} else {
		lcd.print(String(m));
	}
	int s = timeNow.second();
	lcd.print(":" + (s < 10 ? "0" + String(s) : String(s)));

	lcd.setCursor(0,1);
	String dayArray[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
	lcd.print(dayArray[timeNow.dayOfTheWeek()-1] + " ");
	lcd.print(timeNow.day());
	switch (timeNow.month()) {
		case 1:
			lcd.print(" Jan ");
			break;
		case 2:
			lcd.print(" Feb ");
			break;
		case 3:
			lcd.print(" Mar ");
			break;
		case 4:
			lcd.print(" Apr ");
			break;
		case 5:
			lcd.print(" May ");
			break;
		case 6:
			lcd.print(" Jun ");
			break;
		case 7:
			lcd.print(" Jul ");
			break;
		case 8:
			lcd.print(" Aug ");
			break;
		case 9:
			lcd.print(" Sep ");
			break;
		case 10:
			lcd.print(" Oct ");
			break;
		case 11:
			lcd.print(" Nov ");
			break;
		case 12:
			lcd.print(" Dec ");
			break;
	}
	lcd.print(timeNow.year());
	activatedAlarms();
}

int menu(int menuIndex) {
	/**
	 * Menu function that handles every menu.
	 * Uses local menu arrays in order to keep as much dynamical memory free to use for other variables.
	 */
	String title;
	int len;
	String arr[] = {"", "", "", ""};
	disableLeds();
	if (menuIndex == 0) {
		title = "Main Menu";
		arr[0] = "Set Time/Date";
		arr[1] = "Kitchen Timer";
		arr[2] = "Stopwatch";
		arr[3] = "Alarms";
		len = 4;
	} else if (menuIndex == 1) {
		title = "Set Time/Date";
		arr[0] = "Set Time";
		arr[1] = "Set Date";
		len = 2;
	} else if (menuIndex == 2) {
		title = "Alarms";
		arr[0] = "Alarm A";
		arr[1] = "Alarm B";
		arr[2] = "Alarm C";
		arr[3] = "Alarm D";
		len = 4;
	} else if (menuIndex == 3) {
		title = "Alarm On/Off";
		arr[0] = "On";
		arr[1] = "Off";
		len = 2;
	} else if (menuIndex == 4) {
		title = "Alarm Frequency";
		arr[0] = "Daily";
		arr[1] = "Weekdays";
		arr[2] = "Weekends";
		arr[3] = "Once";
		len = 4;
	} else {
		return;
	}
	menuActive = true;
	int index = 0;
	lcd.clear();
	lcd.home();
	lcd.print(title);
	lcd.setCursor(0,1);
	lcd.print(arr[index]);
	if (menuIndex == 2 && index != len) {
		alarmsOnOff(index);
	}
	while (menuActive) {
		switch (button()) {
			case buttonDown:
				if (index == len) {
					return;
				}
				int ln;
				if (menuIndex == 0 && index == 0) {
					menu(1);
				} else if (menuIndex == 1 && index == 0) {
					setRTCTime();
				} else if (menuIndex == 1 && index == 1) {
					setRTCDate();
				} else if (menuIndex == 0 && index == 1) {
					kitchenTimer();
				} else if (menuIndex == 0 && index == 2) {
					stopwatch();
				} else if (menuIndex == 0 && index == 3) {
					menu(2);
				} else if (menuIndex == 2 && index == 0) {
					setAlarm(0);
				} else if (menuIndex == 2 && index == 1) {
					setAlarm(1);
				} else if (menuIndex == 2 && index == 2) {
					setAlarm(2);
				} else if (menuIndex == 2 && index == 3) {
					setAlarm(3);
				} else if (menuIndex == 3 || menuIndex == 4) {
					return index;
				}
				lcd.clear();
				lcd.home();
				lcd.print(title);
				lcd.setCursor(0,1);
				lcd.print(arr[index]);
				if (menuIndex == 2 && index != len) {
					alarmsOnOff(index);
				}
				disableLeds();
				break;
			case buttonRight:
				index++;
				if (index > len) index = 0;
				lcd.setCursor(0,1);
				lcd.print("                ");
				lcd.setCursor(0,1);
				lcd.print(index == len ? "Back" : arr[index]);
				if (menuIndex == 2 && index != len) {
					alarmsOnOff(index);
				}
				delay(10);
				break;
			case buttonLeft:
				index--;
				if (index < 0) index = len;
				lcd.setCursor(0,1);
				lcd.print("                ");
				lcd.setCursor(0,1);
				lcd.print(index == len ? "Back" : arr[index]);
				if (menuIndex == 2 && index != len) {
					alarmsOnOff(index);
				}
				delay(10);
				break;
			case buttonTop:
				menuActive = false;
				break;
		}
	}
}

void setRTCTime() {
	/**
	 * Function to edit the RTC time.
	 * Called in the menu function.
	 */
	lcd.clear();
	lcd.home();
	lcd.print("Set hour");
	timeNow = rtc.now();
	lcd.setCursor(0,1);
	lcd.print((timeNow.hour() < 10 ? "0" + String(timeNow.hour()) : String(timeNow.hour())) + ":" + (timeNow.minute() < 10 ? "0" + String(timeNow.minute()) : String(timeNow.minute())));
	int h = scroll(0, timeNow.hour(), 0, 23);
	lcd.noCursor();
	if (h == -1) return;
	lcd.home();
	lcd.print("Set minute");
	int m = scroll(3, timeNow.minute(), 0, 59);
	lcd.noCursor();
	if (m == -1) return;
	if (h != -1 && m != -1) {
		timeNow = rtc.now();
		DateTime dt0(timeNow.year(), timeNow.month(), timeNow.day(), h, m, 0);
		rtc.adjust(dt0);
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print("Time set!   ");
		delay(1000);
	}
}

void setRTCDate() {
	/**
	 * Function to edit the RTC date.
	 * Called in the menu function.
	 */
	lcd.clear();
	lcd.home();
	lcd.print("Set year   ");
	timeNow = rtc.now();
	lcd.setCursor(0,1);
	lcd.print(String(timeNow.year()));
	int y = scroll(0, timeNow.year(), 2018, 2150);
	lcd.noCursor();
	if (y == -1) return;
	lcd.home();
	lcd.print("Set month ");
	lcd.setCursor(0,1);
	lcd.print(String(timeNow.month()) + "    ");
	int m = scroll(0, timeNow.month(), 1, 12);
	lcd.noCursor();
	if (m == -1) return;
	int tempInt;
	if (m == 1) {
		tempInt = 31;
	} else if (m == 2) {
		if (!(y % 4 != 0 || (y % 100 == 0 && y % 400 != 0))) { // Sets leap years
			tempInt = 29;
		} else {
			tempInt = 28;
		}
	} else if (m == 3) {
		tempInt = 31;
	} else if (m == 4) {
		tempInt = 30;
	} else if (m == 5) {
		tempInt = 31;
	} else if (m == 6) {
		tempInt = 30;
	} else if (m == 7) {
		tempInt = 31;
	} else if (m == 8) {
		tempInt = 31;
	} else if (m == 9) {
		tempInt = 30;
	} else if (m == 10) {
		tempInt = 31;
	} else if (m == 11) {
		tempInt = 30;
	} else if (m == 12) {
		tempInt = 31;
	}
	lcd.home();
	lcd.print("Set day   ");
	lcd.setCursor(0,1);
	lcd.print(timeNow.day() + "    ");
	int d = scroll(0, timeNow.day(), 1, tempInt);
	lcd.noCursor();
	if (d == -1) return;
	if (y != -1 && m != -1 && d != -1) {
		timeNow = rtc.now();
		DateTime dt0(y, m, d, timeNow.hour(), timeNow.minute(), timeNow.second());
		rtc.adjust(dt0);
		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print("Date set!");
		delay(1000);
	}
}

void stopwatch() {
	/**
	 * Stopwatch function called from the menu function.
	 * Press the lower button to start and pause.
	 * Use the upper button to get out of the stopwatch or reset it when paused.
	 */
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Stopwatch");
	DateTime start;
	int b;
	restartStopwatch:
	lcd.setCursor(0,1);
	lcd.print("00:00'00\"000");
	b = 0;
	while (b != buttonDown) {
		b = button();
		if (b == buttonTop) return;
	}
	disableLeds();
	timeNow = rtc.now();
	start = timeNow;
	auto startM = millis();
	while (true) {
		b = instantButton();
		if (b == buttonTop) {
			while (instantButton() == buttonTop);
			return;
		}
		if (b == buttonDown) {
			DateTime pause = rtc.now();
			long p = millis();
			while (button() != 0);
			disableLeds();
			do {
				b = button();
				if (b == buttonTop) goto restartStopwatch;
			} while (b != buttonDown);
			disableLeds();
			DateTime unpause = rtc.now();
			long up = millis();
			startM += up-p;
			DateTime tempDT = start + TimeSpan(unpause - pause);
			start = tempDT;
			b = 0;
		}
		tempStr = "";
		lcd.setCursor(0,1);
		h = timeNow.hour()-start.hour() + 24*(timeNow.hour()<start.hour());
		lcd.print((h < 10 ? "0" : "") + String(h));
		lcd.setCursor(3,1);
		m = timeNow.minute()-start.minute() + 60*(timeNow.minute()<start.minute());
		lcd.print((m < 10 ? "0" : "") + String(m));
		lcd.setCursor(6,1);
		s = timeNow.second()-start.second() + 60*(timeNow.second()<start.second());
		lcd.print((s < 10 ? "0" : "") + String(s));
		lcd.setCursor(9,1);
		tempStr = String(millis()-startM + 1000*(millis()<startM));
		lcd.print(tempStr.substring(tempStr.length()-3));
		activatedAlarms();
		timeNow = rtc.now();
	}
}

void kitchenTimer() {
	/**
	 * Kitchen Timer function called from the menu function.
	 * Sets a countdown timer, max 99 hours, 59 minutes and 59 seconds.
	 */
	lcd.clear();
	lcd.home();
	lcd.print("Set hours");
	lcd.setCursor(0,1);
	lcd.print("00:00:00");
	int h = scroll(0, 0, 0, 99);
	lcd.noCursor();
	if (h == -1) return;
	lcd.home();
	lcd.print("Set minutes");
	int m = scroll(3, 0, 0, 59);
	lcd.noCursor();
	if (m == -1) return;
	lcd.home();
	lcd.print("Set seconds");
	int s = scroll(6, 0, 0, 59);
	lcd.noCursor();
	if (s == -1) return;
	DateTime ktEnd = rtc.now() + TimeSpan(0,h,m,s);
	lcd.clear();
	lcd.setCursor(0,0);
	lcd.print("Countdown");
	TimeSpan kt = ktEnd-rtc.now();
	int b;
	do {
		b = button();
		if (b == buttonTop) return;
		kt = ktEnd-rtc.now();
		int d = kt.days();
		h = kt.hours();
		m = kt.minutes();
		s = kt.seconds();
		lcd.setCursor(0,1);
		lcd.print((d*24 + h < 10 ? "0" : "") + String(d*24 + h));
		lcd.print(":");
		lcd.print((m < 10 ? "0" : "") + String(m));
		lcd.print(":");
		lcd.print((s < 10 ? "0" : "") + String(s));
		lcd.print("    ");
		activatedAlarms();
		delay(50);
	} while (kt.totalseconds() != 0);
	long a = millis();
	while (b == 0) {
		b = button();
		if (a < millis()) {
			digitalWrite(ledRight, LOW);
			digitalWrite(ledDown, HIGH);
		}
		if (a + 125 < millis()) {
			digitalWrite(ledDown, LOW);
			digitalWrite(ledLeft, HIGH);
		}
		if (a + 250 < millis()) {
			digitalWrite(ledLeft, LOW);
			digitalWrite(ledTop, HIGH);
		}
		if (a + 375 < millis()) {
			digitalWrite(ledTop, LOW);
			digitalWrite(ledRight, HIGH);
			a += 500;
		}
	}
	delay(1000);
	disableLeds();
}

bool checkAlarm(Alarm al) {
	/**
	 * Function to handle alarms.
	 * Checks whether or not an alarm should be triggered,
	 * if triggered starts luminating LEDs randomly. 
	 */
	if (al.t.hour() == timeNow.hour() && al.t.minute() == timeNow.minute() && al.t.second() == timeNow.second() || al.alarmTriggered) {
		int dow = timeNow.dayOfTheWeek();
		alarmFrequency freq = al.frequency;
		if (freq == once || freq == daily || (freq == weekdays && (dow == 0 || dow == 1 || dow == 2 || dow == 3 || dow == 4)) || (freq == weekdays && (dow == 5 || dow == 6))) {
			bool on = true;
			int i = 0;
			alarmTiming = millis();
			while (button() == 0) {
				if (alarmTiming <= millis() && alarmTiming != 0) {
					disableLeds();
					alarmTiming += 200;
					digitalWrite(2*random(0,4)+1, HIGH);
				}
				updateDisplay();
				delay(20);
			}
			if (freq == once) return false;
		}
	}
	return true;
}

void activatedAlarms() {
	/**
	 * Checks which alarms are on and off.
	 * Used to display them on the screen when in the main screen, stopwatch and kitchen timer.
	 */
	String alarmString = "";
	String str = "ABCD";
	bool on = false;
	for (int i = 0; i < 4; i++) {
		if (alarms[i].alarmStatus) {
			on = true;
			lcd.setCursor(11,0);
			lcd.write(int(0));
			lcd.setCursor(12+i,0);
			lcd.print(str.substring(i,i+1));
		} else {
			lcd.setCursor(12+i,0);
			lcd.print(" ");
		}
	}
	if (!on) {
		lcd.setCursor(11,0);
		lcd.print(" ");
	}
}

int button() {
	/**
	 * Handles all button presses, outputs 0 when no button is pressed.
	 * When a button is pressed, it is illuminated
	 * Waits untill the button is not pressed anymore.
	 */
	if (digitalRead(buttonDown)) {
		digitalWrite(ledDown, HIGH);
		while (digitalRead(buttonDown)) delay(20);
		digitalWrite(ledDown, LOW);
		return buttonDown;
	} else if (digitalRead(buttonTop)) {
		digitalWrite(ledTop, HIGH);
		while (digitalRead(buttonTop)) delay(20);
		digitalWrite(ledTop, LOW);
		return buttonTop;
	} else if (digitalRead(buttonLeft)) {
		digitalWrite(ledLeft, HIGH);
		while (digitalRead(buttonLeft)) delay(20);
		digitalWrite(ledLeft, LOW);
		return buttonLeft;
	} else if (digitalRead(buttonRight)) {
		digitalWrite(ledRight, HIGH);
		while (digitalRead(buttonRight)) delay(20);
		digitalWrite(ledRight, LOW);
		return buttonRight;
	}
	return 0;
}

int instantButton() {
	/**
	 * The same as the button() function, but does not wait untill a button isn't pressed anymore.
	 */
	digitalWrite(ledDown, digitalRead(buttonDown));
	digitalWrite(ledTop, digitalRead(buttonTop));
	digitalWrite(ledLeft, digitalRead(buttonLeft));
	digitalWrite(ledRight, digitalRead(buttonRight));
	
	if (digitalRead(buttonDown)) {
		return buttonDown;
	} else if (digitalRead(buttonTop)) {
		return buttonTop;
	} else if (digitalRead(buttonLeft)) {
		return buttonLeft;
	} else if (digitalRead(buttonRight)) {
		return buttonRight;
	}
	return 0;
}

void alarmsOnOff(byte i) {
	/**
	 * Used in the alarm menu
	 * Displays which alarm is toggled on.
	 */
	if (alarms[i].alarmStatus) {
		lcd.print(" (On)");
	} else {
		lcd.print(" (Off)");
	}
}

void disableLeds() {
	digitalWrite(ledDown, 0);
	digitalWrite(ledTop, 0);
	digitalWrite(ledLeft, 0);
	digitalWrite(ledRight, 0);
}

void enableLeds() {
	digitalWrite(ledDown, 1);
	digitalWrite(ledTop, 1);
	digitalWrite(ledLeft, 1);
	digitalWrite(ledRight, 1);
}

int scroll(int col, int start, int min, int max) {
	/**
	 * Function used to scroll through numbers.
	 * Used when setting an alarm, hour, date or kitchen timer.
	 */
	int out = start;
	if (out > max) out = min;
	if (out < min) out = max;
	lcd.setCursor(col, 1);
	lcd.print((out < 10 ? "0" + String(out) : String(out)));
	lcd.setCursor(1+col,1);
	lcd.cursor();
	int b = 0;
	while (b != buttonDown) {
		b = 0;
		while (b == 0) b = instantButton();
		if (b == buttonTop) return -1;
		if (b == buttonLeft) {
			out--;
			if (out < min) out = max;
		} else if (b == buttonRight) {
			out++;
			if (out > max) out = min;
		} else if (b == buttonDown) {
			button();
		}
		if (b != 0) {
			lcd.setCursor(col, 1);
			lcd.print((out < 10 ? "0" + String(out) : String(out)));
			lcd.setCursor(1+col,1);
		}
		delay(200);
	}
	return out;
}

void setAlarm(byte n) {
	/**
	 * Sets up an alarm.
	 * Called from the menu.
	 */
	lcd.clear();
	lcd.home();
	String EDArray[2] = {"On", "Off"};
	int i = menu(3);
	if (i == 0) {
		alarms[n].alarmStatus = true;
	} else if (i == 1) {
		alarms[n].alarmStatus = false;
		lcd.clear();
		lcd.home();
		lcd.print("Alarm " + EDArray[i]);
		delay(1000);
		return;
	} else {
		return;
	}
	lcd.clear();
	lcd.home();
	lcd.print("Set hour");
	lcd.setCursor(0,1);
	lcd.print((alarms[n].t.hour() < 10 ? "0" + String(alarms[n].t.hour()) : String(alarms[n].t.hour())) + ":" + (alarms[n].t.minute() < 10 ? "0" + String(alarms[n].t.minute()) : String(alarms[n].t.minute())));
	int h = scroll(0, alarms[n].t.hour(), 0, 23);
	lcd.noCursor();
	if (h == -1) return;
	lcd.home();
	lcd.print("Set minute");
	int m = scroll(3, alarms[n].t.minute(), 0, 59);
	lcd.noCursor();
	if (m == -1) return;
	if (h != -1 && m != -1) {
		timeNow = rtc.now();
		alarms[n].setTime(h, m);
		lcd.clear();
		lcd.setCursor(0,0);
	}
	i = menu(4);
	if (i == 4) {
		return;
	}
	lcd.clear();
	lcd.home();
	lcd.print("Alarm set");
	lcd.setCursor(0,1);
	if (i == 0) {
		alarms[n].frequency = daily;
		lcd.print("daily!");
	} else if (i == 1) {
		alarms[n].frequency = weekdays;
		lcd.print("on weekdays!");
	} else if (i == 2) {
		alarms[n].frequency = weekends;
		lcd.print("on weekends!");
	} else if (i == 3) {
		alarms[n].frequency = once;
		lcd.print("once!");
	}
	alarms[n].alarmStatus = true;
	lcd.print("!");
	delay(1000);
}
