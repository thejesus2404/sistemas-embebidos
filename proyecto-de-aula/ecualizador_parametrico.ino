#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_MCP4725.h>

#define SCREEN_WIDTH 128  // Ancho de la pantalla OLED
#define SCREEN_HEIGHT 64  // Alto de la pantalla OLED

#define CLK 3  // Pin CLK del encoder
#define DT 4   // Pin DT del encoder

volatile int contador = 0;  // Almacena el valor virtual del encoder
int clkLastState;

const float min_vp = -(35.0/15.0);
const float vpp_reducido = 2.5 - min_vp;

// Inicializa la pantalla OLED
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// Configurar el DAC MCP4725
Adafruit_MCP4725 dac;

// Función de interrupción para leer el encoder
void readEncoder() {
  int clkState = digitalRead(CLK);
  int dtState = digitalRead(DT);

  if (clkState != clkLastState) {
    if (dtState != clkState) {
      contador = min((int)(contador * 1.1) + 1, 4095);  // Aumenta el voltaje (máx. 5V)
    } else {
      contador = max((int)(contador / 1.1) - 1, 0);  // Disminuye el voltaje (mín. 0V)
    }
  }
  clkLastState = clkState;
}

void setup() {
  Wire.begin();  // Iniciar el bus I²C

  // Inicializar pantalla OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // 0x3C es la dirección I2C de la pantalla
    Serial.println(F("Error al iniciar la pantalla OLED"));
    while(1);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  // Iniciar DAC MCP4725
  if (!dac.begin(0x60)) {
    Serial.println("No se encontró MCP4725 en 0x60");
    while (1);
  }

  Serial.begin(9600);

  // Configurar pines del encoder
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DT, INPUT_PULLUP);

  // Configurar interrupción en el pin CLK
  attachInterrupt(digitalPinToInterrupt(CLK), readEncoder, CHANGE);
   
  clkLastState = digitalRead(CLK);
}

void loop() {
  float voltajeDAC = (contador/4095.0) * 5.0;
  char buffer[10];  // Buffer para almacenar el número convertido
  dtostrf(voltajeDAC, 5, 3, buffer);  // (Valor, Ancho mínimo, Decimales, Buffer)

  float V_Control= (vpp_reducido - 2.3) * 6;

  float fc = (voltajeDAC/5.000) * 20000.0;


  display.clearDisplay();
  display.setCursor(0, 10);
  display.println("DAC Output: ");
  display.print(buffer);
  display.println(" V");
  display.println("Cutoff: ");
  display.print(fc);
  display.print(" Hz");
  
  display.display();
  
  dac.setVoltage(contador, false);
}
