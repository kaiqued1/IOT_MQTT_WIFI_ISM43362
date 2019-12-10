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

ISM43362Interface net;
// WiFiInterface *wifi;

void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    logMessage("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    logMessage("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    //++arrivedcount;
}

int main(void){

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

    NetworkInterface* network = &net;
    if (!network) {
        return -1;
    }

    MQTTNetwork mqttNetwork(network);

    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    const char* hostname = "192.168.0.120";
    int port = 1883;
    logMessage("Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
        logMessage("rc from TCP connect is %d\r\n", rc);

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = "mbed-sample";
    data.username.cstring = "testuser";
    data.password.cstring = "testpassword";
    if ((rc = client.connect(data)) != 0)
        logMessage("rc from MQTT connect is %d\r\n", rc);

    if ((rc = client.subscribe(publishtopic, MQTT::QOS2, messageArrived)) != 0)
        logMessage("rc from MQTT subscribe is %d\r\n", rc);
    if ((rc = client.subscribe(subscribetopic, MQTT::QOS2, messageArrived)) != 0)
        logMessage("rc from MQTT subscribe is %d\r\n", rc);
   

    // Get device health data, send to Treasure Data every 10 seconds
    while(1){
        
        printf("\test\n");
         MQTT::Message message;

    // QoS 0
    char buf[100];
    sprintf(buf, "Hello World!  QoS 0 message from app version %f\r\n", version);
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(publishtopic, message);
    client.yield(100);


    //int x = 0;
        wait(1);

    }

   // net.disconnect();

}
