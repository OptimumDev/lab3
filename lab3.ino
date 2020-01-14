#include <LedControl.h>

const int displaysInRow = 6;
const int rowsCount = 3;
const int displaySize = 8;

const int echoPin = 2;
const int trigPin = 3;
const float voltsPerMeasurement = 5.0 / 1024.0;

const int sensorPin = A0;

const int matrixHeight = rowsCount * displaySize;
const int matrixWidth = displaysInRow * displaySize;

bool matrix[matrixHeight][matrixWidth];

const float speedOfSoundMPerSec = 340.0;
const float speedOfSoundCmPerUs = speedOfSoundMPerSec / 10000.0;

const int upperDistanceValue = 120;

LedControl lcs[rowsCount] = {
  LedControl(6, 4, 5, displaysInRow),
  LedControl(9, 7, 8, displaysInRow),
  LedControl(12, 10, 11, displaysInRow)
};

void setup_displays() {
  for (int i = 0; i < rowsCount; i++) {
    for (int j = 0; j < displaysInRow; j++) {
      lcs[i].shutdown(j, false);
      lcs[i].setIntensity(j, 16);
      lcs[i].clearDisplay(j);
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

// drawing

byte count_row_value(int displayX, int displayY, int y) {
  byte value = 0;
  for (int x = 0; x < displaySize; x++) {
    bool pixel_value = matrix[displayY * displaySize + y][displayX * displaySize + x];
    if (!pixel_value)
      continue;
    value += (byte) 1 << displaySize - 1 - x;
  }
  return value;
}

void fill_display(int displayX, int displayY) {
  for (int y = 0; y < displaySize; y++) {
    byte value = count_row_value(displayX, displayY, y);
    lcs[displayY].setRow(displayX, y, value);
  }
}

void draw_matrix() {
  for (int displayX = 0; displayX < displaysInRow; displayX++) {
    for (int displayY = 0; displayY < rowsCount; displayY++) {
      fill_display(displayX, displayY);
    }
  }
}

// reading distance

float read_distance_us()
{
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 1000 * 1000 * 15);

  return duration * speedOfSoundCmPerUs / 2.0;
}

float read_distance_ir() {
  float volts = analogRead(sensorPin) * voltsPerMeasurement;
  return pow(14.7737 / volts, 1.2134);
}

// building graph

void shift_matrix_half(int halfNumber) {
  const int start = matrixWidth / 2 * halfNumber;
  const int finish = matrixWidth / 2 * (halfNumber + 1) - 1;

  for (int x = start; x < finish; x++) {
    for (int y = 0; y < matrixHeight; y++) {
      matrix[y][x] = matrix[y][x + 1];
    }
  }

  for (int y = 0; y < matrixHeight; y++) {
    matrix[y][finish] = false;
  }
}

int normalize_distance(float distance) {
  int constrained = constrain(distance, 0, upperDistanceValue);
  int normalized = constrained / upperDistanceValue * (matrixHeight - 1);
  return normalized;
}

void update_graph(int matrixHalfNumber, float distance) {
  const int matrixColumn = matrixWidth / 2 * (matrixHalfNumber + 1) - 1;

  shift_matrix_half(matrixHalfNumber);

  int graphValue = normalize_distance(distance);
  matrix[graphValue][matrixColumn] = true;
}

void loop() {
  update_graph(0, read_distance_ir());
  update_graph(1, read_distance_us());

  draw_matrix();
}
