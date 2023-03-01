const int trigPin = 2;
const int echoPin = 3;
const int buzzerPin = 4;
const int threshold = 100; // 设置速度阈值，单位cm/s

long duration, distance, prevDistance = 2000;
unsigned long prevTime = 0, currTime = 0, deltaTime = 0;

void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  Serial.begin(9600);
}

void loop() {
  // 发送10微秒的高电平脉冲信号，启动距离测量
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // 使用pulseIn函数测量超声波信号回响时间，单位为微秒
  duration = pulseIn(echoPin, HIGH);

  // 计算距离，单位为厘米
  distance = duration / 58.0;

  // 判断当前检测到的距离是否为1200
  if (distance == 1200) {
    distance = prevDistance; // 如果是，将其赋为上一次检测到的距离
  } else {
    prevDistance = distance; // 否则，更新上一次检测到的距离
  }

  // 计算时间间隔
  currTime = millis();
  deltaTime = currTime - prevTime;
  prevTime = currTime;

  // 计算速度，单位为cm/s
  float speed = abs(distance - prevDistance) / (deltaTime / 1000.0);

  // 输出距离值和速度值到串口监视器
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm, ");
  Serial.print("Speed: ");
  Serial.print(speed);
  Serial.println(" cm/s");

  // 如果速度超过阈值，发出警报
  if (speed > threshold) {
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, LOW);
    delay(500);
  }
}
