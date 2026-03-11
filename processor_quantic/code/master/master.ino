void setup() {
  Serial.begin(9600);     // Debug to PC
  Serial1.begin(115200);      // Communication to slave

  Serial.println("MASTER STARTED");
}

void sendCmd(String cmd)
{
  Serial.print("[MASTER] send: ");
  Serial.println(cmd);

  Serial1.println(cmd);   // send command
}

void loop() {
  sendCmd("test");
  delay(5000);

  sendCmd("open");
  delay(5000);
  sendCmd("close");
  delay(5000);

  sendCmd("msg:1");
  delay(2000);
  sendCmd("msg:2");
  delay(2000);
  sendCmd("msg:3");
  delay(2000);
  sendCmd("msg:4");
  delay(2000);

  
  sendCmd("col:1");
  delay(2000);
  sendCmd("col:2");
  delay(2000);
  sendCmd("col:3");
  delay(2000);
  sendCmd("col:4");
  delay(2000);

  sendCmd("seq_led:1");
  delay(10000);
  sendCmd("seq_led:2");
  delay(10000);
  sendCmd("seq_led:3");
  delay(10000);
  sendCmd("seq_led:0");
  delay(10000);

  sendCmd("buzz:1");
  delay(2000);
  sendCmd("buzz:2");
  delay(2000);
  sendCmd("buzz:3");
  delay(2000);
  sendCmd("buzz:4");
  delay(2000);
  sendCmd("buzz:5");
  delay(2000);
  sendCmd("buzz:6");
  delay(2000);
  sendCmd("buzz:7");
  delay(2000);
  sendCmd("buzz:8");
  delay(2000);
}