#include <SoftwareSerial.h> // Incluye la librería SoftwareSerial para puerto serie por software

const uint8_t softRX = 4;                      // Pin RX del SoftwareSerial (ajusta según placa) Uno PinD4(softRX)   NodeMcu Gpio0 = pinD3(softRX)
const uint8_t softTX = 2;                      // Pin TX del SoftwareSerial (ajusta según placa) Uno PinD2(softTX)   NodeMcu Gpio2 = pinD4(softTX)
const uint16_t softSerialBautRate = 9600;      // Baudrate para SoftwareSerial
const uint32_t hardSerialBautRate = 115200;    // Baudrate para Serial hardware
const uint8_t targetID = 0;                    // Un adressID especial reservado para algun fin especifico.
const uint8_t mySlaveID = 8;                   // Mi adressID de Esclavo (ajusta según placa)
const uint8_t toMasterID = 10;                 // El adressID de al menos un Maestro Uno (ajusta según placa)
const uint8_t broadcastID = 255;               // Un ID para MultiDifusion
const unsigned long interval = 500;            // Intervalo de envío en ms
unsigned long timeLine0 = 0;                   // Marca de tiempo para temporización

SoftwareSerial softSerial(softRX, softTX);     // Instancia de SoftwareSerial con RX, TX

const uint8_t pinA0 = A0, pinA1 = A1, pinA2 = A2, pinA3 = A3, pinA4 = A4, pinA5 = A5;
const uint8_t/*pinD0 = 0, pinD1 = 1, pinD2 = 2,*/ pinD3 = 3, /*pinD4 = 4, */pinD5 = 5;
const uint8_t pinD6 = 6, pinD7 = 7, pinD8 = 8, pinD9 = 9, pinD10 = 10, pinD11 = 11;
const uint8_t pinD12 = 12, pinD13 = 13;

bool master_loadConfig = false;
bool slave_loadConfig = false;
bool flag_restart = true;

char valCharInit = '<';
uint16_t valPinA0 = 0, valPinA1 = 0, valPinA2 = 0, valPinA3 = 0, valPinA4 = 0, valPinA5 = 0;
uint8_t /*valPinD0 = 0, valPinD1 = 0, valPinD2 = 0,*/ valPinD3 = 0, /*valPinD4 = 0,*/ valPinD5 = 0;
uint8_t valPinD6 = 0, valPinD7 = 0, valPinD8 = 0, valPinD9 = 0, valPinD10 = 0, valPinD11 = 0, valPinD12 = 0, valPinD13 = 0;
char valCharEnd = '>';

struct __attribute__((packed, aligned(1))) DataPacket {
  char charInit;            // Carácter de inicio '<' (1 byte)
  uint8_t sourceID;         // ID de dispositivo remitente de origen
  uint8_t destinationID;    // ID de dispositivo destinatario

  bool varMasterLoadConfig; // Estado lógico = 8 bits = 1 Byte
  bool varSlaveLoadConfig;  // Estado lógico = 8 bits = 1 Byte
  bool varFlagRestart;      // Estado lógico = 8 bits = 1 Byte
  
  uint8_t varPinA0, varPinA1, varPinA2, varPinA3, varPinA4, varPinA5; // [(un solo float = 32 bits = 4 Bytes) x6] = 24 Bytes
  uint8_t /*varPinD0, varPinD1, varPinD2,*/ varPinD3, /*varPinD4,*/ varPinD5; // [(8 bits = 1 Byte) x2] = 2 Bytes
  uint8_t varPinD6, varPinD7, varPinD8, varPinD9, varPinD10, varPinD11, varPinD12, varPinD13; // [(8 bits = 1 Byte) x8] = 8 Bytes

  uint8_t structSize;       // Tamaño del struct en bytes (1 byte)
  char charEnd;             // Carácter de fin '>' (1 byte)
};

DataPacket dataPacketIN  = {'<', 0, 0, false, false, true, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, sizeof(DataPacket), '>'};  // Estructura para datos entrantes
DataPacket dataPacketOUT = {'<', 0, 0, false, false, false, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, sizeof(DataPacket), '>'};   // Estructura para datos salientes

#define FIFO_SIZE 64                               // Tamaño fijo del FIFO (ajustable)
uint8_t fifoRX[FIFO_SIZE];                         // Buffer RX
uint8_t fifoTX[FIFO_SIZE];                         // Buffer TX
uint8_t fifoIndexReadRX = 0;                       // Índice de lectura RX
uint8_t fifoIndexWriteRX = 0;                      // Índice de escritura RX
uint8_t fifoCountRX = 0;                           // Cantidad de bytes en RX
uint8_t fifoIndexReadTX = 0;                       // Índice de lectura TX
uint8_t fifoIndexWriteTX = 0;                      // Índice de escritura TX
uint8_t fifoCountTX = 0;                           // Cantidad de bytes en TX
unsigned long overflowCountRX = 0;                 // Veces que RX intentó escribir con buffer lleno
unsigned long overflowCountTX = 0;                 // Veces que TX intentó escribir con buffer lleno
bool paqueteCompleto = false;                      // Indica que hay un paquete listo para procesar

enum EstadoSoftSerial {                             // Estados posibles para la máquina
  ESTADO_IDLE,                                      // Esperando envío o llegada de datos
  ESTADO_RECEIVING,                                 // Estado de recepción (llenar FIFO RX)
  ESTADO_PROCESSING,                                // Procesar paquete recibido
  ESTADO_SENDING                                    // Enviar paquete desde FIFO TX
};
EstadoSoftSerial estadoActual = ESTADO_IDLE;        // Estado inicial

void setupInit();                                   // Inicializaciones generales
void setupHardSerial();                             // Configura Serial hardware
void setupSoftSerial();                             // Configura SoftwareSerial y pines
void setupConfigPins();                             // Configura pines (LED, etc.)
void setupEnd();                                    // Mensaje de fin de setup

void fifoWriteRX(uint8_t dato);                     // Inserta byte en FIFO RX (si hay espacio)
int  fifoReadRX();                                  // Extrae byte de FIFO RX o devuelve -1
int  fifoPeekRX(uint8_t offset);                    // Observa byte en RX sin remover (-1 si fuera índice)
void fifoClearRX();                                 // Limpia FIFO RX (resetea índices y count)

void fifoWriteTX(uint8_t dato);                     // Inserta byte en FIFO TX (si hay espacio)
int  fifoReadTX();                                  // Extrae byte de FIFO TX o devuelve -1
int  fifoPeekTX(uint8_t offset);                    // Observa byte en TX sin remover (-1 si fuera índice)
void fifoClearTX();                                 // Limpia FIFO TX (resetea índices y count)

void updateStruct();                                 // Actualiza campos del struct de salida
void updateVars();                                   // Copia valores recibidos a variables internas
void rwDataPins();                                   // Lógica de lectura/escritura de pines

void printSending();                                 // Imprime paquete enviado (por Serial)
void printReceived();                                // Imprime paquete recibido (por Serial)
void printVarsInternal();                            // Imprime variables internas (por Serial)

void sendSoftSerial();                               // Prepara y envía bytes desde FIFO TX
void readSoftSerial();                               // Lee desde SoftSerial y almacena en FIFO RX sin bloquear
void procesarPaqueteRecibido();                      // Procesa dataPacketIN cuando paqueteCompleto == true
void handleSoftSerial();                             // Manejador principal llamado en loop()

void setup() {
  setupInit();                                       // Inicializaciones generales
  setupSoftSerial();                                 // Configura SoftwareSerial
  setupEnd();                                        // Mensaje de fin de setup
}

void loop() {
  handleSoftSerial();                                // Única llamada en loop: manejar SoftSerial (máquina de estados)
}

void setupInit() {
  setupHardSerial();                                  // Inicia Serial hardware
  setupConfigPins();                                  // Configura pines (LED, etc.)
}
void setupHardSerial() {
  Serial.begin(hardSerialBautRate, SERIAL_8N1);       // Inicia puerto serie hardware con configuración 8N1
  Serial.println(F("Serial hardware: iniciado correctamente.")); // Mensaje de inicio
}
void setupSoftSerial() {
  pinMode(softRX, INPUT);                             // Configura pin RX como entrada
  pinMode(softTX, OUTPUT);                            // Configura pin TX como salida
  softSerial.begin(softSerialBautRate);               // Inicia SoftwareSerial a la velocidad definida
  Serial.println(F("SoftwareSerial: iniciado correctamente.")); // Mensaje
}
void setupConfigPins() {
  pinMode(LED_BUILTIN, OUTPUT);  
  digitalWrite(LED_BUILTIN, HIGH); 
    pinMode(pinA0, INPUT);
    pinMode(pinA1, INPUT);
    pinMode(pinA2, INPUT);
    pinMode(pinA3, INPUT);
    pinMode(pinA4, INPUT);
    pinMode(pinA5, INPUT);
      
    //pinMode(pinD0, INPUT); digitalWrite(pinD0, valPinD0);
    //pinMode(pinD1, OUTPUT); digitalWrite(pinD1, valPinD1);
    //pinMode(pinD2, INPUT); digitalWrite(pinD2, valPinD2);
    pinMode(pinD3, OUTPUT); digitalWrite(pinD3, valPinD3);
    //pinMode(pinD4, OUTPUT); digitalWrite(pinD4, valPinD4);
    pinMode(pinD5, OUTPUT); digitalWrite(pinD5, valPinD5);
    pinMode(pinD6, OUTPUT); digitalWrite(pinD6, valPinD6);
    pinMode(pinD7, OUTPUT); digitalWrite(pinD7, valPinD7);
    pinMode(pinD8, OUTPUT); digitalWrite(pinD8, valPinD8);
    pinMode(pinD9, OUTPUT); digitalWrite(pinD9, valPinD9);
    pinMode(pinD10, OUTPUT); digitalWrite(pinD10, valPinD10);
    pinMode(pinD11, OUTPUT); digitalWrite(pinD11, valPinD11);
    pinMode(pinD12, OUTPUT); digitalWrite(pinD12, valPinD12);
    pinMode(pinD13, OUTPUT); digitalWrite(pinD13, valPinD13); // Configura LED integrado como salida
    digitalWrite(LED_BUILTIN, LOW);                     // Apaga LED inicialmente
  Serial.println(F("Pines configurados."));           // Mensaje
}
void setupEnd() {
  digitalWrite(LED_BUILTIN, HIGH);                    // Enciende LED para indicar fin de setup
  delay(80);                                          // Pequeña pausa visual (aceptable en setup)
  digitalWrite(LED_BUILTIN, LOW);                     // Apaga LED nuevamente
  Serial.println(F("Setup completo."));               // Mensaje final
}

void fifoWriteRX(uint8_t dato) {                      // Inserta un byte en FIFO RX si hay espacio
  if (fifoCountRX >= FIFO_SIZE) {                     // Si el buffer está lleno
    overflowCountRX++;                                 // Incrementa contador de overflow RX
    return;                                            // Ignora el byte nuevo (protege datos existentes)
  }
  fifoRX[fifoIndexWriteRX] = dato;                     // Escribe dato en la posición de escritura
  fifoIndexWriteRX++;                                  // Incrementa índice de escritura
  if (fifoIndexWriteRX >= FIFO_SIZE) fifoIndexWriteRX = 0; // Rebote circular del índice
  fifoCountRX++;                                       // Incrementa contador de bytes presentes
}
int fifoReadRX() {                                     // Lee y elimina un byte del FIFO RX o devuelve -1 si vacío
  if (fifoCountRX == 0) return -1;                     // Nada por leer: underflow -> -1
  uint8_t dato = fifoRX[fifoIndexReadRX];              // Lee el byte en la posición de lectura
  fifoIndexReadRX++;                                   // Avanza índice de lectura
  if (fifoIndexReadRX >= FIFO_SIZE) fifoIndexReadRX = 0; // Rebote circular
  fifoCountRX--;                                       // Decrementa contador de bytes presentes
  return dato;                                         // Retorna el byte leído
}
int fifoPeekRX(uint8_t offset) {                       // Observa un byte en RX sin removerlo; offset relativo a read index
  if (offset >= fifoCountRX) return -1;                // Offset fuera de rango -> -1
  uint8_t idx = fifoIndexReadRX + offset;              // Calcula índice lógico
  if (idx >= FIFO_SIZE) idx -= FIFO_SIZE;              // Ajusta por rebote circular si es necesario
  return fifoRX[idx];                                  // Retorna el byte observado
}
void fifoClearRX() {                                    // Resetea completamente el FIFO RX
  fifoIndexReadRX = 0;                                 // Reinicia índice de lectura
  fifoIndexWriteRX = 0;                                // Reinicia índice de escritura
  fifoCountRX = 0;                                     // Pone contador a 0 (vacía buffer lógico)
}
void fifoWriteTX(uint8_t dato) {                       // Inserta un byte en FIFO TX si hay espacio
  if (fifoCountTX >= FIFO_SIZE) {                      // Si el buffer TX está lleno
    overflowCountTX++;                                  // Cuenta evento de overflow TX
    return;                                             // Descarta el byte para no corromper buffer
  }
  fifoTX[fifoIndexWriteTX] = dato;                      // Escribe dato en posición de escritura TX
  fifoIndexWriteTX++;                                   // Avanza índice de escritura TX
  if (fifoIndexWriteTX >= FIFO_SIZE) fifoIndexWriteTX = 0; // Rebote circular
  fifoCountTX++;                                        // Incrementa el contador de bytes en TX
}
int fifoReadTX() {                                      // Extrae y elimina un byte del FIFO TX o devuelve -1
  if (fifoCountTX == 0) return -1;                      // No hay datos -> -1
  uint8_t dato = fifoTX[fifoIndexReadTX];               // Lee byte en índice de lectura TX
  fifoIndexReadTX++;                                    // Avanza índice lectura TX
  if (fifoIndexReadTX >= FIFO_SIZE) fifoIndexReadTX = 0; // Rebote circular
  fifoCountTX--;                                        // Decrementa contador de bytes en TX
  return dato;                                          // Retorna byte extraído
}
int fifoPeekTX(uint8_t offset) {                        // Observa un byte en TX sin removerlo; offset relativo a read index
  if (offset >= fifoCountTX) return -1;                 // Offset fuera de rango -> -1
  uint8_t idx = fifoIndexReadTX + offset;               // Calcula índice lógico
  if (idx >= FIFO_SIZE) idx -= FIFO_SIZE;               // Ajusta por rebote circular
  return fifoTX[idx];                                   // Retorna byte observado
}
void fifoClearTX() {                                    // Resetea completamente el FIFO TX
  fifoIndexReadTX = 0;                                  // Reinicia índice lectura TX
  fifoIndexWriteTX = 0;                                 // Reinicia índice escritura TX
  fifoCountTX = 0;                                      // Reinicia contador TX
}

void updateStruct() {                                  // Actualiza campo structSize y calcula integridad antes de enviar
  
  dataPacketOUT.sourceID = mySlaveID;               // Actualiza sourceID
  dataPacketOUT.destinationID = toMasterID;     // Actualiza destinationID

  dataPacketOUT.varMasterLoadConfig = master_loadConfig;
  dataPacketOUT.varSlaveLoadConfig = slave_loadConfig;
  dataPacketOUT.varFlagRestart = flag_restart;

  dataPacketOUT.varPinA0 = map(valPinA0, 0, 1023, 0, 255); // Comprime A0 a un rango menor (0-255 desde 0-1023)
  dataPacketOUT.varPinA1 = map(valPinA1, 0, 1023, 0, 255); // Comprime A1 a un rango menor (0-255 desde 0-1023)
  dataPacketOUT.varPinA2 = map(valPinA2, 0, 1023, 0, 255); // Comprime A2 a un rango menor (0-255 desde 0-1023)
  dataPacketOUT.varPinA3 = map(valPinA3, 0, 1023, 0, 255); // Comprime A3 a un rango menor (0-255 desde 0-1023)
  dataPacketOUT.varPinA4 = map(valPinA4, 0, 1023, 0, 255); // Comprime A4 a un rango menor (0-255 desde 0-1023)
  dataPacketOUT.varPinA5 = map(valPinA5, 0, 1023, 0, 255); // Comprime A5 a un rango menor (0-255 desde 0-1023)

  //dataPacketOUT.varPinD0 = valPinD0;
  //dataPacketOUT.varPinD1 = valPinD1;
  //dataPacketOUT.varPinD2 = valPinD2;
  dataPacketOUT.varPinD3 = valPinD3;
  //dataPacketOUT.varPinD4 = valPinD4;
  dataPacketOUT.varPinD5 = valPinD5;
  dataPacketOUT.varPinD6 = valPinD6;
  dataPacketOUT.varPinD7 = valPinD7;
  dataPacketOUT.varPinD8 = valPinD8;
  dataPacketOUT.varPinD9 = valPinD9;
  dataPacketOUT.varPinD10 = valPinD10;
  dataPacketOUT.varPinD11 = valPinD11;
  dataPacketOUT.varPinD12 = valPinD12;
  dataPacketOUT.varPinD13 = valPinD13;
  dataPacketOUT.structSize = sizeof(dataPacketOUT);   // Tamaño actual del struct en bytes
}
void updateVars() {                                    // Copia valores recibidos a variables internas
  master_loadConfig = dataPacketIN.varMasterLoadConfig;// Actualiza variables internas desde struct IN
  slave_loadConfig = dataPacketIN.varSlaveLoadConfig;  
  flag_restart = dataPacketIN.varFlagRestart;  

  valPinA0 = map(dataPacketIN.varPinA0, 0, 255, 0, 1023);   // Descomprime A0 a su rango original (0-1023 desde 0-255)
  valPinA1 = map(dataPacketIN.varPinA1, 0, 255, 0, 1023);   // Descomprime A1 a su rango original (0-1023 desde 0-255)
  valPinA2 = map(dataPacketIN.varPinA2, 0, 255, 0, 1023);   // Descomprime A2 a su rango original (0-1023 desde 0-255)
  valPinA3 = map(dataPacketIN.varPinA3, 0, 255, 0, 1023);   // Descomprime A3 a su rango original (0-1023 desde 0-255)
  valPinA4 = map(dataPacketIN.varPinA4, 0, 255, 0, 1023);   // Descomprime A4 a su rango original (0-1023 desde 0-255)
  valPinA5 = map(dataPacketIN.varPinA5, 0, 255, 0, 1023);   // Descomprime A5 a su rango original (0-1023 desde 0-255)

  //valPinD0 = dataPacketIN.varPinD0;
  //valPinD1 = dataPacketIN.varPinD1;
  //valPinD2 = dataPacketIN.varPinD2;
  valPinD3 = dataPacketIN.varPinD3; 
  //valPinD4 = dataPacketIN.varPinD4;
  valPinD5 = dataPacketIN.varPinD5;
  valPinD6 = dataPacketIN.varPinD6;
  valPinD7 = dataPacketIN.varPinD7;
  valPinD8 = dataPacketIN.varPinD8;
  valPinD9 = dataPacketIN.varPinD9;
  valPinD10 = dataPacketIN.varPinD10;
  valPinD11 = dataPacketIN.varPinD11;
  valPinD12 = dataPacketIN.varPinD12;
  valPinD13 = dataPacketIN.varPinD13;

}
void rwDataPins() {                                    // Lógica para actualizar pines o variables periódicas
  valPinA0 = analogRead(pinA0);
  valPinA1 = analogRead(pinA1);
  valPinA2 = analogRead(pinA2);
  valPinA3 = analogRead(pinA3);
  valPinA4 = analogRead(pinA4);
  valPinA5 = analogRead(pinA5);

  //valPinD0 = digitalRead(pinD0);
  //valPinD1 = digitalRead(pinD1);
  //valPinD2 = digitalRead(pinD2);
  valPinD3 = digitalRead(pinD3);
  //valPinD4 = digitalRead(pinD4);
  valPinD5 = digitalRead(pinD5);
  valPinD6 = digitalRead(pinD6);
  valPinD7 = digitalRead(pinD7);
  valPinD8 = digitalRead(pinD8);
  valPinD9 = digitalRead(pinD9);
  valPinD10 = digitalRead(pinD10);
  valPinD11 = digitalRead(pinD11);
  valPinD12 = digitalRead(pinD12);
  valPinD13 = digitalRead(pinD13);
}

void printSending() {                                  // Imprime en Serial los datos que se enviaron
  Serial.println(F("Paquete Enviado:"));               // Cabecera
  Serial.print(F("Inicio: ")); Serial.println(dataPacketOUT.charInit);   // Muestra '<'
  Serial.print(F("Origen: ")); Serial.println(dataPacketOUT.sourceID);       // Muestra sourceID
  Serial.print(F("Destino: ")); Serial.println(dataPacketOUT.destinationID); // Muestra destinationID
  Serial.print(F("Estado del Maestro: ")); Serial.print(dataPacketOUT.varMasterLoadConfig ? "true" : "false");
  Serial.print(F(", Estado del Esclavo: ")); Serial.print(dataPacketOUT.varSlaveLoadConfig ? "true" : "false");
  Serial.print(F(", Estado de Reinicio: ")); Serial.print(dataPacketOUT.varFlagRestart ? "true" : "false");
  Serial.println(F(". ")); 

  Serial.println(F("Valores Pines Analogicos: "));
  Serial.print(F("A0 ")); Serial.print(dataPacketOUT.varPinA0); 
  Serial.print(F(", A1 ")); Serial.print(dataPacketOUT.varPinA1); 
  Serial.print(F(", A2 ")); Serial.print(dataPacketOUT.varPinA2); 
  Serial.print(F(", A3 ")); Serial.print(dataPacketOUT.varPinA3);
  Serial.print(F(", A4 ")); Serial.print(dataPacketOUT.varPinA4); 
  Serial.print(F(", A5 ")); Serial.print(dataPacketOUT.varPinA5); 
  Serial.println(F(". ")); 
  
  Serial.println(F("Valores Pines Digitales: "));
  //Serial.print(F("D0 ")); Serial.print(dataPacketOUT.varPinD0); 
  //Serial.print(F(", D1 ")); Serial.print(dataPacketOUT.varPinD1); 
  //Serial.print(F(", D2 ")); Serial.print(dataPacketOUT.varPinD2); 
  Serial.print(F("D3 ")); Serial.print(dataPacketOUT.varPinD3); 
  //Serial.print(F(", D4 ")); Serial.print(dataPacketOUT.varPinD4);
  Serial.print(F(", D5 ")); Serial.print(dataPacketOUT.varPinD5); 
  Serial.print(F(", D6 ")); Serial.print(dataPacketOUT.varPinD6);
  Serial.print(F(", D7 ")); Serial.print(dataPacketOUT.varPinD7); 
  Serial.print(F(", D8 ")); Serial.print(dataPacketOUT.varPinD8);
  Serial.print(F(", D9 ")); Serial.print(dataPacketOUT.varPinD9);
  Serial.print(F(", D10 ")); Serial.print(dataPacketOUT.varPinD10);
  Serial.print(F(", D11 ")); Serial.print(dataPacketOUT.varPinD11); 
  Serial.print(F(", D12 ")); Serial.print(dataPacketOUT.varPinD12); 
  Serial.print(F(", D13 ")); Serial.print(dataPacketOUT.varPinD13); 
  Serial.println(F(". "));

  Serial.println(F("Informacion del Paquete: "));
  Serial.print(F("Tamaño: ")); Serial.print(dataPacketOUT.structSize, DEC); // Muestra tamaño
  Serial.print(F(" Fin: ")); Serial.println(dataPacketOUT.charEnd);     // Muestra '>'
  Serial.println(F(" "));                               // Línea en blanco
}
void printReceived() {                                 // Imprime en Serial los datos recibidos en dataPacketIN
  Serial.println(F("Paquete Recibido:"));              // Cabecera
  Serial.print(F(" Inicio: ")); Serial.println(dataPacketIN.charInit);     // Muestra '<'
  Serial.print(F("Origen: ")); Serial.println(dataPacketIN.sourceID);       // Muestra sourceID
  Serial.print(F("Destino: ")); Serial.println(dataPacketIN.destinationID); // Muestra destinationID
  Serial.print(F("Estado del Maestro: ")); Serial.print(dataPacketIN.varMasterLoadConfig ? "true" : "false");
  Serial.print(F(", Estado del Esclavo: ")); Serial.print(dataPacketIN.varSlaveLoadConfig ? "true" : "false");
  Serial.print(F(", Estado de Reinicio: ")); Serial.print(dataPacketIN.varFlagRestart ? "true" : "false");
  Serial.println(F(". ")); 

  Serial.println(F("Valores Pines Analogicos: "));
  Serial.print(F("A0 ")); Serial.print(dataPacketIN.varPinA0); 
  Serial.print(F(", A1 ")); Serial.print(dataPacketIN.varPinA1); 
  Serial.print(F(", A2 ")); Serial.print(dataPacketIN.varPinA2); 
  Serial.print(F(", A3 ")); Serial.print(dataPacketIN.varPinA3);
  Serial.print(F(", A4 ")); Serial.print(dataPacketIN.varPinA4); 
  Serial.print(F(", A5 ")); Serial.print(dataPacketIN.varPinA5); 
  Serial.println(F(". ")); 
  
  Serial.println(F("Valores Pines Digitales: "));
  //Serial.print(F("D0 ")); Serial.print(dataPacketIN.varPinD0); 
  //Serial.print(F(", D1 ")); Serial.print(dataPacketIN.varPinD1); 
  //Serial.print(F(", D2 ")); Serial.print(dataPacketIN.varPinD2); 
  Serial.print(F("D3 ")); Serial.print(dataPacketIN.varPinD3); 
  //Serial.print(F(", D4 ")); Serial.print(dataPacketIN.varPinD4);
  Serial.print(F(", D5 ")); Serial.print(dataPacketIN.varPinD5); 
  Serial.print(F(", D6 ")); Serial.print(dataPacketIN.varPinD6);
  Serial.print(F(", D7 ")); Serial.print(dataPacketIN.varPinD7); 
  Serial.print(F(", D8 ")); Serial.print(dataPacketIN.varPinD8);
  Serial.print(F(", D9 ")); Serial.print(dataPacketIN.varPinD9);
  Serial.print(F(", D10 ")); Serial.print(dataPacketIN.varPinD10);
  Serial.print(F(", D11 ")); Serial.print(dataPacketIN.varPinD11); 
  Serial.print(F(", D12 ")); Serial.print(dataPacketIN.varPinD12); 
  Serial.print(F(", D13 ")); Serial.print(dataPacketIN.varPinD13); 
  Serial.println(F(". ")); 

  Serial.println(F("Informacion del Paquete: "));
  Serial.print(F("Tamaño: ")); Serial.print(dataPacketIN.structSize, DEC); // Muestra tamaño recibido
  Serial.print(F(" Fin: ")); Serial.println(dataPacketIN.charEnd);       // Muestra '>'
  Serial.println(F(" "));                               // Línea en blanco
}
void printVarsInternal() {                             // Imprime variables internas de ejemplo
  Serial.println(F("Paquete Interno:"));               // Cabecera
  Serial.print(F("Inicio: ")); Serial.println(valCharInit);
  Serial.print(F("Estado del Maestro: ")); Serial.print(master_loadConfig ? "true" : "false");
  Serial.print(F(", Estado del Esclavo: ")); Serial.print(slave_loadConfig ? "true" : "false");
  Serial.print(F(", Estado de Reinicio: ")); Serial.print(flag_restart ? "true" : "false");
  Serial.println(F(". ")); 

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
  //Serial.print(F(", D4 ")); Serial.print(valPinD4);
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
  Serial.println(F(" "));                               // Línea en blanco
}

void readSoftSerial() {
  int availableBytes = softSerial.available();         // Lee cantidad de bytes disponibles en puerto (no bloqueante)
  if (availableBytes <= 0) return;                     // Si no hay bytes, salir inmediatamente

  // Determina cuantos bytes podemos leer sin provocar overflow en FIFO RX
  int freeSpace = FIFO_SIZE - fifoCountRX;             // Espacio libre en FIFO RX
  int toRead = availableBytes;                         // Inicializa toRead con disponibles
  if (toRead > freeSpace) toRead = freeSpace;          // Limita lectura al espacio libre para evitar overflow

  // Lee hasta 'toRead' bytes (sin usar while, usamos for)
  for (int i = 0; i < toRead; i++) {                   // Recorre la cantidad de bytes a leer
    int b = softSerial.read();                         // Lee un byte del puerto (puede devolver -1)
    if (b == -1) break;                                // Si falla la lectura, salimos del loop (seguro)
    fifoWriteRX((uint8_t)b);                           // Inserta el byte leído en FIFO RX (controla overflow internamente)
  }

  // Intentamos detectar si hay un paquete completo dentro del FIFO RX usando fifoPeekRX
  uint8_t structLen = sizeof(DataPacket);              // Longitud esperada del paquete
  if (fifoCountRX < structLen) return;                 // No hay suficientes bytes para formar un paquete

  // Buscamos el offset del caracter '<' que además tenga suficiente espacio para el paquete completo
  int startOffset = -1;                                // Offset donde estaría el '<' válido
  uint8_t maxSearch = fifoCountRX - (structLen - 1);   // Máximo offset posible para iniciar un paquete completo
  for (uint8_t off = 0; off < maxSearch; off++) {      // Recorremos offsets posibles (sin while)
    int c = fifoPeekRX(off);                           // Observa byte en offset sin removerlo
    if (c == -1) break;                                // Si peek falló, salir
    if ((char)c == valCharInit) {                      // Si encontramos '<'
      int endByte = fifoPeekRX(off + (structLen - 1));// Observa byte que debería ser '>' al final del paquete
      if (endByte != -1 && (char)endByte == valCharEnd) { // Si existe y es '>'
        startOffset = off;                             // Guardamos offset de inicio válido
        break;                                         // Salimos del for (encontrado)
      }
    }
  }

  // Si encontramos un paquete completo en algún offset, extraemos y verificamos integridad
  if (startOffset >= 0) {                              // Si se detectó offset válido
    // Descartamos bytes anteriores al inicio real para resinarizar (sin while)
    for (int d = 0; d < startOffset; d++) {            // Recorre cada byte previo
      int discarded = fifoReadRX();                    // Lee y descarta un byte
      (void)discarded;                                 // Evita warning por variable sin usar
    }
    // Ahora extraemos exactamente structLen bytes al struct de entrada
    uint8_t* ptrIn = (uint8_t*)&dataPacketIN;          // Puntero al struct de entrada
    for (uint8_t k = 0; k < structLen; k++) {          // Extrae exactamente structLen bytes al struct
      int dato = fifoReadRX();                         // Lee siguiente byte del FIFO RX
      if (dato == -1) return;                          // Si falla por underflow, aborta (seguro)
      *(ptrIn + k) = (uint8_t)dato;                    // Copia byte al struct
    }
    // Verificamos delimitador final por seguridad
    if (dataPacketIN.charEnd != valCharEnd) {          // Si no coincide el fin
      int dummy = fifoReadRX();                        // Lee y descarta 1 byte para re-sincronizar
      (void)dummy;                                     // Evita warning
      return;                                          // Salimos sin marcar paquete completo
    }

    // Si llegamos aquí, paquete íntegro: marcamos listo para procesamiento
    paqueteCompleto = true;                            // Indica que paquete listo para ser procesado
  }
}
void sendSoftSerial() {
  updateStruct();                                     // Asegura que structSize y checks estén calculados
  uint8_t* ptrOut = (uint8_t*)&dataPacketOUT;         // Puntero a bytes del struct OUT
  uint8_t structLen = sizeof(DataPacket);             // Longitud del struct a enviar

  // Inserta cada byte del struct al FIFO TX (sin while)
  for (uint8_t i = 0; i < structLen; i++) {           // Recorre cada byte del struct
    fifoWriteTX(*(ptrOut + i));                       // Inserta byte en FIFO TX (control de overflow interno)
  }

  // Envía hasta structLen bytes por iteración (evita bloqueos prolongados), usando fifoReadTX
  for (uint8_t s = 0; s < structLen; s++) {           // Limita envíos a la longitud del struct por llamada
    int datoTX = fifoReadTX();                        // Extrae byte del FIFO TX (o -1 si vacío)
    if (datoTX == -1) break;                          // Si no hay datos, salimos del for
    softSerial.write((uint8_t)datoTX);                // Envía byte por SoftwareSerial
  }

  printSending();                                     // Muestra en Serial lo enviado
  rwDataPins();                                       // Actualiza contador interno para próximo envío
}
void procesarPaqueteRecibido() {
  // Verifica si el mensaje NO está dirigido a mí, y NO es broadcast, y NO viene del master permitido
  if ( dataPacketIN.destinationID != mySlaveID      // Si el destinatario NO soy yo
       && dataPacketIN.destinationID != targetID      // Y TAMPOCO es target específico
       && dataPacketIN.destinationID != broadcastID // Y TAMPOCO es broadcast general
       && dataPacketIN.sourceID != toMasterID ) {   // Y NO viene del maestro autorizado
    
    return;                                         // Entonces ignoramos el paquete completamente
  }
  printReceived();                                    // Imprime los contenidos del paquete recibido
  updateVars();                                       // Actualiza variables internas desde el paquete
  printVarsInternal();                                // Muestra estado interno (contador)
  rwDataPins();                                       // Ejecuta acciones relacionadas a los datos/pines
  paqueteCompleto = false;                            // Resetea bandera de paquete completo
}
void handleSoftSerial() {
  switch (estadoActual) {                             // Evalúa estado actual
    case ESTADO_IDLE:                                 // Si está idle
      if (softSerial.available() > 0) {               // Si hay bytes en el puerto
        estadoActual = ESTADO_RECEIVING;              // Pasa a recibir
      } else if (millis() - timeLine0 >= interval) {  // Si pasó el intervalo de envío
        estadoActual = ESTADO_SENDING;                // Pasa a enviar
      }
      break;

    case ESTADO_RECEIVING:                            // Si está recibiendo
      readSoftSerial();                               // Llama a la función que llena FIFO RX y detecta paquetes
      if (paqueteCompleto) {                          // Si se detectó paquete
        estadoActual = ESTADO_PROCESSING;             // Pasa a procesarlo
      } else {                                        // Si no, vuelve a IDLE para no saturar el loop
        estadoActual = ESTADO_IDLE;
      }
      break;

    case ESTADO_PROCESSING:                           // Si debe procesar paquete
      procesarPaqueteRecibido();                      // Procesa el paquete ya extraído a dataPacketIN
      estadoActual = ESTADO_IDLE;                     // Luego vuelve a IDLE
      break;

    case ESTADO_SENDING:                              // Si está en envío
      sendSoftSerial();                               // Prepara y envía bytes (no bloqueante por diseño)
      timeLine0 = millis();                           // Reinicia temporizador de envío
      estadoActual = ESTADO_IDLE;                     // Vuelve a IDLE
      break;

    default:                                          // Caso por seguridad si estado inválido
      Serial.println(F("Error: estadoSoftSerial inválido, reseteando a IDLE.")); // Mensaje de error
      estadoActual = ESTADO_IDLE;                     // Resetea a IDLE por seguridad
      break;
  }
}
