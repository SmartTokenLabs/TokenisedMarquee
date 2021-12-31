// Use the MD_MAX72XX library to scroll text on the display
// received through the ESP8266 WiFi interface.
//
// Demonstrates the use of the callback function to control what
// is scrolled on the display text. User can enter text through
// a web browser and this will display as a scrolling message on
// the display.
//
// IP address for the ESP8266 is displayed on the scrolling display
// after startup initialization and connected to the WiFi network.
//
// Connections for ESP8266 hardware SPI are:
// Vcc       3v3     LED matrices seem to work at 3.3V
// GND       GND     GND
// DIN        D7     HSPID or HMOSI
// CS or LD   D8     HSPICS or HCS
// CLK        D5     CLK or HCLK
//
#define DEBUG 1

#if DEBUG
#define PRINT(s, v) { Serial.print(F(s)); Serial.print(v); }
#define PRINTS(s)   { Serial.print(F(s)); }
#else
#define PRINT(s, v)
#define PRINTS(s)
#endif

#include <Arduino.h>
#include <Web3.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WifiServer.h>
#include "esp_wifi.h"
#include <TcpBridge.h>
#include <EEPROM.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include <string>
#include "TokenCache.h"

const char *ssid = "<YOUR WiFi SSID>";
const char *password = "<YOUR WiFi Password>";

const char *ssid2 = "<Second WiFi SSID>";
const char *password2 = "<PASSWORD FOR ssid2>";

#define MESSAGE_CONTRACT "0x0000000000000000000000000000000000000000" 

int wificounter = 0;

#define PRINT_CALLBACK  0

enum APIRoutes
{
    api_unknown,
    api_checkSignature,
    api_End
};

std::map<std::string, APIRoutes> s_apiRoutes;

void Initialize()
{
    s_apiRoutes["checkSignature"] = api_checkSignature;
    s_apiRoutes["end"] = api_End;
}

// Define the number of devices we have in the chain and the hardware interface
// NOTE: These pin numbers will probably not work with your hardware and may
// need to be adapted
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 12

#define CLK_PIN   2 // or SCK green
#define DATA_PIN  4 // or MOSI orange
#define CS_PIN    3 // or SS   yellow

MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// Global message buffers shared by Wifi and Scrolling functions
const uint8_t MESG_SIZE = 255;
const uint8_t STACK_SIZE = 128;
const uint8_t CHAR_SPACING = 1;
const uint8_t SCROLL_DELAY = 75;

char **messageStack;
TokenCache *tokenCache;

int counter = 0;
int currentShowingIndex = 0;
int currentIndex = 0;

const char *INFURA_HOST = "rinkeby.infura.io";
const char *INFURA_PATH = "/v3/c7df4c29472d4d54a39f7aa78f146853";

void setupWifi();
Web3 web3(INFURA_HOST, INFURA_PATH);
KeyID *keyID;

UdpBridge *udpConnection;
TcpBridge *tcpConnection;

const char *apiRoute = "api/";

uint8_t scrollDataSource(uint8_t dev, MD_MAX72XX::transformType_t t)
{
  static enum { S_IDLE, S_NEXT_CHAR, S_SHOW_CHAR, S_SHOW_SPACE } state = S_IDLE;
  static char *p;
  static uint16_t curLen, showLen;
  static uint8_t  cBuf[8];
  uint8_t colData = 0;

  // finite state machine to control what we do on the callback
  switch (state)
  {
  case S_IDLE: // reset the message pointer and check for new message to load
    if (currentShowingIndex != currentIndex)  // there is a new message waiting
    {
      currentShowingIndex++;
      if (currentShowingIndex >= STACK_SIZE) currentShowingIndex = 0;
    }
    p = messageStack[currentShowingIndex]; // reset the pointer to start of message
    state = S_NEXT_CHAR;
    break;

  case S_NEXT_CHAR: // Load the next character from the font table
    if (*p == '\0')
      state = S_IDLE;
    else
    {
      showLen = mx.getChar(*p++, sizeof(cBuf) / sizeof(cBuf[0]), cBuf);
      curLen = 0;
      state = S_SHOW_CHAR;
    }
    break;

  case S_SHOW_CHAR: // display the next part of the character
    colData = cBuf[curLen++];
    if (curLen < showLen)
      break;

    // set up the inter character spacing
    showLen = (*p != '\0' ? CHAR_SPACING : (MAX_DEVICES*COL_SIZE)/2);
    curLen = 0;
    state = S_SHOW_SPACE;
    // fall through

  case S_SHOW_SPACE:  // display inter-character spacing (blank column)
    curLen++;
    if (curLen == showLen)
      state = S_NEXT_CHAR;
    break;

  default:
    state = S_IDLE;
  }

  return(colData);
}

void scrollText(void)
{
  static uint32_t	prevTime = 0;

  // Is it time to scroll the text?
  if (millis() - prevTime >= SCROLL_DELAY)
  {
    mx.transform(MD_MAX72XX::TSL);  // scroll along - the callback will load all the data
    prevTime = millis();      // starting point for next time
    counter++;
  }
}

void scrollDataSink(uint8_t dev, MD_MAX72XX::transformType_t t, uint8_t col)
{

}

void writeToMessageStack(const char *message)
{
  currentIndex++;
  Serial.println(currentIndex);
  if (currentIndex >= STACK_SIZE) currentIndex = 0;
  sprintf(messageStack[currentIndex], message);
}

void toLowerCase(string &data)
{
    std::for_each(data.begin(), data.end(), [](char & c) {
		c = ::tolower(c);
	});
}

bool hasValue(BYTE *value, int length)
{
    for (int i = 0; i < length; i++)
    {
        if ((*value++) != 0)
        {
            return true;
        }
    }

    return false;
}

bool QueryBalance(std::string *userAddress)
{
    bool hasToken = tokenCache->CheckToken(userAddress);

    if (hasToken)
    {
        Serial.println("Cache hit");
        return true;
    }

    // transaction
    const char *contractAddr = MESSAGE_CONTRACT;
    Contract contract(&web3, contractAddr);
    string func = "balanceOf(address)";
    string param = contract.SetupContractData(func.c_str(), userAddress);
    string result = contract.ViewCall(&param);

    Serial.println(result.c_str());

    BYTE tokenVal[32];

    //break down the result
    uint256_t baseBalance = web3.getUint256(&result);
    
    if (baseBalance > 0)
    {
      hasToken = true;
      Serial.println("Has token");
      tokenCache->AddToken(userAddress);
    }

    return hasToken;
}

std::string handleTCPAPI(APIReturn *apiReturn)
{
    char numstr[21];

    Serial.print("ESP Free Heap: ");
    Serial.println(ESP.getFreeHeap());
    Serial.print("ESP Max alloc Heap: ");
    Serial.println(ESP.getMaxAllocHeap());

    Serial.println(apiReturn->apiName.c_str());

    std::string message = apiReturn->params["msg"];
    std::string signature = apiReturn->params["sig"];

    std::string address = Crypto::ECRecoverFromPersonalMessage(&signature, &message);
    Serial.print("EC Recover: ");
    Serial.println(address.c_str());
    boolean hasToken = QueryBalance(&address);

    if (hasToken)
    {
      writeToMessageStack(message.c_str());
      return string("pass");
    }
    else
    {
      return string("fail");
    }
}

void setup()
{
  Serial.begin(115200);

  // Display initialization
  mx.begin();
  mx.setShiftDataInCallback(scrollDataSource);
  mx.setShiftDataOutCallback(scrollDataSink);
  mx.control(MD_MAX72XX::INTENSITY, 1);

  tokenCache = new TokenCache(10);

  Serial.println("1");
  keyID = new KeyID(&web3);
  Serial.println("2");

  setupWifi();
  Initialize();

  messageStack = new char*[STACK_SIZE];
  char *messageData = new char[STACK_SIZE*MESG_SIZE];
  for (int i = 0; i < STACK_SIZE; i++)
  { 
    messageStack[i] = (messageData + MESG_SIZE*i);
  }

  currentIndex = -1;

  writeToMessageStack("...");

  tcpConnection = new TcpBridge();
  tcpConnection->setKey(keyID, &web3);
  tcpConnection->startConnection();
}

void loop()
{
  scrollText();

  delay(5);
  tcpConnection->checkClientAPI(&handleTCPAPI);
}

void setupWifi()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return;
    }

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    if (WiFi.status() != WL_CONNECTED)
    {
        esp_wifi_set_ps(WIFI_PS_MIN_MODEM);
        WiFi.begin(ssid, password);
    }

    wificounter = 0;
    while (WiFi.status() != WL_CONNECTED && wificounter < 20)
    {
        for (int i = 0; i < 500; i++)
        {
            delay(1);
        }
        Serial.print(".");
        wificounter++;
    }

    if (wificounter >= 20)
    {
        Serial.print("-");
        WiFi.begin(ssid2, password2);
    }

    while (WiFi.status() != WL_CONNECTED && wificounter < 30)
    {
        delay(500);
        Serial.print(".");
        wificounter++;
    }

    if (wificounter >= 30)
    {
        Serial.println("Restarting ...");
        ESP.restart(); //targetting 8266 & Esp32 - you may need to replace this
    }

    esp_wifi_set_max_tx_power(78); //save a little power if your unit is near the router. If it's located away then use 78 - max
    delay(10);

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}