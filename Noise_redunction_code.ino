#include <VoiceRecognitionV3.h>
#include <arduinoFFT.h>

// 定义降噪算法所需变量
double alpha = 0.75;
double* vReal;
double* vImag;
arduinoFFT FFT = arduinoFFT();
const int sampleRate = 10000;
const int sampleWindow = 512;
const int noiseThreshold = 150;

// 定义VoiceRecognitionV3对象
VR myVR(8, 9);  // RX, TX

// 定义词汇
uint8_t hello[7] = {0x01,0x02,0x03,0x04,0x05,0x06,0x07};

void setup()
{
  Serial.begin(9600);
  
  // 初始化降噪算法所需数组
  vReal = new double[sampleWindow];
  vImag = new double[sampleWindow];
  
  // 初始化VoiceRecognitionV3
  myVR.begin(9600);
  delay(1000);

  // 添加词汇到VoiceRecognitionV3
  myVR.addCommand(hello, 7);
  delay(1000);
}

void loop()
{
  int recog = myVR.recognize();  // 获取语音识别结果

  if (recog == 0) {
    Serial.println("未识别到语音指令");
    return;
  } else if (recog == 1) {
    Serial.println("已识别到语音指令 hello");
  }

  // 降噪处理
  double sum = 0;
  for (int i = 0; i < sampleWindow; i++) {
    int sample = myVR.getSample();  // 获取语音模块采集到的原始音频数据
    vReal[i] = sample;
    vImag[i] = 0.0;
    sum += vReal[i];
  }
  double average = sum / sampleWindow;
  for (int i = 0; i < sampleWindow; i++) {
    vReal[i] = vReal[i] - average;
  }
  FFT.Windowing(vReal, sampleWindow, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, sampleWindow, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, sampleWindow);
  int noiseCount = 0;
  for (int i = 0; i < (sampleWindow / 2); i++) {
    double magnitude = vReal[i];
    if (magnitude < noiseThreshold) {
      noiseCount++;
    }
  }
  if (noiseCount > 10) {
    Serial.println("检测到噪声");
    return;
  }
  
  // 处理语音指令
  switch (recog) {
    case 1:
      // 处理 hello 指令
      break;
    default:
      // 处理其他指令
      break;
  }
}
