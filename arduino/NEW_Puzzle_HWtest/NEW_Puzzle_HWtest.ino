/*=================================================================
Ce sketch est un scanner I2C: il essaye de communiquer avec toutes 
les adresses I2C possibles et affiche celle(s) qui réponde(nt).
  
                      BRANCHEMENT
* Pin SCD du moule à scanner  ----------->  SCD de l’Arduino
* Pin SDA du moule à scanner  ----------->  SDA de l’Arduino
================================================================ */


#include <Wire.h>
int led = 16;
int pb = 13;

void setup()
{
  pinMode(led, OUTPUT);
  digitalWrite(led,HIGH);
  pinMode(pb, INPUT_PULLUP);
  delay(300);
  Wire.begin(4,5);
  Serial.begin(115200);
  Serial.println("\nI2C Scanner");
}

void loop()
{
  byte error, address;
  int nDevices;
  Serial.println("Recherche en cours...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) 
  {
    Wire.beginTransmission(address);
    //Wire.write("R");
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("Equiment I2C trouve a l'addresse 0x");
      if (address<16) 
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Erreur inconnue a l'address 0x");
      if (address<16) 
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("Aucun Equipement I2C trouve\n");
  else
    Serial.println("Fini\n");

  digitalWrite(led, !digitalRead(led));
  Serial.println("PushButton State : " + String(digitalRead(pb)));
  delay(5000);           
}
