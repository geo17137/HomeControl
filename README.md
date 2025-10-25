# ESP32 Home Control System<br>
This system controls multiple home systems (readme partial IA generate)

## System Architecture

### Hardware

- Based on ESP32 microcontroller
  Supports two hardware configurations:
  - Standard ESP32 with direct GPIO connections
  - ES32A08 board using 74HC595/74HC165 shift registers for I/O expansion
  
- LCD display with I2C interface for local control
- Rotary encoder with push button for user interaction
- Various relays for controlling home systems

### Software

- Modular architecture with separate components for different functions
  Multiple control interfaces:
  - Local LCD display with rotary encoder
  - MQTT for remote control via smartphone/home automation system
  - Alexa voice control integration
  - Persistent storage using LittleFS for configuration parameters
  - Sophisticated scheduling system with up to 4 time slots per day per device
  - Safety features including watchdog timers and timeout monitoring

## Controlled Systems

- Irrigation System. 
  
  - Includes safety features for pump protection
  
  - Supports scheduled operation
  
- Manages tank filling for irrigation
  - Supports scheduled operation
  
- Controls irrigation for lemon trees (east valve)
  - Supports scheduled operation
  
- Controls watering lance with timeout protection

- HVAC (PAC - Heat Pump)
  - Controls power to heat pump
  - Includes delayed shutdown sequence
  - Sends IR commands for control
  - Supports scheduled operation
  
- Ventilation (VMC)
  - Multiple operation modes (off, programmed, slow, fast)
  - Controls both local and remote VMC boards
  - Supports scheduled operation
  
- Kitchen Power
  - Controls power to cooking appliances
  - Supports scheduled operation
  
- Suppressor water Management 
  - Controls water pump and pressure system
  - Includes safety features to detect leaks or system failures
  - Monitors pressure system operation

## Key Features

- Scheduling System
  - Each device can have up to 4 time slots per day
  - Global enable/disable for each device's schedule
  - Persistent scheduling across reboots

- User Interface
  - Multi-level menu system on LCD display
  - Rotary encoder with push button for navigation
  - Status display showing current state of all systems

- Remote Control

  - MQTT integration for remote monitoring and control

  - Alexa voice control for key functions

  - OTA (Over-The-Air) update capability


- Safety Features

  - Watchdog timer to prevent system lockups

  - Timeout protection for critical operations

  - Monitoring of pressure system for leak detection

  - Automatic recovery from WiFi/MQTT disconnections


- Logging System

  - Event logging to flash memory

  - Retrievable logs via MQTT

  - Configurable logging enable/disable


## Code Implementation

- The code demonstrates sophisticated programming techniques:
  - Event-driven architecture using function pointers and callbacks
  - RTOS task management for concurrent operations
  - Context switching between interrupt handlers and main loop
  - Persistent parameter management
  - Comprehensive error handling and recovery

This system represents a complete home automation solution with multiple interfaces, sophisticated scheduling, and robust safety features. The code is well-structured and extensively documented, making it maintainable and extensible.

## Links

### Hardware links

- ES32A08 board using 74HC595/74HC165 shift registers for I/O expansion
  https://fr.aliexpress.com/item/1005006185382987.html?spm=a2g0o.productlist.main.1.698elv0Ylv0YwW&algo_pvid=f51ea843-ec14-4632-9974-88f5128b477e&algo_exp_id=f51ea843-ec14-4632-9974-88f5128b477e-0&pdp_npi=4%40dis%21EUR%2140.96%2133.69%21%21%21313.64%21257.97%21%4021039fae17272463435504450e2d37%2112000036180720579%21sea%21FR%210%21ABX&curPageLogUid=gOixLiNgnswU&utparam-url=scene%3Asearch%7Cquery_from%3A<br>

  https://github.com/geo17137/HomeControl/blob/master/DocsPdf/Schematic_ES32A08.pdf<br>

- LCD display with I2C interface for local control<br>
  https://fr.aliexpress.com/item/32704155771.html?gatewayAdapt=glo2fra<br>

- Rotary encoder with push button for user interaction<br>
  https://fr.aliexpress.com/item/1005006105510076.html?aem_p4p_detail=20250921231705735014141854740004780311&algo_exp_id=447db19f-1e75-47da-b000-2e1db313c155-3&pdp_ext_f=%7B%22order%22%3A%22204%22%2C%22eval%22%3A%221%22%2C%22fromPage%22%3A%22search%22%7D&utparam-url=scene%3Asearch%7Cquery_from%3A%7Cx_object_id%3A1005006105510076%7C_p_origin_prod%3A&search_p4p_id=20250921231705735014141854740004780311_1

### Remote peripherals

- HVAC (PAC - Heat Pump)

  - Project<br>
    https://github.com/geo17137/MitsubishiPAC_IR

  - IR command hardware<br>
    https://fr.aliexpress.com/item/1005005777409596.html?spm=a2g0o.order_list.order_list_main.247.15e75e5bc2Vkbf&gatewayAdapt=glo2fra

- Fast  VMC

  - Project<br>
    https://github.com/geo17137/VmcSpeedUp

  - Wifi card <br>
    https://fr.aliexpress.com/item/1005003493100800.html?spm=a2g0o.productlist.main.3.4d062676Fq4FnN&algo_pvid=a803610d-f58a-49a4-b435-8789b4cb9b29&algo_exp_id=a803610d-f58a-49a4-b435-8789b4cb9b29-1&pdp_npi=4%40dis%21EUR%212.46%212.46%21%21%212.68%212.68%21%40213db7b317269055681243227e4b9a%2112000026040971359%21sea%21FR%21917560485%21X&curPageLogUid=KEOAsm3pATLN&utparam-url=scene%3Asearch%7Cquery_from%3A

  - Wifi card power supply 220/5V<br>
    https://fr.aliexpress.com/item/32408565688.html?spm=a2g0o.order_list.order_list_main.321.24865e5bui7fWN&gatewayAdapt=glo2fra

### Remote control (Android project)

https://github.com/geo17137/HomeControlAndroidFrontEnd

### Plans<br>

- Water distribution<br>
  https://github.com/geo17137/HomeControl/blob/master/DocsPdf/Plan%20distribution%20eau.pdf

- Control cabinet wiring<br>
  https://github.com/geo17137/HomeControl/blob/master/DocsPdf/Plan%20de%20cablage%20armoire%20automate%20ESP32%20-%20V6.pdf

- Control board wiring<br>
  https://github.com/geo17137/HomeControl/blob/master/DocsPdf/CablageES32A08.pdf

- General documentation<br>
  https://github.com/geo17137/HomeControl/blob/master/DocsPdf/DocGeneraleHomeControl.pdf



