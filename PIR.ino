// 定义PIR Sensor连接的数字引脚
int pirPin = A0;

void setup() {
  // 初始化串行监视器
  Serial.begin(115200);

  // 设置PIR Sensor引脚为输入
  pinMode(pirPin, INPUT);
}

void loop() {
  // 读取PIR Sensor的数字输出
  int pirState = digitalRead(pirPin);

  // 如果检测到运动，则输出一条消息
  if (pirState == HIGH) {
    Serial.println("Motion detected!");
  }

  // 延迟100毫秒以避免频繁输出消息
  delay(100);
}
