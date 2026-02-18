/*
 * usb_audio.h
 *
 * Author: R9OFG.RU
 *
 * Аудио-модуль для составного устройства CDC + UAC1
 * Обработчики контрольных запросов UAC1
 */

#ifndef USB_AUDIO_H_
#define USB_AUDIO_H_

#include "tusb.h"

// Обработчики контрольных запросов UAC1
bool tud_audio_get_req_entity_cb(uint8_t rhport, tusb_control_request_t const * p_request);
bool tud_audio_set_req_entity_cb(uint8_t rhport, tusb_control_request_t const * p_request, uint8_t *pBuff);
bool tud_audio_set_itf_cb(uint8_t rhport, tusb_control_request_t const * p_request);

#endif
