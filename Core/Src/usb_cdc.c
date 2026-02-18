/*
 * usb_cdc.c
 *
 * Author: R9OFG.RU
 * https://r9ofg.ru
 *
 * CDC-модуль для составного устройства CDC + UAC1
 * Обработка команд управления генератором через виртуальный COM-порт
 */

#include "usb_cdc.h"
#include "led_pcb.h"
#include "audio_generator.h"

// Флаги состояния (глобальные для доступа из main.c)
bool usb_mounted = false;          // Флаг подключения к хосту
bool usb_suspended = false;        // Флаг режима энергосбережения
static bool terminal_opened = false;  // Флаг открытия терминала на ПК
static bool greeting_sent = false;    // Флаг отправки приветствия

// Инициализация
void usb_cdc_init(void)
{
  usb_mounted = false;
  usb_suspended = false;
  terminal_opened = false;
  greeting_sent = false;
  LED_OFF();  // Светодиод выключен при старте (ожидание подключения)
}

// Обработчик подключения устройства к хосту (кабель вставлен)
void tud_mount_cb(void)
{
  usb_mounted = true;
  usb_suspended = false;
  LED_ON();  // Индикация: устройство подключено к ПК
}

// Обработчик отключения устройства от хоста (кабель вынут)
void tud_umount_cb(void)
{
  usb_mounted = false;
  usb_suspended = true;
  terminal_opened = false;
  greeting_sent = false;
  LED_OFF();  // Индикация: устройство отключено
}

// Обработчик перехода хоста в режим энергосбережения (suspend)
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;  // Не используется
  usb_suspended = true;
  LED_OFF();  // Гасим светодиод в suspend для экономии энергии
}

// Обработчик выхода хоста из режима энергосбережения (resume)
void tud_resume_cb(void)
{
  usb_suspended = false;
  if (usb_mounted)
  {
    LED_ON();  // Включаем индикацию при выходе из suspend
  }
}

// Безопасная отправка одного символа с таймаутом 10 мс
static void cdc_write_char_safe(char c)
{
  uint32_t start = HAL_GetTick();
  while (!tud_cdc_write_available())
  {
    if (HAL_GetTick() - start > 10) return;  // Таймаут защиты
    tud_task();  // Обслуживаем стек USB во время ожидания
  }
  tud_cdc_write_char(c);
}

// Безопасная отправка строки посимвольно с финальной отправкой буфера
static void cdc_write_str_safe(const char *str)
{
  if (!str) return;
  while (*str)
  {
    cdc_write_char_safe(*str++);
  }
  tud_cdc_write_flush();  // Принудительная отправка буфера в хост
}

// Основной цикл обработки CDC-терминала
void usb_cdc_task(void)
{
  if (!usb_mounted || usb_suspended) return;  // Нет подключения или в suspend

  bool connected = tud_cdc_connected();  // Терминал открыт на стороне ПК

  // Первое открытие терминала — сброс состояния и подготовка к приветствию
  if (connected && !terminal_opened)
  {
    terminal_opened = true;
    greeting_sent = false;

    // Очищаем мусор из буфера (остатки от предыдущих сессий)
    while (tud_cdc_available()) (void)tud_cdc_read_char();
    return;
  }

  // Отправка приветственного сообщения при первом подключении терминала
  if (connected && terminal_opened && !greeting_sent)
  {
    cdc_write_str_safe("\r\nSDR_DEV ready.\r\n");
    cdc_write_str_safe("Commands:\r\n gen_on\r\n gen_off\r\n");
    cdc_write_str_safe("SDR_DEV> ");
    greeting_sent = true;
    return;
  }

  // Защита: терминал не открыт или нет подключения
  if (!connected || !terminal_opened) return;

  static char cmd_buf[16] = {0};   // Буфер команд (макс. 15 символов + '\0')
  static uint8_t cmd_idx = 0;      // Текущая позиция в буфере

  // Обработка всех доступных символов из буфера приёмника
  while (tud_cdc_available())
  {
    char ch = tud_cdc_read_char();

    // Эхо латинских символов и цифр (32..126 = печатные ASCII)
    if (ch >= 32 && ch <= 126) cdc_write_char_safe(ch);

    // Обработка окончания команды (Enter)
    if (ch == '\r' || ch == '\n')
    {
      cdc_write_str_safe("\r\n\r\n");  // Двойной перевод строки для читаемости
      if (cmd_idx > 0)
      {
        cmd_buf[cmd_idx] = '\0';  // Завершаем строку нулём

        // Команда включения генератора: 1кГц комплексный тон (I/Q) + шум
        if (strcmp(cmd_buf, "gen_on") == 0)
        {
          audio_generator_set_state(AUDIO_GEN_STATE_ON);
          cdc_write_str_safe("Audio generator: ON (IQ +1kHz S9+20 sine)\r\n");
        }
        // Команда выключения генератора: остаётся только шумовая полка
        else if (strcmp(cmd_buf, "gen_off") == 0)
        {
          audio_generator_set_state(AUDIO_GEN_STATE_OFF);
          cdc_write_str_safe("Audio generator: OFF\r\n");
        }
        // Неизвестная команда — вывод справки
        else
        {
          cdc_write_str_safe("Unknown command.\r\nAvailable:\r\n gen_on\r\n gen_off\r\n");
        }
        cmd_idx = 0;  // Сброс буфера после обработки
      }
      cdc_write_str_safe("SDR_DEV> ");  // Приглашение к вводу следующей команды
      return;
    }

    // Накопление символов команды
    if (ch >= 32 && ch <= 126 && cmd_idx < sizeof(cmd_buf) - 1)
    {
      cmd_buf[cmd_idx++] = ch;  // Добавляем печатный символ в буфер
    }
    // Обработка клавиши Backspace/Delete
    else if ((ch == 0x08 || ch == 0x7F) && cmd_idx > 0)
    {
      cmd_idx--;
      cdc_write_str_safe("\b \b");  // Стираем символ на экране терминала
    }
    // Сброс буфера при управляющих символах (кроме Backspace)
    else if (ch < 32) cmd_idx = 0;
  }
}
