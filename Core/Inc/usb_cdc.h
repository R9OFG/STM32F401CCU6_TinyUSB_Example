/*
 * usb_cdc.h
 *
 * Author: R9OFG.RU
 *
 * CDC-модуль для составного устройства CDC + UAC1
 */

#ifndef USB_CDC_H_
#define USB_CDC_H_

#include "tusb.h"

void usb_cdc_init(void);
void usb_cdc_task(void);

// Обработчики событий TinyUSB (автоматически вызываются)
void tud_mount_cb(void);
void tud_umount_cb(void);
void tud_suspend_cb(bool remote_wakeup_en);
void tud_resume_cb(void);

#endif
