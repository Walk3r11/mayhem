#include <TFT_eSPI.h>

#define UART_RX 16
#define UART_TX 17
#define UART_BAUD 115200

#define GREEN_LED 12
#define RED_LED 13

TFT_eSPI tft = TFT_eSPI();

enum ProcessState {
  PROCESS_READY,
  PROCESS_RUNNING,
  PROCESS_BLOCKED,
  PROCESS_TERMINATED
};

struct Process {
  int pid;
  ProcessState state;
  void (*entry_point)();
  int priority;
  unsigned long last_run;
  struct Process *next;
};

Process *process_list = NULL;
Process *running_process = NULL;
int next_pid = 1;

enum SysCall {
  SYS_YIELD = 1,
  SYS_EXIT = 2,
  SYS_LOG = 3
};

void send_create_process(int pid, int priority) {
  char msg[32];
  snprintf(msg, sizeof(msg), "CREATE:%d:%d\n", pid, priority);
  Serial2.print(msg);
}

void send_destroy_process(int pid) {
  char msg[32];
  snprintf(msg, sizeof(msg), "DESTROY:%d\n", pid);
  Serial2.print(msg);
}

void handle_uart_message(const char *msg) {
  if (strncmp(msg, "CREATE:", 7) == 0) {
    int pid, priority;
    if (sscanf(msg + 7, "%d:%d", &pid, &priority) == 2) {
      Process *p = (Process *)malloc(sizeof(Process));
      if (!p) return;
      p->pid = pid;
      p->state = PROCESS_READY;
      p->entry_point = NULL;
      p->priority = priority;
      p->last_run = 0;
      p->next = process_list;
      process_list = p;
      Serial.printf("[UART] Created remote process: PID=%d, priority=%d\n", pid, priority);
    }
  } else if (strncmp(msg, "DESTROY:", 8) == 0) {
    int pid;
    if (sscanf(msg + 8, "%d", &pid) == 1) {
      Process **pp = &process_list;
      while (*pp) {
        if ((*pp)->pid == pid) {
          Process *to_delete = *pp;
          *pp = (*pp)->next;
          to_delete->state = PROCESS_TERMINATED;
          free(to_delete);
          Serial.printf("[UART] Destroyed remote process: PID=%d\n", pid);
          return;
        }
        pp = &((*pp)->next);
      }
    }
  }
}

int create_process(const char *name, void (*entry_point)(), int priority) {
  Process *p = (Process *)malloc(sizeof(Process));
  if (!p) return -1;
  p->pid = next_pid++;
  p->state = PROCESS_READY;
  p->entry_point = entry_point;
  p->priority = priority;
  p->last_run = 0;
  p->next = process_list;
  process_list = p;
  Serial.printf("[KERNEL] Created process: %s (PID: %d)\n", name, p->pid);
  send_create_process(p->pid, priority);
  return p->pid;
}

void destroy_process(int pid) {
  Process **pp = &process_list;
  while (*pp) {
    if ((*pp)->pid == pid) {
      Process *to_delete = *pp;
      *pp = (*pp)->next;
      to_delete->state = PROCESS_TERMINATED;
      Serial.printf("[KERNEL] Destroyed process: PID: %d\n", pid);
      send_destroy_process(pid);
      free(to_delete);
      return;
    }
    pp = &((*pp)->next);
  }
}

void schedule() {
  Process *p = process_list;
  Process *next_proc = NULL;
  unsigned long now = millis();
  int highest_priority = -1;
  while (p) {
    if (p->state == PROCESS_READY && p->priority > highest_priority) {
      highest_priority = p->priority;
      next_proc = p;
    }
    p = p->next;
  }
  if (next_proc && next_proc != running_process) {
    if (running_process)
      running_process->state = PROCESS_READY;
    running_process = next_proc;
    running_process->state = PROCESS_RUNNING;
    running_process->last_run = now;
    Serial.printf("[SCHEDULER] Switched to PID: %d\n", running_process->pid);
  }
}

int syscall(int syscall_num, int arg1, int arg2, int arg3) {
  switch (syscall_num) {
    case SYS_YIELD:
      if (running_process)
        running_process->state = PROCESS_READY;
      return 0;
    case SYS_EXIT:
      if (running_process) {
        running_process->state = PROCESS_TERMINATED;
        Serial.printf("[KERNEL] Process %d exited\n", running_process->pid);
      }
      return 0;
    case SYS_LOG:
      Serial.println((const char *)arg1);
      return 0;
    default:
      Serial.printf("[KERNEL] Unknown syscall: %d\n", syscall_num);
      return -1;
  }
}

#define UART_BUF_SIZE 128
char uart_buf[UART_BUF_SIZE];
int uart_buf_pos = 0;
void poll_uart() {
  while (Serial2.available()) {
    char c = Serial2.read();
    if (c == '\n' || uart_buf_pos >= UART_BUF_SIZE - 1) {
      uart_buf[uart_buf_pos] = 0;
      handle_uart_message(uart_buf);
      uart_buf_pos = 0;
    } else {
      uart_buf[uart_buf_pos++] = c;
    }
  }
}

void clear_screen() {
  tft.fillScreen(TFT_BLACK);
}

void print_centered(int y, const char *text, uint16_t color = TFT_WHITE) {
  tft.setTextColor(color);
  tft.setTextSize(2);
  int x = (tft.width() - tft.textWidth(text)) / 2;
  tft.setCursor(x, y);
  tft.print(text);
}

const char *correct_password = "1234";

void draw_login_ui() {
  tft.fillScreen(TFT_DARKGREY);
  tft.fillRoundRect(20, 40, 280, 180, 15, TFT_LIGHTGREY);
  tft.setTextColor(TFT_BLACK);
  tft.setTextSize(3);
  print_centered(60, "MAYHEM Login");
  tft.setTextSize(2);
  print_centered(120, "Enter Password:");
  tft.fillRoundRect(60, 150, 200, 30, 8, TFT_WHITE);
}

void login_process() {
  static bool init = false;
  static char input_buf[32];
  static int input_pos = 0;

  if (!init) {
    draw_login_ui();
    input_pos = 0;
    input_buf[0] = 0;
    init = true;
    Serial.println("Enter password:");
  }

  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\r' || c == '\n') {
      input_buf[input_pos] = 0;
      Serial.println();

      if (strcmp(input_buf, correct_password) == 0) {
        clear_screen();
        print_centered(40, "Login successful!", TFT_GREEN);
        delay(1000);
        destroy_process(running_process->pid);
        create_process("desktop", desktop_process, 10);
        return;
      } else {
        draw_login_ui();
        print_centered(200, "Wrong password", TFT_RED);
        Serial.println("Wrong password");
        input_pos = 0;
        input_buf[0] = 0;
      }
    } else if (c >= 32 && c < 127 && input_pos < 31) {
      input_buf[input_pos++] = c;
      input_buf[input_pos] = 0;
      tft.fillRoundRect(65, 155, 190, 20, 4, TFT_WHITE);
      tft.setCursor(70, 160);
      tft.setTextColor(TFT_BLACK);
      for (int i = 0; i < input_pos; i++) tft.print('*');
      Serial.print('*');
    }
  }

  syscall(SYS_YIELD, 0, 0, 0);
}

void draw_wallpaper() {
  int h = tft.height();
  int w = tft.width();
  for (int y = 0; y < h; y++) {
    uint8_t blend = map(y, 0, h, 30, 180);
    uint16_t color = tft.color565(blend / 3, blend / 3, blend);
    tft.drawFastHLine(0, y, w, color);
  }
}

void desktop_process() {
  static bool init = false;

  if (!init) {
    draw_wallpaper();
    print_centered(40, "MAYHEM Desktop", TFT_WHITE);
    print_centered(80, "Welcome, user!", TFT_CYAN);
    init = true;
  }

  syscall(SYS_YIELD, 0, 0, 0);
}

void kernel_init() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX, UART_TX);
  Serial.begin(115200);

  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);

  Serial.println("[KERNEL] Kernel initialized");

  create_process("login", login_process, 10);
}

void setup() {
  delay(1000);
  kernel_init();
  Serial.println("[KERNEL] System ready - starting scheduler");
}

void loop() {
  poll_uart();
  schedule();
  if (running_process && running_process->state == PROCESS_RUNNING && running_process->entry_point) {
    running_process->entry_point();
  }
}