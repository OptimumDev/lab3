#include <LedControl.h>

const int displaysCount = 6;
const int dataPin = 12;
const int clkPin = 10;
const int csPin = 11;

const int echoPin = 2;
const int trigPin = 3;
const float voltsPerMeasurement = 5.0 / 1024.0;

const int sensorPin = A0;

bool matrix[24][48];

LedControl lcs[3] = {
  LedControl(6, 4, 5, displaysCount),
  LedControl(9, 7, 8, displaysCount),
  LedControl(12, 10, 11, displaysCount)
};

void setup_displays() {
  for (int j = 0; j < 3; j++) {
    for (int i = 0; i < displaysCount; i++) {
      lcs[j].shutdown(i, false);
      lcs[j].setIntensity(i, 16);
      lcs[j].clearDisplay(i);
    }
  }
}

void draw(int x, int y, bool is_on) {
  const int lc_num = y / 8;
  const int display_num = x / 8;
  const int display_y = x % 8;
  const int display_x = 7 - y % 8;
  lcs[lc_num].setLed(display_num, display_x, display_y, is_on);
}

void print_matrix() {
  for (int y = 0; y < 24; y++) {
    for (int x = 0; x < 48; x++) {
      Serial.print(matrix[y][x] ? "#" : ".");
    }
    Serial.println();
  }
}

void draw_matrix() {
  for (int display_x = 0; display_x < 6; display_x++) {
    for (int display_y = 0; display_y < 3; display_y++) {
      for (int y = 0; y < 8; y++) {
        byte value = 0;
        for (int x = 0; x < 8; x++) {
          bool pixel_value = matrix[display_y * 8 + y][display_x * 8 + x];
          if (!pixel_value)
            continue;
          value += (byte) 1 << 7 - x;
        }
        lcs[display_y].setRow(display_x, y, value);
      }
    }
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(echoPin, INPUT);
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);

  setup_displays();
}

float readPulseUs()
{
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 1000 * 1000 * 15);

  return duration;
}

float readDistUs()
{
  const float speedOfSoundMPerSec = 340.0;
  const float speedOfSoundCmPerUs = speedOfSoundMPerSec / 10000.0;
  return readPulseUs() * speedOfSoundCmPerUs / 2.0;
}

float readDistIR() {
  float volts = analogRead(sensorPin) * voltsPerMeasurement;
  return pow(14.7737 / volts, 1.2134);
}

int irGraph[24];
int usGraph[24];

void add_to_irgraph(float value) {
  int graph_value = value / 120 * 23;
  for (int i = 0; i < 23; i++) {
    irGraph[i] = irGraph[i + 1];
  }
  irGraph[23] = graph_value;
}

void add_to_usgraph(float value) {
  int graph_value = value / 120 * 23;
  for (int i = 0; i < 23; i++) {
    usGraph[i] = usGraph[i + 1];
  }
  usGraph[23] = graph_value;
}

void loop() {
  float irDistance = readDistIR();
  irDistance = constrain(irDistance, 0, 120);
  add_to_irgraph(irDistance);

  float usDistance = readDistUs();
  usDistance = constrain(usDistance, 0, 120);
  add_to_usgraph(usDistance);

  for (int y = 0; y < 24; y++) {
    for (int x = 0; x < 24; x++) {
      matrix[23 - y][x] = irGraph[x] == y;
      matrix[23 - y][24 + x] = usGraph[x] == y;
    }
  }

  draw_matrix();
}
