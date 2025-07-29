#include <TFT_eSPI.h>

#define GREEN_LED 12
#define RED_LED 13

TFT_eSPI tft = TFT_eSPI();

enum UIState
{
  UI_LOGIN,
  UI_DESKTOP
};
UIState ui_state = UI_LOGIN;
unsigned long login_start_time = 0;

enum App
{
  APP_TERMINAL,
  APP_COUNT
};
bool app_open[APP_COUNT] = {false};

// Serial command buffer
char serial_cmd_buf[32];
int serial_cmd_pos = 0;

unsigned long last_cursor_toggle = 0;
bool cursor_visible = true;

void clear_screen() { tft.fillScreen(tft.color565(10, 8, 24)); }

void print_centered(int y, const char *text, uint16_t color = TFT_WHITE)
{
  tft.setTextColor(color);
  tft.setTextSize(2);
  int x = (tft.width() - tft.textWidth(text)) / 2;
  tft.setCursor(x, y);
  tft.print(text);
}

void draw_login_ui()
{
  tft.fillScreen(tft.color565(10, 8, 24));
  int box_w = 280, box_h = 180;
  int box_x = (tft.width() - box_w) / 2;
  int box_y = (tft.height() - box_h) / 2;
  int radius = 22;
  for (int i = 0; i < box_h; i++)
  {
    uint8_t r = 40 + (120 - 40) * i / box_h;
    uint8_t g = 200 + (40 - 200) * i / box_h;
    uint8_t b = 255 + (255 - 255) * i / box_h;
    tft.drawRoundRect(box_x, box_y + i, box_w, 1, radius,
                      tft.color565(r, g, b));
  }
  tft.drawRoundRect(box_x - 2, box_y - 2, box_w + 4, box_h + 4, radius + 2,
                    tft.color565(0, 255, 255));
  tft.drawRoundRect(box_x - 4, box_y - 4, box_w + 8, box_h + 8, radius + 4,
                    tft.color565(120, 0, 255));
  tft.fillRoundRect(box_x, box_y, box_w, box_h, radius,
                    tft.color565(120, 80, 200));
  tft.setTextColor(tft.color565(0, 255, 255));
  tft.setTextSize(3);
  print_centered(box_y + 32, "MAYHEM Login");
  tft.setTextColor(tft.color565(180, 0, 255));
  tft.setTextSize(2);
  print_centered(box_y + 80, "Enter Password:");
  int input_w = 200, input_h = 38;
  int input_x = (tft.width() - input_w) / 2;
  int input_y = box_y + 110;
  int input_radius = 12;
  tft.drawRoundRect(input_x - 2, input_y - 2, input_w + 4, input_h + 4,
                    input_radius + 2, tft.color565(0, 255, 255));
  tft.drawRoundRect(input_x - 4, input_y - 4, input_w + 8, input_h + 8,
                    input_radius + 4, tft.color565(120, 0, 255));
  tft.fillRoundRect(input_x, input_y, input_w, input_h, input_radius,
                    tft.color565(20, 18, 40));
  tft.drawRoundRect(input_x, input_y, input_w, input_h, input_radius,
                    tft.color565(0, 255, 255));
}

void draw_wallpaper()
{
  int h = tft.height();
  int w = tft.width();
  tft.fillScreen(tft.color565(10, 8, 24));
  for (int i = 0; i < 32; i++)
  {
    int x0 = random(0, w);
    int y0 = random(0, h);
    int x1 = random(0, w);
    int y1 = random(0, h);
    uint16_t color =
        tft.color565(random(100, 255), random(0, 100), random(180, 255));
    tft.drawLine(x0, y0, x1, y1, color);
  }
  for (int i = 0; i < 8; i++)
  {
    int cx = random(w);
    int cy = random(h);
    int r = random(10, 32);
    uint16_t color =
        tft.color565(random(180, 255), random(0, 100), random(180, 255));
    tft.drawCircle(cx, cy, r, color);
  }
  tft.setTextSize(6);
  int logo_w = tft.textWidth("MAYHEM");
  int logo_h = 8 * 6; // 8px per char row, size=6
  int logo_x = (w - logo_w) / 2;
  int logo_y = (h - logo_h) / 2;
  for (int i = 0; i < 4; i++)
  {
    int dx = random(-3, 4);
    int dy = random(-3, 4);
    uint16_t color = tft.color565(200 + random(-30, 30), 0, 255);
    tft.setTextColor(color);
    tft.setCursor(logo_x + dx, logo_y + dy);
    tft.print("MAYHEM");
  }
  tft.setTextColor(tft.color565(255, 255, 255));
  tft.setCursor(logo_x, logo_y);
  tft.print("MAYHEM");
  tft.setTextSize(2);
}

void draw_icon(int x, int y, bool selected)
{
  int w = 48, h = 48, r = 10;
  uint16_t border =
      selected ? tft.color565(255, 255, 0) : tft.color565(0, 255, 255);
  tft.fillRoundRect(x, y, w, h, r, tft.color565(30, 30, 40));
  tft.drawRoundRect(x, y, w, h, r, border);
  tft.setTextColor(border);
  tft.setTextSize(2);
  tft.setCursor(x + 12, y + 16);
  tft.print(">_");
  tft.setTextSize(2);
}

void draw_window(int x, int y, int w, int h, const char *title, bool focused)
{
  int r = 12;
  tft.fillRoundRect(x, y, w, h, r, tft.color565(20, 20, 30));
  tft.drawRoundRect(x, y, w, h, r,
                    focused ? tft.color565(0, 255, 255)
                            : tft.color565(80, 80, 80));
  tft.fillRoundRect(x, y, w, 32, r, tft.color565(40, 40, 60));
  tft.setTextColor(tft.color565(0, 255, 255));
  tft.setTextSize(2);
  tft.setCursor(x + 16, y + 8);
  tft.print(title);
  tft.setTextColor(tft.color565(255, 80, 80));
  tft.setCursor(x + w - 32, y + 8);
  tft.print("X");
  tft.setTextSize(2);
}

void draw_terminal_content(bool cursor)
{
  int win_x = 20;
  int win_y = 30;
  tft.setTextColor(tft.color565(0, 255, 0));
  tft.setTextSize(2);
  tft.setCursor(win_x + 16, win_y + 48);
  tft.print("> ");
  if (cursor)
  {
    tft.print("_");
  }
  else
  {
    tft.print(" ");
  }
  tft.setTextSize(2);
}

void erase_terminal_cursor()
{
  int win_x = 20;
  int win_y = 30;
  int cursor_x = win_x + 16 + tft.textWidth("> ");
  int cursor_y = win_y + 48;
  tft.setTextSize(2);
  int cursor_w = tft.textWidth("_");
  int cursor_h = 16;
  tft.fillRect(cursor_x, cursor_y, cursor_w, cursor_h,
               tft.color565(20, 20, 30));
}

void draw_desktop()
{
  draw_wallpaper();
  draw_icon(32, tft.height() - 80, false);
  if (app_open[APP_TERMINAL])
  {
    draw_window(20, 30, tft.width() - 40, tft.height() - 60, "Terminal", true);
    draw_terminal_content(cursor_visible);
  }
}

void desktop_process()
{
  static bool init = false;
  if (!init)
  {
    draw_desktop();
    print_centered(40, "MAYHEM Desktop", tft.color565(0, 255, 255));
    print_centered(80, "Welcome, user!", tft.color565(180, 0, 255));
    init = true;
  }
}

void handle_serial_command(const char *cmd)
{
  if (strcmp(cmd, "open") == 0)
  {
    app_open[APP_TERMINAL] = true;
    draw_desktop();
    Serial.println("[DESKTOP] Terminal opened");
  }
  else if (strcmp(cmd, "close") == 0)
  {
    app_open[APP_TERMINAL] = false;
    draw_desktop();
    Serial.println("[DESKTOP] Terminal closed");
  }
}

void kernel_init()
{
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(tft.color565(10, 8, 24));
  tft.setTextSize(2);
  tft.setTextColor(tft.color565(0, 255, 255));
  Serial.println("[KERNEL] Kernel initialized");
  draw_login_ui();
  login_start_time = millis();
  ui_state = UI_LOGIN;
}

void setup()
{
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(tft.color565(10, 8, 24));
  tft.setTextSize(2);
  tft.setTextColor(tft.color565(0, 255, 255));
  draw_login_ui();
  login_start_time = millis();
  ui_state = UI_LOGIN;
}

void loop()
{
  if (ui_state == UI_LOGIN)
  {
    if (millis() - login_start_time > 1500)
    {
      draw_desktop();
      ui_state = UI_DESKTOP;
    }
  }
  else if (ui_state == UI_DESKTOP)
  {
    while (Serial.available())
    {
      char c = Serial.read();
      if (c == '\n' || c == '\r')
      {
        serial_cmd_buf[serial_cmd_pos] = 0;
        handle_serial_command(serial_cmd_buf);
        serial_cmd_pos = 0;
      }
      else if (serial_cmd_pos < (int)sizeof(serial_cmd_buf) - 1)
      {
        serial_cmd_buf[serial_cmd_pos++] = c;
      }
    }
    if (app_open[APP_TERMINAL])
    {
      if (millis() - last_cursor_toggle > 500)
      {
        erase_terminal_cursor();
        cursor_visible = !cursor_visible;
        last_cursor_toggle = millis();
        draw_terminal_content(cursor_visible);
      }
    }
  }
}