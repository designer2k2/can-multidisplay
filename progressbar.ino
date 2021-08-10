// Progressbar:
// https://www.designer2k2.at


void pbar(int va, int x, int y, int w, int h , boolean hor, uint16_t bcolor, uint16_t fcolor) {
  // X/Y = origin
  // W/H = size
  // va = Value (0-100)
  // hor = Horizontal / Vertical
  // bcolor = Color of the frame
  // fcolor = Color of the bar

  // Outline Box:
  tft.drawRect(x, y,  w, h, bcolor);

  if (hor) {
    // Calc:
    int filler = map(va, 0, 100, 0, w - 2);
    // Fill:
    tft.fillRect(x + 1, y + 1, filler, h - 2, fcolor);
  } else {
    // Calc:
    int filler = map(va, 100, 0, 0, h - 2);
    // Fill:
    tft.fillRect(x + 1, y + filler, w - 2, h - filler-1, fcolor);
  }

}
