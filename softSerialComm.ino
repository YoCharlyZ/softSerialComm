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
  uint8_t contador8bits;   // Contador de 8 bits
  uint16_t contador16bits; // Contador de 16 bits
  uint32_t contador32bits; // Contador de 32 bits
}; // #pragma pack() // Restaura las directivas globales del empaquetamiento a su propio predeterminado.

// Variables globales para el struct
DataPacket dataEnvio = {0, 256, 65536};  // Inicialización explícita
DataPacket dataRecibido = {0, 0, 0};     // Inicialización explícita

unsigned long timeLine0 = 0; // Variable para controlar el tiempo
const unsigned long intervalo = 500; // Intervalo de tiempo en milisegundos

// Prototipos de funciones
void setupInit();
void setupSoftSerial();
void setupEnd();
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

void sendSoftSerial() { // Enviar datos mediante SoftwareSerial
  if (millis() >= timeLine0 + intervalo) { // Comprueba si ha pasado el intervalo
    timeLine0 = millis(); // Actualiza el tiempo de referencia
    digitalWrite(LED_BUILTIN, HIGH); // Enciende el LED para indicar envío

    // Envía el struct como un bloque de bytes
    size_t structSize = sizeof(dataEnvio);
    softSerial.write(reinterpret_cast<uint8_t*>(&dataEnvio), structSize);

    // Imprime los datos enviados en varios formatos
    Serial.println(F("Datos enviados:"));
    Serial.print(F("8 bits: ")); Serial.println(dataEnvio.contador8bits);
    Serial.print(F("16 bits: ")); Serial.println(dataEnvio.contador16bits);
    Serial.print(F("32 bits: ")); Serial.println(dataEnvio.contador32bits);
    Serial.print(F("Bytes Enviados: ")); Serial.println(structSize); // Imprime el tamaño del struct
    Serial.println(F(" "));

    // Incrementa los contadores con desbordamiento
    dataEnvio.contador8bits++;
    dataEnvio.contador16bits++;
    dataEnvio.contador32bits++;

    digitalWrite(LED_BUILTIN, LOW); // Apaga el LED tras el envío
  }
}

void readSoftSerial() { // Leer datos recibidos mediante SoftwareSerial
  if (softSerial.available() >= sizeof(dataRecibido)) { // Comprueba si hay suficientes datos para leer
    size_t structSize = sizeof(dataRecibido);
    softSerial.readBytes(reinterpret_cast<uint8_t*>(&dataRecibido), structSize);

    // Imprime los datos recibidos en varios formatos
    Serial.println(F("Datos recibidos:"));
    Serial.print(F("8 bits: ")); Serial.println(dataRecibido.contador8bits);
    Serial.print(F("16 bits: ")); Serial.println(dataRecibido.contador16bits);
    Serial.print(F("32 bits: ")); Serial.println(dataRecibido.contador32bits);
    Serial.print(F("Bytes Recibidos: ")); Serial.println(structSize); // Imprime el tamaño del struct
    Serial.println(F(" "));
  }
}

void handleSoftSerial() { // Manejo del Software Serial (envío y recepción)
  sendSoftSerial();
  readSoftSerial();
}
