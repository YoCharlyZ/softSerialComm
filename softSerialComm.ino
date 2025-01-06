#include <SoftwareSerial.h> // Incluye la librería SoftwareSerial

// Define los pines para el SoftwareSerial
const uint8_t softRX = 0; // Uno PinD4(softRX)   NodeMcu Gpio0 = pinD3(softRX)
const uint8_t softTX = 2; // Uno PinD2(softTX)   NodeMcu Gpio2 = pinD4(softTX)
const uint16_t softSerialBautRate = 9600; // Velocidad del Software Serial
const uint32_t hardSerialBautRate = 115200; // Velocidad del Hardware Serial

// Configuración del Software Serial
SoftwareSerial softSerial(softRX, softTX);

//#pragma pack(1) // Definición del struct para manejar datos con directivas globales de empaquetamiento alineado a 1 byte sin relleno.
// Definición del struct para manejar datos con atributo especifico de empaquetamiento alineado a 1 byte sin relleno.
struct __attribute__((packed, aligned(1))) DataPacket {
  uint8_t contador; // Contador de datos
}; // #pragma pack() // Restaura las directivas globales del empaquetamiento a su propio predeterminado.

// Variables globales para el struct
DataPacket dataEnvio = {0}; // Inicializa el contador en 0
DataPacket dataRecibido = {0}; // Estructura para almacenar datos recibidos

unsigned long timeLine0 = 0; // Variable para controlar el tiempo
const unsigned long intervalo = 500; // Intervalo entre envíos (en milisegundos)

// Prototipos de funciones
void setupInit();
void setupSoftSerial();
void setupEnd();
void sendSoftSerial();
void readSoftSerial();
void handleSoftSerial();

void setup() {
  setupInit();
  setupSoftSerial();
  setupEnd();
}

void loop() {
  handleSoftSerial();
}

void setupInit() { // Configuración inicial
  Serial.begin(hardSerialBautRate, SERIAL_8N1); // Inicializa el puerto serie
  Serial.println(F("Puerto Hardware Serial: Iniciado Correctamente."));
  pinMode(softRX, INPUT); // Configura el pin RX como entrada
  pinMode(softTX, OUTPUT); // Configura el pin TX como salida
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println(F("Inicialización y configuración de pines completada."));
}
void setupSoftSerial() { // Inicialización del Software Serial
  softSerial.begin(softSerialBautRate); // Inicializa SoftwareSerial
  Serial.println(F("Puerto Software Serial: Iniciado Correctamente."));
}
void setupEnd() { // Finalización de la configuración
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println(F("Gracias por esperar."));
  digitalWrite(LED_BUILTIN, LOW);
}
void sendSoftSerial() { // Enviar datos por Software Serial
  
  if (millis() >= timeLine0 + intervalo) { // Si paso el lapso de tiempo
    
    timeLine0 = millis(); // Actualiza el tiempo anterior
    digitalWrite(LED_BUILTIN, HIGH);

    // Envía el struct como bytes
    size_t structSize = sizeof(dataEnvio); // Tamaño del struct en bytes
    softSerial.write(reinterpret_cast<uint8_t*>(&dataEnvio), structSize);

    // Imprime el valor enviado en diferentes formatos
    Serial.print(F("Enviado (Binario): ")); Serial.println(dataEnvio.contador, BIN);
    Serial.print(F("Enviado (Decimal): ")); Serial.println(dataEnvio.contador, DEC);
    Serial.print(F("Enviado (Hexadecimal): ")); Serial.println(dataEnvio.contador, HEX);
    Serial.print(F("Enviado (Octal): ")); Serial.println(dataEnvio.contador, OCT);
    Serial.print(F("Enviado (ASCII): "));
    if (dataEnvio.contador >= 32 && dataEnvio.contador <= 126) {
      Serial.println((char)dataEnvio.contador); // Imprime carácter ASCII visible
    } else {
      Serial.println(F("No visible"));
    }
    Serial.print(F("Bytes enviados: ")); Serial.println(structSize); // Imprime el tamaño del struct
    Serial.println(F(" "));

    dataEnvio.contador++; // Incrementa el contador y reinicia si excede 255
    if (dataEnvio.contador > 255) {
      dataEnvio.contador = 0; // Reinicia si excede 255
    }
    timeLine0 = millis(); // Actualiza el tiempo anterior
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    return;
  }

}
void readSoftSerial() { // Leer datos recibidos por Software Serial
  
  if (softSerial.available() < sizeof(dataRecibido)) return; // Espera hasta recibir el tamaño completo del struct

  // Lee los datos y los almacena en el struct
  size_t structSize = sizeof(dataRecibido); // Tamaño del struct en bytes
  softSerial.readBytes(reinterpret_cast<uint8_t*>(&dataRecibido), structSize);

  // Imprime el valor recibido en diferentes formatos
  Serial.print(F("Recibido (Binario): ")); Serial.println(dataRecibido.contador, BIN);
  Serial.print(F("Recibido (Decimal): ")); Serial.println(dataRecibido.contador, DEC);
  Serial.print(F("Recibido (Hexadecimal): ")); Serial.println(dataRecibido.contador, HEX);
  Serial.print(F("Recibido (Octal): ")); Serial.println(dataRecibido.contador, OCT);
  Serial.print(F("Recibido (ASCII): "));
  if (dataRecibido.contador >= 32 && dataRecibido.contador <= 126) {
    Serial.println((char)dataRecibido.contador); // Imprime carácter ASCII visible
  } else {
    Serial.println(F("No visible"));
  }
  Serial.print(F("Bytes recibidos: ")); Serial.println(structSize); // Imprime el tamaño del struct
  Serial.println(F(" "));
}
void handleSoftSerial() { // Manejo del Software Serial (envío y recepción)
  sendSoftSerial();
  readSoftSerial();
}
