# Release Notes for Ethernet of Everything Digital Building Firmware
## 1. What is Ethernet of Everything Digital Building Firmware
With development support for nearly anything, Microchip’s Ethernet solutions build upon an established networking foundation, providing improved system performance, reduced latency, ease of installation and scaling, as well as the option to incorporate Power over Ethernet (PoE) for single-line power and communications.

## 2. System Recommendations
* MPLAB® X IDE v5.00 or later
* XC8 Compiler v2.00 (**PRO** version C90)
* TCP/IP Lite Stack v2.2.11

## 3. Documentation Support
* [AN1921-Microchip TCP/IP Lite Stack](http://ww1.microchip.com/downloads/en/AppNotes/Microchip-AN1921-8-bit-PICMCU-TCP-IP-LiteStack-ApplicationNote-00001921D.pdf)
* [AN2512-Microchip CoAP Stack](http://ww1.microchip.com/downloads/en/AppNotes/00002512A.pdf)
* [Cisco Digital Building Ecosystem & Partners](https://www.cisco.com/c/en/us/solutions/digital-ceiling/partner-ecosystem.html)
* [PIC18 PoE Development Kit User’s Guide](http://ww1.microchip.com/downloads/en/DeviceDoc/40001930A.pdf)

## 4. Repairs and Enhancements
* **Issues Fixed:**
1.	M8TS-679: DB version number should only be updated after a successful creation of the database.
2.	CAE_MCU8-5958: i2cWrite clock gets out of sequence
3.	CAE_MCU8-5898: EEPROM Loop to read MAC address bug
4.	M8TS-690: For Certain CoAP Packets an improper Ethernet FCS is being received.
5.	M8TS-688: DFU updateTime needs to be updated to 0 after flashing occurs and before Bootload starts.
* **Enhancements:**
1. Adjust Time was implemented so the user has the ability to adjust the percent intensity of an actuator over a set period of time.

## 5. Features
The User has the capability to add sensor resources (i.e. light, temperature, etc.) and actuator resources (i.e. lights, motors, etc.). The user has the ability to configure the number of resources desired. 

After the first initialization of resources, the user has the capability to dynamically allocate or deallocate sensor and/or actuator resources in a user defined application. This can be defined by the user in the main.c file.

Utilizing a Trivial File Transfer Protocol (TFTP) Server and both Device Firmware Download (DFD) and Device Firmware Upload (DFU) helper resources, the user has the capability to download a boot image from the server and update firmware of multiple EoE devices simultaneously via the Bootloader. The firmware version is automatically updated in DFD after issuing a PUT, it is the user’s responsibility to update the firmware version in the inventory.c file.

This firmware uses Constrained Application Protocol (CoAP) for communication purposes. Currently supported functions of CoAP are DISCOVER, GET, PUT and OBSERVE. POST and DELETE are currently unsupported. It must be noted that OBSERVE is still in the beta-testing phase and as such has not been fully tested. If the MTU is greater than allowed size, CoAP block-wise transfer is also implemented to handle this event. Currently only Block2 type is supported for block-wise transfer. 

## 6. Supported Platform
This firmware is designed to run independent of any specific platform, however specific driver requirements and memory sizes must be taken into account. 

### Minimum Memory Requirements XC8 compiler (PRO C90):
---
Configuration | Program Memory | Data Memory
--- | ---| ---|
IO Starter | 128 Kb | 4 Kb
Light | 128 Kb | 4 Kb 
Gestic | 128 Kb | 4 Kb
Pot Switch | 128 Kb | 4 Kb
Toggle Switch | 128 Kb | 4 Kb
---
## 7. Customer support
### 7.1 The Microchip Website
Microchip provides online support via our website at http://microchip.com. This website is used as a means to make files and information easily available to customers. Accessible by using your favorite Internet browser, the website contains the following information:
* Product Support – Data sheets and errata, application notes and sample programs, design resources, user’s guides and hardware support documents, latest software releases and archived software
* General Technical Support – Frequently Asked Questions (FAQs), technical support requests, online discussion groups/forums (http://forum.microchip.com), Microchip consultant program member listing
* Business of Microchip – Product selector and ordering guides, latest Microchip press releases, listing of seminars and events, listings of Microchip sales offices, distributors and factory representatives

Customers may also request support via the following email address eoe@microchip.com 
### 7.2 Additional Support
Users of Microchip products can receive assistance through several channels:
* Distributor or Representative
* Local Sales Office
* Field Application Engineering (FAE)
* Technical Support
Customers should contact their distributor, representative or field application engineer (FAE) for support. Local sales offices are also available to help customers. A listing of sales offices and locations is available on our website.

Technical support is available through the website at: http://support.microchip.com
