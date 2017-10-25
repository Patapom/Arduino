 #include <ELECHOUSE_CC1101.h>

#define size 61

byte TX_buffer[size]={0};
byte i;

void setup()
{
  Serial.begin(9600);
  ELECHOUSE_cc1101.Init();
  for(i=0;i<size;i++)
  {
     TX_buffer[i]=i;
  }
}

void loop()
{
  ELECHOUSE_cc1101.SendData(TX_buffer,size);
  delay(1);
}
