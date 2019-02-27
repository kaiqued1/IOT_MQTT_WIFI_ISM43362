/* WiFi Example
 * Copyright (c) 2016 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "TCPSocket.h"
#include "wifi-ism43362/ISM43362Interface.h"
#include "treasure-data-rest.h"
// Sensors drivers present in the BSP library
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_accelero.h"

#define BUFF_SIZE   200

ISM43362Interface net;
// WiFiInterface *wifi;

int main(void){

    int count = 0;

    printf("\r\nTreasure Data REST API Demo\n");

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

    // Create Treasure data objects (Network, Database, Table, APIKey)
    TreasureData_RESTAPI* heap  = new TreasureData_RESTAPI(&net,"iot_test","heap_info", MBED_CONF_APP_API_KEY);
    TreasureData_RESTAPI* cpu   = new TreasureData_RESTAPI(&net,"iot_test","cpu_info",  MBED_CONF_APP_API_KEY);
    TreasureData_RESTAPI* stack = new TreasureData_RESTAPI(&net,"iot_test","stack_info",MBED_CONF_APP_API_KEY);
    TreasureData_RESTAPI* sys   = new TreasureData_RESTAPI(&net,"iot_test","sys_info",  MBED_CONF_APP_API_KEY);
    TreasureData_RESTAPI* sensor   = new TreasureData_RESTAPI(&net,"iot_test","sensor_info",  MBED_CONF_APP_API_KEY);


    // Device Information Objects
    mbed_stats_cpu_t    cpuinfo;
    mbed_stats_heap_t   heapinfo;
    mbed_stats_stack_t  stackinfo;
    mbed_stats_sys_t    sysinfo;

    // Buffers to create strings in
    char cpu_buff  [BUFF_SIZE] = {0};
    char heap_buff [BUFF_SIZE] = {0};
    char stack_buff[BUFF_SIZE] = {0};
    char sys_buff  [BUFF_SIZE] = {0};
    char sensor_buff  [BUFF_SIZE] = {0};
    
    // Initiate sensor variables
    float sensor_value_temp = 0;
    float sensor_value_humid = 0;
    float sensor_value_pressure = 0;

    int16_t pDataXYZ[3] = {0};
    float pGyroDataXYZ[3] = {0};
 
    BSP_TSENSOR_Init();
    BSP_HSENSOR_Init();
    BSP_PSENSOR_Init();
         
    BSP_MAGNETO_Init();
    BSP_GYRO_Init();
    BSP_ACCELERO_Init();

    int x = 0;

    // Get device health data, send to Treasure Data every 10 seconds
    while(1){
        {
            
            
            // Collect local data
            mbed_stats_cpu_get(  &cpuinfo);

            // Construct strings to send
            x = sprintf(cpu_buff,"{\"uptime\":\"%d\",\"idle_time\":\"%d\",\"sleep_time\":\"%d\",\"deep_sleep_time\":\"%d\"}",
                                    cpuinfo.uptime,
                                    cpuinfo.idle_time,
                                    cpuinfo.sleep_time,
                                    cpuinfo.deep_sleep_time);
            cpu_buff[x]=0; // null terminate the string

            // Send data to Treasure data
            printf("\r\n Sending CPU Data: '%s'\r\n",cpu_buff);
            cpu->sendData(cpu_buff,strlen(cpu_buff));
        }
        {
            // Collect local data
            mbed_stats_heap_get(  &heapinfo);

            // Construct strings to send
            x=sprintf(heap_buff,"{\"current_size\":\"%d\",\"max_size\":\"%d\",\"total_size\":\"%d\",\"reserved_size\":\"%d\",\"alloc_cnt\":\"%d\",\"alloc_fail_cnt\":\"%d\"}",
                                heapinfo.current_size,
                                heapinfo.max_size,
                                heapinfo.total_size,
                                heapinfo.reserved_size,
                                heapinfo.alloc_cnt,
                                heapinfo.alloc_fail_cnt);
            heap_buff[x]=0; // null terminate the string

            // Send data to Treasure data
            printf("\r\n Sending Heap Data: '%s'\r\n",heap_buff);
            heap->sendData(heap_buff,strlen(heap_buff));
        }
        {
            // Collect local data
            mbed_stats_stack_get(  &stackinfo);

            // Construct strings to send
            x=sprintf(stack_buff,"{\"thread_id\":\"%d\",\"max_size\":\"%d\",\"reserved_size\":\"%d\",\"stack_cnt\":\"%d\"}",
                                stackinfo.thread_id,
                                stackinfo.max_size,
                                stackinfo.reserved_size,
                                stackinfo.stack_cnt);
            stack_buff[x]=0; // null terminate the string

            // Send data to Treasure data
            printf("\r\n Sending Stack Data: '%s'\r\n",stack_buff);
            stack->sendData(stack_buff,strlen(stack_buff));
        }
        {
            // Collect local data
            mbed_stats_sys_get(  &sysinfo);

            // Construct strings to send
            x=sprintf(sys_buff,"{\"os_version\":\"%d\",\"cpu_id\":\"%d\",\"compiler_id\":\"%d\",\"compiler_version\":\"%d\"}",
                                sysinfo.os_version,
                                sysinfo.cpu_id,
                                sysinfo.compiler_id,
                                sysinfo.compiler_version);
            sys_buff[x]=0; // null terminate the string

            // Send data to Treasure data
            printf("\r\n Sending System Data: '%s'\r\n",sys_buff);
            sys->sendData(sys_buff,strlen(sys_buff));
        }
        {
            sensor_value_temp = BSP_TSENSOR_ReadTemp();
            printf("\nTEMPERATURE = %.2f degC\n", sensor_value_temp);
     
            sensor_value_humid = BSP_HSENSOR_ReadHumidity();
            printf("HUMIDITY    = %.2f %%\n", sensor_value_humid);
     
            sensor_value_pressure = BSP_PSENSOR_ReadPressure();
            printf("PRESSURE is = %.2f mBar\n", sensor_value_pressure);
            
//            BSP_MAGNETO_GetXYZ(pDataXYZ);
//            printf("\nMAGNETO_X = %d\n", pDataXYZ[0]);
//            printf("MAGNETO_Y = %d\n", pDataXYZ[1]);
//            printf("MAGNETO_Z = %d\n", pDataXYZ[2]);
//     
//            BSP_GYRO_GetXYZ(pGyroDataXYZ);
//            printf("\nGYRO_X = %.2f\n", pGyroDataXYZ[0]);
//            printf("GYRO_Y = %.2f\n", pGyroDataXYZ[1]);
//            printf("GYRO_Z = %.2f\n", pGyroDataXYZ[2]);
//     
//            BSP_ACCELERO_AccGetXYZ(pDataXYZ);
//            printf("\nACCELERO_X = %d\n", pDataXYZ[0]);
//            printf("ACCELERO_Y = %d\n", pDataXYZ[1]);
//            printf("ACCELERO_Z = %d\n", pDataXYZ[2]);
            
            // Construct strings to send
            x=sprintf(sensor_buff,"{\"temperature\":\"%f\",\"humidity\":\"%f\",\"pressure\":\"%f\"}",
                                sensor_value_temp,
                                sensor_value_humid,
                                sensor_value_pressure
                                );
            sensor_buff[x]=0; // null terminate the string
     
            // Send data to Treasure data
            printf("\r\n Sending System Data: '%s'\r\n",sensor_buff);
            sensor->sendData(sensor_buff,strlen(sensor_buff));
            
        }
        wait(10);

    }

    net.disconnect();

    printf("\nDone, x=%d\n",x);


}
