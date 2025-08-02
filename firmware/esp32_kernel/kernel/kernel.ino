#include <TFT_eSPI.h>
#include <esp_system.h>
#include <esp_wifi.h>
#include <driver/adc.h>

#define GREEN_LED 12
#define RED_LED 13

TFT_eSPI tft = TFT_eSPI();

struct Window
{
    int x, y, w, h;
    const char *title;
    bool focused;
};

enum App
{
    APP_TERMINAL,
    APP_FILES,
    APP_BENCHMARK,
    APP_SETTINGS,
    APP_ACTIVITY_MONITOR,
    APP_COUNT
};

bool app_open[APP_COUNT] = {false, false, false, false, false};
const int desktop_app_count = 5;
int desktop_selected = 0;
int tab_count = 0;

char serial_cmd_buf[32];
int serial_cmd_pos = 0;

unsigned long last_cursor_toggle = 0;
bool cursor_visible = true;
unsigned long last_frame = 0;
const int FRAME_DELAY = 30;
bool needs_redraw = true;
bool wallpaper_drawn = false;

enum UIState
{
    UI_LOGIN,
    UI_DESKTOP
};

unsigned long login_start_time = 0;
UIState ui_state = UI_LOGIN;

void draw_desktop(bool partial = false);

void clear_screen()
{
    tft.fillScreen(tft.color565(10, 8, 24));
}

void print_centered(int y, const char *text, uint16_t color)
{
    tft.setTextColor(color);
    tft.setTextSize(2);
    int x = (tft.width() - tft.textWidth(text)) / 2;
    tft.setCursor(x, y);
    tft.print(text);
}

void draw_top_bar()
{
    int bar_h = 28;
    tft.fillRect(0, 0, tft.width(), bar_h, tft.color565(24, 24, 32));

    char stats[64];
    uint32_t heap = ESP.getFreeHeap();
    uint32_t freq = getCpuFrequencyMhz();

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11);
    int raw = adc1_get_raw(ADC1_CHANNEL_0);
    float v = raw * 3.3 / 4095.0;

    snprintf(stats, sizeof(stats), "%.2fV %dMHz %dKB", v, freq, heap / 1024);

    bool any_app_open = false;
    for (int i = 0; i < APP_COUNT; ++i)
    {
        if (app_open[i])
        {
            any_app_open = true;
            break;
        }
    }

    if (!any_app_open)
    {
        tft.fillCircle(18, bar_h / 2, 10, tft.color565(120, 0, 255));
        tft.setTextColor(tft.color565(255, 255, 255));
        tft.setTextSize(2);
        tft.setCursor(36, 6);
        tft.print("MAYHEM");

        int stats_w = tft.textWidth(stats);
        tft.setTextColor(tft.color565(180, 255, 180));
        tft.setCursor(tft.width() - stats_w - 8, 6);
        tft.print(stats);
    }

    if (any_app_open)
    {
        tft.setTextColor(tft.color565(255, 255, 0));
        tft.setTextSize(2);
        tft.setCursor(36, 6);
        if (app_open[APP_TERMINAL])
            tft.print("Terminal");
        else if (app_open[APP_FILES])
            tft.print("Files");
        else if (app_open[APP_BENCHMARK])
            tft.print("Benchmark");
        else if (app_open[APP_SETTINGS])
            tft.print("Settings");
        else if (app_open[APP_ACTIVITY_MONITOR])
            tft.print("Activity Monitor");

        if (app_open[APP_TERMINAL] || app_open[APP_FILES] || app_open[APP_ACTIVITY_MONITOR])
        {
            tft.setTextColor(tft.color565(180, 255, 255));
            tft.print("  File  Edit  View");
        }
    }
}

void draw_login_ui()
{
    tft.fillScreen(tft.color565(10, 8, 24));
    draw_top_bar();

    int box_w = 280, box_h = 180;
    int box_x = (tft.width() - box_w) / 2;
    int box_y = (tft.height() - box_h) / 2;
    int radius = 22;

    for (int i = 0; i < box_h; i++)
    {
        uint8_t r = 40 + (120 - 40) * i / box_h;
        uint8_t g = 200 + (40 - 200) * i / box_h;
        uint8_t b = 255 + (255 - 255) * i / box_h;
        tft.drawRoundRect(box_x, box_y + i, box_w, 1, radius, tft.color565(r, g, b));
    }

    tft.drawRoundRect(box_x - 2, box_y - 2, box_w + 4, box_h + 4, radius + 2, tft.color565(0, 255, 255));
    tft.drawRoundRect(box_x - 4, box_y - 4, box_w + 8, box_h + 8, radius + 4, tft.color565(120, 0, 255));
    tft.fillRoundRect(box_x, box_y, box_w, box_h, radius, tft.color565(120, 80, 200));

    tft.setTextColor(tft.color565(0, 255, 255));
    tft.setTextSize(3);
    print_centered(box_y + 32, "MAYHEM Login", tft.color565(0, 255, 255));

    tft.setTextColor(tft.color565(180, 0, 255));
    tft.setTextSize(2);
    print_centered(box_y + 80, "Enter Password:", tft.color565(180, 0, 255));

    int input_w = 200, input_h = 38;
    int input_x = (tft.width() - input_w) / 2;
    int input_y = box_y + 110;
    int input_radius = 12;

    tft.drawRoundRect(input_x - 2, input_y - 2, input_w + 4, input_h + 4, input_radius + 2, tft.color565(0, 255, 255));
    tft.drawRoundRect(input_x - 4, input_y - 4, input_w + 8, input_h + 8, input_radius + 4, tft.color565(120, 0, 255));
    tft.fillRoundRect(input_x, input_y, input_w, input_h, input_radius, tft.color565(20, 18, 40));
    tft.drawRoundRect(input_x, input_y, input_w, input_h, input_radius, tft.color565(0, 255, 255));
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
        uint16_t color = tft.color565(random(100, 255), random(0, 100), random(180, 255));
        tft.drawLine(x0, y0, x1, y1, color);
    }

    for (int i = 0; i < 8; i++)
    {
        int cx = random(w);
        int cy = random(h);
        int r = random(10, 32);
        uint16_t color = tft.color565(random(180, 255), random(0, 100), random(180, 255));
        tft.drawCircle(cx, cy, r, color);
    }

    tft.setTextSize(6);
    int logo_w = tft.textWidth("MAYHEM");
    int logo_h = 8 * 6;
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

void draw_icon(int x, int y, bool selected, int app)
{
    int w = 48, h = 48, r = 10;
    extern int tab_count;

    if (selected && tab_count > 0)
    {
        tft.drawRoundRect(x - 4, y - 4, w + 8, h + 8, r + 4, tft.color565(255, 255, 0));
    }

    uint16_t border = tft.color565(0, 255, 255);
    tft.fillRoundRect(x, y, w, h, r, tft.color565(30, 30, 40));
    tft.drawRoundRect(x, y, w, h, r, border);
    tft.setTextColor(border);
    tft.setTextSize(2);
    const char *app_names[5] = {"Terminal", "Files", "Benchmark", "Settings", "Activity"};

    if (app == 0)
    {
        tft.setCursor(x + 12, y + 16);
        tft.print(">_");
    }
    else if (app == 1)
    {
        tft.fillRect(x + 10, y + 18, 28, 18, tft.color565(200, 200, 40));
        tft.fillRect(x + 10, y + 14, 14, 8, tft.color565(220, 220, 80));
        tft.drawRect(x + 10, y + 18, 28, 18, tft.color565(120, 120, 0));
    }
    else if (app == 2)
    {
        tft.drawRect(x + 14, y + 14, 20, 20, tft.color565(255, 255, 0));
        tft.drawLine(x + 24, y + 14, x + 24, y + 34, tft.color565(255, 255, 0));
        tft.drawLine(x + 14, y + 24, x + 34, y + 24, tft.color565(255, 255, 0));
        tft.drawLine(x + 20, y + 18, x + 28, y + 30, tft.color565(255, 255, 0));
        tft.drawLine(x + 28, y + 18, x + 20, y + 30, tft.color565(255, 255, 0));
    }
    else if (app == 3)
    {
        int cx = x + 24, cy = y + 24, r = 14;
        for (int i = 0; i < 8; ++i)
        {
            float angle = i * 3.14159 / 4;
            int sx = cx + (int)(r * 1.1 * cos(angle));
            int sy = cy + (int)(r * 1.1 * sin(angle));
            tft.drawLine(cx, cy, sx, sy, tft.color565(180, 180, 180));
        }
        tft.drawCircle(cx, cy, r, tft.color565(180, 180, 180));
        tft.drawCircle(cx, cy, r - 5, tft.color565(120, 120, 120));
        tft.fillCircle(cx, cy, 4, tft.color565(180, 180, 180));
    }
    else if (app == 4)
    {
        int bx = x + 10, by = y + 34;
        tft.fillRect(bx, by - 10, 6, 10, tft.color565(0, 255, 0));
        tft.fillRect(bx + 10, by - 18, 6, 18, tft.color565(255, 255, 0));
        tft.fillRect(bx + 20, by - 6, 6, 6, tft.color565(255, 0, 0));
        tft.drawRect(bx, by - 18, 26, 18, tft.color565(120, 120, 120));
    }

    tft.setTextColor(border);
    tft.setTextSize(1);
    const char *label = app_names[app];
    char buf[10];
    strncpy(buf, label, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = 0;
    int max_w = w;
    int text_w = tft.textWidth(buf);

    if (text_w > max_w)
    {
        int len = strlen(buf);
        while (len > 3 && tft.textWidth(buf) > max_w - tft.textWidth("..."))
        {
            buf[--len] = 0;
        }
        strcat(buf, "...");
    }

    int text_x = x + (w - tft.textWidth(buf)) / 2;
    tft.setCursor(text_x, y + h + 2);
    tft.print(buf);
    tft.setTextSize(2);
}

void animate_window_open(const Window &targetWin)
{
    int steps = 12;
    int prev_x = -1, prev_y = -1, prev_w = -1, prev_h = -1;
    tft.startWrite();
    for (int i = 1; i <= steps; ++i)
    {
        float t = (float)i / steps;
        int x = targetWin.x + (targetWin.w / 2) * (1 - t);
        int y = targetWin.y + (targetWin.h / 2) * (1 - t);
        int w = targetWin.w * t;
        int h = targetWin.h * t;
        if (i > 1)
        {
            tft.fillRoundRect(prev_x, prev_y, prev_w, prev_h, 12, tft.color565(10, 8, 24));
        }
        Window animWin = {x, y, w, h, targetWin.title, targetWin.focused};
        draw_window(animWin);
        prev_x = x;
        prev_y = y;
        prev_w = w;
        prev_h = h;
        delay(10);
    }
    tft.endWrite();
}

void animate_window_close(const Window &targetWin)
{
    int steps = 12;
    int prev_x = -1, prev_y = -1, prev_w = -1, prev_h = -1;
    tft.startWrite();
    for (int i = steps; i >= 1; --i)
    {
        float t = (float)i / steps;
        int x = targetWin.x + (targetWin.w / 2) * (1 - t);
        int y = targetWin.y + (targetWin.h / 2) * (1 - t);
        int w = targetWin.w * t;
        int h = targetWin.h * t;
        if (i < steps)
        {
            tft.fillRoundRect(prev_x, prev_y, prev_w, prev_h, 12, tft.color565(10, 8, 24));
        }
        Window animWin = {x, y, w, h, targetWin.title, targetWin.focused};
        draw_window(animWin);
        prev_x = x;
        prev_y = y;
        prev_w = w;
        prev_h = h;
        delay(10);
    }
    tft.fillRoundRect(prev_x, prev_y, prev_w, prev_h, 12, tft.color565(10, 8, 24));
    tft.endWrite();
    draw_desktop();
}

void draw_window(const Window &win)
{
    int r = 12;
    tft.fillRoundRect(win.x, win.y, win.w, win.h, r, tft.color565(20, 20, 30));
    tft.drawRoundRect(win.x, win.y, win.w, win.h, r, win.focused ? tft.color565(0, 255, 255) : tft.color565(80, 80, 80));
    int bar_y = win.y + 2;
    tft.fillRoundRect(win.x, bar_y, win.w, 32, r, tft.color565(40, 40, 60));

    int btn_r = 7;
    int btn_y = bar_y + 16;
    int btn_x0 = win.x + 18;
    tft.fillCircle(btn_x0, btn_y, btn_r, tft.color565(255, 60, 60));
    tft.fillCircle(btn_x0 + 22, btn_y, btn_r, tft.color565(255, 220, 60));
    tft.fillCircle(btn_x0 + 44, btn_y, btn_r, tft.color565(60, 255, 60));
    tft.drawCircle(btn_x0, btn_y, btn_r, tft.color565(120, 0, 0));
    tft.drawCircle(btn_x0 + 22, btn_y, btn_r, tft.color565(120, 120, 0));
    tft.drawCircle(btn_x0 + 44, btn_y, btn_r, tft.color565(0, 120, 0));

    tft.setTextColor(tft.color565(0, 255, 255));
    tft.setTextSize(2);
    int title_w = tft.textWidth(win.title);
    char sizeinfo[32];
    snprintf(sizeinfo, sizeof(sizeinfo), "%dx%d", win.w, win.h);
    int sizeinfo_w = tft.textWidth(sizeinfo);
    int bar_padding = 16;
    int min_gap = 24;
    int title_x = win.x + 80;
    int sizeinfo_x = win.x + win.w - sizeinfo_w - bar_padding;
    if (title_x + title_w + min_gap > sizeinfo_x)
    {
        char title_buf[32];
        strncpy(title_buf, win.title, sizeof(title_buf) - 1);
        title_buf[sizeof(title_buf) - 1] = 0;
        int max_title_w = sizeinfo_x - title_x - min_gap;
        int len = strlen(title_buf);
        while (len > 3 && tft.textWidth(title_buf) > max_title_w)
        {
            title_buf[--len] = 0;
        }
        if (len > 3)
            strcat(title_buf, "...");
        tft.setCursor(title_x, bar_y + 8);
        tft.print(title_buf);
    }
    else
    {
        tft.setCursor(title_x, bar_y + 8);
        tft.print(win.title);
    }
    tft.setTextColor(tft.color565(180, 255, 180));
    tft.setCursor(sizeinfo_x, bar_y + 8);
    tft.print(sizeinfo);
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
    int cursor_x = win_x + 16 + tft.textWidth("> ");
    int cursor_y = win_y + 48;
    tft.setTextSize(2);
    int cursor_w = tft.textWidth("_");
    int cursor_h = 16;
    tft.fillRect(cursor_x, cursor_y, cursor_w, cursor_h, tft.color565(20, 20, 30));
    tft.setCursor(cursor_x, cursor_y);
    tft.setTextColor(tft.color565(0, 255, 0));
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
    tft.fillRect(cursor_x, cursor_y, cursor_w, cursor_h, tft.color565(20, 20, 30));
}

enum FocusState
{
    FOCUS_DESKTOP,
    FOCUS_MENU,
    FOCUS_WINDOW
};

FocusState focus_state = FOCUS_DESKTOP;
int menu_selected = 0;
const int menu_count = 3;

void draw_menu_bar(int x, int y, int selected)
{
    const char *items[menu_count] = {"File", "Edit", "View"};
    int mx = x;
    for (int i = 0; i < menu_count; ++i)
    {
        int w = tft.textWidth(items[i]) + 12;
        if (selected == i && focus_state == FOCUS_MENU)
        {
            tft.fillRoundRect(mx - 2, y - 2, w + 4, 28, 6, tft.color565(255, 255, 0));
            tft.setTextColor(tft.color565(40, 40, 60));
        }
        else
        {
            tft.setTextColor(tft.color565(180, 255, 255));
        }
        tft.setCursor(mx, y);
        tft.print(items[i]);
        mx += w;
    }
}

unsigned long bench_start = 0;
unsigned long bench_end = 0;
bool bench_running = false;
char bench_result[512] = "";
unsigned long bench_last_draw = 0;
int bench_seconds_left = 60;
char bench_current_test[32] = "";

void run_benchmark_tests()
{
    bench_running = true;
    bench_result[0] = 0;
    bench_start = millis();
    bench_last_draw = bench_start;
    bench_seconds_left = 60;

    unsigned long t_now = bench_start;
    unsigned long t_end = bench_start + 60000;
    unsigned long int_ops = 0, float_ops = 0, memcpy_ops = 0, rand_ops = 0, matrix_ops = 0, obj_spawns = 0;
    int matrix_size = 16;
    int **matA = nullptr, **matB = nullptr, **matC = nullptr;

    matA = (int **)malloc(matrix_size * sizeof(int *));
    matB = (int **)malloc(matrix_size * sizeof(int *));
    matC = (int **)malloc(matrix_size * sizeof(int *));

    for (int i = 0; i < matrix_size; ++i)
    {
        matA[i] = (int *)malloc(matrix_size * sizeof(int));
        matB[i] = (int *)malloc(matrix_size * sizeof(int));
        matC[i] = (int *)malloc(matrix_size * sizeof(int));
    }

    struct DummyObj
    {
        int a, b, c;
        float d;
        DummyObj() : a(0), b(0), c(0), d(0) {}
    };

    int margin_x = 30;
    int margin_y = 30;
    Window benchWin = {margin_x, margin_y, tft.width() - 2 * margin_x, tft.height() - 2 * margin_y, "Benchmark", true};
    draw_window(benchWin);
    int x = benchWin.x + 20;
    int y = benchWin.y + 48;
    tft.fillRect(x, y + 64, benchWin.w - 40, benchWin.h - 60 - 64, tft.color565(20, 20, 30));

    while ((t_now = millis()) < t_end)
    {
        strcpy(bench_current_test, "Integer math");
        volatile int isum = 0;
        for (int i = 0; i < 1000; ++i)
            isum += i * 3 - i / 2;
        int_ops += 1000;

        strcpy(bench_current_test, "Float math");
        volatile float fsum = 0;
        for (int i = 0; i < 500; ++i)
            fsum += i * 1.2345f - i / 2.345f;
        float_ops += 500;

        strcpy(bench_current_test, "Memcpy");
        char buf1[256], buf2[256];
        for (int r = 0; r < 10; ++r)
        {
            memcpy(buf2, buf1, 256);
            memcpy_ops++;
        }

        strcpy(bench_current_test, "Random");
        volatile int rsum = 0;
        for (int i = 0; i < 500; ++i)
            rsum += random(0, 1000);
        rand_ops += 500;

        strcpy(bench_current_test, "Matrix multiply");
        for (int i = 0; i < matrix_size; ++i)
            for (int j = 0; j < matrix_size; ++j)
            {
                matC[i][j] = 0;
                for (int k = 0; k < matrix_size; ++k)
                    matC[i][j] += matA[i][k] * matB[k][j];
                matrix_ops++;
            }

        strcpy(bench_current_test, "Object spawns");
        for (int i = 0; i < 100; ++i)
        {
            DummyObj *obj = new DummyObj();
            obj_spawns++;
            delete obj;
        }

        if (millis() - bench_last_draw >= 1000)
        {
            bench_seconds_left = (t_end - millis()) / 1000;
            if (bench_seconds_left < 0)
                bench_seconds_left = 0;
            bench_last_draw = millis();
            int update_h = 64;
            tft.fillRect(x, y, benchWin.w - 40, update_h, tft.color565(20, 20, 30));
            char countdown[32];
            snprintf(countdown, sizeof(countdown), "Time left: %ds", bench_seconds_left);
            tft.setTextColor(tft.color565(255, 255, 0));
            tft.setCursor(x, y);
            tft.print(countdown);
            tft.setCursor(x, y + 32);
            tft.setTextColor(tft.color565(180, 255, 255));
            tft.print("Test: ");
            tft.print(bench_current_test);
        }
        delay(1);
    }

    for (int i = 0; i < matrix_size; ++i)
    {
        free(matA[i]);
        free(matB[i]);
        free(matC[i]);
    }
    free(matA);
    free(matB);
    free(matC);

    bench_end = millis();
    snprintf(bench_result, sizeof(bench_result),
             "1-min Stress Test Results:\nInteger ops: %lu\nFloat ops: %lu\nMemcpy ops: %lu\nRandom ops: %lu\nMatrix mults: %lu\nObject spawns: %lu\nTotal time: %lums",
             int_ops, float_ops, memcpy_ops, rand_ops, matrix_ops, obj_spawns, bench_end - bench_start);
    bench_running = false;
    bench_current_test[0] = 0;
    bench_seconds_left = 0;
}

void draw_activity_monitor_content(const Window &win)
{
    int x = win.x + 20;
    int y = win.y + 48;
    int line_h = 24;
    tft.setTextColor(tft.color565(0, 255, 255));
    tft.setTextSize(2);
    tft.setCursor(x, y);
    tft.print("System Usage:");
    y += line_h;

    tft.setTextColor(tft.color565(255, 255, 0));
    tft.setCursor(x, y);
    tft.print("CPU: ");
    tft.print(getCpuFrequencyMhz());
    tft.print(" MHz");
    tft.print(" (ESP32)");
    y += line_h;

    tft.setTextColor(tft.color565(0, 255, 0));
    tft.setCursor(x, y);
    uint32_t heap = ESP.getFreeHeap();
    tft.print("RAM: ");
    tft.print(heap / 1024);
    tft.print(" KB free");
    y += line_h;

    tft.setTextColor(tft.color565(0, 200, 255));
    tft.setCursor(x, y);
    tft.print("EEPROM: ");
    tft.print("N/A");
    y += line_h;

    tft.setTextColor(tft.color565(255, 180, 180));
    tft.setCursor(x, y);
    tft.print("Running Apps:");
    y += line_h;
    tft.setTextColor(tft.color565(255, 255, 255));
    const char *app_names[5] = {"Terminal", "Files", "Benchmark", "Settings", "Activity Monitor"};
    for (int i = 0; i < APP_COUNT; ++i)
    {
        if (app_open[i])
        {
            tft.setCursor(x + 16, y);
            tft.print("- ");
            tft.print(app_names[i]);
            y += line_h;
        }
    }
}

void draw_benchmark_content(const Window &win)
{
    int x = win.x + 20;
    int y = win.y + 48;
    static int last_seconds_left = -1;
    static char last_test[32] = "";

    if (bench_result[0] == 0 && !bench_running)
    {
        tft.fillRect(x, y, win.w - 40, win.h - 60, tft.color565(20, 20, 30));
        tft.setTextColor(tft.color565(255, 255, 0));
        tft.setTextSize(2);
        tft.setCursor(x, y);
        tft.print("Running 1-min stress test...");
        run_benchmark_tests();
        tft.fillRect(x, y, win.w - 40, win.h - 60, tft.color565(20, 20, 30));
        tft.setCursor(x, y);
        tft.setTextColor(tft.color565(180, 255, 180));
        tft.setTextSize(2);

        char *result_copy = strdup(bench_result);
        char *line = strtok(result_copy, "\n");
        int line_y = y;
        char last_line[128] = "";
        while (line)
        {
            if (strstr(line, "Total time") == line)
            {
                strncpy(last_line, line, sizeof(last_line) - 1);
                last_line[sizeof(last_line) - 1] = 0;
                break;
            }
            tft.setCursor(x, line_y);
            tft.print(line);
            line_y += 24;
            line = strtok(NULL, "\n");
        }
        if (last_line[0])
        {
            tft.setCursor(x, line_y);
            tft.print(last_line);
        }
        free(result_copy);
        last_seconds_left = -1;
        last_test[0] = 0;
    }
    else if (bench_running)
    {
        if (bench_seconds_left != last_seconds_left || strcmp(bench_current_test, last_test) != 0)
        {
            int update_h = 64;
            tft.fillRect(x, y, win.w - 40, update_h, tft.color565(20, 20, 30));
            char countdown[32];
            snprintf(countdown, sizeof(countdown), "Time left: %ds", bench_seconds_left);
            tft.setTextColor(tft.color565(255, 255, 0));
            tft.setCursor(x, y);
            tft.print(countdown);
            tft.setCursor(x, y + 32);
            tft.setTextColor(tft.color565(180, 255, 255));
            tft.print("Test: ");
            tft.print(bench_current_test);
            last_seconds_left = bench_seconds_left;
            strncpy(last_test, bench_current_test, sizeof(last_test));
        }
    }
    else
    {
        tft.fillRect(x, y, win.w - 40, win.h - 60, tft.color565(20, 20, 30));
        tft.setTextColor(tft.color565(180, 255, 255));
        tft.setTextSize(2);
        tft.setCursor(x, y);
        tft.print(bench_result);
        last_seconds_left = -1;
        last_test[0] = 0;
    }
}

void draw_desktop(bool partial)
{
    tft.startWrite();
    int margin = 32;
    int icon_y = margin + 16;
    int icon_w = 48;
    int icon_gap = 16;
    int icon_x[5] = {margin, margin + icon_w + icon_gap, margin + 2 * (icon_w + icon_gap), margin + 3 * (icon_w + icon_gap), margin + 4 * (icon_w + icon_gap)};

    if (!partial)
    {
        draw_wallpaper();
        wallpaper_drawn = true;
        draw_top_bar();
    }

    int icons_area_x = margin - 8;
    int icons_area_y = icon_y - 8;
    int icons_area_w = 5 * (icon_w + icon_gap) - icon_gap + 16;
    int icons_area_h = icon_w + 32;
    if (partial)
    {
        tft.fillRect(icons_area_x, icons_area_y, icons_area_w, icons_area_h, tft.color565(10, 8, 24));
    }

    if (focus_state != FOCUS_DESKTOP)
    {
        tab_count = 0;
    }

    for (int i = 0; i < desktop_app_count; ++i)
    {
        bool selected = (focus_state == FOCUS_DESKTOP && desktop_selected == i && tab_count > 0);
        draw_icon(icon_x[i], icon_y, selected, i);
    }

    if (!partial)
    {
        if (app_open[APP_ACTIVITY_MONITOR])
        {
            int margin_x = 40;
            int margin_y = 30;
            Window actWin = {margin_x, margin_y, tft.width() - 2 * margin_x, tft.height() - 2 * margin_y, "Activity Monitor", true};
            draw_window(actWin);
            draw_activity_monitor_content(actWin);
        }
        if (app_open[APP_TERMINAL])
        {
            Window termWin = {20, 30, tft.width() - 40, tft.height() - 40, "Terminal", true};
            draw_window(termWin);
            draw_terminal_content(cursor_visible);
        }
        if (app_open[APP_FILES])
        {
            Window filesWin = {40, 50, tft.width() - 80, tft.height() - 80, "Files", true};
            draw_window(filesWin);
        }
        if (app_open[APP_BENCHMARK])
        {
            int margin_x = 30;
            int margin_y = 30;
            Window benchWin = {margin_x, margin_y, tft.width() - 2 * margin_x, tft.height() - 2 * margin_y, "Benchmark", true};
            draw_window(benchWin);
            draw_benchmark_content(benchWin);
        }
        if (app_open[APP_SETTINGS])
        {
            Window settingsWin = {60, 70, tft.width() - 120, tft.height() - 120, "Settings", true};
            draw_window(settingsWin);
            tft.setTextColor(tft.color565(180, 255, 255));
            tft.setTextSize(2);
            tft.setCursor(settingsWin.x + 32, settingsWin.y + 60);
            tft.print("Settings coming soon...");
        }
    }
    tft.endWrite();
}

void desktop_process()
{
    static bool init = false;
    if (!init)
    {
        draw_desktop();
        init = true;
    }
}

void handle_serial_command(const char *cmd)
{
    if (strcmp(cmd, "open") == 0)
    {
        if (focus_state == FOCUS_DESKTOP)
        {
            if (desktop_selected == APP_TERMINAL)
            {
                Window termWin = {20, 30, tft.width() - 40, tft.height() - 40, "Terminal", true};
                animate_window_open(termWin);
                app_open[APP_TERMINAL] = true;
                focus_state = FOCUS_WINDOW;
                draw_desktop();
                Serial.println("[DESKTOP] Terminal opened");
            }
            else if (desktop_selected == APP_FILES)
            {
                Window filesWin = {40, 50, tft.width() - 80, tft.height() - 80, "Files", true};
                animate_window_open(filesWin);
                app_open[APP_FILES] = true;
                focus_state = FOCUS_WINDOW;
                draw_desktop();
                Serial.println("[DESKTOP] Files opened");
            }
            else if (desktop_selected == APP_BENCHMARK)
            {
                int margin_x = 30;
                int margin_y = 30;
                Window benchWin = {margin_x, margin_y, tft.width() - 2 * margin_x, tft.height() - 2 * margin_y, "Benchmark", true};
                animate_window_open(benchWin);
                app_open[APP_BENCHMARK] = true;
                focus_state = FOCUS_WINDOW;
                bench_result[0] = 0;
                draw_desktop();
                Serial.println("[DESKTOP] Benchmark opened");
            }
            else if (desktop_selected == APP_SETTINGS)
            {
                Window settingsWin = {60, 70, tft.width() - 120, tft.height() - 120, "Settings", true};
                animate_window_open(settingsWin);
                app_open[APP_SETTINGS] = true;
                focus_state = FOCUS_WINDOW;
                draw_desktop();
                Serial.println("[DESKTOP] Settings opened");
            }
            else if (desktop_selected == APP_ACTIVITY_MONITOR)
            {
                Window actWin = {80, 60, tft.width() - 160, tft.height() - 120, "Activity Monitor", true};
                animate_window_open(actWin);
                app_open[APP_ACTIVITY_MONITOR] = true;
                focus_state = FOCUS_WINDOW;
                draw_desktop();
                Serial.println("[DESKTOP] Activity Monitor opened");
            }
        }
        else if (focus_state == FOCUS_MENU)
        {
            Serial.print("[MENU] Selected: ");
            if (menu_selected == 0)
                Serial.println("File");
            else if (menu_selected == 1)
                Serial.println("Edit");
            else if (menu_selected == 2)
                Serial.println("View");
        }
    }
    else if (strcmp(cmd, "close") == 0)
    {
        tft.startWrite();
        if (app_open[APP_TERMINAL])
        {
            Window termWin = {20, 30, tft.width() - 40, tft.height() - 40, "Terminal", true};
            animate_window_close(termWin);
        }
        if (app_open[APP_FILES])
        {
            Window filesWin = {40, 50, tft.width() - 80, tft.height() - 80, "Files", true};
            animate_window_close(filesWin);
        }
        if (app_open[APP_BENCHMARK])
        {
            int margin_x = 30;
            int margin_y = 30;
            Window benchWin = {margin_x, margin_y, tft.width() - 2 * margin_x, tft.height() - 2 * margin_y, "Benchmark", true};
            animate_window_close(benchWin);
        }
        if (app_open[APP_SETTINGS])
        {
            Window settingsWin = {60, 70, tft.width() - 120, tft.height() - 120, "Settings", true};
            animate_window_close(settingsWin);
        }
        if (app_open[APP_ACTIVITY_MONITOR])
        {
            Window actWin = {80, 60, tft.width() - 160, tft.height() - 120, "Activity Monitor", true};
            animate_window_close(actWin);
        }
        tft.endWrite();
        for (int i = 0; i < APP_COUNT; ++i)
            app_open[i] = false;
        focus_state = FOCUS_DESKTOP;
        tab_count = 1;
        needs_redraw = true;
        draw_desktop();
        Serial.println("[DESKTOP] All apps closed");
    }
    else if (strcmp(cmd, "tab") == 0)
    {
        if (focus_state == FOCUS_DESKTOP)
        {
            if (tab_count == 0)
            {
                tab_count = 1;
                draw_desktop(true);
                Serial.print("[DESKTOP] Selected: ");
                if (desktop_selected == APP_TERMINAL)
                    Serial.println("Terminal");
                else if (desktop_selected == APP_FILES)
                    Serial.println("Files");
            }
            else
            {
                desktop_selected = (desktop_selected + 1) % desktop_app_count;
                draw_desktop(true);
                Serial.print("[DESKTOP] Selected: ");
                if (desktop_selected == APP_TERMINAL)
                    Serial.println("Terminal");
                else if (desktop_selected == APP_FILES)
                    Serial.println("Files");
            }
        }
        else if (focus_state == FOCUS_WINDOW)
        {
            focus_state = FOCUS_MENU;
            menu_selected = 0;
            draw_desktop();
            Serial.println("[WINDOW] Menu bar focused");
        }
        else if (focus_state == FOCUS_MENU)
        {
            menu_selected = (menu_selected + 1) % menu_count;
            draw_desktop();
            Serial.print("[MENU] Selected: ");
            if (menu_selected == 0)
                Serial.println("File");
            else if (menu_selected == 1)
                Serial.println("Edit");
            else if (menu_selected == 2)
                Serial.println("View");
        }
    }
    else if (strcmp(cmd, "shift-tab") == 0)
    {
        if (focus_state == FOCUS_MENU)
        {
            focus_state = FOCUS_WINDOW;
            draw_desktop();
            Serial.println("[WINDOW] Window focused");
        }
        else if (focus_state == FOCUS_WINDOW)
        {
            focus_state = FOCUS_DESKTOP;
            tab_count = 1;
            draw_desktop();
            Serial.println("[DESKTOP] Desktop focused");
        }
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
    if (millis() - last_frame > FRAME_DELAY)
    {
        last_frame = millis();
        if (ui_state == UI_LOGIN)
        {
            if (millis() - login_start_time > 1500 && needs_redraw)
            {
                draw_desktop();
                ui_state = UI_DESKTOP;
                needs_redraw = false;
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
                    draw_terminal_content(cursor_visible);
                    cursor_visible = !cursor_visible;
                    last_cursor_toggle = millis();
                }
            }
        }
    }
}