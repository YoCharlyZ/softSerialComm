#include <SoftwareSerial.h> // Incluye la librería SoftwareSerial

// Define los pines para el SoftwareSerial
const uint8_t softRX = 4; // Uno PinD4(softRX)   NodeMcu Gpio0 = pinD3(softRX)
const uint8_t softTX = 2; // Uno PinD2(softTX)   NodeMcu Gpio2 = pinD4(softTX)
const uint16_t softSerialBautRate = 9600; // Velocidad del Software Serial
const uint32_t hardSerialBautRate = 115200; // Velocidad del Hardware Serial

// Configuración del Software Serial
SoftwareSerial softSerial(softRX, softTX);

uint8_t contador = 0; // Contador de envíos, empieza en 0
uint8_t datoRecibido = 0; // Para almacenar el dato recibido
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
  Serial.println(F("Inicialización y Configuración de Pines Completada."));
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

  if (millis() >= timeLine0 + intervalo) {
    
    timeLine0 = millis(); // Actualiza el tiempo anterior
    digitalWrite(LED_BUILTIN, HIGH);

    softSerial.write(contador); // Envía el valor del contador como byte
    Serial.print(F("Enviado (Binario): ")); Serial.println(contador, BIN);
    Serial.print(F("Enviado (Decimal): ")); Serial.println(contador, DEC);
    Serial.print(F("Enviado (Hexadecimal): ")); Serial.println(contador, HEX);
    Serial.print(F("Enviado (Octal): ")); Serial.println(contador, OCT);
    Serial.print(F("Enviado (ASCII): "));
    if (contador >= 32 && contador <= 126) {
      Serial.println((char)contador); // Imprime carácter ASCII visible
    } else {
      Serial.println(F("No visible"));
    }

    contador++; // Incrementa el contador
    if (contador > 255) {
      contador = 0; // Reinicia si excede 255
    }
    timeLine0 = millis(); // Actualiza el tiempo anterior
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    return;
  }

}
void readSoftSerial() { // Leer datos recibidos por Software Serial

  if (!softSerial.available()) return; // Salir si no hay datos disponibles

  datoRecibido = softSerial.read(); // Lee el dato recibido
  Serial.print(F("Recibido (Binario): ")); Serial.println(datoRecibido, BIN);
  Serial.print(F("Recibido (Decimal): ")); Serial.println(datoRecibido, DEC);
  Serial.print(F("Recibido (Hexadecimal): ")); Serial.println(datoRecibido, HEX);
  Serial.print(F("Recibido (Octal): ")); Serial.println(datoRecibido, OCT);
  Serial.print(F("Recibido (ASCII): "));
  if (datoRecibido >= 32 && datoRecibido <= 126) {
    Serial.println((char)datoRecibido); // Imprime carácter ASCII visible
  } else {
    Serial.println(F("No visible"));
  }

}
void handleSoftSerial() { // Manejo del Software Serial (envío y recepción)
  sendSoftSerial();
  readSoftSerial();
}
