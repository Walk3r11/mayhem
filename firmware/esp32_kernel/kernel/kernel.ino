/*
 * ESP32 Kernel v1.0
 * UART-based task sharing between two ESP32s
 */
#include <Arduino.h>

// ====== UART CONFIGURATION ======
#define UART_RX 16
#define UART_TX 17
#define UART_BAUD 115200

// ====== LED CONFIGURATION ======
#define GREEN_LED 12
#define RED_LED 13

// ====== KERNEL CONSTANTS ======
#define HEAP_SIZE 8192

// ====== PROCESS CONTROL BLOCK ======
typedef enum
{
    PROCESS_READY,
    PROCESS_RUNNING,
    PROCESS_BLOCKED,
    PROCESS_TERMINATED
} ProcessState;

struct Process
{
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

// ====== SYSTEM CALLS ======
enum SysCall
{
    SYS_YIELD = 1,
    SYS_EXIT = 2,
    SYS_LOG = 3
};

// ====== UART TASK SHARING PROTOCOL ======
// Message format: "CREATE:<pid>:<priority>" or "DESTROY:<pid>"
void send_create_process(int pid, int priority)
{
    char msg[32];
    snprintf(msg, sizeof(msg), "CREATE:%d:%d\n", pid, priority);
    Serial2.print(msg);
}
void send_destroy_process(int pid)
{
    char msg[32];
    snprintf(msg, sizeof(msg), "DESTROY:%d\n", pid);
    Serial2.print(msg);
}
void handle_uart_message(const char *msg)
{
    if (strncmp(msg, "CREATE:", 7) == 0)
    {
        int pid, priority;
        if (sscanf(msg + 7, "%d:%d", &pid, &priority) == 2)
        {
            // Only create a stub process (no entry_point)
            Process *p = (Process *)malloc(sizeof(Process));
            if (!p)
                return;
            p->pid = pid;
            p->state = PROCESS_READY;
            p->entry_point = NULL;
            p->priority = priority;
            p->last_run = 0;
            p->next = process_list;
            process_list = p;
            Serial.printf("[UART] Created remote process: PID=%d, priority=%d\n", pid, priority);
        }
    }
    else if (strncmp(msg, "DESTROY:", 8) == 0)
    {
        int pid;
        if (sscanf(msg + 8, "%d", &pid) == 1)
        {
            Process **pp = &process_list;
            while (*pp)
            {
                if ((*pp)->pid == pid)
                {
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

// ====== KERNEL CORE ======
int create_process(const char *name, void (*entry_point)(), int priority)
{
    Process *p = (Process *)malloc(sizeof(Process));
    if (!p)
        return -1;
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

void destroy_process(int pid)
{
    Process **pp = &process_list;
    while (*pp)
    {
        if ((*pp)->pid == pid)
        {
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

void schedule()
{
    Process *p = process_list;
    Process *next_proc = NULL;
    unsigned long now = millis();
    int highest_priority = -1;
    while (p)
    {
        if (p->state == PROCESS_READY && p->priority > highest_priority)
        {
            highest_priority = p->priority;
            next_proc = p;
        }
        p = p->next;
    }
    if (next_proc && next_proc != running_process)
    {
        if (running_process)
            running_process->state = PROCESS_READY;
        running_process = next_proc;
        running_process->state = PROCESS_RUNNING;
        running_process->last_run = now;
        Serial.printf("[SCHEDULER] Switched to PID: %d\n", running_process->pid);
    }
}

int syscall(int syscall_num, int arg1, int arg2, int arg3)
{
    switch (syscall_num)
    {
    case SYS_YIELD:
        if (running_process)
            running_process->state = PROCESS_READY;
        return 0;
    case SYS_EXIT:
        if (running_process)
        {
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

// ====== UART RECEIVE HANDLER ======
#define UART_BUF_SIZE 128
char uart_buf[UART_BUF_SIZE];
int uart_buf_pos = 0;
void poll_uart()
{
    while (Serial2.available())
    {
        char c = Serial2.read();
        if (c == '\n' || uart_buf_pos >= UART_BUF_SIZE - 1)
        {
            uart_buf[uart_buf_pos] = 0;
            handle_uart_message(uart_buf);
            uart_buf_pos = 0;
        }
        else
        {
            uart_buf[uart_buf_pos++] = c;
        }
    }
}

// ====== KERNEL INIT ======
void kernel_init()
{
    pinMode(GREEN_LED, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    Serial2.begin(UART_BAUD, SERIAL_8N1, UART_RX, UART_TX);
    Serial.println("[KERNEL] Kernel initialized");
    create_process("led_task", NULL, 10);
    // Add your own process creation here
}

// ====== ARDUINO SETUP/LOOP ======
void setup()
{
    Serial.begin(115200);
    delay(1000);
    kernel_init();
    Serial.println("[KERNEL] System ready - starting scheduler");
}

void loop()
{
    poll_uart();
    schedule();
    if (running_process && running_process->state == PROCESS_RUNNING && running_process->entry_point)
    {
        running_process->entry_point();
    }
    delay(10);
}