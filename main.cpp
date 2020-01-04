/* WiFi+MQTT Example
  */

#include "mbed.h"
#include "TCPSocket.h"
#include "wifi-ism43362/ISM43362Interface.h"
#define logMessage printf
#define MQTTCLIENT_QOS2 1
#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"
#include "HTS221Sensor.h"
//#define MQTT_HOST               "demo.thingsboard.io"
#define MQTT_HOST               "192.168.43.14"
#define MQTT_PORT               1883
#define MQTT_TOPIC              "Pubtest"
#include <string>
#include "VL53L0X.h"
ISM43362Interface net;
// WiFiInterface *wifi;
Serial pc(USBTX,USBRX);
void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    logMessage("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    logMessage("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    //++arrivedcount;
}
static DevI2C devI2c(PB_11,PB_10);
static DigitalOut shutdown_pin(PC_6);
static VL53L0X range(&devI2c, &shutdown_pin, PC_7);
Serial uart(PC_4, PC_5);//TX,RX

int idx=0;
std::string inputdata;
char ch1;
int main(void){
    uart.baud(115200);
    range.init_sensor(VL53L0X_DEFAULT_ADDRESS);
    int count = 0;

    printf("\r\nWiFi+MQTT Example Demo\n");

    // Connect to Wifi
    printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
    int ret = net.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
    if (ret != 0) {
        printf("\nConnection error: %d\n", ret);
        return -1;
    }

    printf("Success\n\n");
    printf("MAC: %s\n", net.get_mac_address());
    printf("IP: %s\n", net.get_ip_address());
    printf("Netmask: %s\n", net.get_netmask());
    printf("Gateway: %s\n", net.get_gateway());
    printf("RSSI: %d\n\n", net.get_rssi());

    printf("\Wifi Example Done,MQTT Example Start\n");
    
    // MQTT Example Start
    float version = 0.6;
    char* publishtopic = "publishtest";
    char* subscribetopic = "subscribtest";

    logMessage("HelloMQTT: version is %.2f\r\n", version);
    char assess_token[] = "uNYseQgqntIrL7q5F2tL";
    NetworkInterface* network = &net;
    if (!network) {
        return -1;
    }

    MQTTNetwork mqttNetwork(network);

    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    const char* hostname = MQTT_HOST;
    int port = MQTT_PORT;
    logMessage("Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
        logMessage("rc from TCP connect is %d\r\n", rc);

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    //data.MQTTVersion = 3;
    data.clientID.cstring = "mbed-sample";
    data.username.cstring = assess_token;
    //data.password.cstring = "";
    if ((rc = client.connect(data)) != 0)
        logMessage("rc from MQTT connect is %d\r\n", rc);

    if ((rc = client.subscribe(publishtopic, MQTT::QOS2, messageArrived)) != 0)
        logMessage("rc from MQTT subscribe is %d\r\n", rc);
    if ((rc = client.subscribe(subscribetopic, MQTT::QOS2, messageArrived)) != 0)
        logMessage("rc from MQTT subscribe is %d\r\n", rc);
   

    
    printf("successfully connect!\n\n");
    
    // Initialize sensors --------------------------------------------------

    //uint8_t id;
   //DevI2C i2c_2(PB_11, PB_10);
    //HTS221Sensor hum_temp(&i2c_2);

   // hum_temp.init(NULL);
    //hum_temp.enable();
    //hum_temp.read_id(&id);
   
 //printf("HTS221  humidity & temperature sensor = 0x%X\r\n", id);
 //std::string inputdata;
 //inputdata="{\"PM2.5\":10,\"PM10\":20,\"temperature\":20,\"humidity\":45}";
  while (1) {
     
     //buffer variables
char ch;
//if data is ready in the buffer
while (uart.readable()) {
//read 1 character

ch = uart.getc();
//pc.printf("datacomming:%c",ch);
/////Pm25///////

if (ch == 's') {
//so the pointer should be set to the first position
inputdata.clear();
//pc.printf("start string");
}
//write buffer character to big buffer string
inputdata += ch;
//pc.printf("input string:%s",inputdata);
//if the character is # than the end of the sentence is
//reached and some stuff has to be done
if (ch == '#') {
//remove start and stop characters
inputdata.erase(0,1);
inputdata.erase(inputdata.length()-1,1);
pc.printf("%s",inputdata);
char msg[inputdata.size()+1];
            inputdata.copy(msg,inputdata.size()+1);
            msg[inputdata.size()]='\0';
            int n=strlen(msg); 
        void *payload = reinterpret_cast<void*>(msg);
        size_t payload_len = n;

        printf("publish to: %s %d %s\r\n", MQTT_HOST, MQTT_PORT, MQTT_TOPIC);

        if (client.publish(MQTT_TOPIC, payload, n) < 0) {
            printf("failed to publish MQTT message");
        }
        }
}
         
}
        
    

   // net.disconnect();

}