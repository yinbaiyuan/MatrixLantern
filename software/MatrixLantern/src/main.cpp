#include <Arduino.h>
#include <U8g2lib.h>
#include <FastLED.h>
#include <Adafruit_GFX.h>
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>
// 包含所必需的库
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

char BLEbuf[256] = {0};
uint8_t funcEnable[7] = {1,1,1,1,1,1,0}; 
String data = "";

bool  m_freshData = false;
uint8_t m_colorCount = 0;
uint32_t m_colorArr[256];
uint8_t m_frameCount = 0;
uint8_t m_frameArr[100][514];

#define M_W         48
#define M_H         12
#define M_C         (M_W * M_H)
CRGB leds[M_C];
FastLED_NeoMatrix   *matrix;

// 定义收发服务的UUID（唯一标识）
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
// RX串口标识
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
// TX串口标识
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

int8_t r = 0;
int8_t g = 0;
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        Serial.println(rxValue.length());
        Serial.println();


        long dataPointer = 0;
        for(int i = 0;i<7;i++)
        {
          funcEnable[i] = rxValue[dataPointer++];
          Serial.println( funcEnable[i]);
        }
        if(funcEnable[6] == 1)
        {
           m_colorCount = rxValue[dataPointer++];
           Serial.print("colorCount:");
           Serial.println(m_colorCount);
           for (int i = 0; i < m_colorCount; i++)
           {
             uint8_t r = rxValue[dataPointer++];
             uint8_t g = rxValue[dataPointer++];
             uint8_t b = rxValue[dataPointer++];
             m_colorArr[i] = uint32_t(r<<16) + uint32_t(g<<8) + b;
            //  Serial.println(String(r) + " " + String(g) + " " +String(b));
           }
           m_frameCount = rxValue[dataPointer++];
           
           Serial.print("m_frameCount:");
           Serial.println(m_frameCount);
           for(int i = 0;i<m_frameCount;i++)
           {
             m_frameArr[i][0] = rxValue[dataPointer++];
             m_frameArr[i][1] = rxValue[dataPointer++];
             uint16_t frameDataLength = uint16_t(m_frameArr[i][0]<<8) + uint16_t(m_frameArr[i][1]);
             
            Serial.print("frameDataLength:");
            Serial.println(frameDataLength);
             for(int j = 0;j<frameDataLength;j++)
             {
               m_frameArr[i][j+2] = rxValue[dataPointer++];
              //  Serial.println("d:" + String(m_frameArr[i][j+2]));
             }
           }
        }
        m_freshData = true;
        
        // data =rxValue.c_str();
        // r = random(200)+50;
        // g = random(200)+50;
        //Serial.println(data);
        Serial.println("*********");
        Serial.println();
      }
    }
};

void matrixLoop(void *pvParameters)
{
  while (1)
  {
    newBegin:
    m_freshData = false;
    Serial.println(funcEnable[6]);
    if(funcEnable[0] == 1)
    {
      for(int times = 0;times<5;times++)
      {
        for(int a = 0;a<M_W;a++)
        {
          matrix->clear(); 
          for(int x = 0;x<M_W;x++)
          {
            int hue = 255 * x / M_W;
            for(int y = 0;y<M_H;y++)
            {
              CHSV hsv(hue,255,255);
              CRGB rgb;
              hsv2rgb_rainbow(hsv, rgb);
              matrix->drawPixel((a+x+y)%M_W,y,rgb.r,rgb.g,rgb.b);
              if(m_freshData){
                  goto newBegin;
              }
            }
          }
          matrix->show(); 
          delay(50);  
        }
      }
    }
    #define COOLING  55
    #define SPARKING 120
    if(funcEnable[1] == 1)
    {
      for(int c = 0 ;c<30*10;c++)
      {
        static byte heat[M_W][M_H];
        for(int w = 0;w<M_W;w++)
        {
          for(int h = 0;h<M_H;h++)
          {
            heat[w][h] = qsub8( heat[w][h],  random8(0, ((COOLING * 10) / M_H) + 2));
          }
          for( int k= M_H - 1; k >= 2; k--) {
            heat[w][k] = (heat[w][k - 1] + heat[w][k - 2] + heat[w][k - 2] ) / 3;
          }
          if( random8() < SPARKING ) {
            int y = random8(7);
            heat[w][y] = qadd8( heat[w][y], random8(160,255) );
          }
          for( int j = 0; j < M_H; j++) {
            CRGB color = HeatColor( heat[w][j]);
            matrix->drawPixel(w,M_H-1-j,color.r,color.g,color.b);
          }
        }
        matrix->show();  // display this frame
        delay(30);
        if(m_freshData){
          goto newBegin;
        }
      }
    }
    if(funcEnable[2] == 1)
    {
      FastLED.clear();
      uint16_t showArr[M_C];
      for(int i = 0;i<M_C;i++)
      {
        showArr[i] = i;
      }
      uint16_t randRange = M_C;
      for (int i = 0; i < M_C; i++)
      {
        uint16_t l = random(randRange);
        uint16_t c = showArr[l];
        showArr[l] = showArr[--randRange];
        uint8_t h = random(256);
        uint8_t v = random(100,256);
        leds[c] = CHSV(h,255,v);
        FastLED.show();
        delay(15000/M_C);
        if(m_freshData){
          goto newBegin;
        }
      }
    }
    if(funcEnable[3] == 1)
    {
      for (int times = 0; times < 5; times++)
      {
        FastLED.clear();
        for(int y = 0;y<M_H;y++)
        {
          uint8_t h = random(256);
          for (int dy = 0; dy < M_H-y; dy++)
          {
            for(int x = 0;x<M_W;x++)
            {
              CHSV hsv(h,255,255);
              CRGB rgb;
              hsv2rgb_rainbow(hsv, rgb);
              matrix->drawPixel(x,dy,rgb.r,rgb.g,rgb.b);
              if(dy>=1)
              {
                matrix->drawPixel(x,dy-1,0);
              }
              if(m_freshData){
                goto newBegin;
              }
            }
            FastLED.show();
            delay(50);
          }
        }
        FastLED.clear();
        for(int x = 0;x<M_W / 4;x++)
        {
          uint8_t h = random(256);
          for (int dx = 0; dx < (M_W / 4)-x; dx++)
          {
            for(int y = 0;y<M_H;y++)
            {
              CHSV hsv(h,255,255);
              CRGB rgb;
              hsv2rgb_rainbow(hsv, rgb);
              for(int s = 0;s<4;s++)
              {
                matrix->drawPixel(dx+(M_W/4)*s,y,rgb.r,rgb.g,rgb.b);
                if(dx>=1)
                {
                  matrix->drawPixel((dx-1)+(M_W/4)*s,y,0);
                }
              }
              if(m_freshData){
                goto newBegin;
              }
            }
            FastLED.show();
            delay(50);
          }
        }
      }
    }
    if(funcEnable[4] == 1)
    {
      uint8_t nqct[8][12] = {{0x04,0x00,0x24,0x00,0x24,0x00,0x3F,0xC0,0x44,0x00,0x84,0x00},
                            {0x04,0x00,0xFF,0xE0,0x04,0x00,0x04,0x00,0x04,0x00,0x04,0x00},/*"牛",0*/

                            {0x20,0x00,0x3F,0xE0,0x40,0x00,0xBF,0xC0,0x00,0x00,0x7F,0x80},
                            {0x00,0x80,0x00,0x80,0x00,0x80,0x00,0xA0,0x00,0x60,0x00,0x20},/*"气",1*/

                            {0x01,0x00,0x81,0x00,0x4F,0xE0,0x09,0x20,0x09,0x20,0x09,0x20},
                            {0x09,0x20,0x2F,0xE0,0x49,0x20,0x81,0x00,0x01,0x00,0x01,0x00},/*"冲",2*/

                            {0x00,0x00,0x7F,0xC0,0x04,0x00,0x04,0x00,0x04,0x00,0xFF,0xE0},
                            {0x04,0x00,0x0A,0x00,0x0A,0x00,0x11,0x00,0x20,0x80,0xC0,0x60}/*"天",3*/};
      for (int times = 0; times < 5; times++)
      {
        for(int b = M_W-1;b>=0;b--)
        {
          matrix->clear();
          for(int z = 0;z<4;z++)
          {
            for(int h = 0;h<12;h++)
            {
              for(int s = 0;s<12;s++)
              {
                int x = b + z * 12 + h;
                x %= 48;
                int y = s;
                uint8_t c = nqct[z*2+s/6][s%6*2+h/8];
                if(c>>(7-h%8) & 0x01)
                {
                matrix->drawPixel(x,y,255,0,0);
                }
                if(m_freshData){
                  goto newBegin;
                }
              }
            }
          }
          matrix->show();
          delay(100);
        }
      }
    }
    if(funcEnable[5] == 1)
    {
      for(int c = 0;c<10000/50;c++)
      {
        FastLED.clear();
        for(int x = 0;x<M_W;x++)
        {
            for(int y = 0;y<M_H;y++)
            {
              leds[y*M_W+x] = CHSV(random(256),255,random(100,256));
              if(m_freshData){
                goto newBegin;
              }
            }
        }
        FastLED.show();
        delay(50);
        }
    }
    if(funcEnable[6] == 1)
    {
      for (int times = 0; times < 10; times++)
      {
        for (int f = 0; f < m_frameCount; f++)
        {
          matrix->clear();
          uint16_t d = uint16_t(m_frameArr[f][0]<<8) + uint16_t(m_frameArr[f][1]);
          for (int dc = 0; dc < d/2; dc++)
          {
            uint8_t p = m_frameArr[f][2+dc*2];
            uint8_t ci = m_frameArr[f][2+dc*2+1];
            uint8_t x = p & 0x0f;
            uint8_t y = p>>4 & 0x0f;
            uint32_t color = m_colorArr[ci];
            if(x<12&&y<12)
            {
              for(int s = 0 ;s<3;s++)
              {
                matrix->drawPixel(x+s*16,y,color>>16&0xFF,color>>8&0xFF,color>>0&0xFF);
              }
            } 
            if(m_freshData){
              goto newBegin;
            }
          }
          matrix->show();
          delay(100);
        }
      }
      
    }
    // Serial.println("matrixLoop");
  }
  
  vTaskDelete(NULL);
}

// setup()在复位或上电后运行一次:
void setup() {
  Serial.begin(115200);
  // 初始化蓝牙设备
  BLEDevice::init("点阵灯笼");
  // 为蓝牙设备创建服务器
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // 基于SERVICE_UUID来创建一个服务
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pTxCharacteristic = pService->createCharacteristic(
                                        CHARACTERISTIC_UUID_TX,
                                        BLECharacteristic::PROPERTY_NOTIFY
                                    );
  pTxCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
                                             CHARACTERISTIC_UUID_RX,
                                            BLECharacteristic::PROPERTY_WRITE
                                        );
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  // 开启服务
  pService->start();
  // 开启通知
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
  Serial.println();


  matrix = new FastLED_NeoMatrix(leds,M_W,M_H,NEO_MATRIX_TOP +NEO_MATRIX_LEFT + NEO_MATRIX_COLUMNS);
  FastLED.addLeds<NEOPIXEL,14>(leds,M_C);
  matrix->begin();
  matrix->setTextWrap(false);
  matrix->setBrightness(80);

  xTaskCreatePinnedToCore(matrixLoop,"matrixLoop",8192,NULL,1,NULL,1);

}



// loop()一直循环执行:
void loop() {

    if (deviceConnected && data.length()>0) {
        memset(BLEbuf, 0, 32);
        memcpy(BLEbuf, data.c_str(), 32);
        Serial.println(BLEbuf);

        pTxCharacteristic->setValue(BLEbuf);   //收到数据后返回数据
        pTxCharacteristic->notify();
        data = "";  //返回数据后进行清空，否则一直发送data
    }

    // 没有新连接时
    if (!deviceConnected && oldDeviceConnected) {
        // 给蓝牙堆栈准备数据的时间
        delay(500);
        pServer->startAdvertising();
        // 重新开始广播
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // 正在连接时
    if (deviceConnected && !oldDeviceConnected) {
        // 正在连接时进行的操作
        oldDeviceConnected = deviceConnected;
    }

    // Serial.println("loop");

    

}