#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include <Adafruit_Fingerprint.h>


#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3);
#else
#define mySerial Serial1
#endif


Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);


#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above
#define buzzerPin       A1

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
int state1 = 0,state2 = 0,state3 = 0,state4 = 0;
int n = 0,Student_ID=0;//n is for the total number of students//j is for to detect the card is valid or not



void setup()
{
    Serial.begin(9600);
    while (!Serial);  
    delay(100);
    Serial.println("\n\nAdafruit finger detect test");
    SPI.begin();      // Init SPI bus
    mfrc522.PCD_Init();   // Init MFRC522
    delay(4);       // Optional delay. Some board do need more time after init to be ready, see Readme
    pinMode(buzzerPin, OUTPUT);
    // set the data rate for the sensor serial port
    finger.begin(57600);
    delay(5);
    if (finger.verifyPassword()) 
    {
      Serial.println("Found fingerprint sensor!");
    } 
    else 
    {
      Serial.println("Did not find fingerprint sensor :(");
      while (1) 
      { 
        delay(1); 
      }
    }
    
    Serial.println(F("Reading sensor parameters"));
    finger.getParameters();
    Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
    Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
    Serial.print(F("Security level: ")); Serial.println(finger.security_level);
    Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
    Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
    Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
  
    finger.getTemplateCount();
  
    if (finger.templateCount == 0) 
    {
      Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    }
    else 
    {
      Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
    }    
    Serial.println("\t\t\t<<<< FingerPrint And RFID Based Student Attendance >>>>\n"); 
}

void loop() 
{
  getFingerprintID();
  delay(2000);            //don't ned to run this at full speed.
}


uint8_t getFingerprintID() 
{
  uint8_t p = finger.getImage();
  switch (p) 
  {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) 
  {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) 
  {
    Serial.println("Found a print match!");
    Student_ID=finger.fingerID;
    Serial.print("Found ID #"); Serial.print(finger.fingerID);
    Serial.print(" with confidence of "); Serial.println(finger.confidence);
    delay(5000);
    RFID_Read();
  } 
  else if (p == FINGERPRINT_PACKETRECIEVEERR) 
  {
    Serial.println("Communication error");
    return p;
  } 
  else if (p == FINGERPRINT_NOTFOUND) 
  {
    Serial.println("Did not find a match");
    return p;
  } 
  else 
  {
    Serial.println("Unknown error");
    return p;
  }
  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez()
{
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  
  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  
  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  
  return -1;
  return finger.fingerID;
}

void RFID_Read()
{
  int cnt=20,i;
  Serial.print("\n");
  Serial.print("Swipe the card");
  delay(1000);
   if ( ! mfrc522.PICC_IsNewCardPresent())
  {
    return;
  }
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //Show UID on serial monitor
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  content.toUpperCase();

  if(Student_ID==1)
  {
    if (content.substring(1) == "0A 40 70 1E" && state1 == 0) //change here the UID of the card/cards that you want to give access
    {
      beepON();
      Serial.print("\n");
      Serial.print("1   XYZ     Present");
    }
    else
    {
      beepON();
      Serial.print("\n");
      Serial.print("UID Not Matches");
    }
  }
  else if(Student_ID==2)
  {
      if (content.substring(1) == "1A E9 E7 1E" && state2 == 0) //change here the UID of the card/cards that you want to give access
      {
        beepON();
        Serial.print("\n");
        Serial.print("2   MNK     Present");
      }
      else
      {
        beepON();
        Serial.print("\n");
        Serial.print("UID Not Matches");
      }
  }
  else if(Student_ID==3)
  {
      if (content.substring(1) == "0A 44 AB 1E" && state3 == 0) //change here the UID of the card/cards that you want to give access
      {
        beepON();
        Serial.print("\n");
        Serial.print("3   PQR     Present");
      }
      else
      {
        beepON();
        Serial.print("\n");
        Serial.print("UID Not Matches");
      }
  }

  else if(Student_ID==4)
  {
    if (content.substring(1) == "1A 00 E6 1E" && state4 == 0) //change here the UID of the card/cards that you want to give access
    {
      beepON();
      Serial.print("\n");
      Serial.print("4   ABC     Present");
    }
    else
    {
      beepON();
      Serial.print("\n");
      Serial.print("UID Not Matches");
    }
  }
  else  
  {   
    digitalWrite(buzzerPin, HIGH);
    Serial.print("ID : ");
    Serial.print("Unknown");
    Serial.println(" Access denied");
    delay(1500);
    digitalWrite(buzzerPin, LOW);
  }
}

void beepON()
{
  digitalWrite(buzzerPin, HIGH);
  delay(200);
  digitalWrite(buzzerPin, LOW);
  delay(100);
}


