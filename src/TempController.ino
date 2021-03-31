	/*
 * Project Novus N322 modbus RTU driver
 * Description: temperature controller driver
 * Author: tim@hudsonsonoma.com
 * Date: 5 September 2020
 */

// Test code
SYSTEM_MODE(AUTOMATIC);
SYSTEM_THREAD(ENABLED);

#define NODE_UNIT_ID 1   // The address of the modbus unit to connect to
#define NODE_BAUD 9600   // The baud rate of the modbus unit

#define CTRL_PIN D6
#define TEMP_REGISTERS 0 // starting offset of modbus holding registers

#include "ModbusMaster.h"
#include "JsonParserGeneratorRK.h"

SerialLogHandler logHandler(9600,LOG_LEVEL_WARN, {
    {"app", LOG_LEVEL_TRACE},
    {"system", LOG_LEVEL_INFO}
});

// connect to modbus node NODE_UNIT_ID
ModbusMaster node(NODE_UNIT_ID);
JsonWriterStatic<512> jw;

void setup() {
	pinMode(CTRL_PIN,OUTPUT);

	node.begin(NODE_BAUD);
	node.enableTXpin(CTRL_PIN);
	node.enableDebug();

    delay(5000);

	// Edit or comment out to control what options are presented 
	// in the Particle.io mobile app interface
    Particle.function("set_setpoint_1",set_setpoint_1);
    Particle.function("set_hysteresis_1",set_hysteresis_1);
    Particle.function("set_setpoint_2",set_setpoint_2);
    Particle.function("set_hysteresis_2",set_hysteresis_2);
    Particle.function("set_offset",set_offset);

}


// Functions to set various modbus registers.
// Code is identical, except for the REGISTER and CONSTRAIN macros which are custom to each register.
int set_setpoint_1(String command) {

#define REGISTER 0
#define CONSTRAIN floatval = constrain(floatval, -199.0, 999.0);
	uint8_t result;
	uint16_t value;

	if (command.length() > 0 && command.toFloat() == 0 && command != "0") { return -4; }
	if (command == "") {
		result = node.readHoldingRegisters(REGISTER,1);
    	if (result == node.ku8MBSuccess) {
            value = node.getResponseBuffer(0);
        	return (int16_t) value;
		} else { return -1; }
	}
	delay(250);
	float floatval = command.toFloat();
	CONSTRAIN
	int16_t newval = (int16_t)(floatval*10);
	result = node.writeSingleRegister(REGISTER,(uint16_t)newval);
	if (result != node.ku8MBSuccess) { return -2; }
	delay(250);
	result = node.readHoldingRegisters(REGISTER,1);
    if (result == node.ku8MBSuccess) {
       	value = node.getResponseBuffer(0);
       	return (int16_t) value;
	} else { return -3; }

}

int set_hysteresis_1(String command) {
#define REGISTER 8
#define CONSTRAIN floatval = constrain(floatval, 0.1, 50.0);
	uint8_t result;
	uint16_t value;

	if (command.length() > 0 && command.toFloat() == 0 && command != "0") { return -4; }
	if (command == "") {
		result = node.readHoldingRegisters(REGISTER,1);
    	if (result == node.ku8MBSuccess) {
            value = node.getResponseBuffer(0);
        	return (int16_t) value;
		} else { return -1; }
	}
	delay(250);
	float floatval = command.toFloat();
	CONSTRAIN
	int16_t newval = (int16_t)(floatval*10);
	result = node.writeSingleRegister(REGISTER,(uint16_t)newval);
	if (result != node.ku8MBSuccess) { return -2; }
	delay(250);
	result = node.readHoldingRegisters(REGISTER,1);
    if (result == node.ku8MBSuccess) {
       	value = node.getResponseBuffer(0);
       	return (int16_t) value;
	} else { return -3; }
}
int set_setpoint_2(String command) {
#define REGISTER 10
#define CONSTRAIN floatval = constrain(floatval, -199.0, 999.0);
	uint8_t result;
	uint16_t value;

	if (command.length() > 0 && command.toFloat() == 0 && command != "0") { return -4; }
	if (command == "") {
		result = node.readHoldingRegisters(REGISTER,1);
    	if (result == node.ku8MBSuccess) {
            value = node.getResponseBuffer(0);
        	return (int16_t) value;
		} else { return -1; }
	}
	delay(250);
	float floatval = command.toFloat();
	CONSTRAIN
	int16_t newval = (int16_t)(floatval*10);
	result = node.writeSingleRegister(REGISTER,(uint16_t)newval);
	if (result != node.ku8MBSuccess) { return -2; }
	delay(250);
	result = node.readHoldingRegisters(REGISTER,1);
    if (result == node.ku8MBSuccess) {
       	value = node.getResponseBuffer(0);
       	return (int16_t) value;
	} else { return -3; }
}
int set_hysteresis_2(String command) {
#define REGISTER 11
#define CONSTRAIN floatval = constrain(floatval, 0.1, 50.0);
	uint8_t result;
	uint16_t value;

	if (command.length() > 0 && command.toFloat() == 0 && command != "0") { return -4; }
	if (command == "") {
		result = node.readHoldingRegisters(REGISTER,1);
    	if (result == node.ku8MBSuccess) {
            value = node.getResponseBuffer(0);
        	return (int16_t) value;
		} else { return -1; }
	}
	delay(250);
	float floatval = command.toFloat();
	CONSTRAIN
	int16_t newval = (int16_t)(floatval*10);
	result = node.writeSingleRegister(REGISTER,(uint16_t)newval);
	if (result != node.ku8MBSuccess) { return -2; }
	delay(250);
	result = node.readHoldingRegisters(REGISTER,1);
    if (result == node.ku8MBSuccess) {
       	value = node.getResponseBuffer(0);
       	return (int16_t) value;
	} else { return -3; }
}
int set_offset(String command) {
#define REGISTER 12
#define CONSTRAIN floatval = constrain(floatval, -10.0, 10.0);
	uint8_t result;
	uint16_t value;

	if (command.length() > 0 && command.toFloat() == 0 && command != "0") { return -4; }
	if (command == "") {
		result = node.readHoldingRegisters(REGISTER,1);
    	if (result == node.ku8MBSuccess) {
            value = node.getResponseBuffer(0);
        	return (int16_t) value;
		} else { return -1; }
	}
	delay(250);
	float floatval = command.toFloat();
	CONSTRAIN
	int16_t newval = (int16_t)(floatval*10);
	result = node.writeSingleRegister(REGISTER,(uint16_t)newval);
	if (result != node.ku8MBSuccess) { return -2; }
	delay(250);
	result = node.readHoldingRegisters(REGISTER,1);
    if (result == node.ku8MBSuccess) {
       	value = node.getResponseBuffer(0);
       	return (int16_t) value;
	} else { return -3; }
}


unsigned long previousMillis1 = 0;
void loop() {
    // do nothing :P

	unsigned long currentMillis1 = millis();
    if(currentMillis1 - previousMillis1 > 60 * 1000) {
        previousMillis1 = currentMillis1;

	    uint8_t result = node.readHoldingRegisters(TEMP_REGISTERS,4);

	    if (result == node.ku8MBSuccess) {
	        uint16_t value = node.getResponseBuffer(0);
	        //Log.info("Received: %0x",value);

				jw.startObject();

				// Add various types of data
				// Edit or comment out lines to control what
				// data is reported in Particle.io mobile app Events
				jw.insertKeyValue("sp1", ((double)((int16_t) node.getResponseBuffer(0)))/10 );
				jw.insertKeyValue("pv", ((double)((int16_t) node.getResponseBuffer(1)))/10  );
				jw.insertKeyValue("ihm_status", node.getResponseBuffer(2));
				jw.insertKeyValue("control_status_1", node.getResponseBuffer(3));


				result = node.readHoldingRegisters(TEMP_REGISTERS+4,4);
				if (result == node.ku8MBSuccess) {
					jw.insertKeyValue("display", node.getResponseBuffer(0));
					jw.insertKeyValue("firmware_version", node.getResponseBuffer(1));
					jw.insertKeyValue("serial_number_high", node.getResponseBuffer(2));
					jw.insertKeyValue("serial_number_low", node.getResponseBuffer(3));
				}

				result = node.readHoldingRegisters(TEMP_REGISTERS+8,4);
				if (result == node.ku8MBSuccess) {
					jw.insertKeyValue("hysteresis_1", node.getResponseBuffer(0)/10);
					jw.insertKeyValue("control_status_2", node.getResponseBuffer(1));
					jw.insertKeyValue("sp2", ((double)((int16_t) node.getResponseBuffer(2)))/10  );
					jw.insertKeyValue("hysteresis_2", node.getResponseBuffer(3)/10);
				}

				result = node.readHoldingRegisters(TEMP_REGISTERS+12,1);
				if (result == node.ku8MBSuccess) {
					jw.insertKeyValue("offset", ((double)((int16_t) node.getResponseBuffer(0)))/10 );
				}

				jw.finishObjectOrArray();

			Serial.printlnf("%s", jw.getBuffer());
			String json = jw.getBuffer();
			if (json.charAt(0) == ',') {
				json.setCharAt(0,' ');
			} // stupid bug.  --- replace with Particle JSON Parser.!!
			Particle.publish("temp_controller", json.c_str(), PRIVATE); 
			jw.clear();

	    } else {
	        Serial.print("Failed, Response Code: ");
			Serial.print(result, HEX); 
			Serial.println("");
	    }

	}

	delay(250);
	


}
