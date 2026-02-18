/*
 * usb_audio.c
 *
 * Author: R9OFG.RU
 * https://r9ofg.ru
 *
 * Аудио-модуль для составного устройства CDC + UAC1
 * TinyUSB UAC1 IN-only (микрофонная эмуляция)
 * Генерация комплексного тона 1кГц (I/Q) по команде gen_on через CDC
 * При генерации выключен: шумовая полка -96 dBFS (+/-1) вместо цифрового нуля
 */

#include "usb_audio.h"
#include "audio_generator.h"
#include "tusb.h"
#include "led_pcb.h"
#include <string.h>

// Буфер изохронной передачи: 48 фреймов × 2 канала (I/Q) × 2 байта = 192 байт
// Выравнивание 4 байта требуется для корректной работы с USB DMA на STM32
static int16_t audio_buf[48 * 2] __attribute__((aligned(4)));

//--------------------------------------------------------------------+
// Обработчики контрольных запросов к аудио-сущностям (Feature Unit 0x03)
// Вызываются при запросах хоста на чтение/запись параметров громкости/мьюта
//--------------------------------------------------------------------+

bool tud_audio_get_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const * p_request)
{
  (void) rhport;

  uint8_t entityID = TU_U16_HIGH(p_request->wIndex);  // Старший байт = ID сущности
  uint8_t ctrlSel  = TU_U16_HIGH(p_request->wValue);  // Старший байт = тип контроля

  // Обрабатываем только запросы к Feature Unit (ID=0x03 из дескрипторов)
  if (entityID != 0x03) return false;

  // Запрос состояния мьюта: всегда возвращаем 0 (размучено)
  if (ctrlSel == AUDIO10_FU_CTRL_MUTE)
  {
    uint8_t mute = 0;
    return tud_control_xfer(rhport, p_request, &mute, 1);
  }

  // Запрос диапазона громкости: минимум -127.5 дБ (0x8000), максимум 0 дБ (0x0000)
  if (ctrlSel == AUDIO10_FU_CTRL_VOLUME)
  {
    static const int16_t volume[2] = {0x0000, 0x8000};
    uint8_t len = tu_min8(p_request->wLength, 2);
    return tud_control_xfer(rhport, p_request, (void*)volume, len);
  }

  return false;  // Неизвестный запрос — отклоняем
}

// Приём команд мьюта/громкости от хоста (игнорируем, только для совместимости)
bool tud_audio_set_req_entity_cb(uint8_t rhport,
                                 tusb_control_request_t const * p_request,
                                 uint8_t *pBuff)
{
  (void) rhport;
  (void) p_request;
  (void) pBuff;
  return true;  // Принимаем все команды без изменений состояния
}

//--------------------------------------------------------------------+
// Обработчик установки альтернативного интерфейса
// Вызывается при включении (alt=1) или выключении (alt=0) аудиопотока хостом
// ВАЖНО: генератор НЕ включается автоматически — только по команде gen_on через CDC
//--------------------------------------------------------------------+

bool tud_audio_set_itf_cb(uint8_t rhport,
                          tusb_control_request_t const * p_request)
{
  (void) rhport;

  uint8_t itf = TU_U16_LOW(p_request->wIndex);  // Номер интерфейса
  uint8_t alt = TU_U16_LOW(p_request->wValue);  // Альтернативная настройка

  // Обрабатываем только AudioStreaming Interface (номер 3 согласно дескрипторам)
  if (itf == 3)
  {
    if (alt == 1)
    {
      // Хост запросил включение аудиопотока (начал запись)
      LED_OFF();  // Индикация активного стриминга

      // Первый пакет ОБЯЗАТЕЛЕН для запуска изохронного потока
      // Содержимое определяет генератор: сигнал+шум (ON) или только шум (OFF)
      audio_generator_fill_buffer(audio_buf, 48);
      tud_audio_write(audio_buf, sizeof(audio_buf));
    }
    else
    {
      // Хост запросил выключение аудиопотока (остановил запись)
      LED_ON();  // Индикация ожидания
      // Очередь передачи очистится автоматически при следующих write
    }
  }

  return true;
}

//--------------------------------------------------------------------+
// Callback завершения изохронной передачи (вызывается из контекста прерывания)
// Должен быть максимально быстрым — заполняем буфер и отправляем следующий пакет
//--------------------------------------------------------------------+

bool tud_audio_tx_done_isr(uint8_t rhport,
                           uint16_t n_bytes_sent,
                           uint8_t func_id,
                           uint8_t ep_in,
                           uint8_t cur_alt_setting)
{
  (void) rhport;
  (void) n_bytes_sent;
  (void) func_id;
  (void) ep_in;
  (void) cur_alt_setting;

  // Заполнение следующего пакета: генератор сам решает содержимое
  // AUDIO_GEN_STATE_ON  → комплексный тон 1кГц (S9+20) + шум +/-1
  // AUDIO_GEN_STATE_OFF → только шум +/-1 (-96 dBFS), имитация "пустого эфира"
  audio_generator_fill_buffer(audio_buf, 48);

  // Отправка следующего пакета (192 байта = 1 мс аудио при 48 кГц)
  tud_audio_write(audio_buf, sizeof(audio_buf));

  return true;
}
