# softSerialComm
Comunicación Entre Placas de Arquitecturas Diferentes 
  
###### <br> "a hombros de gigantes"  
###### <br>"la informacion que no es compartida en tiempo y forma, simplemente muere."  
###### <br>  
Un Programa Bidireccional por emulacion UART  
para comunicar dos placas de arquitecturas diferentes,  
brindando un puente de control y monitorizacion de sus pines,    
sincronizado, no bloqueante y, totalmente simetrico y estructurado.  

___________________________________________________________________  
Diagrama de conexión:  
Arduino Uno - - - - Level Shifter - - - - NodeMCU8266  
(5V) <------------> ref Higth Voltage <-----------> Vin (5v.)  
(3.3V) <-----------> ref Low Voltage <-----------> Vcc(3.3V)  
(GND) <-----------------> GND <-------------------> (GND)  
softTXpinD2 <---> HV1 <--| |--> LV1 <---> softRXgpio0(D3)  
softRXpinD4 <---> HV3 <--| |--> LV3 <---> softTXgpio2(D4)  
___________________________________________________________________  
_Son bienvenidos los Aportes, Colaboraciones, y Consejos._  
_GRACIAS._  
___________________________________________________________________  
  + __Trabajando :__  
    - [ ] Reservado. (evaluando posibles cambios y mejoras necesarias) [V-0.1.3]()  
    - [x] Se Implementa, Maquina de Estados y Buffer FiFo Circular Doble. [V-0.1.2](https://github.com/YoCharlyZ/softSerialComm/tree/5fec22abdf0df6faf1920335497d9f37900d6f18)  
    - [x] Se Reestrucctura el proyecto completo adecuandolo a los fines. [V-0.1.1](https://github.com/YoCharlyZ/softSerialComm/tree/87d0dcdc147cba6d6bb18bc3c74963ef7ce72394)  
    - [x] Se Extraen y Añaden los Datos como Variables Independientes Compartidas. [V-0.1.0](https://github.com/YoCharlyZ/softSerialComm/tree/1887d0b892f3ad211064d3d358e6b1861913d55d)  
    - [x] Se Implementa un Header con remitenteID, destinatarioID, multidifusionID. [V-0.0.9](https://github.com/YoCharlyZ/softSerialComm/tree/c91d917d4d309e035c1601fa0055fcb9134862db)  
    - [x] Se Implementan Algoritmos para Calcular CheckSum y CRC. [V-0.0.8](https://github.com/YoCharlyZ/softSerialComm/tree/e851ccc7cf4776808f693cebe5f48562ee6db417)  
    - [x] Mejorar la Robustez y Fiabilidad de los Datos. [V-0.0.7](https://github.com/YoCharlyZ/softSerialComm/tree/b039c206649030fab34b0191d344d7698cd7c501)  
    - [x] Enviar y Recibir 1 Struct con mas cantidad de datos primitivos. [V-0.0.6](https://github.com/YoCharlyZ/softSerialComm/tree/14a7c725aa79f98a9d7cfd7cac1b5d1be97c1671)  
    - [x] Enviar y Recibir 1 Struct con multiples miembros. [V-0.0.5](https://github.com/YoCharlyZ/softSerialComm/tree/15d5f5c5254ac4e6eac2c732f2015d3620aad8f0)  
    - [x] Enviar y Recibir 1 Struct con 4 Bytes = uint32_t. [V-0.0.4](https://github.com/YoCharlyZ/softSerialComm/tree/c544f7bdef98ba38f00b83f5f4f0e7d0362cb146)  
    - [x] Enviar y Recibir 1 Struct con 2 Bytes = uint16_t. [V-0.0.3](https://github.com/YoCharlyZ/softSerialComm/tree/7cf0bfdd02d89aa4e187c9cc02cf86e6d8d6c69d)  
    - [x] Enviar y Recibir 1 Struct con 1 Byte = uint8_t. [V-0.0.2](https://github.com/YoCharlyZ/softSerialComm/tree/18c2fcd0e1deb2451b9af7b2064d4ec3ff305fda)  
    - [x] Enviar y Recibir 1 Byte = uint8_t. [V-0.0.1](https://github.com/YoCharlyZ/softSerialComm/tree/7a434fc2e448347123941b75455a69b3c336e96a)    
    - [x] Trabajando Comunicacion Software Serial. [V-0.0.0](https://github.com/YoCharlyZ/softSerialComm/tree/887bb294da2a93624801aa9b457df3904cadd03a)  
___________________________________________________________________  
