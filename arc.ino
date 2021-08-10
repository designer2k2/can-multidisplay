// Arc function
// https://www.designer2k2.at
// Based on this here: https://github.com/marekburiak/ILI9341_due/blob/master/ILI9341_due.cpp

void drawArc(uint16_t cx, uint16_t cy, uint16_t radius, uint16_t thickness, int start, int end, uint16_t color) {
  start = start % 360;
  end = end % 360;

  while (start < 0) start += 360;
  while (end < 0) end += 360;

  if (end == 0) end = 360;

#define DEG_TO_RAD 0.017453292519943295769236907684886

  if (start > end) {
    drawArc(cx, cy, radius, thickness, start, 360, color);
    drawArc(cx, cy, radius, thickness, 360, end, color);
  }
  else {

    float sslope = (float)cos(start * DEG_TO_RAD) / (float)sin(start * DEG_TO_RAD);
    float eslope = (float)cos(end * DEG_TO_RAD) / (float)sin(end * DEG_TO_RAD);

    //Serial << "sslope: " << sslope << " eslope:" << eslope << endl;

    if (end == 360) eslope = -1000000;

    int ir2 = (radius - thickness) * (radius - thickness);
    int or2 = radius * radius;

    for (int x = -radius; x <= radius; x++)
      for (int y = -radius; y <= radius; y++)
      {
        int x2 = x * x;
        int y2 = y * y;

        if (
          (x2 + y2 < or2 && x2 + y2 >= ir2) &&
          (
            (y > 0 && start < 180 && x <= y * sslope) ||
            (y < 0 && start > 180 && x >= y * sslope) ||
            (y < 0 && start <= 180) ||
            (y == 0 && start <= 180 && x < 0) ||
            (y == 0 && start == 0 && x > 0)
          ) &&
          (
            (y > 0 && end < 180 && x >= y * eslope) ||
            (y < 0 && end > 180 && x <= y * eslope) ||
            (y > 0 && end >= 180) ||
            (y == 0 && end >= 180 && x < 0) ||
            (y == 0 && start == 0 && x > 0)
          )
        )
          tft.drawPixel(cx + x, cy + y, color);
      }
  }
}
