#include "DHT.h"

#define lineDown()      HAL_GPIO_WritePin(sensor->DHT_Port, sensor->DHT_Pin, GPIO_PIN_RESET)
#define lineUp()        HAL_GPIO_WritePin(sensor->DHT_Port, sensor->DHT_Pin, GPIO_PIN_SET)
#define getLine()       (HAL_GPIO_ReadPin(sensor->DHT_Port, sensor->DHT_Pin) == GPIO_PIN_SET)
#define Delay(d)        HAL_Delay(d)

static void goToOutput(DHT_sensor *sensor) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // By default, the line is at high level
    lineUp();

    // Configure the port as output
    GPIO_InitStruct.Pin = sensor->DHT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;   // Open drain
    GPIO_InitStruct.Pull = sensor->pullUp;        // Pull-up to power

    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; // High-speed port operation
    HAL_GPIO_Init(sensor->DHT_Port, &GPIO_InitStruct);
}

static void goToInput(DHT_sensor *sensor) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configure the port as input
    GPIO_InitStruct.Pin = sensor->DHT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = sensor->pullUp;        // Pull-up to power
    HAL_GPIO_Init(sensor->DHT_Port, &GPIO_InitStruct);
}

DHT_data DHT_getData(DHT_sensor *sensor) {
    DHT_data data = {-128.0f, -128.0f};

    #if DHT_POLLING_CONTROL == 1
    /* Limiting the sensor polling frequency */
    // Determine polling interval depending on sensor type
    uint16_t pollingInterval;
    if (sensor->type == DHT11) {
        pollingInterval = DHT_POLLING_INTERVAL_DHT11;
    } else {
        pollingInterval = DHT_POLLING_INTERVAL_DHT22;
    }

    // If interval is too short, return the last successful value
    if ((HAL_GetTick() - sensor->lastPollingTime < pollingInterval) && sensor->lastPollingTime != 0) {
        data.hum = sensor->lastHum;
        data.temp = sensor->lastTemp;
        return data;
    }
    sensor->lastPollingTime = HAL_GetTick()+1;
    #endif

    /* Request data from the sensor */
    // Switch pin to "output"
    goToOutput(sensor);
    // Pull the data line low for 18 ms
    lineDown();
    Delay(18);
    // Pull the line high, then switch port to "input"
    lineUp();
    goToInput(sensor);

    #ifdef DHT_IRQ_CONTROL
    // Disable interrupts so nothing interferes with data handling
    __disable_irq();
    #endif

    /* Wait for sensor response */
    uint16_t timeout = 0;
    // Wait for falling edge
    while(getLine()) {
        timeout++;
        if (timeout > DHT_TIMEOUT) {
            #ifdef DHT_IRQ_CONTROL
            __enable_irq();
            #endif
            // If sensor did not respond, it definitely does not exist
            // Reset the last successful value to avoid phantom data
            sensor->lastHum = -128.0f;
            sensor->lastTemp = -128.0f;

            return data;
        }
    }
    timeout = 0;
    // Wait for rising edge
    while(!getLine()) {
        timeout++;
        if (timeout > DHT_TIMEOUT) {
            #ifdef DHT_IRQ_CONTROL
            __enable_irq();
            #endif
            // If sensor did not respond, it definitely does not exist
            // Reset the last successful value to avoid phantom data
            sensor->lastHum = -128.0f;
            sensor->lastTemp = -128.0f;

            return data;
        }
    }
    timeout = 0;
    // Wait for falling edge
    while(getLine()) {
        timeout++;
        if (timeout > DHT_TIMEOUT) {
            #ifdef DHT_IRQ_CONTROL
            __enable_irq();
            #endif
            return data;
        }
    }

    /* Read sensor response */
    uint8_t rawData[5] = {0,0,0,0,0};
    for(uint8_t a = 0; a < 5; a++) {
        for(uint8_t b = 7; b != 255; b--) {
            uint16_t hT = 0, lT = 0;
            // While the line is low, increment lT
            while(!getLine() && lT != 65535) lT++;
            // While the line is high, increment hT
            timeout = 0;
            while(getLine() && hT != 65535) hT++;
            // If hT is greater than lT, then a "1" has been received
            if(hT > lT) rawData[a] |= (1<<b);
        }
    }

    #ifdef DHT_IRQ_CONTROL
    // Re-enable interrupts after data reception
    __enable_irq();
    #endif

    /* Verify data integrity */
    if((uint8_t)(rawData[0] + rawData[1] + rawData[2] + rawData[3]) == rawData[4]) {
        // If checksum matches, convert and return received values
        if (sensor->type == DHT22) {
            data.hum = (float)(((uint16_t)rawData[0]<<8) | rawData[1])*0.1f;
            // Check if temperature is negative
            if(!(rawData[2] & (1<<7))) {
                data.temp = (float)(((uint16_t)rawData[2]<<8) | rawData[3])*0.1f;
            } else {
                rawData[2] &= ~(1<<7);
                data.temp = (float)(((uint16_t)rawData[2]<<8) | rawData[3])*-0.1f;
            }
        }
        if (sensor->type == DHT11) {
            data.hum = (float)rawData[0];
            data.temp = (float)rawData[2];
        }
    }

    #if DHT_POLLING_CONTROL == 1
    sensor->lastHum = data.hum;
    sensor->lastTemp = data.temp;
    #endif

    return data;
}
