#include <SoftwareSerial.h> // Incluye la librería SoftwareSerial
// Define los pines para el SoftwareSerial
const uint8_t softRX = 0; // Uno gpio0(softRX) 
const uint8_t softTX = 2; // Uno gpio2(softTX) 
const uint16_t softSerialBautRate = 9600; // Velocidad del SoftwareSerial
const uint32_t hardSerialBautRate = 115200; // Velocidad del Hardware Serial
const uint8_t targetID = 0; // Un adressID especial reservado para algun fin especifico.
const uint8_t mySlaveID = 10; // Mi adressID de Esclavo
const uint8_t toMasterID = 8; // El adressID de al menos un Maestro Uno
const uint8_t broadcastID = 255; // Un ID para MultiDifusion
const unsigned long intervalo = 500; // Intervalo de tiempo en milisegundos
unsigned long timeLine0 = 0; // Variable para controlar el tiempo
// Configuración del SoftwareSerial
SoftwareSerial softSerial(softRX, softTX);

// Definición de constantes para los pines analógicos y digitales
// gpio17

// Variables Independientes Internaspara almacenar los valores de los pines analógicos y digitales
bool master_loadConfig = false;
bool slave_loadConfig = false;
bool flag_restart = true;

char valCharInit = '<';
float valPinA0 = 000.00, valPinA1 = 000.00, valPinA2 = 000.00, valPinA3 = 000.00, valPinA4 = 000.00, valPinA5 = 000.00;
uint8_t /*valPinD0 = 0, valPinD1 = 0, valPinD2 = 0,*/ valPinD3 = 0, /*valPinD4 = 0,*/ valPinD5 = 0;
uint8_t valPinD6 = 0, valPinD7 = 0, valPinD8 = 0, valPinD9 = 0, valPinD10 = 0, valPinD11 = 0, valPinD12 = 0, valPinD13 = 0;
char valCharEnd = '>';

//#pragma pack(1) // Definición del struct para manejar datos con directivas globales de empaquetamiento alineado a 1 byte sin relleno.
// Definición del struct para manejar datos con atributo especifico de empaquetamiento alineado a 1 byte sin relleno.
struct __attribute__((packed, aligned(1))) DataPacket {
  char charInit;            // Carácter de inicio del paquete
  uint8_t sourceID;         // ID de dispositivo remitente de origen
  uint8_t destinationID;    // ID de dispositivo destinatario
  
  bool varMasterLoadConfig; // Estado lógico = 8 bits = 1 Byte
  bool varSlaveLoadConfig; // Estado lógico = 8 bits = 1 Byte
  bool varFlagRestart; // Estado lógico = 8 bits = 1 Byte
  float varPinA0, varPinA1, varPinA2, varPinA3, varPinA4, varPinA5; // [(un solo float = 32 bits = 4 Bytes) x6] = 24 Bytes
  uint8_t /*varPinD0, varPinD1, varPinD2,*/ varPinD3, /*varPinD4,*/ varPinD5; // [(8 bits = 1 Byte) x2] = 2 Bytes
  uint8_t varPinD6, varPinD7, varPinD8, varPinD9, varPinD10, varPinD11, varPinD12, varPinD13; // [(8 bits = 1 Byte) x8] = 8 Bytes
  
  uint8_t structSize;       // Tamaño del struct = 8 bits = 1 Byte
  uint8_t checksum;         // Para el checksum (1 Byte)
  uint16_t crc;             // Para el CRC (2 Bytes)
  char charEnd;             // Carácter de fin del paquete
}; // #pragma pack() // Restaura las directivas globales del empaquetamiento a su propio predeterminado.

// Variables globales para el struct
DataPacket dataEnvio = {'<', 0, 0, false, false, true, 000.00, 000.00, 000.00, 000.00, 000.00, 000.00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, sizeof(DataPacket), 0, 0, '>'}; // Inicialización explícita con caracteres de control
DataPacket dataRecibido = {'<', 0, 0, true, true, false, 000.00, 000.00, 000.00, 000.00, 000.00, 000.00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '>'}; // Inicialización para recepción con caracteres de control

// Prototipos de funciones
void setupInit();
void setupHardSerial();
void setupSoftSerial();
void setupConfigPins();
void setupEnd();
void calcularChecksum();
void calcularCRC();
void printSending();
void printReceived();
void prinVarsInternal();
void updateStruct();
void updateVars();
void rwDataPins();
void sendSoftSerial();
void readSoftSerial();
void handleSoftSerial();

void setup() {
  setupInit();       // Configuración inicial del hardware
  setupEnd();        // Mensaje de finalización del setup
}

void loop() {
  handleSoftSerial(); // Manejo de envío y recepción de datos
}

void setupInit() { // Configuración inicial de hardware
  setupHardSerial(); // Configuración del HardwareSerial
  setupSoftSerial(); // Configuración del SoftwareSerial
  setupConfigPins(); // inicializacion de Pines
}
void setupHardSerial(){ // Inicialización del SoftwareSerial
  Serial.begin(hardSerialBautRate, SERIAL_8N1);
  Serial.println(F("Puerto Hardware Serial: Iniciado Correctamente."));
}
void setupSoftSerial() { // Inicialización del SoftwareSerial
  pinMode(softRX, INPUT);    // Configura el pin RX como entrada
  pinMode(softTX, OUTPUT);   // Configura el pin TX como salida
  Serial.println(F("Inicialización y configuración de pines Software Serial completada."));
  softSerial.begin(softSerialBautRate);
  Serial.println(F("Puerto Software Serial: Iniciado Correctamente."));
}
void setupConfigPins(){ // inicializacion de Pines
  
  //gpio17
  
  Serial.println(F("Inicialización y configuración de pines completada."));
  
}
void setupEnd() { // Finalización de la configuración

  digitalWrite(LED_BUILTIN, HIGH);

  Serial.println(F("Gracias por esperar."));
  digitalWrite(LED_BUILTIN, LOW);
}

void calcularChecksum(DataPacket* paquete) { // Calcula el checksum y lo almacena directamente en el paquete
  uint8_t suma = 0;
  uint8_t* ptr = reinterpret_cast<uint8_t*>(paquete);
  for (size_t i = 0; i < sizeof(DataPacket) - 3; i++) { // Excluye checksum y CRC
    suma += ptr[i];
  }
  paquete->checksum = suma; // Actualiza el campo checksum
}
void calcularCRC(DataPacket* paquete) { // Calcula el CRC y lo almacena directamente en el paquete
  uint16_t crc = 0xFFFF; // Valor inicial
  uint8_t* ptr = reinterpret_cast<uint8_t*>(paquete);
  for (size_t i = 0; i < sizeof(DataPacket) - 2; i++) { // Excluye el campo CRC
    crc ^= (uint16_t)ptr[i] << 8;
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) crc = (crc << 1) ^ 0x8005;
      else crc = crc << 1;
    }
  }
  paquete->crc = crc; // Actualiza el campo CRC
}

void printSending() { // Imprime los datos enviados en varios formatos
  Serial.println(F("Datos enviados:"));
  Serial.print(F("Inicio: ")); Serial.print(dataEnvio.charInit); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.charInit, HEX); Serial.println(F(")"));
  Serial.print(F("SourceID: ")); Serial.print(dataEnvio.sourceID); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.sourceID, HEX); Serial.println(F(")"));
  Serial.print(F("DestinationID: ")); Serial.print(dataEnvio.destinationID); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.destinationID, HEX); Serial.println(F(")"));
  
  Serial.print(F("Estado del Maestro: ")); Serial.println(dataEnvio.varMasterLoadConfig ? "true" : "false");
  Serial.print(F("Estado del Esclavo: ")); Serial.println(dataEnvio.varSlaveLoadConfig ? "true" : "false");
  Serial.print(F("Estado de Reinicio: ")); Serial.println(dataEnvio.varFlagRestart ? "true" : "false");
  
  Serial.println(F("Valores Pines Analogicos: "));
  Serial.print(F("A0 ")); Serial.print(dataEnvio.varPinA0); 
  Serial.print(F(", A1 ")); Serial.print(dataEnvio.varPinA1); 
  Serial.print(F(", A2 ")); Serial.print(dataEnvio.varPinA2); 
  Serial.print(F(", A3 ")); Serial.print(dataEnvio.varPinA3);
  Serial.print(F(", A4 ")); Serial.print(dataEnvio.varPinA4); 
  Serial.print(F(", A5 ")); Serial.print(dataEnvio.varPinA5); 
  Serial.println(F(". ")); 
  
  Serial.println(F("Valores Pines Digitales: "));
  //Serial.print(F("D0 ")); Serial.print(dataEnvio.varPinD0); 
  //Serial.print(F(", D1 ")); Serial.print(dataEnvio.varPinD1); 
  //Serial.print(F(", D2 ")); Serial.print(dataEnvio.varPinD2); 
  Serial.print(F("D3 ")); Serial.print(dataEnvio.varPinD3); 
  //Serial.print(F(", D4 ")); Serial.print(dataEnvio.varPind4);
  Serial.print(F(", D5 ")); Serial.print(dataEnvio.varPinD5); 
  Serial.print(F(", D6 ")); Serial.print(dataEnvio.varPinD6);
  Serial.print(F(", D7 ")); Serial.print(dataEnvio.varPinD7); 
  Serial.print(F(", D8 ")); Serial.print(dataEnvio.varPinD8);
  Serial.print(F(", D9 ")); Serial.print(dataEnvio.varPinD9);
  Serial.print(F(", D10 ")); Serial.print(dataEnvio.varPinD10);
  Serial.print(F(", D11 ")); Serial.print(dataEnvio.varPinD11); 
  Serial.print(F(", D12 ")); Serial.print(dataEnvio.varPinD12); 
  Serial.print(F(", D13 ")); Serial.print(dataEnvio.varPinD13); 
  Serial.println(F(". "));
  
  Serial.print(F("Tamaño del paquete: ")); Serial.print(dataEnvio.structSize); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.structSize, HEX); Serial.println(F(")"));
  Serial.print(F("CheckSum del paquete: ")); Serial.print(dataEnvio.checksum); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.checksum, HEX); Serial.println(F(")"));
  Serial.print(F("CRC del paquete: ")); Serial.print(dataEnvio.crc); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.crc, HEX); Serial.println(F(")"));
  Serial.print(F("Fin: ")); Serial.print(dataEnvio.charEnd); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.charEnd, HEX); Serial.println(F(")"));
  Serial.println(F(" "));
}
void printReceived() { // Imprime los datos recibidos en varios formatos
  Serial.println(F("Datos recibidos:"));
  Serial.print(F("Inicio: ")); Serial.print(dataRecibido.charInit); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.charInit, HEX); Serial.println(F(")"));
  Serial.print(F("SourceID: ")); Serial.print(dataRecibido.sourceID);  Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.sourceID, HEX); Serial.println(F(")"));
  Serial.print(F("DestinationID: ")); Serial.print(dataRecibido.destinationID); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.destinationID, HEX); Serial.println(F(")"));
  
  Serial.print(F("Estado del Maestro: ")); Serial.println(dataRecibido.varMasterLoadConfig ? "true" : "false");
  Serial.print(F("Estado del Esclavo: ")); Serial.println(dataRecibido.varSlaveLoadConfig ? "true" : "false");
  Serial.print(F("Estado de Reinicio: ")); Serial.println(dataRecibido.varFlagRestart ? "true" : "false");
  
  Serial.println(F("Valores Pines Analogicos: "));
  Serial.print(F("A0 ")); Serial.print(dataRecibido.varPinA0); 
  Serial.print(F(", A1 ")); Serial.print(dataRecibido.varPinA1); 
  Serial.print(F(", A2 ")); Serial.print(dataRecibido.varPinA2); 
  Serial.print(F(", A3 ")); Serial.print(dataRecibido.varPinA3);
  Serial.print(F(", A4 ")); Serial.print(dataRecibido.varPinA4); 
  Serial.print(F(", A5 ")); Serial.print(dataRecibido.varPinA5); 
  Serial.println(F(". ")); 
  
  Serial.println(F("Valores Pines Digitales: "));
  //Serial.print(F("D0 ")); Serial.print(dataRecibido.varPinD0); 
  //Serial.print(F(", D1 ")); Serial.print(dataRecibido.varPinD1); 
  //Serial.print(F(", D2 ")); Serial.print(dataRecibido.varPinD2); 
  Serial.print(F("D3 ")); Serial.print(dataRecibido.varPinD3); 
  //Serial.print(F(", D4 ")); Serial.print(dataRecibido.varPind4);
  Serial.print(F(", D5 ")); Serial.print(dataRecibido.varPinD5); 
  Serial.print(F(", D6 ")); Serial.print(dataRecibido.varPinD6);
  Serial.print(F(", D7 ")); Serial.print(dataRecibido.varPinD7); 
  Serial.print(F(", D8 ")); Serial.print(dataRecibido.varPinD8);
  Serial.print(F(", D9 ")); Serial.print(dataRecibido.varPinD9);
  Serial.print(F(", D10 ")); Serial.print(dataRecibido.varPinD10);
  Serial.print(F(", D11 ")); Serial.print(dataRecibido.varPinD11); 
  Serial.print(F(", D12 ")); Serial.print(dataRecibido.varPinD12); 
  Serial.print(F(", D13 ")); Serial.print(dataRecibido.varPinD13); 
  Serial.println(F(". ")); 
  
  Serial.print(F("Tamaño del paquete: ")); Serial.print(dataRecibido.structSize); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.structSize, HEX); Serial.println(F(")"));
  Serial.print(F("CheckSum del paquete: ")); Serial.print(dataRecibido.checksum); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.checksum, HEX); Serial.println(F(")"));
  Serial.print(F("CRC del paquete: ")); Serial.print(dataRecibido.crc); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.crc, HEX); Serial.println(F(")"));
  Serial.print(F("Fin: ")); Serial.print(dataRecibido.charEnd); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.charEnd, HEX); Serial.println(F(")"));
  Serial.println(F(" "));
}
void printVarsInternal() { // Imprime los datos de las variables intenas en varios formatos
  
  Serial.println(F("Datos de Variables Independientes Internas:"));
  Serial.print(F("Inicio: ")); Serial.println(valCharInit);
  Serial.print(F("Estado del Maestro: ")); Serial.println(master_loadConfig ? "true" : "false");
  Serial.print(F("Estado del Esclavo: ")); Serial.println(slave_loadConfig ? "true" : "false");
  Serial.print(F("Estado de Reinicio: ")); Serial.println(flag_restart ? "true" : "false");
  
  Serial.println(F("Valores Pines Analogicos: "));
  Serial.print(F("A0 ")); Serial.print(valPinA0); 
  Serial.print(F(", A1 ")); Serial.print(valPinA1); 
  Serial.print(F(", A2 ")); Serial.print(valPinA2); 
  Serial.print(F(", A3 ")); Serial.print(valPinA3);
  Serial.print(F(", A4 ")); Serial.print(valPinA4); 
  Serial.print(F(", A5 ")); Serial.print(valPinA5); 
  Serial.println(F(". ")); 
  
  Serial.println(F("Valores Pines Digitales: "));
  //Serial.print(F("D0 ")); Serial.print(valPinD0); 
  //Serial.print(F(", D1 ")); Serial.print(valPinD1); 
  //Serial.print(F(", D2 ")); Serial.print(valPinD2); 
  Serial.print(F("D3 ")); Serial.print(valPinD3); 
  //Serial.print(F(", D4 ")); Serial.print(valPind4);
  Serial.print(F(", D5 ")); Serial.print(valPinD5); 
  Serial.print(F(", D6 ")); Serial.print(valPinD6);
  Serial.print(F(", D7 ")); Serial.print(valPinD7); 
  Serial.print(F(", D8 ")); Serial.print(valPinD8);
  Serial.print(F(", D9 ")); Serial.print(valPinD9);
  Serial.print(F(", D10 ")); Serial.print(valPinD10);
  Serial.print(F(", D11 ")); Serial.print(valPinD11); 
  Serial.print(F(", D12 ")); Serial.print(valPinD12); 
  Serial.print(F(", D13 ")); Serial.print(valPinD13); 
  Serial.println(F(". ")); 
  Serial.print(F("Fin: ")); Serial.println(valCharEnd);
  Serial.println(F(" "));

}

void updateStruct() { // Función para actualizar el struct desde las variables independientes

  // Asigna identificadores de origen, destino y tamaño.
  dataEnvio.sourceID = mySlaveID; // El remitente es este dispositivo
  dataEnvio.destinationID = toMasterID; // El destinatario es un Maestro específico
  // dataEnvio.destinationID = broadcastID; // El destinatario es MultiDifusión)

  dataEnvio.varMasterLoadConfig = master_loadConfig;
  dataEnvio.varSlaveLoadConfig = slave_loadConfig;
  dataEnvio.varFlagRestart = flag_restart;

  dataEnvio.varPinA0 = valPinA0;
  dataEnvio.varPinA1 = valPinA1;
  dataEnvio.varPinA2 = valPinA2;
  dataEnvio.varPinA3 = valPinA3;
  dataEnvio.varPinA4 = valPinA4;
  dataEnvio.varPinA5 = valPinA5;

  //dataEnvio.varPinD0 = valPinD0;
  //dataEnvio.varPinD1 = valPinD1;
  //dataEnvio.varPinD2 = valPinD2;
  dataEnvio.varPinD3 = valPinD3;
  //dataEnvio.varPinD4 = valPinD4;
  dataEnvio.varPinD5 = valPinD5;
  dataEnvio.varPinD6 = valPinD6;
  dataEnvio.varPinD7 = valPinD7;
  dataEnvio.varPinD8 = valPinD8;
  dataEnvio.varPinD9 = valPinD9;
  dataEnvio.varPinD10 = valPinD10;
  dataEnvio.varPinD11 = valPinD11;
  dataEnvio.varPinD12 = valPinD12;
  dataEnvio.varPinD13 = valPinD13;

  dataEnvio.structSize = sizeof(dataEnvio); // Actualiza el campo structSize con el tamaño actual del struct
  calcularChecksum(&dataEnvio); // Calcula y actualiza el checksum
  calcularCRC(&dataEnvio);      // Calcula y actualiza el CRC

}
void updateVars() { // Función para actualizar las variables independientes desde el struct recibido
  
  master_loadConfig = dataRecibido.varMasterLoadConfig;  
  slave_loadConfig = dataRecibido.varSlaveLoadConfig;  
  flag_restart = dataRecibido.varFlagRestart;  

  valPinA0 = dataRecibido.varPinA0;  
  valPinA1 = dataRecibido.varPinA1;  
  valPinA2 = dataRecibido.varPinA2;  
  valPinA3 = dataRecibido.varPinA3;  
  valPinA4 = dataRecibido.varPinA4;  
  valPinA5 = dataRecibido.varPinA5;  

  //valPinD0 = dataRecibido.varPinD0;
  //valPinD1 = dataRecibido.varPinD1;
  //valPinD2 = dataRecibido.varPinD2;
  valPinD3 = dataRecibido.varPinD3; 
  //valPinD4 = dataRecibido.varPinD4;
  valPinD5 = dataRecibido.varPinD5;
  valPinD6 = dataRecibido.varPinD6;
  valPinD7 = dataRecibido.varPinD7;
  valPinD8 = dataRecibido.varPinD8;
  valPinD9 = dataRecibido.varPinD9;
  valPinD10 = dataRecibido.varPinD10;
  valPinD11 = dataRecibido.varPinD11;
  valPinD12 = dataRecibido.varPinD12;
  valPinD13 = dataRecibido.varPinD13;

}
void rwDataPins(){ // Lee y escribe correspondientemente los pines segun las variables globales
  
  //gpio17

}

void sendSoftSerial() { // Enviar datos mediante SoftwareSerial

  if (millis() >= timeLine0 + intervalo) { // Comprueba si ha pasado el intervalo

    timeLine0 = millis(); // Actualiza el tiempo de referencia

    digitalWrite(LED_BUILTIN, HIGH); // Enciende el LED para indicar envío

    updateStruct(); // Actualiza el struct con las variables antes de enviar

    softSerial.write(reinterpret_cast<uint8_t*>(&dataEnvio), sizeof(dataEnvio)); // Envía el struct como un bloque de bytes

    printSending(); // Imprime los datos de las variables enviadas

    rwDataPins(); // Lee y escribe correspondientemente los pines segun las variables globales

    digitalWrite(LED_BUILTIN, LOW); // Apaga el LED tras el envío
  }
}
void readSoftSerial() { // Leer datos recibidos mediante SoftwareSerial

  if (softSerial.available() >= sizeof(dataRecibido)) { // Comprueba si hay suficientes datos para leer

    // Lee los bytes del SoftwareSerial al struct
    softSerial.readBytes(reinterpret_cast<uint8_t*>(&dataRecibido), sizeof(dataRecibido));

    // Valida el checksum y el CRC
    uint8_t checksumCalculado;
    uint16_t crcCalculado;

    calcularChecksum(&dataRecibido);
    calcularCRC(&dataRecibido);

    checksumCalculado = dataRecibido.checksum; // Checksum ya calculado
    crcCalculado = dataRecibido.crc;          // CRC ya calculado

    if ((dataRecibido.charInit == '<') && // Primera validación: caracteres de control
        (dataRecibido.charEnd == '>') && // Segunda validación: caracteres de control
        (dataRecibido.structSize == sizeof(dataRecibido)) && // Tercera validación: comparar tamaño declarado con el tamaño real
        (dataRecibido.checksum == checksumCalculado) && // Cuarta validación: calcular Checksum
        (dataRecibido.crc == crcCalculado) &&  // Quinta validación: calcular CRC
        (dataRecibido.destinationID == mySlaveID || dataRecibido.destinationID == broadcastID)) { // Sexta validación: Verificar el destinatario especifico o multidifusion

      printReceived(); // Imprime los datos de las variables recibidas
      updateVars(); // Actualiza las variables independientes después de recibir
      printVarsInternal(); // Imprime los datos de las variables intenas
      rwDataPins(); // Lee y escribe correspondientemente los pines segun las variables globales

    } else { // Manejo de Errores.
      if (dataRecibido.charInit != '<') { // Error: Caracteres de control inválido
        Serial.println(F("Error: Caracteres de control inválidos."));
        Serial.print(F("Caracter Inicio Recibido: ")); Serial.println(dataRecibido.charInit);
        Serial.print(F("Bytes Recibidos: ")); Serial.println(dataRecibido.structSize);
        Serial.println(F(" "));
      }
      if (dataRecibido.charEnd != '>') { // Error: Caracteres de control inválido
        Serial.println(F("Error: Caracteres de control inválidos."));
        Serial.print(F("Caracter Fin Recibido: ")); Serial.println(dataRecibido.charEnd);
        Serial.print(F("Bytes Recibidos: ")); Serial.println(dataRecibido.structSize);
        Serial.println(F(" "));
      }
      if (dataRecibido.structSize != sizeof(dataRecibido)) { // Error: Tamaño declarado no coincide con el real
        Serial.println(F("Error: Tamaño del paquete no coincide con el declarado."));
        Serial.print(F("Tamaño Real del Struct: ")); Serial.println(sizeof(dataRecibido));
        Serial.print(F("Tamaño Declarado Recibido: ")); Serial.println(dataRecibido.structSize);
        Serial.print(F("Bytes Recibidos: ")); Serial.println(dataRecibido.structSize);
        Serial.println(F(" "));
      }
      if (dataRecibido.checksum != checksumCalculado) { // Error: Checksum No Corresponde
        Serial.println(F("Error: Checksum incorrecto."));
        Serial.print(F("Checksum Declarado Recibido: ")); Serial.println(dataRecibido.checksum);
        Serial.print(F("Checksum Calculado: ")); Serial.println(checksumCalculado);
        Serial.println(F(" "));
      }
      if (dataRecibido.crc != crcCalculado) { // Error: CRC No Corresponde
        Serial.println(F("Error: CRC incorrecto."));
        Serial.print(F("CRC Declarado Recibido: ")); Serial.println(dataRecibido.crc);
        Serial.print(F("CRC Calculado: ")); Serial.println(crcCalculado);
        Serial.println(F(" "));
      }
      if (dataRecibido.destinationID != mySlaveID &&
          dataRecibido.destinationID != toMasterID &&
          dataRecibido.destinationID != broadcastID &&
          dataRecibido.destinationID != targetID) { // Error: DispositivoID No Reconocido
        Serial.println(F("Error: El ID recibido no está en la lista de IDs válidos."));
        Serial.print(F("ID Recibido: ")); Serial.println(dataRecibido.destinationID);
        Serial.println(F(" "));
      }
    }
  }
}

void handleSoftSerial() { // Manejo del Software Serial (envío y recepción)
  sendSoftSerial();
  readSoftSerial();
}
