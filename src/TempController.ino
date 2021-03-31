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
#define EVENT_NAME String("Novus322") // expect EVENT_NAME/status and EVENT_NAME/cmd_resp events to be published
#define STATUS_FREQ 1 // minutes per status update (default every 1 minute)

/*  
1. Every STATUS_FREQ minutes, this status JSON will be published:
EVENT_NAME/status:
{
  "sp1": 30.1,
  "pv": 40.3,
  "ihm_status": 1024,
  "control_status_1": 0,
  "hysteresis_1": 0.5,
  "control_status_2": 255,
  "sp2": 1,
  "hysteresis_2": 1.8,
  "offset": 0
}

2. When a command is executed, this JSON will be published:
EVENT_NAME/cmd_resp:
[
  {
    "function": "set_setpoint_1",
    "command": "-6.25",
    "result": "-6.200000",
    "error": 0
  }
]
(typically a 1 element JSON array)

3. You may send events that call functions to this device 
Thanks to ParticleFunctionCaller(), you may also call Particle.functions from subscribed events:
Subscribed by default to EVENT_NAME/function/{{{PARTICLE_DEVICE_ID}}} 

for example:
$ particle publish Novus322/function/e00fce68ee01234a8b1ffc0c '{"function":"set_setpoint_1","command":"29.9"}'

You may direct webhook responses to this device (so that webhook response JSON may trigger particle functons)
Add this configuration to the particle console: 
Integrations / Your Integration / Edit / Advanced / Webhook Responses / Response Topic
EVENT_NAME/function/{{{PARTICLE_DEVICE_ID}}}

Functions may be called as such:
{"function":"set_setpoint_1","command":"29.9"}

*/



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
	// Also remember to add function calls to ParticleFunctionCaller()
    Particle.function("set_setpoint_1",set_setpoint_1);
    Particle.function("set_hysteresis_1",set_hysteresis_1);
    Particle.function("set_setpoint_2",set_setpoint_2);
    Particle.function("set_hysteresis_2",set_hysteresis_2);
    Particle.function("set_offset",set_offset);

    // Handle incoming function calls via subscribe
    Particle.subscribe(EVENT_NAME + "/function/" + System.deviceID(), ParticleFunctionCaller, MY_DEVICES);

    Particle.publishVitals(60*60); // publish vitals every hour 

}



void ParticleFunctionCaller(const char *eventName, const char *data) {
    jw.clear();
	jw.startObject();
	jw.insertKeyValue("event", eventName);
	jw.insertKeyValue("data", data);
	jw.finishObjectOrArray();
	String json = jw.getBuffer();
	jw.clear();
	if (json.charAt(0) == ',') {
		json.setCharAt(0,' ');
	} // remove leading comma
	//Particle.publish(EVENT_NAME + "/notice", json.c_str());

	JSONValue outerObj = JSONValue::parseCopy(data);
    JSONObjectIterator iter(outerObj);
    String functionName;
    String command;
    bool hasFunction = false;
    bool hasCommand = false;

    while(iter.next()) {
       	if ( String(iter.name()) == "function") { functionName = String(iter.value().toString()); hasFunction = true;}
       	if ( String(iter.name()) == "command") { command = String(iter.value().toString()); hasCommand = true; }
    }

    // PUBLISHED FUNCTIONS: ADD ALL Particle Functions here
    if (hasFunction && hasCommand) {
    	if (functionName == "set_setpoint_1") { set_setpoint_1(command); }
    	else if (functionName == "set_hysteresis_1") { set_hysteresis_1(command); }
    	else if (functionName == "set_setpoint_2") { set_setpoint_2(command); }
    	else if (functionName == "set_hysteresis_2") { set_hysteresis_2(command); }
    	else if (functionName == "set_offset") { set_offset(command); }
    } 

}


bool cmd_resp(String function, String command, String result, int error) {
	jw.clear();
	jw.startArray();
	jw.startObject();
	jw.insertKeyValue("function", function);
	jw.insertKeyValue("command", command);
	jw.insertKeyValue("result", result);
	jw.insertKeyValue("error", error);
	jw.finishObjectOrArray();
	jw.finishObjectOrArray();
	String json = jw.getBuffer();
	jw.clear();
	if (json.charAt(0) == ',') {
		json.setCharAt(0,' ');
	} // remove leading comma
	return (Particle.publish(EVENT_NAME + "/cmd_resp", json.c_str(), PRIVATE));

}

			


// Functions to set various modbus registers.
// Code is identical, except for the macros macros which are custom to each register.
int set_setpoint_1(String command) {
#define REGISTER 0
#define FUNCTION_NAME "set_setpoint_1"
#define CONSTRAIN(floatval) (constrain(floatval, -199.0, 999.0))
#define COMMAND_TO_MODBUS_VALUE(command,modbus_value) \
	float floatval = command.toFloat();  \
	floatval = CONSTRAIN(floatval);      \
	modbus_value = (int16_t)(floatval*10)
#define MODBUS_VALUE_TO_STRING(modbus_value) (String::format("%f", ((double)((int16_t) modbus_value ))/10 ))

	uint8_t modbus_code;
	uint16_t modbus_value;
	uint16_t value;

	command = command.trim();
	if (command.length() > 0 && command.toFloat() == 0 && command != "0") { cmd_resp(FUNCTION_NAME,command,"",-1); return -1; }
	if (command == "") {
		modbus_code = node.readHoldingRegisters(REGISTER,1);
    	if (modbus_code == node.ku8MBSuccess) {
            modbus_value = node.getResponseBuffer(0);
            cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
        	return (int16_t) modbus_value;
		} else { cmd_resp(FUNCTION_NAME,command,"",-2); return -2; }
	}	
	COMMAND_TO_MODBUS_VALUE(command,modbus_value);
	modbus_code = node.writeSingleRegister(REGISTER,(uint16_t)modbus_value);
	if (modbus_code != node.ku8MBSuccess) { cmd_resp(FUNCTION_NAME,command,"",-3); return -3; }
	delay(250);
	modbus_code = node.readHoldingRegisters(REGISTER,1);
    if (modbus_code == node.ku8MBSuccess) {
       	modbus_value = node.getResponseBuffer(0);
       	cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
       	return (int16_t) modbus_value;
	} else { cmd_resp(FUNCTION_NAME,command,"",-4); return -4; }

}

int set_hysteresis_1(String command) {
#define REGISTER 8
#define FUNCTION_NAME "set_hysteresis_1"
#define CONSTRAIN(floatval) (constrain(floatval, 0.1, 50.0))
#define COMMAND_TO_MODBUS_VALUE(command,modbus_value) \
	float floatval = command.toFloat();  \
	floatval = CONSTRAIN(floatval);      \
	modbus_value = (int16_t)(floatval*10)
#define MODBUS_VALUE_TO_STRING(modbus_value) (String::format("%f", ((double)((int16_t) modbus_value ))/10 ))

	uint8_t modbus_code;
	uint16_t modbus_value;
	uint16_t value;

	command = command.trim();
	if (command.length() > 0 && command.toFloat() == 0 && command != "0") { cmd_resp(FUNCTION_NAME,command,"",-1); return -1; }
	if (command == "") {
		modbus_code = node.readHoldingRegisters(REGISTER,1);
    	if (modbus_code == node.ku8MBSuccess) {
            modbus_value = node.getResponseBuffer(0);
            cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
        	return (int16_t) modbus_value;
		} else { cmd_resp(FUNCTION_NAME,command,"",-2); return -2; }
	}	
	COMMAND_TO_MODBUS_VALUE(command,modbus_value);
	modbus_code = node.writeSingleRegister(REGISTER,(uint16_t)modbus_value);
	if (modbus_code != node.ku8MBSuccess) { cmd_resp(FUNCTION_NAME,command,"",-3); return -3; }
	delay(250);
	modbus_code = node.readHoldingRegisters(REGISTER,1);
    if (modbus_code == node.ku8MBSuccess) {
       	modbus_value = node.getResponseBuffer(0);
       	cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
       	return (int16_t) modbus_value;
	} else { cmd_resp(FUNCTION_NAME,command,"",-4); return -4; }

}
int set_setpoint_2(String command) {

#define REGISTER 10
#define FUNCTION_NAME "set_setpoint_2"
#define CONSTRAIN(floatval) (constrain(floatval, -199.0, 999.0))
#define COMMAND_TO_MODBUS_VALUE(command,modbus_value) \
	float floatval = command.toFloat();  \
	floatval = CONSTRAIN(floatval);      \
	modbus_value = (int16_t)(floatval*10)
#define MODBUS_VALUE_TO_STRING(modbus_value) (String::format("%f", ((double)((int16_t) modbus_value ))/10 ))

	uint8_t modbus_code;
	uint16_t modbus_value;
	uint16_t value;

	command = command.trim();
	if (command.length() > 0 && command.toFloat() == 0 && command != "0") { cmd_resp(FUNCTION_NAME,command,"",-1); return -1; }
	if (command == "") {
		modbus_code = node.readHoldingRegisters(REGISTER,1);
    	if (modbus_code == node.ku8MBSuccess) {
            modbus_value = node.getResponseBuffer(0);
            cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
        	return (int16_t) modbus_value;
		} else { cmd_resp(FUNCTION_NAME,command,"",-2); return -2; }
	}	
	COMMAND_TO_MODBUS_VALUE(command,modbus_value);
	modbus_code = node.writeSingleRegister(REGISTER,(uint16_t)modbus_value);
	if (modbus_code != node.ku8MBSuccess) { cmd_resp(FUNCTION_NAME,command,"",-3); return -3; }
	delay(250);
	modbus_code = node.readHoldingRegisters(REGISTER,1);
    if (modbus_code == node.ku8MBSuccess) {
       	modbus_value = node.getResponseBuffer(0);
       	cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
       	return (int16_t) modbus_value;
	} else { cmd_resp(FUNCTION_NAME,command,"",-4); return -4; }

}
int set_hysteresis_2(String command) {

#define REGISTER 12
#define FUNCTION_NAME "set_hysteresis_2"
#define CONSTRAIN(floatval) (constrain(floatval, 0.1, 50.0))
#define COMMAND_TO_MODBUS_VALUE(command,modbus_value) \
	float floatval = command.toFloat();  \
	floatval = CONSTRAIN(floatval);      \
	modbus_value = (int16_t)(floatval*10)
#define MODBUS_VALUE_TO_STRING(modbus_value) (String::format("%f", ((double)((int16_t) modbus_value ))/10 ))

	uint8_t modbus_code;
	uint16_t modbus_value;
	uint16_t value;

	command = command.trim();
	if (command.length() > 0 && command.toFloat() == 0 && command != "0") { cmd_resp(FUNCTION_NAME,command,"",-1); return -1; }
	if (command == "") {
		modbus_code = node.readHoldingRegisters(REGISTER,1);
    	if (modbus_code == node.ku8MBSuccess) {
            modbus_value = node.getResponseBuffer(0);
            cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
        	return (int16_t) modbus_value;
		} else { cmd_resp(FUNCTION_NAME,command,"",-2); return -2; }
	}	
	COMMAND_TO_MODBUS_VALUE(command,modbus_value);
	modbus_code = node.writeSingleRegister(REGISTER,(uint16_t)modbus_value);
	if (modbus_code != node.ku8MBSuccess) { cmd_resp(FUNCTION_NAME,command,"",-3); return -3; }
	delay(250);
	modbus_code = node.readHoldingRegisters(REGISTER,1);
    if (modbus_code == node.ku8MBSuccess) {
       	modbus_value = node.getResponseBuffer(0);
       	cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
       	return (int16_t) modbus_value;
	} else { cmd_resp(FUNCTION_NAME,command,"",-4); return -4; }

}
int set_offset(String command) {

#define REGISTER 12
#define FUNCTION_NAME "set_offset"
#define CONSTRAIN(floatval) (constrain(floatval, -10.0, 10.0))
#define COMMAND_TO_MODBUS_VALUE(command,modbus_value) \
	float floatval = command.toFloat();  \
	floatval = CONSTRAIN(floatval);      \
	modbus_value = (int16_t)(floatval*10)
#define MODBUS_VALUE_TO_STRING(modbus_value) (String::format("%f", ((double)((int16_t) modbus_value ))/10 ))

	uint8_t modbus_code;
	uint16_t modbus_value;
	uint16_t value;

	command = command.trim();
	if (command.length() > 0 && command.toFloat() == 0 && command != "0") { cmd_resp(FUNCTION_NAME,command,"",-1); return -1; }
	if (command == "") {
		modbus_code = node.readHoldingRegisters(REGISTER,1);
    	if (modbus_code == node.ku8MBSuccess) {
            modbus_value = node.getResponseBuffer(0);
            cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
        	return (int16_t) modbus_value;
		} else { cmd_resp(FUNCTION_NAME,command,"",-2); return -2; }
	}	
	COMMAND_TO_MODBUS_VALUE(command,modbus_value);
	modbus_code = node.writeSingleRegister(REGISTER,(uint16_t)modbus_value);
	if (modbus_code != node.ku8MBSuccess) { cmd_resp(FUNCTION_NAME,command,"",-3); return -3; }
	delay(250);
	modbus_code = node.readHoldingRegisters(REGISTER,1);
    if (modbus_code == node.ku8MBSuccess) {
       	modbus_value = node.getResponseBuffer(0);
       	cmd_resp(FUNCTION_NAME,command,MODBUS_VALUE_TO_STRING(modbus_value),0);
       	return (int16_t) modbus_value;
	} else { cmd_resp(FUNCTION_NAME,command,"",-4); return -4; }

}


unsigned long previousMillis1 = 0;
void loop() {
    // do nothing :P

	unsigned long currentMillis1 = millis();
    if(currentMillis1 - previousMillis1 > (STATUS_FREQ * 60 * 1000) ) {
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
					jw.insertKeyValue("hysteresis_1", ((double)((int16_t) node.getResponseBuffer(0)))/10);
					jw.insertKeyValue("control_status_2", node.getResponseBuffer(1));
					jw.insertKeyValue("sp2", ((double)((int16_t) node.getResponseBuffer(2)))/10  );
					jw.insertKeyValue("hysteresis_2", ((double)((int16_t) node.getResponseBuffer(3)))/10);
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
			} // remove leading comma if neccesary
			Particle.publish(EVENT_NAME + "/status", json.c_str(), PRIVATE); 
			jw.clear();

	    } else {
	        Serial.print("Failed, Response Code: ");
			Serial.print(result, HEX); 
			Serial.println("");
	    }

	}

	delay(250);
	
}





