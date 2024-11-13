//SCREEN LIBS
#include <Elegoo_GFX.h>    
#include <Elegoo_TFTLCD.h>
#include <TouchScreen.h>

//SCREEN PINS
#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET A4

//SCREEN DIMENSIONS
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define TS_MINX 120
#define TS_MAXX 900
#define TS_MINY 70
#define TS_MAXY 920

//COLORS
#define BLACK       0x0000
#define WHITE       0xFFFF
#define LIGHTGREY   0xB5B5
#define RED         0xF800

//BUTTONS TOUCH ZONES
#define NFC_X   30
#define NFC_Y   90
#define NFC_WIDTH 160
#define NFC_HEIGHT 30

#define WIFI_X  30
#define WIFI_Y  130
#define WIFI_WIDTH 160
#define WIFI_HEIGHT 30

#define FAUX_PORTAIL_X  30
#define FAUX_PORTAIL_Y  170
#define FAUX_PORTAIL_WIDTH 160
#define FAUX_PORTAIL_HEIGHT 30

//NFC MENU
#define BUTTON_X1   (SCREEN_WIDTH - BUTTON_WIDTH) / 2
#define BUTTON_Y1   80
#define BUTTON_WIDTH  200
#define BUTTON_HEIGHT 40

//WIFI MENU
#define BUTTON_X1   (SCREEN_WIDTH - BUTTON_WIDTH * 2 - 20) / 2
#define BUTTON_X2   (BUTTON_X1 + BUTTON_WIDTH + 20)
#define BUTTON_Y1   90
#define BUTTON_Y2   130
#define BUTTON_WIDTH 160
#define BUTTON_HEIGHT 30

//FP MENU
#define BUTTON_X1   (SCREEN_WIDTH - BUTTON_WIDTH * 2 - 20) / 2
#define BUTTON_X2   (BUTTON_X1 + BUTTON_WIDTH + 20)
#define BUTTON_Y1   90
#define BUTTON_Y2   130
#define BUTTON_WIDTH 160
#define BUTTON_HEIGHT 40

//COORDINATES
#define BACK_X 5
#define BACK_Y 5
#define BACK_SIZE 30
#define BUTTON_WIDTH 140
#define BUTTON_HEIGHT 40
#define BUTTON_X1 40
#define BUTTON_Y1 80
#define BUTTON_X2 180
#define BUTTON_Y2 80

//TXT ZONE
#define INFO_BOX_X 20
#define INFO_BOX_Y 180
#define INFO_BOX_WIDTH 280
#define INFO_BOX_HEIGHT 40

//PIN TOUCH
#define YP A2  
#define XM A3  
#define YM 8   
#define XP 9   

//OBJECTS
Elegoo_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//MENUS
enum MenuState {
  MAIN_MENU,
  NFC_MENU,
  WIFI_MENU,
  FAUX_PORTAIL_MENU
};

MenuState currentMenu = MAIN_MENU;
//VAR FOR RICKROLL CONFIRM
bool rickrollConfirm = false;

void setup() {
  Serial.begin(9600);
  tft.reset();
  tft.begin(0x9341); 
  tft.setRotation(1);
  tft.fillScreen(BLACK); 
  drawMainMenu(); 
}


void sendArduino2(int info) {
  Serial.write(info);  //SEND IN BIN
  String message = "Code ";
  message += info;
  message += " envoyé";
  displayDataInInfoBox(message);
}

//ERASE TXT
void clearTextArea(int x, int y, int width, int height) {
  tft.fillRect(x, y, width, height, BLACK);
}

//SHOW IN TXT ZONE
void displayDataInInfoBox(String data) {
  clearTextArea(INFO_BOX_X + 5, INFO_BOX_Y + 5, INFO_BOX_WIDTH - 10, INFO_BOX_HEIGHT - 10);
  tft.setTextColor(WHITE);
  tft.setCursor(INFO_BOX_X + 10, INFO_BOX_Y + 10);
  tft.setTextSize(1);
  tft.print(data);
}

void displayTwoLinesInInfoBox(String data1, String data2) {

  clearTextArea(INFO_BOX_X, INFO_BOX_Y, INFO_BOX_WIDTH, INFO_BOX_HEIGHT);

  int totalButtonsWidth = BUTTON_WIDTH * 2 + 20;  
  int centerX = (SCREEN_WIDTH - totalButtonsWidth) / 2; 

  int baseY = BUTTON_Y1 + BUTTON_HEIGHT + 10;

  tft.setTextColor(WHITE);
  tft.setCursor(centerX, baseY);
  tft.setTextSize(1);
  tft.print(data1);

  tft.setCursor(centerX, baseY + 15);  
  tft.print(data2);
}

void receiveDataArduino2() {
  String incomingData = "";
  static String lastData = ""; 

  while (Serial.available() > 0) {
    char incomingByte = Serial.read();
    incomingData += incomingByte;
  }

  if (incomingData.length() > 0) {
    if (currentMenu == FAUX_PORTAIL_MENU) {
      displayTwoLinesInInfoBox(lastData, incomingData);  
      lastData = incomingData; 
    } else {
      displayDataInInfoBox(incomingData);  
    }
  }
}

//MAIN MENU
void drawMainMenu() {
  tft.fillScreen(BLACK);  

  //TITLE
  tft.setTextSize(3); 
  tft.setTextColor(RED);
  tft.setCursor(50, 20); 
  tft.print("QTGLIDER");

  //TXT
  tft.setTextSize(1);  
  tft.setTextColor(WHITE);
  tft.setCursor(30, 50);  
  tft.print("MY CRIME IS THAT OF OUTSMARTING YOU,");
  tft.setCursor(30, 60); 
  tft.print("SOMETHING THAT YOU WILL NEVER FORGIVE ME FOR.");

  //SPACE BETWEEN BUTTONS
  int buttonY = 90;  

  //BUTTONS WIDTH
  int extendedWidth = 160;  

  // NFC
  tft.fillRect(NFC_X, buttonY, extendedWidth, NFC_HEIGHT, LIGHTGREY);
  tft.drawRect(NFC_X, buttonY, extendedWidth, NFC_HEIGHT, WHITE);
  tft.setTextSize(1); 
  tft.setTextColor(BLACK);
  
  //CENTER THE BUTTON
  int textX = NFC_X + (extendedWidth - 6 * strlen("NFC")) / 2;
  tft.setCursor(textX, buttonY + 15);
  tft.print("NFC");

  // WIFI
  buttonY += NFC_HEIGHT + 8; 
  tft.fillRect(WIFI_X, buttonY, extendedWidth, WIFI_HEIGHT, LIGHTGREY);
  tft.drawRect(WIFI_X, buttonY, extendedWidth, WIFI_HEIGHT, WHITE);
  tft.setTextColor(BLACK);
  
  //CENTER THE BUTTON
  textX = WIFI_X + (extendedWidth - 6 * strlen("WIFI")) / 2;
  tft.setCursor(textX, buttonY + 15);  
  tft.print("WIFI");

  //FP
  buttonY += WIFI_HEIGHT + 8;  
  tft.fillRect(NFC_X, buttonY, extendedWidth, FAUX_PORTAIL_HEIGHT, LIGHTGREY);  
  tft.drawRect(NFC_X, buttonY, extendedWidth, FAUX_PORTAIL_HEIGHT, WHITE);
  tft.setTextColor(BLACK);
  
  // CENTER THE BUTTON
  textX = NFC_X + (extendedWidth - 6 * strlen("Faux Portail")) / 2;
  int textY = buttonY + (FAUX_PORTAIL_HEIGHT - 8) / 2; 
  tft.setCursor(textX, textY); 
  tft.print("Faux Portail");
}

// NFC MENU
void drawNFCMenu() {
  tft.fillScreen(BLACK);

  //CENTER TITLE
  String title = "NFC";  
  int titleWidth = 6 * title.length(); 
  int titleX = (SCREEN_WIDTH - titleWidth) / 2; 
  tft.setTextSize(2);
  tft.setTextColor(RED);
  tft.setCursor(titleX, 20); 
  tft.print(title);

  drawBackButton();

  //CENTER BUTTON
  int buttonX = (SCREEN_WIDTH - BUTTON_WIDTH) / 2; 
  int buttonY = BUTTON_Y1; 

  tft.fillRect(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, LIGHTGREY);
  tft.drawRect(buttonX, buttonY, BUTTON_WIDTH, BUTTON_HEIGHT, WHITE);

  // CENTER TXT
  int textX = buttonX + (BUTTON_WIDTH - 6 * strlen("Lire carte jeune")) / 2; 
  tft.setCursor(textX, buttonY + 10); 
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.print("Lire carte jeune");

  tft.drawRect(INFO_BOX_X, INFO_BOX_Y, INFO_BOX_WIDTH, INFO_BOX_HEIGHT, WHITE);
}

// WIFI MENU
void drawWiFiMenu() {
  tft.fillScreen(BLACK);

  // CENTER TITLE
  String title = "WIFI"; 
  int titleWidth = 6 * title.length(); 
  int titleX = (SCREEN_WIDTH - titleWidth) / 2; 
  tft.setTextSize(2);
  tft.setTextColor(RED);
  tft.setCursor(titleX, 20); 
  tft.print(title);

  drawBackButton();

  // CENTER BUTTONS
  int buttonSpacing = 20;  
  int totalButtonsWidth = BUTTON_WIDTH * 2 + buttonSpacing;  
  int startX = (SCREEN_WIDTH - totalButtonsWidth) / 2; 

  //SCAN
  int buttonX1 = startX;
  tft.fillRect(buttonX1, BUTTON_Y1, BUTTON_WIDTH, BUTTON_HEIGHT, LIGHTGREY);
  tft.drawRect(buttonX1, BUTTON_Y1, BUTTON_WIDTH, BUTTON_HEIGHT, WHITE);
  int textX1 = buttonX1 + (BUTTON_WIDTH - 6 * strlen("Scan WiFi")) / 2; 
  tft.setCursor(textX1, BUTTON_Y1 + 10); 
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.print("Scan WiFi");

  // RICKROLL
  int buttonX2 = buttonX1 + BUTTON_WIDTH + buttonSpacing;
  tft.fillRect(buttonX2, BUTTON_Y2, BUTTON_WIDTH, BUTTON_HEIGHT, LIGHTGREY);
  tft.drawRect(buttonX2, BUTTON_Y2, BUTTON_WIDTH, BUTTON_HEIGHT, WHITE);
  int textX2 = buttonX2 + (BUTTON_WIDTH - 6 * strlen("Rickroll")) / 2;
  tft.setCursor(textX2, BUTTON_Y2 + 10);
  tft.print("Rickroll");

  tft.drawRect(INFO_BOX_X, INFO_BOX_Y, INFO_BOX_WIDTH, INFO_BOX_HEIGHT, WHITE);
}


//FP MENU
void drawFauxPortailMenu() {
  tft.fillScreen(BLACK);

  // CENTRE TITRE
  String title = "Faux Portail";  
  int titleWidth = 6 * title.length(); 
  int titleX = (SCREEN_WIDTH - titleWidth) / 2; 
  tft.setTextSize(2);
  tft.setTextColor(RED);
  tft.setCursor(titleX, 20);
  tft.print(title);

  drawBackButton();

  // CENTRE BOUTONS
  int buttonSpacing = 20;  
  int totalButtonsWidth = BUTTON_WIDTH * 2 + buttonSpacing;  
  int startX = (SCREEN_WIDTH - totalButtonsWidth) / 2; 

  // BOUTON START
  int buttonXStart = startX;
  tft.fillRect(buttonXStart, BUTTON_Y1, BUTTON_WIDTH, BUTTON_HEIGHT, LIGHTGREY);
  tft.drawRect(buttonXStart, BUTTON_Y1, BUTTON_WIDTH, BUTTON_HEIGHT, WHITE);
  int textXStart = buttonXStart + (BUTTON_WIDTH - 6 * strlen("Start")) / 2; 
  tft.setCursor(textXStart, BUTTON_Y1 + 10); 
  tft.setTextSize(1);
  tft.setTextColor(WHITE);
  tft.print("Start");

  // BOUTON STOP
  int buttonXStop = buttonXStart + BUTTON_WIDTH + buttonSpacing;  
  tft.fillRect(buttonXStop, BUTTON_Y1, BUTTON_WIDTH, BUTTON_HEIGHT, LIGHTGREY);
  tft.drawRect(buttonXStop, BUTTON_Y1, BUTTON_WIDTH, BUTTON_HEIGHT, WHITE);
  int textXStop = buttonXStop + (BUTTON_WIDTH - 6 * strlen("Stop")) / 2; 
  tft.setCursor(textXStop, BUTTON_Y1 + 10); 
  tft.print("Stop");
}

// BACK
void drawBackButton() {
  //CENTER BUTTON <
  int textX = BACK_X + (BACK_SIZE - 6) / 2;  
  tft.fillRect(BACK_X, BACK_Y, BACK_SIZE, BACK_SIZE, LIGHTGREY);
  tft.drawRect(BACK_X, BACK_Y, BACK_SIZE, BACK_SIZE, WHITE);
  tft.setTextSize(2);
  tft.setTextColor(WHITE);
  tft.setCursor(textX, BACK_Y + 5);  
  tft.print("<");
}


//TOUCH DETECTED ?
bool isButtonPressed(int x, int y, int width, int height, TSPoint p) {
  return p.x > x && p.x < (x + width) && p.y > y && p.y < (y + height);
}


void handleTouch() {
  TSPoint p = ts.getPoint();
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);

  if (p.z > ts.pressureThreshhold) {
    // COORDINATES
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, SCREEN_WIDTH);
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, SCREEN_HEIGHT);

    //BACK
    if (isButtonPressed(BACK_X, BACK_Y, BACK_SIZE, BACK_SIZE, p)) {
      currentMenu = MAIN_MENU;
      drawMainMenu();
      return;
    }

    // MAIN MENU NAVIGATION
    if (currentMenu == MAIN_MENU) {
      if (isButtonPressed(NFC_X, NFC_Y, NFC_WIDTH, NFC_HEIGHT, p)) {
        currentMenu = NFC_MENU;
        drawNFCMenu();
      } else if (isButtonPressed(WIFI_X, WIFI_Y, WIFI_WIDTH, WIFI_HEIGHT, p)) {
        currentMenu = WIFI_MENU;
        drawWiFiMenu();
      } else if (isButtonPressed(FAUX_PORTAIL_X, FAUX_PORTAIL_Y, FAUX_PORTAIL_WIDTH, FAUX_PORTAIL_HEIGHT, p)) {
        currentMenu = FAUX_PORTAIL_MENU;
        drawFauxPortailMenu();
      }
    }

    //NFC MENU
    else if (currentMenu == NFC_MENU) {
      if (isButtonPressed(BUTTON_X1, BUTTON_Y1, BUTTON_WIDTH, BUTTON_HEIGHT, p)) {
        sendArduino2(10);  
      }
    }

    // WIFI MENU
    else if (currentMenu == WIFI_MENU) {
      if (isButtonPressed(BUTTON_X1, BUTTON_Y1, BUTTON_WIDTH, BUTTON_HEIGHT, p)) {
        sendArduino2(20);  
      } else if (isButtonPressed(BUTTON_X2, BUTTON_Y2, BUTTON_WIDTH, BUTTON_HEIGHT, p)) {
        if (!rickrollConfirm) {
          displayDataInInfoBox("Confirmer Rickroll");
          rickrollConfirm = true;
        } else {
          sendArduino2(30);  
          rickrollConfirm = false;
        }
      }
    }

    // FP MENU
    else if (currentMenu == FAUX_PORTAIL_MENU) {
      if (isButtonPressed(BUTTON_X1, BUTTON_Y1, BUTTON_WIDTH, BUTTON_HEIGHT, p)) {
        sendArduino2(40); 
      } else if (isButtonPressed(BUTTON_X2, BUTTON_Y2, BUTTON_WIDTH, BUTTON_HEIGHT, p)) {
        sendArduino2(50); 
      }
    }
  }
}

void loop() {
  handleTouch();         
  receiveDataArduino2(); 
}
