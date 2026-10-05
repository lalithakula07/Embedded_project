#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Keyboard.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

// button pins
#define UP_BUTTON A0
#define DOWN_BUTTON A1
#define SELECT_BUTTON A2

// master pin
String masterPIN = "0000";
String enteredPIN = "";

int selectedNumber = 0;

// different screens
#define PIN_SCREEN 0
#define MAIN_MENU 1
#define SITE_LIST 2
#define SITE_DETAILS 3

int mode = PIN_SCREEN;

// account storage
#define MAX_SITES 5

struct Account {
  char site[20];
  char username[30];
  char password[30];
};

Account accounts[MAX_SITES];

int siteCount = 0;
int selectedSite = 0;

// EEPROM addresses
#define EEPROM_COUNT_ADDRESS 0
#define EEPROM_DATA_ADDRESS 10

void setup() {
  Serial.begin(9600);

  pinMode(UP_BUTTON, INPUT);
  pinMode(DOWN_BUTTON, INPUT);
  pinMode(SELECT_BUTTON, INPUT);

  lcd.init();
  lcd.backlight();

  Keyboard.begin();

  loadAccounts();

  showPINScreen();

  Serial.println("==============================");
  Serial.println("     PASSWORD MANAGER");
  Serial.println("==============================");
  Serial.println("Default PIN: 0000");
  Serial.println();
}

void loop() {
  // PIN screen
  if (mode == PIN_SCREEN) {
    if (digitalRead(UP_BUTTON) == LOW) {
      delay(150);

      selectedNumber--;

      if (selectedNumber < 0) {
        selectedNumber = 9;
      }

      showPINScreen();
      waitForRelease(UP_BUTTON);
    }

    if (digitalRead(DOWN_BUTTON) == LOW) {
      delay(150);

      selectedNumber++;

      if (selectedNumber > 9) {
        selectedNumber = 0;
      }

      showPINScreen();
      waitForRelease(DOWN_BUTTON);
    }

    if (digitalRead(SELECT_BUTTON) == LOW) {
      delay(150);

      enteredPIN += String(selectedNumber);

      Serial.print("PIN digit selected: ");
      Serial.println(selectedNumber);

      if (enteredPIN.length() >= 4) {
        if (enteredPIN == masterPIN) {
          Serial.println("PIN CORRECT");

          enteredPIN = "";
          selectedNumber = 0;
          selectedSite = 0;

          mode = MAIN_MENU;

          showMainMenu();
        }
        else {
          Serial.println("WRONG PIN");

          lcd.clear();

          lcd.setCursor(0, 0);
          lcd.print("WRONG PIN!");

          delay(1200);

          enteredPIN = "";
          selectedNumber = 0;

          showPINScreen();
        }
      }
      else {
        showPINScreen();
      }

      waitForRelease(SELECT_BUTTON);
    }

    return;
  }

  // main menu
  if (mode == MAIN_MENU) {
    // Python can request account addition
    if (Serial.available() > 0) {
      String command = Serial.readStringUntil('\n');
      command.trim();

      if (command == "ADD") {
        addSite();
        return;
      }
    }

    // main menu currently contains only View Sites
    if (digitalRead(SELECT_BUTTON) == LOW) {
      delay(150);

      if (siteCount == 0) {
        lcd.clear();

        lcd.setCursor(0, 0);
        lcd.print("No sites saved");

        lcd.setCursor(0, 1);
        lcd.print("SELECT=back");

        mode = SITE_DETAILS;
      }
      else {
        selectedSite = 0;

        mode = SITE_LIST;

        showSiteList();
      }

      waitForRelease(SELECT_BUTTON);
    }

    return;
  }

  // site list
  if (mode == SITE_LIST) {
    if (digitalRead(UP_BUTTON) == LOW) {
      delay(150);

      selectedSite--;

      if (selectedSite < 0) {
        selectedSite = siteCount;
      }

      showSiteList();
      waitForRelease(UP_BUTTON);
    }

    if (digitalRead(DOWN_BUTTON) == LOW) {
      delay(150);

      selectedSite++;

      if (selectedSite > siteCount) {
        selectedSite = 0;
      }

      showSiteList();
      waitForRelease(DOWN_BUTTON);
    }

    if (digitalRead(SELECT_BUTTON) == LOW) {
      delay(150);

      if (selectedSite == siteCount) {
        mode = MAIN_MENU;
        selectedSite = 0;

        showMainMenu();
      }
      else {
        mode = SITE_DETAILS;

        showSiteDetails();
      }

      waitForRelease(SELECT_BUTTON);
    }

    return;
  }

  // site details
  if (mode == SITE_DETAILS) {
    if (digitalRead(SELECT_BUTTON) == LOW) {
      delay(150);

      mode = MAIN_MENU;
      selectedSite = 0;

      showMainMenu();

      waitForRelease(SELECT_BUTTON);
    }

    return;
  }
}

// display PIN screen
void showPINScreen() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("PIN:");

  for (int i = 0; i < enteredPIN.length(); i++) {
    lcd.print("*");
  }

  lcd.setCursor(0, 1);

  int firstNumber;

  if (selectedNumber == 0) {
    firstNumber = 0;
  }
  else if (selectedNumber == 9) {
    firstNumber = 7;
  }
  else {
    firstNumber = selectedNumber - 1;
  }

  for (int i = 0; i < 3; i++) {
    int number = firstNumber + i;

    if (number == selectedNumber) {
      lcd.print(">");
    }
    else {
      lcd.print(" ");
    }

    lcd.print(number);

    if (i < 2) {
      lcd.print(" ");
    }
  }
}

// display main menu
void showMainMenu() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("> View Sites");
}

// display site list
void showSiteList() {
  lcd.clear();

  lcd.setCursor(0, 0);

  if (selectedSite == siteCount) {
    lcd.print("> BACK");
  }
  else {
    lcd.print("> ");
    lcd.print(accounts[selectedSite].site);
  }

  lcd.setCursor(0, 1);

  int nextItem = selectedSite + 1;

  if (nextItem == siteCount) {
    lcd.print("  BACK");
  }
  else if (nextItem < siteCount) {
    lcd.print("  ");
    lcd.print(accounts[nextItem].site);
  }
}

// display selected site's details
void showSiteDetails() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(accounts[selectedSite].site);

  lcd.setCursor(0, 1);
  lcd.print(accounts[selectedSite].username);

  delay(2000);

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Password:");

  lcd.setCursor(0, 1);
  lcd.print("********");

  delay(500);

  Serial.println();
  Serial.println("==============================");
  Serial.println("         HID OUTPUT");
  Serial.println("==============================");

  Serial.print("Site: ");
  Serial.println(accounts[selectedSite].site);

  Serial.print("Username: ");
  Serial.println(accounts[selectedSite].username);

  Serial.println("Typing username:");

  Keyboard.print(accounts[selectedSite].username);

  delay(100);

  Keyboard.press(KEY_TAB);
  delay(50);
  Keyboard.release(KEY_TAB);

  Serial.println("[TAB]");

  Serial.println("Typing password:");

  Keyboard.print(accounts[selectedSite].password);

  delay(100);

  Keyboard.press(KEY_RETURN);
  delay(50);
  Keyboard.release(KEY_RETURN);

  Serial.println("[ENTER]");

  Serial.println("==============================");
}

// add a new site through Python
void addSite() {
  if (siteCount >= MAX_SITES) {
    Serial.println("ERROR: Memory full.");

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Memory full!");

    delay(1500);

    showMainMenu();

    return;
  }

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Add Site");

  lcd.setCursor(0, 1);
  lcd.print("> BACK");

  Serial.println();
  Serial.println("==============================");
  Serial.println("        ADD NEW SITE");
  Serial.println("==============================");
  Serial.println("Enter site name:");

  while (Serial.available() == 0) {
    if (digitalRead(UP_BUTTON) == LOW) {
      delay(150);

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Cancelled");

      delay(800);

      showMainMenu();

      waitForRelease(UP_BUTTON);

      return;
    }

    delay(10);
  }

  String site = Serial.readStringUntil('\n');
  site.trim();

  Serial.println("Enter username:");

  while (Serial.available() == 0) {
    if (digitalRead(UP_BUTTON) == LOW) {
      delay(150);

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Cancelled");

      delay(800);

      showMainMenu();

      waitForRelease(UP_BUTTON);

      return;
    }

    delay(10);
  }

  String username = Serial.readStringUntil('\n');
  username.trim();

  Serial.println("Enter password:");

  while (Serial.available() == 0) {
    if (digitalRead(UP_BUTTON) == LOW) {
      delay(150);

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Cancelled");

      delay(800);

      showMainMenu();

      waitForRelease(UP_BUTTON);

      return;
    }

    delay(10);
  }

  String password = Serial.readStringUntil('\n');
  password.trim();

  site.toCharArray(accounts[siteCount].site, 20);
  username.toCharArray(accounts[siteCount].username, 30);
  password.toCharArray(accounts[siteCount].password, 30);

  siteCount++;

  saveAccounts();

  Serial.println();
  Serial.println("SITE SAVED!");
  Serial.println();

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("SITE SAVED!");

  lcd.setCursor(0, 1);
  lcd.print(site);

  delay(1500);

  mode = MAIN_MENU;
  selectedSite = 0;

  showMainMenu();
}

// save accounts to EEPROM
void saveAccounts() {
  EEPROM.put(EEPROM_COUNT_ADDRESS, siteCount);

  int address = EEPROM_DATA_ADDRESS;

  for (int i = 0; i < siteCount; i++) {
    EEPROM.put(address, accounts[i]);

    address += sizeof(Account);
  }

  Serial.println("Accounts saved to EEPROM.");
}

// load accounts from EEPROM
void loadAccounts() {
  EEPROM.get(EEPROM_COUNT_ADDRESS, siteCount);

  if (siteCount < 0 || siteCount > MAX_SITES) {
    siteCount = 0;
  }

  int address = EEPROM_DATA_ADDRESS;

  for (int i = 0; i < siteCount; i++) {
    EEPROM.get(address, accounts[i]);

    address += sizeof(Account);
  }

  Serial.print("Sites loaded: ");
  Serial.println(siteCount);
}

// wait until button is released
void waitForRelease(int button) {
  while (digitalRead(button) == LOW) {
    delay(10);
  }
}