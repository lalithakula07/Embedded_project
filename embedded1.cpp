/*works without most functionalities only add and view
need to put BAck function while in stuff like during input of a new account
*/
#include <Adafruit_LiquidCrystal.h>
#include <EEPROM.h>

Adafruit_LiquidCrystal lcd(0);

// button pins

#define UP_BUTTON 2
#define DOWN_BUTTON 3
#define SELECT_BUTTON 4

// master pin

String masterPIN = "0000";
String enteredPIN = "";

int selectedNumber = 0;

// different screens in the program

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

// eeprom addresses

#define EEPROM_COUNT_ADDRESS 0
#define EEPROM_DATA_ADDRESS 10;


// setup

void setup() {
    Serial.begin(9600);

    pinMode(UP_BUTTON, INPUT_PULLUP);
    pinMode(DOWN_BUTTON, INPUT_PULLUP);
    pinMode(SELECT_BUTTON, INPUT_PULLUP);

    lcd.begin(16, 2);
    lcd.setBacklight(1);

    loadAccounts();

    showPINScreen();

    Serial.println("==============================");
    Serial.println("     PASSWORD MANAGER");
    Serial.println("==============================");
    Serial.println("Default PIN: 0000");
    Serial.println();
}


// main program loop

void loop() {

    // check the pin screen

    if (mode == PIN_SCREEN) {

        // move to the previous number

        if (digitalRead(UP_BUTTON) == LOW) {

            delay(150);

            selectedNumber--;

            if (selectedNumber < 0) {
                selectedNumber = 9;
            }

            showPINScreen();

            waitForRelease(UP_BUTTON);
        }


        // move to the next number

        if (digitalRead(DOWN_BUTTON) == LOW) {

            delay(150);

            selectedNumber++;

            if (selectedNumber > 9) {
                selectedNumber = 0;
            }

            showPINScreen();

            waitForRelease(DOWN_BUTTON);
        }


        // select the current number

        if (digitalRead(SELECT_BUTTON) == LOW) {

            delay(150);

            enteredPIN += String(selectedNumber);

            Serial.print("PIN digit selected: ");
            Serial.println(selectedNumber);

            // check the pin after 4 digits are entered

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


    // check the main menu

    if (mode == MAIN_MENU) {

        // move up in the menu

        if (digitalRead(UP_BUTTON) == LOW) {

            delay(150);

            selectedSite--;

            if (selectedSite < 0) {
                selectedSite = 1;
            }

            showMainMenu();

            waitForRelease(UP_BUTTON);
        }


        // move down in the menu

        if (digitalRead(DOWN_BUTTON) == LOW) {

            delay(150);

            selectedSite++;

            if (selectedSite > 1) {
                selectedSite = 0;
            }

            showMainMenu();

            waitForRelease(DOWN_BUTTON);
        }


        // select a menu option

        if (digitalRead(SELECT_BUTTON) == LOW) {

            delay(150);

            if (selectedSite == 0) {

                // show a message if there are no accounts

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

                addSite();
            }

            waitForRelease(SELECT_BUTTON);
        }

        return;
    }


    // check the site list

    if (mode == SITE_LIST) {

        // move to the previous site

        if (digitalRead(UP_BUTTON) == LOW) {

            delay(150);

            selectedSite--;

            if (selectedSite < 0) {
                selectedSite = siteCount - 1;
            }

            showSiteList();

            waitForRelease(UP_BUTTON);
        }


        // move to the next site

        if (digitalRead(DOWN_BUTTON) == LOW) {

            delay(150);

            selectedSite++;

            if (selectedSite >= siteCount) {
                selectedSite = 0;
            }

            showSiteList();

            waitForRelease(DOWN_BUTTON);
        }


        // open the selected site

        if (digitalRead(SELECT_BUTTON) == LOW) {

            delay(150);

            mode = SITE_DETAILS;

            showSiteDetails();

            waitForRelease(SELECT_BUTTON);
        }

        return;
    }


    // show the selected site's details

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


// display the pin screen

void showPINScreen() {

    lcd.clear();

    // show the entered digits on the first line

    lcd.setCursor(0, 0);

    lcd.print("PIN:");

    for (int i = 0; i < enteredPIN.length(); i++) {
        lcd.print("*");
    }

    // show the numbers that can be selected

    lcd.setCursor(0, 1);

    int firstNumber;

    // make sure the selected number stays on the screen

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


// display the main menu

void showMainMenu() {

    lcd.clear();

    if (selectedSite == 0) {

        lcd.setCursor(0, 0);
        lcd.print("> View Sites");

        lcd.setCursor(0, 1);
        lcd.print("  Add Site");

    }
    else {

        lcd.setCursor(0, 0);
        lcd.print("  View Sites");

        lcd.setCursor(0, 1);
        lcd.print("> Add Site");
    }
}


// display the list of saved sites

void showSiteList() {

    lcd.clear();

    lcd.setCursor(0, 0);

    lcd.print("SITE ");

    lcd.print(selectedSite + 1);

    lcd.print("/");

    lcd.print(siteCount);

    lcd.setCursor(0, 1);

    lcd.print("> ");

    lcd.print(accounts[selectedSite].site);
}


// display the selected site's details

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


    // simulate the hid keyboard output

    Serial.println();
    Serial.println("==============================");
    Serial.println("       HID SIMULATION");
    Serial.println("==============================");

    Serial.print("Site: ");
    Serial.println(accounts[selectedSite].site);

    Serial.print("Username: ");
    Serial.println(accounts[selectedSite].username);

    Serial.println();

    Serial.println("Typing username:");

    Serial.println(accounts[selectedSite].username);

    Serial.println("[TAB]");

    Serial.println("Typing password:");

    Serial.println(accounts[selectedSite].password);

    Serial.println("[ENTER]");

    Serial.println("==============================");
}


// add a new site

void addSite() {

    // check if there is space for another account

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
    lcd.print("AdD site");

    lcd.setCursor(0, 1);
    lcd.print("Check seroutput");


    // get the new account details from the serial monitor

    Serial.println();
    Serial.println("==============================");
    Serial.println("         add new site");
    Serial.println("==============================");

    Serial.println("Enter site name:");

    while (Serial.available() == 0) {
    }

    String site = Serial.readStringUntil('\n');
    site.trim();


    Serial.println("Enter username:");

    while (Serial.available() == 0) {
    }

    String username = Serial.readStringUntil('\n');
    username.trim();


    Serial.println("Enter password:");

    while (Serial.available() == 0) {
    }

    String password = Serial.readStringUntil('\n');
    password.trim();


    // copy the entered details into the account

    site.toCharArray(accounts[siteCount].site, 20);

    username.toCharArray(accounts[siteCount].username, 30);

    password.toCharArray(accounts[siteCount].password, 30);


    siteCount++;


    // save the new account

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


// save the accounts to eeprom

void saveAccounts() {

    EEPROM.put(EEPROM_COUNT_ADDRESS, siteCount);

    int address = EEPROM_DATA_ADDRESS;

    for (int i = 0; i < siteCount; i++) {

        EEPROM.put(address, accounts[i]);

        address += sizeof(Account);
    }

    Serial.println("Accounts saved to EEPROM.");
}


// load the saved accounts from eeprom

void loadAccounts() {

    EEPROM.get(EEPROM_COUNT_ADDRESS, siteCount);

    // make sure the saved account count is valid

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


// wait until the button is released

void waitForRelease(int button) {

    while (digitalRead(button) == LOW) {

        delay(10);
    }
}