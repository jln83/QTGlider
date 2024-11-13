//Librairies
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

// Pins pour MFRC522
#define RST_PIN 9
#define SS_PIN 10

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Instance du lecteur MFRC522

// Utilisation de SoftwareSerial pour la communication avec l'ESP32 (sur d'autres pins que RX et TX par défaut)
SoftwareSerial espSerial(2, 3); // RX, TX

// Clé B pour chaque secteur (secteurs 0-15)
byte skeys[16][6] = {
  {0xF9, 0x82, 0xE9, 0x71, 0xCF, 0xED},  // Key B pour le secteur 0
  {0x1F, 0x42, 0xAB, 0x91, 0x59, 0xEE},  // Key B pour le secteur 1
  // {0xBB, 0xFB, 0x83, 0x6A, 0x48, 0xB8},  // Key B pour le secteur 2
  // {0xB5, 0xD1, 0x70, 0xB2, 0xE8, 0xF5},  // Key B pour le secteur 3
  // {0xE7, 0x69, 0x78, 0xA0, 0x5F, 0x10},  // Key B pour le secteur 4
  // {0x0B, 0x1A, 0x99, 0x5D, 0xD0, 0x07},  // Key B pour le secteur 5
  // {0x65, 0x0D, 0xB9, 0xCE, 0xDB, 0x6B},  // Key B pour le secteur 6
  // {0x13, 0xE5, 0x4B, 0x44, 0x48, 0xB7},  // Key B pour le secteur 7
  // {0x3E, 0x35, 0x40, 0xC2, 0xC2, 0x73},  // Key B pour le secteur 8
  // {0xA7, 0x61, 0x52, 0x84, 0x01, 0x17},  // Key B pour le secteur 9
  // {0x06, 0x6C, 0xCC, 0x76, 0x66, 0xBC},  // Key B pour le secteur 10
  // {0x3C, 0x0B, 0x3A, 0xC3, 0xAF, 0xA3},  // Key B pour le secteur 11
  // {0xCC, 0xB5, 0x41, 0x59, 0x8D, 0x72},  // Key B pour le secteur 12
  // {0x19, 0x88, 0xB5, 0xD4, 0x8E, 0xC3},  // Key B pour le secteur 13
  // {0x89, 0x2E, 0xEF, 0x0D, 0x30, 0xFB},  // Key B pour le secteur 14
  // {0x0F, 0xE5, 0xCE, 0x5C, 0xC6, 0x40},  // Key B pour le secteur 15
  // {0x10, 0xA1, 0xA2, 0xA3, 0xA4, 0x15},  // Key A pour tous les secteurs sauf le 1
  // {0xE9, 0xA5, 0x53, 0x10, 0x2e, 0xA5}   // Key A pour le secteur 1
}; 
// Les dernières clés ne servent pas pour lire les secteurs importants de la carte,
// (seuls les 2 premiers secteurs ne sont pas vides et il ne faut qu'une des 2 clés pour lire les données dessus).
// Simplement, si on veut copier la carte, il faut avoir toutes les clés (A et B) donc pratique de les connaîtr



void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  
  espSerial.begin(9600);   // Communication avec l'ESP32
}

void loop() {
  // Vérifie les commandes du Master Arduino
  if (Serial.available() > 0) {
    int command = Serial.read();

    switch (command) {
      case 10:
        readCarteJeune(); // Lecture de la carte jeune NFC
        break;
        break;
      case 20:
        scanWifi(); // Déclenche un scan WiFi
        break;
      case 30:
        rickroll(); // Envoie un signal à l’ESP32 pour lancer le rickroll
        break;
      case 40:
        startPortail(); // Démarre le portail captif
        break;
      case 50:
        stopPortail(); // Arrête le portail captif
        break;
    }
  }

  // Écoute les réponses de l'ESP32 pour les afficher
  if (espSerial.available()) {
    String espResponse = espSerial.readStringUntil('\n') + "\n"; // Lit jusqu'a la fin de la ligne
    writeString(espResponse); // Envoie la réponse de l'esp32 à l'arduino master
  }

}

// Fonction pour envoyer une chaîne de caractères au maître
void writeString(String stringData) {
  for (int i = 0; i < stringData.length(); i++) {
    Serial.write(stringData[i]);
  }
}

// Fonction pour lire la carte jeune NFC
void readCarteJeune() {
  for (int attempt = 1; attempt <= 2; attempt++) {
    while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
      delay(100);
    }

    MFRC522::MIFARE_Key key;
    memcpy(key.keyByte, key_B[attempt - 1], 6); // Utiliser la clé B pour le secteur actuel
    byte blockAddr = (attempt - 1) * 4;

    if (mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, blockAddr, &key, &(mfrc522.uid)) == MFRC522::STATUS_OK) {
      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      
      if (attempt == 1) {
        delay(2000);  // Pause entre les deux lectures de secteur
      }
    } else {
      String errorMsg = "Échec de l'authentification pour le secteur " + String(attempt - 1);
      writeString(errorMsg);  // Envoyer l'erreur au maître Arduino
      return;
    }
  }


  // Affichage de l'UID 
  String uidMsg = "Carte jeune lue avec succès. UID: ";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    uidMsg += String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : "") + String(mfrc522.uid.uidByte[i], HEX);
  }
  uidMsg += "\n";
  writeString(uidMsg);  // Envoyer au maître Arduino
}

// Fonction pour envoyer less instructions à l'esp32
void scanWifi() {
  espSerial.write('1');
}

void rickroll() {
  espSerial.write('2');
}

void startPortail() {
  espSerial.write('3');
}

void stopPortail() {
  espSerial.write('4');
}
