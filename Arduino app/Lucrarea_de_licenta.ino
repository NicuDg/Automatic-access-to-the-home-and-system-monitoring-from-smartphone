#include <SPI.h>
#include <Ethernet.h>
#include <Servo.h>
#include <MFRC522.h>

#define SS_PIN 7                    //SDA
#define RST_PIN 6                   //RST
MFRC522 mfrc522(SS_PIN, RST_PIN);   // creare instanta MFRC522
int rfid_card = 0;                  //variabila pentru citirea cardului
int obstacol_garaj_pin = 3;         // pinul de la senzor IR pentru garaj
int isObstacle = HIGH;              // HIGH inseamna ca nu este obstacol
int obstacol_bariera_pin = 2;       // pinul de la senzor IR pentru bariera
int isObstacleBariera = HIGH;
int led = A3;                        //lumini
int det_ob_garaj = 0;               //initial nu e detectat niciun obiect
int det_ob_bariera = 0;
int pos_servo_garaj = 0;            //initial servomotorul se afla la 0 grade
int pos_servo_usa = 0;
int pos_servo_bariera = 0;
Servo myservo_garaj;
Servo myservo_usa;
Servo myservo_bariera;
int phtr = A0;
int detectie_masina = A1;
int PIRsensor = A2;              // the pin that the sensor is atteched to
int detectiePIR = 0;
int acces_casa = 0;
int lumini_buton = 0;

String buffer;      // stores the HTTP request
int val = 0;

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};   //mac address
byte ip[] = {
  192, 168, 1, (unsigned char)102

};                      // Lan IP
byte gateway[] = {
  192, 168, 1, 1
};                   // gateway IP
byte subnet[] = {
  255, 255, 255, 0
};
EthernetServer server(80);
EthernetClient ethernet_shield;       // server iar clientul este androidu-l

void setup() {
  Serial.begin(9600);
  pinMode(obstacol_garaj_pin, INPUT); //initializare pin ca intrare
  pinMode(obstacol_bariera_pin, INPUT);
  pinMode(phtr, INPUT);
  pinMode(detectie_masina, INPUT);
  pinMode(PIRsensor, INPUT);
  pinMode(led, OUTPUT);
  pinMode(A4, OUTPUT); //initializare pin ca iesire

  myservo_garaj.attach(9);
  myservo_bariera.attach(5);
  myservo_garaj.write(0);
  myservo_bariera.write(0);
  myservo_usa.attach(8);
  myservo_usa.write(0);

  SPI.begin();      // Initializare  SPI bus
  mfrc522.PCD_Init();   // Initializare MFRC522
  Serial.println("Apropie cardul de cititor...");
  Serial.println();

  Ethernet.begin(mac, ip, gateway, subnet);   //  //initializeaza setarile de internet
  server.begin();  // incepe ascultarea clientului
  Serial.print("server is at ");  // afiseaza ip-ul unde ruleaza serveru
  Serial.println(Ethernet.localIP());  //ip server

}

void rfid() {
  if ( ! mfrc522.PICC_IsNewCardPresent()) //verifica daca nu este prezent cardul
  {
    if ( ! mfrc522.PICC_IsNewCardPresent())
    { rfid_card = 0;
      return;
    }
  }
  // selecteaza unul din carduri
  if ( ! mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  //arata ID-ul
  Serial.print("UID tag :");
  String content = "";
  byte letter;
  for (byte i = 0; i < mfrc522.uid.size; i++)
  {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX)); //se concateneaza un sir cu valorile de pe card
  }
  Serial.println();
  Serial.print("Mesaj : ");
  content.toUpperCase();
  if (content.substring(1) == "B0 70 80 A6")//daca continutul cardului este egal cu ID autorizat
  {
    rfid_card = 1;
    Serial.println("Acces autorizat");
    Serial.println();
    delay(500);
  }

  else   {
    Serial.println("Acces neautorizat");
    delay(2000);
  }
}


void intrare() {
  deschide_bariera();
  //se verifica daca a trecut masina
  while (det_ob_bariera != 1)
  {
    isObstacleBariera = digitalRead(obstacol_bariera_pin);
    if (isObstacleBariera == LOW)
    {
      det_ob_bariera = 1;
    }
  }
  delay(1000);

  inchide_bariera();

  //citire card
  while (rfid_card != 1) {
    rfid();
  }
  //se asteapta detectarea de catre senzorul PIR
  while (detectiePIR != 1) {
    semnalPIR();
  }
  if (detectiePIR == 1) {
    rfid();
  }

  //conditia pentru intrarea in garaj
  if (rfid_card == 0 && detectiePIR == 1)
  {
    deschide_poarta();
    while (det_ob_garaj != 1)
    {
      isObstacle = digitalRead(obstacol_garaj_pin);
      if (isObstacle == LOW)
      {
        det_ob_garaj = 1;
      }
    }
    delay(3000);
    Serial.println("masina este in garaj");
  }


  //inchidere poarta garaj
  if (det_ob_garaj == 1)
  {
    analogWrite(led, 1000);
    inchide_poarta();
    delay(1000);
    analogWrite(led, 0);
  }

  //conditia pentru accesul in casa
  if (rfid_card == 1 && detectiePIR == 1) {
    deschide_usa();
    delay(2000);
    inchide_usa();
    acces_casa = 1;
  }
  detectiePIR = 0;
}

void iesire() {
  //deschidere poarta garaj
  deschide_poarta();
  //asteapta sa iasa masina din garaj
  while (det_ob_garaj != 0)
  {
    isObstacle = digitalRead(obstacol_garaj_pin);
    if (isObstacle == LOW)
    {
      det_ob_garaj = 0;
    }
  }
  delay(100);
  //inchide poarta
  inchide_poarta();
  //asteapta citirea ID-ului pentru deschiderea barierei
  while (rfid_card != 1) {
    rfid();
  }

  if ( rfid_card == 1) {
    deschide_bariera();
  }
  analogWrite(led, 0);
  //detecteaza trecerea de bariera
  while (det_ob_bariera != 0)
  {
    isObstacleBariera = digitalRead(obstacol_bariera_pin);
    if (isObstacleBariera == LOW)
    {
      det_ob_bariera = 0;
    }
  }
  delay(100);
  inchide_bariera();
}

void loop() {
  //daca a intrat in casa, pentru a deschide bariera pentru iesire se verifica miscarea masinii
  if (acces_casa == 1) {
    rfid();
    iesire_curte();
  }
  //se verifica prin functia lumina daca este intuneric pentru a aprinde ledurile
  lumina();
  //conditia pentru iesirea din garaj
  if (analogRead(detectie_masina) > 500 && det_ob_garaj == 1) {
    iesire();
  }

  ethernet_shield = server.available();  // serverul asteapta client
  if (ethernet_shield) { //cand clientul se conecteaza
    boolean currentLineIsBlank = true;
    while (ethernet_shield.connected()) {
      if (ethernet_shield.available()) {  // datele clientului sunt gata de citire
        char c = ethernet_shield.read(); //citeste caracter cu caracter datele de la client
        //citeste datele de la client
        if (buffer.length() < 100) {
          buffer += c; //salveaza cererea HTTP cate un caracter pe o perioada de timp
          // ultima linie a cererii de catre client este goala si se termina cu \n
          // raspunde clientului numai dupa ce a primit ultima linie
        }

        if (c == '\n'  && currentLineIsBlank) { //verifica daca a primit toate datele de la client
          Serial.println(buffer);
          ethernet_shield.println("HTTP/1.1 200 OK");
          ethernet_shield.println("Content-Type: text/html");
          ethernet_shield.println("Connection: close");
          ethernet_shield.println();

          stare(ethernet_shield);

          // semnal de la smartphone
          //comanda pentru intrare in curte
          if (buffer.indexOf("?cmd=9") > 0 && det_ob_garaj == 0 && acces_casa == 0) {
            intrare();
            buffer = "0";
          }

          //comanda pentru a aprinde ledurile
          if (buffer.indexOf("?cmd=1") > 0 ) {
            if (analogRead(led) < 100) {
              analogWrite(led, 1000);
              if (lumini_buton == 0) {
                lumini_buton = 1;
              }
              else {
                lumini_buton = 0;
              }
            }
          }

          //comanda pentru a stinge ledurile
          if (buffer.indexOf("?cmd=2") > 0 ) {
            if (analogRead(led) > 100) {
              analogWrite(led, 0);
              if (lumini_buton == 1) {
                lumini_buton = 0;
              }
              else {
                lumini_buton = 1;
              }
            }
          }

          //comanda pentru a deschide bariera
          if (buffer.indexOf("?cmd=3") > 0) {
            deschide_bariera();
          }
          //comanda pentu a inchide bariera
          if (buffer.indexOf("?cmd=4") > 0) {
            inchide_bariera_buton();
          }
          //comanda pentru a deschide poarta de la garaj
          if (buffer.indexOf("?cmd=5") > 0) {
            deschide_poarta();
          }
          //comanda pentru a inchide poarta de la garaj
          if (buffer.indexOf("?cmd=6") > 0) {
            inchide_poarta_buton();
          }
          //comanda pentru a deschide usa
          if (buffer.indexOf("?cmd=7") > 0) {
            deschide_usa();
          }
          //comanda pentru a inchide usa
          if (buffer.indexOf("?cmd=8") > 0) {
            inchide_usa();
          }


          //se sterg datele pentru urmatoarea citire
          Serial.print(buffer);
          buffer = "";
          break;
        }
        if (c == '\n') {
          // ultimul caracter pe linia de text primita 
          // se incepe o linie noua cu citirea urmatorului caracter
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // un caracter text a fost primit de la client 
          currentLineIsBlank = false;
        }

      }// end if (client.available())
    }// end while (client.connected())
    delay(1); 
    ethernet_shield.stop(); //inchide conexiunea
  }
}

void stare(EthernetClient client) {
  if (analogRead(led) > 100) {
    client.println("{\r\n\"status_1\":\"" + String("on") + "\",\r\n");
  }
  else
    client.println("{\r\n\"status_1\":\"" + String("off") + "\",\r\n");

  if (pos_servo_bariera < 3) {
    client.println("\r\n\"status_2\":\"" + String("closed") + "\",\r\n");
  }
  else
    client.println("\r\n\"status_2\":\"" + String("opened") + "\",\r\n");
  if (pos_servo_garaj < 3) {
    client.println("\r\n\"status_3\":\"" + String("closed") + "\",\r\n");
  }
  else
    client.println("\r\n\"status_3\":\"" + String("opened") + "\",\r\n");
  if (pos_servo_usa < 3) {
    client.println("\r\n\"status_4\":\"" + String("closed") + "\",\r\n");
  }
  else
    client.println("\r\n\"status_4\":\"" + String("opened") + "\",\r\n");
  if (analogRead(detectie_masina) && det_ob_garaj == 1) {
    client.println("\r\n\"status_5\":\"" + String("car is inside") + "\"\r\n}");
  }
  else
    client.println("\r\n\"status_5\":\"" + String("empty") + "\"\r\n}");

}

void deschide_bariera() {
  if (pos_servo_bariera < 3)
  {
    for (pos_servo_bariera = 0; pos_servo_bariera <= 90; pos_servo_bariera += 1) // goes from 0 degrees to 180 degrees
    { 
      myservo_bariera.write(pos_servo_bariera);              // tell servo to go to pos_servo_garajition in variable 'pos_servo_garaj'
      delay(15);                       // waits 15ms for the servo to reach the position
    }
  }
}

void inchide_bariera() {
  if (pos_servo_bariera > 85)
  {
    for (pos_servo_bariera = 90; pos_servo_bariera >= 0; pos_servo_bariera -= 1) // goes from 180 degrees to 0 degrees
    {
      isObstacleBariera = digitalRead(obstacol_bariera_pin);

      if (isObstacleBariera == LOW)
      {
        delay(2000);
      }
      else
      {
        myservo_bariera.write(pos_servo_bariera);        
        delay(15);                       
        //}
      }
    }
  }
}

void inchide_bariera_buton() {
  if (pos_servo_bariera > 85)
  {
    for (pos_servo_bariera = 90; pos_servo_bariera >= 0; pos_servo_bariera -= 1) //se roteste de la 90 la 0 grade
    {
      myservo_bariera.write(pos_servo_bariera);             
      delay(15);                      
    }
  }
}

void deschide_poarta() {
  if (pos_servo_garaj < 3) {
    for (pos_servo_garaj = 0; pos_servo_garaj <= 180; pos_servo_garaj += 1) // se roteste de la 0 la 180 grade
    { 
      myservo_garaj.write(pos_servo_garaj);              
      delay(15);                       
    }
  }
}

void inchide_poarta() {
  if (pos_servo_garaj > 175) {
    for (pos_servo_garaj = 180; pos_servo_garaj >= 0; pos_servo_garaj -= 1) 
    {
      isObstacle = digitalRead(obstacol_garaj_pin);
      if (isObstacle == LOW)
      {
        Serial.println("OBSTACLE!!");
        delay(3000);
      }
      else
      {
        myservo_garaj.write(pos_servo_garaj);             
        delay(15);                       
      }
    }
  }
}

void inchide_poarta_buton() {
  if (pos_servo_garaj > 175) {

    for (pos_servo_garaj = 180; pos_servo_garaj >= 0; pos_servo_garaj -= 1) 
    {
      myservo_garaj.write(pos_servo_garaj);              
      delay(15);                      
    }
  }
}

void deschide_usa() {
  if (pos_servo_usa < 3)
  {
    for (pos_servo_usa = 0; pos_servo_usa <= 90; pos_servo_usa += 1) 
    { // in steps of 1 degree
      myservo_usa.write(pos_servo_usa);           
      delay(15);                      
    }
  }
}

void inchide_usa() {
  if (pos_servo_usa > 85)
  {
    for (pos_servo_usa = 90; pos_servo_usa >= 0; pos_servo_usa -= 1) 
    {
      myservo_usa.write(pos_servo_usa);              
      delay(15);                      
    }
  }
}
//functia pentru detectarea miscarii de catre senzorul PIR
void semnalPIR() {
  if (analogRead(PIRsensor) > 200 ) {
    delay(100);
    detectiePIR = 1;
  }
}

void lumina() {
  if (lumini_buton == 0) {
    if (analogRead(phtr) > 80) {
      analogWrite(led, 0); //stinge led
    }
    else {
      analogWrite(led, 1000); //aprinde led
    }
  }
}

void iesire_curte() {
  if (acces_casa == 1 && rfid_card == 0) {
    deschide_bariera();
    while (det_ob_bariera != 0)
    {
      isObstacleBariera = digitalRead(obstacol_bariera_pin);
      if (isObstacleBariera == LOW)
      {
        det_ob_bariera = 0;
      }
    }
    delay(100);
    inchide_bariera();
    acces_casa = 0;
  }
}
