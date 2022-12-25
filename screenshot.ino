// Rework to send the Framebuffer, so not actually the image shown
// based on this: https://github.com/marekburiak/ILI9341_due/
// RRGGBB = 460.808 chars to transmit.
// Lossless Compression system:
// RRGGBBCCCC   where CCCC is the count of consecutive identical pixels
// screen 2 = 65.938 chars
// use screenshotreceive2.py to decode it into png.


// Future Idea:
// 4 bit = 16, that's enough for numbers https://stackoverflow.com/questions/48883329/encoding-two-numbers-in-a-4-bit-binary-string
// so this will cut the transmit in half, but it must be directly received, as it contains non string things.

void screenshotToConsole() {
  uint8_t lastColor[3];
  uint8_t color[3];
  uint32_t totalImageDataLength = 0;
  uint32_t sameColorPixelCount = 0;
  uint16_t sameColorPixelCount16 = 0;
  uint32_t sameColorStartIndex = 0;

  Serial.println(F("==== PIXEL DATA START ===="));

  // Initial Pixel:
  uint8_t r, g, b;
  uint16_t coltodecode;
  coltodecode = fb1[0];
  tft.color565toRGB(coltodecode, r, g, b);
  color[0] = r;
  color[1] = g;
  color[2] = b;
  printHex8(color, 3);  //write color of the first pixel
  lastColor[0] = color[0];
  lastColor[1] = color[1];
  lastColor[2] = color[2];
  totalImageDataLength += 6;
  sameColorStartIndex = 0;

  for (uint32_t i = 1; i < (320 * 240); i++) {
    coltodecode = fb1[i];
    tft.color565toRGB(coltodecode, r, g, b);
    color[0] = r;
    color[1] = g;
    color[2] = b;

    if (color[0] != lastColor[0] || color[1] != lastColor[1] || color[2] != lastColor[2]) {
      sameColorPixelCount = i - sameColorStartIndex;
      if (sameColorPixelCount > 65535) {
        sameColorPixelCount16 = 65535;
        printHex16(&sameColorPixelCount16, 1);
        printHex8(lastColor, 3);
        totalImageDataLength += 10;
        sameColorPixelCount16 = sameColorPixelCount - 65535;
      } else {
        sameColorPixelCount16 = sameColorPixelCount;
      }
      printHex16(&sameColorPixelCount16, 1);
      printHex8(color, 3);
      totalImageDataLength += 10;

      sameColorStartIndex = i;
      lastColor[0] = color[0];
      lastColor[1] = color[1];
      lastColor[2] = color[2];
    }
  }
  sameColorPixelCount = 320 * 240 - sameColorStartIndex;
  if (sameColorPixelCount > 65535) {
    sameColorPixelCount16 = 65535;
    printHex16(&sameColorPixelCount16, 1);
    printHex8(lastColor, 3);
    totalImageDataLength += 10;
    sameColorPixelCount16 = sameColorPixelCount - 65535;
  } else
    sameColorPixelCount16 = sameColorPixelCount;
  printHex16(&sameColorPixelCount16, 1);
  totalImageDataLength += 4;
  printHex32(&totalImageDataLength, 1);

  Serial.println();
  Serial.println(F("==== PIXEL DATA END ===="));
  Serial.print(F("Total Image Data Length: "));
  Serial.println(totalImageDataLength);
}

// Helpers:
void printHex8(uint8_t *data, uint8_t length)  // prints 8-bit data in hex
{
  char tmp[length * 2 + 1];
  byte first;
  byte second;
  for (int i = 0; i < length; i++) {
    first = (data[i] >> 4) & 0x0f;
    second = data[i] & 0x0f;
    // base for converting single digit numbers to ASCII is 48
    // base for 10-16 to become upper-case characters A-F is 55
    // note: difference is 7
    tmp[i * 2] = first + 48;
    tmp[i * 2 + 1] = second + 48;
    if (first > 9) tmp[i * 2] += 7;
    if (second > 9) tmp[i * 2 + 1] += 7;
  }
  tmp[length * 2] = 0;
  Serial.print(tmp);
}
void printHex16(uint16_t *data, uint8_t length)  // prints 8-bit data in hex
{
  char tmp[length * 4 + 1];
  byte first;
  byte second;
  byte third;
  byte fourth;
  for (int i = 0; i < length; i++) {
    first = (data[i] >> 12) & 0x0f;
    second = (data[i] >> 8) & 0x0f;
    third = (data[i] >> 4) & 0x0f;
    fourth = data[i] & 0x0f;
    //Serial << first << " " << second << " " << third << " " << fourth << endl;
    // base for converting single digit numbers to ASCII is 48
    // base for 10-16 to become upper-case characters A-F is 55
    // note: difference is 7
    tmp[i * 4] = first + 48;
    tmp[i * 4 + 1] = second + 48;
    tmp[i * 4 + 2] = third + 48;
    tmp[i * 4 + 3] = fourth + 48;
    //tmp[i*5+4] = 32; // add trailing space
    if (first > 9) tmp[i * 4] += 7;
    if (second > 9) tmp[i * 4 + 1] += 7;
    if (third > 9) tmp[i * 4 + 2] += 7;
    if (fourth > 9) tmp[i * 4 + 3] += 7;
  }
  tmp[length * 4] = 0;
  Serial.print(tmp);
}

void printHex32(uint32_t *data, uint8_t length)  // prints 8-bit data in hex
{
  char tmp[length * 8 + 1];
  byte dataByte[8];
  for (int i = 0; i < length; i++) {
    dataByte[0] = (data[i] >> 28) & 0x0f;
    dataByte[1] = (data[i] >> 24) & 0x0f;
    dataByte[2] = (data[i] >> 20) & 0x0f;
    dataByte[3] = (data[i] >> 16) & 0x0f;
    dataByte[4] = (data[i] >> 12) & 0x0f;
    dataByte[5] = (data[i] >> 8) & 0x0f;
    dataByte[6] = (data[i] >> 4) & 0x0f;
    dataByte[7] = data[i] & 0x0f;
    //Serial << first << " " << second << " " << third << " " << fourth << endl;
    // base for converting single digit numbers to ASCII is 48
    // base for 10-16 to become upper-case characters A-F is 55
    // note: difference is 7
    tmp[i * 4] = dataByte[0] + 48;
    tmp[i * 4 + 1] = dataByte[1] + 48;
    tmp[i * 4 + 2] = dataByte[2] + 48;
    tmp[i * 4 + 3] = dataByte[3] + 48;
    tmp[i * 4 + 4] = dataByte[4] + 48;
    tmp[i * 4 + 5] = dataByte[5] + 48;
    tmp[i * 4 + 6] = dataByte[6] + 48;
    tmp[i * 4 + 7] = dataByte[7] + 48;
    //tmp[i*5+4] = 32; // add trailing space
    if (dataByte[0] > 9) tmp[i * 4] += 7;
    if (dataByte[1] > 9) tmp[i * 4 + 1] += 7;
    if (dataByte[2] > 9) tmp[i * 4 + 2] += 7;
    if (dataByte[3] > 9) tmp[i * 4 + 3] += 7;
    if (dataByte[4] > 9) tmp[i * 4 + 4] += 7;
    if (dataByte[5] > 9) tmp[i * 4 + 5] += 7;
    if (dataByte[6] > 9) tmp[i * 4 + 6] += 7;
    if (dataByte[7] > 9) tmp[i * 4 + 7] += 7;
  }
  tmp[length * 8] = 0;
  Serial.print(tmp);
}
