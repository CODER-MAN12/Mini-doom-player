#include <Arduino.h>
#include <SPI.h>

// ============================================================================
// MINI-DOOM-PLAYER (ESP32-S3-WROOM-2) HARDWARE PINOUT
// Verify these GPIO numbers against your KiCad schematic netlist!
// ============================================================================
#define TFT_MOSI  11
#define TFT_MISO  13
#define TFT_SCLK  12
#define TFT_CS    10
#define TFT_DC     9
#define TFT_RST   14

// 8-Button Cluster (SW1 - SW8) - Assumes active-LOW (wired to GND)
#define BTN_UP     4   // SW1
#define BTN_RIGHT  5   // SW2
#define BTN_DOWN   6   // SW3
#define BTN_LEFT   7   // SW4
#define BTN_FIRE   15  // SW5
#define BTN_USE    16  // SW6
#define BTN_STRAFE 17  // SW7
#define BTN_WEAPON 18  // SW8

const uint8_t BTN_PINS[8] = {
  BTN_UP, BTN_RIGHT, BTN_DOWN, BTN_LEFT,
  BTN_FIRE, BTN_USE, BTN_STRAFE, BTN_WEAPON
};

// ============================================================================
// RAYCASTER ENGINE CONFIG
// ============================================================================
#define SCREEN_W 320
#define SCREEN_H 200
#define MAP_W 16
#define MAP_H 16

const uint8_t worldMap[MAP_W][MAP_H] = {
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
  {1,0,2,2,0,0,1,0,3,3,0,0,4,0,0,1},
  {1,0,2,0,0,0,0,0,0,3,0,0,4,0,0,1},
  {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,1},
  {1,1,1,0,1,1,1,1,1,0,1,1,1,1,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,4,0,0,2,2,0,0,3,3,0,0,2,0,1},
  {1,0,4,0,0,2,2,0,0,3,3,0,0,2,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,0,1,1,1,1,1,0,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,3,3,3,0,0,4,4,0,0,2,2,2,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

float posX = 2.5, posY = 2.5;
float dirX = 1.0, dirY = 0.0;
float planeX = 0.0, planeY = 0.66;

uint16_t frameBuffer[SCREEN_W * SCREEN_H];

// EA eDIPTFT70-A / SPI Display Transfer Stub
void pushFrameToDisplay() {
  // If using EA eDIPTFT70-A over SPI, wrap frameBuffer in the EA SmallProtocol
  // packet format (DC1 + len + ESC U L + x + y + image_data + BCC checksum).
  // If using a raw SPI TFT (ILI9341/ST7789/RA8875), stream frameBuffer via DMA.
  digitalWrite(TFT_CS, LOW);
  SPI.writeBytes((uint8_t*)frameBuffer, sizeof(frameBuffer));
  digitalWrite(TFT_CS, HIGH);
}

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 8; i++) {
    pinMode(BTN_PINS[i], INPUT_PULLUP);
  }

  pinMode(TFT_CS, OUTPUT);
  digitalWrite(TFT_CS, HIGH);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_RST, OUTPUT);

  digitalWrite(TFT_RST, LOW);
  delay(50);
  digitalWrite(TFT_RST, HIGH);
  delay(120);

  SPI.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, TFT_CS);
  SPI.setFrequency(40000000);
}

void renderFrame() {
  for (int x = 0; x < SCREEN_W; x++) {
    float cameraX = 2 * x / (float)SCREEN_W - 1;
    float rayDirX = dirX + planeX * cameraX;
    float rayDirY = dirY + planeY * cameraX;

    int mapX = int(posX);
    int mapY = int(posY);
    float sideDistX, sideDistY;
    float deltaDistX = (rayDirX == 0) ? 1e30 : abs(1 / rayDirX);
    float deltaDistY = (rayDirY == 0) ? 1e30 : abs(1 / rayDirY);
    float perpWallDist;
    int stepX, stepY, hit = 0, side;

    if (rayDirX < 0) { stepX = -1; sideDistX = (posX - mapX) * deltaDistX; }
    else { stepX = 1; sideDistX = (mapX + 1.0 - posX) * deltaDistX; }
    if (rayDirY < 0) { stepY = -1; sideDistY = (posY - mapY) * deltaDistY; }
    else { stepY = 1; sideDistY = (mapY + 1.0 - posY) * deltaDistY; }

    while (hit == 0) {
      if (sideDistX < sideDistY) { sideDistX += deltaDistX; mapX += stepX; side = 0; }
      else { sideDistY += deltaDistY; mapY += stepY; side = 1; }
      if (worldMap[mapX][mapY] > 0) hit = 1;
    }

    perpWallDist = (side == 0) ? (sideDistX - deltaDistX) : (sideDistY - deltaDistY);
    int lineHeight = (int)(SCREEN_H / perpWallDist);
    int drawStart = max(0, -lineHeight / 2 + SCREEN_H / 2);
    int drawEnd = min(SCREEN_H - 1, lineHeight / 2 + SCREEN_H / 2);

    uint16_t color = 0x7BEF;
    if (worldMap[mapX][mapY] == 2) color = 0xA000;
    if (worldMap[mapX][mapY] == 3) color = 0x4208;
    if (worldMap[mapX][mapY] == 4) color = 0x8200;
    if (side == 1) color = (color >> 1) & 0x7BEF;

    for (int y = 0; y < SCREEN_H; y++) {
      if (y < drawStart) frameBuffer[y * SCREEN_W + x] = 0x18E3;
      else if (y <= drawEnd) frameBuffer[y * SCREEN_W + x] = color;
      else frameBuffer[y * SCREEN_W + x] = 0x3186;
    }
  }
}

void handleInput() {
  float moveSpeed = 0.08;
  float rotSpeed = 0.05;

  // SW1: Forward
  if (digitalRead(BTN_UP) == LOW) {
    if (worldMap[int(posX + dirX * moveSpeed)][int(posY)] == 0) posX += dirX * moveSpeed;
    if (worldMap[int(posX)][int(posY + dirY * moveSpeed)] == 0) posY += dirY * moveSpeed;
  }
  // SW3: Backward
  if (digitalRead(BTN_DOWN) == LOW) {
    if (worldMap[int(posX - dirX * moveSpeed)][int(posY)] == 0) posX -= dirX * moveSpeed;
    if (worldMap[int(posX)][int(posY - dirY * moveSpeed)] == 0) posY -= dirY * moveSpeed;
  }
  // SW2: Turn Right (or Strafe Right if SW7 held)
  if (digitalRead(BTN_RIGHT) == LOW) {
    if (digitalRead(BTN_STRAFE) == LOW) {
      if (worldMap[int(posX + planeX * moveSpeed)][int(posY)] == 0) posX += planeX * moveSpeed;
      if (worldMap[int(posX)][int(posY + planeY * moveSpeed)] == 0) posY += planeY * moveSpeed;
    } else {
      float oldDirX = dirX;
      dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
      dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
      float oldPlaneX = planeX;
      planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
      planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
    }
  }
  // SW4: Turn Left (or Strafe Left if SW7 held)
  if (digitalRead(BTN_LEFT) == LOW) {
    if (digitalRead(BTN_STRAFE) == LOW) {
      if (worldMap[int(posX - planeX * moveSpeed)][int(posY)] == 0) posX -= planeX * moveSpeed;
      if (worldMap[int(posX)][int(posY - planeY * moveSpeed)] == 0) posY -= planeY * moveSpeed;
    } else {
      float oldDirX = dirX;
      dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
      dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
      float oldPlaneX = planeX;
      planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
      planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
    }
  }
}

void loop() {
  handleInput();
  renderFrame();
  pushFrameToDisplay();
}
