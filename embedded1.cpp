#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>
#include <Keyboard.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

#define UP_BUTTON A0
#define DOWN_BUTTON A1
#define SELECT_BUTTON A2

// screen modes
#define PIN_SCREEN 0
#define MAIN_MENU 1
#define SITE_LIST 2
#define SITE_DETAILS 3
#define SETTINGS 4
#define WIPE_CONFIRM 5
#define CHANGE_CURRENT_PIN 6
#define CHANGE_NEW_PIN 7
#define CHANGE_CONFIRM_PIN 8

int mode = PIN_SCREEN;

// pin
String masterPIN = "";
String enteredPIN = "";
String newPIN = "";
int selectedNumber = 0;
int failedAttempts = 0;

// eeprom layout
#define EEPROM_PIN_ADDRESS 0
#define EEPROM_PIN_VALID_ADDRESS 5

#define EEPROM_COUNT_ADDRESS 10
#define EEPROM_DATA_ADDRESS 20

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

// menu selections
int selectedSetting = 0;

// function declarations
void showPINScreen();
void showPINEntryScreen(String title);
void showMainMenu();
void showSiteList();
void showSiteDetails();
void showSettings();
void showWipeConfirm();

void loadPIN();
void savePIN();
void startChangePIN();

void loadAccounts();
void saveAccounts();
void wipeAllAccounts();

void addSite();
void waitForRelease(int button);

void setup() {
    Serial.begin(9600);

    pinMode(UP_BUTTON, INPUT);
    pinMode(DOWN_BUTTON, INPUT);
    pinMode(SELECT_BUTTON, INPUT);

    lcd.init();
    lcd.backlight();

    Keyboard.begin();

    loadPIN();
    loadAccounts();

    showPINScreen();

    Serial.println("==============================");
    Serial.println("     PASSWORD MANAGER");
    Serial.println("==============================");
    Serial.println("Serial ADD command ready.");
    Serial.println();
}

void loop() {

    // startup pin screen
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

            if (enteredPIN.length() >= 4) {

                if (enteredPIN == masterPIN) {

                    Serial.println("PIN CORRECT");

                    failedAttempts = 0;
                    enteredPIN = "";
                    selectedNumber = 0;
                    selectedSite = 0;

                    mode = MAIN_MENU;

                    showMainMenu();
                }
                else {

                    Serial.println("WRONG PIN");

                    failedAttempts++;

                    lcd.clear();

                    lcd.setCursor(0, 0);
                    lcd.print("WRONG PIN!");

                    delay(1200);

                    if (failedAttempts >= 3) {

                        lcd.clear();

                        lcd.setCursor(0, 0);
                        lcd.print("TOO MANY TRIES");

                        lcd.setCursor(0, 1);
                        lcd.print("DEVICE LOCKED");

                        delay(2000);

                        // currently only a warning
                        // user can continue trying
                        failedAttempts = 0;
                    }

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

        if (digitalRead(UP_BUTTON) == LOW) {
            delay(150);

            selectedSite--;

            if (selectedSite < 0) {
                selectedSite = 1;
            }

            showMainMenu();

            waitForRelease(UP_BUTTON);
        }

        if (digitalRead(DOWN_BUTTON) == LOW) {
            delay(150);

            selectedSite++;

            if (selectedSite > 1) {
                selectedSite = 0;
            }

            showMainMenu();

            waitForRelease(DOWN_BUTTON);
        }

        if (digitalRead(SELECT_BUTTON) == LOW) {
            delay(150);

            if (selectedSite == 0) {

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
            }
            else {

                selectedSetting = 0;
                mode = SETTINGS;

                showSettings();
            }

            waitForRelease(SELECT_BUTTON);
        }

        // python account adding
        if (Serial.available() > 0) {

            String command = Serial.readStringUntil('\n');
            command.trim();

            if (command == "ADD") {
                addSite();
            }
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

    // settings
    if (mode == SETTINGS) {

        if (digitalRead(UP_BUTTON) == LOW) {
            delay(150);

            selectedSetting--;

            if (selectedSetting < 0) {
                selectedSetting = 2;
            }

            showSettings();

            waitForRelease(UP_BUTTON);
        }

        if (digitalRead(DOWN_BUTTON) == LOW) {
            delay(150);

            selectedSetting++;

            if (selectedSetting > 2) {
                selectedSetting = 0;
            }

            showSettings();

            waitForRelease(DOWN_BUTTON);
        }

        if (digitalRead(SELECT_BUTTON) == LOW) {
            delay(150);

            // wipe all
            if (selectedSetting == 0) {

                mode = WIPE_CONFIRM;

                showWipeConfirm();
            }

            // change pin
            else if (selectedSetting == 1) {

                startChangePIN();
            }

            // back
            else {

                mode = MAIN_MENU;
                selectedSite = 0;

                showMainMenu();
            }

            waitForRelease(SELECT_BUTTON);
        }

        return;
    }

    // wipe confirmation
    if (mode == WIPE_CONFIRM) {

        // UP = no
        if (digitalRead(UP_BUTTON) == LOW) {
            delay(150);

            mode = SETTINGS;

            showSettings();

            waitForRelease(UP_BUTTON);
        }

        // DOWN = yes
        if (digitalRead(DOWN_BUTTON) == LOW) {
            delay(150);

            wipeAllAccounts();

            lcd.clear();

            lcd.setCursor(0, 0);
            lcd.print("ALL DATA WIPED");

            lcd.setCursor(0, 1);
            lcd.print("PIN UNCHANGED");

            delay(1800);

            mode = MAIN_MENU;
            selectedSite = 0;

            showMainMenu();

            waitForRelease(DOWN_BUTTON);
        }

        // SELECT = no
        if (digitalRead(SELECT_BUTTON) == LOW) {
            delay(150);

            mode = SETTINGS;

            showSettings();

            waitForRelease(SELECT_BUTTON);
        }

        return;
    }

    // enter current pin for change
    if (mode == CHANGE_CURRENT_PIN) {

        if (digitalRead(UP_BUTTON) == LOW) {
            delay(150);

            selectedNumber--;

            if (selectedNumber < 0) {
                selectedNumber = 9;
            }

            showPINEntryScreen("CURRENT PIN");

            waitForRelease(UP_BUTTON);
        }

        if (digitalRead(DOWN_BUTTON) == LOW) {
            delay(150);

            selectedNumber++;

            if (selectedNumber > 9) {
                selectedNumber = 0;
            }

            showPINEntryScreen("CURRENT PIN");

            waitForRelease(DOWN_BUTTON);
        }

        if (digitalRead(SELECT_BUTTON) == LOW) {
            delay(150);

            enteredPIN += String(selectedNumber);

            if (enteredPIN.length() >= 4) {

                if (enteredPIN == masterPIN) {

                    enteredPIN = "";
                    selectedNumber = 0;
                    newPIN = "";

                    mode = CHANGE_NEW_PIN;

                    showPINEntryScreen("NEW PIN");
                }
                else {

                    lcd.clear();

                    lcd.setCursor(0, 0);
                    lcd.print("WRONG PIN");

                    delay(1200);

                    enteredPIN = "";
                    selectedNumber = 0;

                    mode = SETTINGS;

                    showSettings();
                }
            }
            else {
                showPINEntryScreen("CURRENT PIN");
            }

            waitForRelease(SELECT_BUTTON);
        }

        return;
    }

    // enter new pin
    if (mode == CHANGE_NEW_PIN) {

        if (digitalRead(UP_BUTTON) == LOW) {
            delay(150);

            selectedNumber--;

            if (selectedNumber < 0) {
                selectedNumber = 9;
            }

            showPINEntryScreen("NEW PIN");

            waitForRelease(UP_BUTTON);
        }

        if (digitalRead(DOWN_BUTTON) == LOW) {
            delay(150);

            selectedNumber++;

            if (selectedNumber > 9) {
                selectedNumber = 0;
            }

            showPINEntryScreen("NEW PIN");

            waitForRelease(DOWN_BUTTON);
        }

        if (digitalRead(SELECT_BUTTON) == LOW) {
            delay(150);

            newPIN += String(selectedNumber);

            if (newPIN.length() >= 4) {

                enteredPIN = "";
                selectedNumber = 0;

                mode = CHANGE_CONFIRM_PIN;

                showPINEntryScreen("CONFIRM PIN");
            }
            else {
                showPINEntryScreen("NEW PIN");
            }

            waitForRelease(SELECT_BUTTON);
        }

        return;
    }

    // confirm new pin
    if (mode == CHANGE_CONFIRM_PIN) {

        if (digitalRead(UP_BUTTON) == LOW) {
            delay(150);

            selectedNumber--;

            if (selectedNumber < 0) {
                selectedNumber = 9;
            }

            showPINEntryScreen("CONFIRM PIN");

            waitForRelease(UP_BUTTON);
        }

        if (digitalRead(DOWN_BUTTON) == LOW) {
            delay(150);

            selectedNumber++;

            if (selectedNumber > 9) {
                selectedNumber = 0;
            }

            showPINEntryScreen("CONFIRM PIN");

            waitForRelease(DOWN_BUTTON);
        }

        if (digitalRead(SELECT_BUTTON) == LOW) {
            delay(150);

            enteredPIN += String(selectedNumber);

            if (enteredPIN.length() >= 4) {

                if (enteredPIN == newPIN) {

                    masterPIN = newPIN;

                    savePIN();

                    lcd.clear();

                    lcd.setCursor(0, 0);
                    lcd.print("PIN CHANGED");

                    lcd.setCursor(0, 1);
                    lcd.print("SAVED");

                    delay(1500);

                    enteredPIN = "";
                    newPIN = "";
                    selectedNumber = 0;

                    mode = SETTINGS;

                    showSettings();
                }
                else {

                    lcd.clear();

                    lcd.setCursor(0, 0);
                    lcd.print("PIN MISMATCH");

                    lcd.setCursor(0, 1);
                    lcd.print("TRY AGAIN");

                    delay(1500);

                    enteredPIN = "";
                    newPIN = "";
                    selectedNumber = 0;

                    mode = SETTINGS;

                    showSettings();
                }
            }
            else {
                showPINEntryScreen("CONFIRM PIN");
            }

            waitForRelease(SELECT_BUTTON);
        }

        return;
    }
}

// startup PIN screen
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

// PIN entry screen used for changing PIN
void showPINEntryScreen(String title) {

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print(title);

    lcd.setCursor(0, 1);

    for (int i = 0; i < enteredPIN.length(); i++) {
        lcd.print("*");
    }

    if (enteredPIN.length() < 4) {
        lcd.print(" ");
    }

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

// main menu
void showMainMenu() {

    lcd.clear();

    if (selectedSite == 0) {

        lcd.setCursor(0, 0);
        lcd.print("> View Sites");

        lcd.setCursor(0, 1);
        lcd.print("  Settings");
    }
    else {

        lcd.setCursor(0, 0);
        lcd.print("  View Sites");

        lcd.setCursor(0, 1);
        lcd.print("> Settings");
    }
}

// settings menu
void showSettings() {

    lcd.clear();

    if (selectedSetting == 0) {

        lcd.setCursor(0, 0);
        lcd.print("> Wipe All");

        lcd.setCursor(0, 1);
        lcd.print("  Change PIN");
    }
    else if (selectedSetting == 1) {

        lcd.setCursor(0, 0);
        lcd.print("  Wipe All");

        lcd.setCursor(0, 1);
        lcd.print("> Change PIN");
    }
    else {

        lcd.setCursor(0, 0);
        lcd.print("  Change PIN");

        lcd.setCursor(0, 1);
        lcd.print("> BACK");
    }
}

// wipe confirmation
void showWipeConfirm() {

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("WIPE ALL DATA?");

    lcd.setCursor(0, 1);
    lcd.print("UP=NO DN=YES");
}

// start changing PIN
void startChangePIN() {

    enteredPIN = "";
    newPIN = "";
    selectedNumber = 0;

    mode = CHANGE_CURRENT_PIN;

    showPINEntryScreen("CURRENT PIN");
}

// site list
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

// site details
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

    Serial.println();
    Serial.println("==============================");
    Serial.println("          HID LOGIN");
    Serial.println("==============================");

    Serial.print("Site: ");
    Serial.println(accounts[selectedSite].site);

    Serial.print("Username: ");
    Serial.println(accounts[selectedSite].username);

    Serial.println("Typing username:");

    Keyboard.print(accounts[selectedSite].username);

    Serial.println("[TAB]");

    Keyboard.press(KEY_TAB);
    delay(100);
    Keyboard.release(KEY_TAB);

    Serial.println("Typing password:");

    Keyboard.print(accounts[selectedSite].password);

    Serial.println("[ENTER]");

    Keyboard.press(KEY_RETURN);
    delay(100);
    Keyboard.release(KEY_RETURN);

    Serial.println("==============================");
}

// add new account
void addSite() {

    if (siteCount >= MAX_SITES) {

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
    lcd.print("Check serial");

    Serial.println();
    Serial.println("==============================");
    Serial.println("         ADD NEW SITE");
    Serial.println("==============================");

    // site
    Serial.println("Enter site name:");

    while (Serial.available() == 0) {

        // UP cancels
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

    // username
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

    // password
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

    // store account
    site.toCharArray(
        accounts[siteCount].site,
        sizeof(accounts[siteCount].site)
    );

    username.toCharArray(
        accounts[siteCount].username,
        sizeof(accounts[siteCount].username)
    );

    password.toCharArray(
        accounts[siteCount].password,
        sizeof(accounts[siteCount].password)
    );

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

// load PIN
void loadPIN() {

    byte valid = EEPROM.read(EEPROM_PIN_VALID_ADDRESS);

    if (valid != 0xA5) {

        // first startup
        // store four ASCII zero characters
        // without putting the PIN itself in the program
        for (int i = 0; i < 4; i++) {
            EEPROM.update(EEPROM_PIN_ADDRESS + i, '0');
        }

        EEPROM.update(EEPROM_PIN_VALID_ADDRESS, 0xA5);

        masterPIN = "0000";

        Serial.println("First startup PIN initialized.");
    }
    else {

        char pinBuffer[5];

        for (int i = 0; i < 4; i++) {
            pinBuffer[i] = EEPROM.read(EEPROM_PIN_ADDRESS + i);
        }

        pinBuffer[4] = '\0';

        masterPIN = String(pinBuffer);

        Serial.println("Master PIN loaded from EEPROM.");
    }
}

// save PIN
void savePIN() {

    for (int i = 0; i < 4; i++) {
        EEPROM.update(EEPROM_PIN_ADDRESS + i, masterPIN[i]);
    }

    EEPROM.update(EEPROM_PIN_VALID_ADDRESS, 0xA5);
}

// save accounts
void saveAccounts() {

    EEPROM.put(EEPROM_COUNT_ADDRESS, siteCount);

    int address = EEPROM_DATA_ADDRESS;

    for (int i = 0; i < siteCount; i++) {

        EEPROM.put(address, accounts[i]);

        address += sizeof(Account);
    }

    Serial.println("Accounts saved to EEPROM.");
}

// load accounts
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

// wipe accounts only
void wipeAllAccounts() {

    Serial.println("Wiping all account data...");

    int totalBytes = MAX_SITES * sizeof(Account);

    for (int i = 0; i < totalBytes; i++) {
        EEPROM.update(EEPROM_DATA_ADDRESS + i, 0);
    }

    siteCount = 0;

    EEPROM.put(EEPROM_COUNT_ADDRESS, siteCount);

    // clear account data from RAM too
    for (int i = 0; i < MAX_SITES; i++) {
        memset(&accounts[i], 0, sizeof(Account));
    }

    Serial.println("All account data wiped.");
    Serial.println("Master PIN unchanged.");
}

// wait for button release
void waitForRelease(int button) {

    while (digitalRead(button) == LOW) {
        delay(10);
    }
}