#include <SoftwareSerial.h> // Incluye la librería SoftwareSerial

// Define los pines para el SoftwareSerial
const uint8_t softRX = 4; // Uno PinD4(softRX)   NodeMcu Gpio0 = pinD3(softRX)
const uint8_t softTX = 2; // Uno PinD2(softTX)   NodeMcu Gpio2 = pinD4(softTX)
const uint16_t softSerialBautRate = 9600; // Velocidad del SoftwareSerial
const uint32_t hardSerialBautRate = 115200; // Velocidad del Hardware Serial

// Configuración del SoftwareSerial
SoftwareSerial softSerial(softRX, softTX);

//#pragma pack(1) // Definición del struct para manejar datos con directivas globales de empaquetamiento alineado a 1 byte sin relleno.
// Definición del struct para manejar datos con atributo especifico de empaquetamiento alineado a 1 byte sin relleno.
struct __attribute__((packed, aligned(1))) DataPacket {
  char charInit;            // Carácter de inicio del paquete
  uint8_t contador8bits;    // Contador de 8 bits = 1 Byte
  uint16_t contador16bits;  // Contador de 16 bits = 2 Bytes
  uint32_t contador32bits;  // Contador de 32 bits = 4 Bytes
  char caracter;            // Caracter ASCII = 8 bits = 1 Byte
  bool estado;              // Estado lógico = 8 bits = 1 Byte
  float valorFlotante;      // Número de punto flotante 32 bits = 4 Bytes
  uint8_t structSize;       // Tamaño del struct = 8 bits = 1 Byte
  uint8_t checksum;         // Para el checksum (1 Byte)
  uint16_t crc;             // Para el CRC (2 Bytes)
  char charEnd;             // Carácter de fin del paquete
}; // #pragma pack() // Restaura las directivas globales del empaquetamiento a su propio predeterminado.

// Variables globales para el struct
DataPacket dataEnvio = {'<', 0, 256, 65536, 'A', true, 0.00, sizeof(DataPacket), 0, 0, '>'}; // Inicialización explícita con caracteres de control
DataPacket dataRecibido = {'<', 0, 0, 0, ' ', false, 0.00, 0, 0, 0, '>'};   // Inicialización para recepción con caracteres de control

unsigned long timeLine0 = 0; // Variable para controlar el tiempo
const unsigned long intervalo = 500; // Intervalo de tiempo en milisegundos

// Prototipos de funciones
void setupInit();
void setupSoftSerial();
void setupEnd();
void calcularChecksum();
void calcularCRC();
void sendSoftSerial();
void readSoftSerial();
void handleSoftSerial();

void setup() {
  setupInit();       // Configuración inicial del hardware
  setupSoftSerial(); // Configuración del SoftwareSerial
  setupEnd();        // Mensaje de finalización del setup
}

void loop() {
  handleSoftSerial(); // Manejo de envío y recepción de datos
}

void setupInit() { // Configuración inicial de pines y Serial hardware
  Serial.begin(hardSerialBautRate, SERIAL_8N1);
  Serial.println(F("Puerto Hardware Serial: Iniciado Correctamente."));
  pinMode(softRX, INPUT);    // Configura el pin RX como entrada
  pinMode(softTX, OUTPUT);   // Configura el pin TX como salida
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println(F("Inicialización y configuración de pines completada."));
}

void setupSoftSerial() { // Inicialización del SoftwareSerial
  softSerial.begin(softSerialBautRate);
  Serial.println(F("Puerto Software Serial: Iniciado Correctamente."));
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

void sendSoftSerial() { // Enviar datos mediante SoftwareSerial
  if (millis() >= timeLine0 + intervalo) { // Comprueba si ha pasado el intervalo
    timeLine0 = millis(); // Actualiza el tiempo de referencia
    digitalWrite(LED_BUILTIN, HIGH); // Enciende el LED para indicar envío

    dataEnvio.structSize = sizeof(dataEnvio); // Actualiza el campo structSize con el tamaño actual del struct
    calcularChecksum(&dataEnvio); // Calcula y actualiza el checksum
    calcularCRC(&dataEnvio);      // Calcula y actualiza el CRC
    
    softSerial.write(reinterpret_cast<uint8_t*>(&dataEnvio), sizeof(dataEnvio)); // Envía el struct como un bloque de bytes

    // Imprime los datos enviados en varios formatos
    Serial.println(F("Datos enviados:"));
    Serial.print(F("Inicio: ")); Serial.print(dataEnvio.charInit); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.charInit, HEX); Serial.println(F(")"));
    Serial.print(F("8 bits: ")); Serial.print(dataEnvio.contador8bits); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.contador8bits, HEX); Serial.println(F(")"));
    Serial.print(F("16 bits: ")); Serial.print(dataEnvio.contador16bits); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.contador16bits, HEX); Serial.println(F(")"));
    Serial.print(F("32 bits: ")); Serial.print(dataEnvio.contador32bits); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.contador32bits, HEX); Serial.println(F(")"));
    Serial.print(F("Caracter: ")); Serial.print(dataEnvio.caracter); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.caracter, HEX); Serial.println(F(")"));
    Serial.print(F("Estado (bool): ")); Serial.print(dataEnvio.estado ? "true" : "false"); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.estado, HEX); Serial.println(F(")"));
    Serial.print(F("Valor Flotante: ")); Serial.print(dataEnvio.valorFlotante); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.valorFlotante, HEX); Serial.println(F(")"));
    Serial.print(F("Tamaño del paquete: ")); Serial.print(dataEnvio.structSize); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.structSize, HEX); Serial.println(F(")"));
    Serial.print(F("CheckSum del paquete: ")); Serial.print(dataEnvio.checksum); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.checksum, HEX); Serial.println(F(")"));
    Serial.print(F("CRC del paquete: ")); Serial.print(dataEnvio.crc); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.crc, HEX); Serial.println(F(")"));
    Serial.print(F("Fin: ")); Serial.print(dataEnvio.charEnd); Serial.print(F(" (Hex: 0x")); Serial.print(dataEnvio.charEnd, HEX); Serial.println(F(")"));
    Serial.println(F(" "));

    // Incrementa los contadores con desbordamiento
    dataEnvio.contador8bits++;
    dataEnvio.contador16bits++;
    dataEnvio.contador32bits++;
    dataEnvio.caracter = (dataEnvio.caracter >= 'Z') ? 'A' : dataEnvio.caracter + 1; // Rota entre 'A' y 'Z'
    dataEnvio.estado = !dataEnvio.estado; // Alterna el estado lógico
    dataEnvio.valorFlotante += 0.01; // Incrementa ligeramente el valor flotante

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
        (dataRecibido.crc == crcCalculado)) { // Quinta validación: calcular CRC

      Serial.println(F("Datos recibidos correctamente:"));
      // Imprime los datos recibidos en varios formatos
      Serial.println(F("Datos recibidos:"));
      Serial.print(F("Inicio: ")); Serial.print(dataRecibido.charInit); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.charInit, HEX); Serial.println(F(")"));
      Serial.print(F("8 bits: ")); Serial.print(dataRecibido.contador8bits); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.contador8bits, HEX); Serial.println(F(")"));
      Serial.print(F("16 bits: ")); Serial.print(dataRecibido.contador16bits); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.contador16bits, HEX); Serial.println(F(")"));
      Serial.print(F("32 bits: ")); Serial.print(dataRecibido.contador32bits); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.contador32bits, HEX); Serial.println(F(")"));
      Serial.print(F("Caracter: ")); Serial.print(dataRecibido.caracter); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.caracter, HEX); Serial.println(F(")"));
      Serial.print(F("Estado (bool): ")); Serial.print(dataRecibido.estado ? "true" : "false"); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.estado, HEX); Serial.println(F(")"));
      Serial.print(F("Valor Flotante: ")); Serial.print(dataRecibido.valorFlotante); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.valorFlotante, HEX); Serial.println(F(")"));
      Serial.print(F("Tamaño del paquete: ")); Serial.print(dataRecibido.structSize); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.structSize, HEX); Serial.println(F(")"));
      Serial.print(F("CheckSum del paquete: ")); Serial.print(dataRecibido.checksum); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.checksum, HEX); Serial.println(F(")"));
      Serial.print(F("CRC del paquete: ")); Serial.print(dataRecibido.crc); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.crc, HEX); Serial.println(F(")"));
      Serial.print(F("Fin: ")); Serial.print(dataRecibido.charEnd); Serial.print(F(" (Hex: 0x")); Serial.print(dataRecibido.charEnd, HEX); Serial.println(F(")"));
      Serial.println(F(" "));
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
      }
      if (dataRecibido.crc != crcCalculado) { // Error: CRC No Corresponde
        Serial.println(F("Error: CRC incorrecto."));
        Serial.print(F("CRC Declarado Recibido: ")); Serial.println(dataRecibido.crc);
        Serial.print(F("CRC Calculado: ")); Serial.println(crcCalculado);
      }
    
    }
  }
}

void handleSoftSerial() { // Manejo del Software Serial (envío y recepción)
  sendSoftSerial();
  readSoftSerial();
}
