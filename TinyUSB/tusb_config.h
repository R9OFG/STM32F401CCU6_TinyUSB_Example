/*
 * tusb_config.h
 *
 * Author: R9OFG.RU
 *
 * MCU: STM32F401CCU6
 * Составное устройство: CDC + UAC1 (радиоприемник)
 */

#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

#ifdef __cplusplus
 extern "C" {
#endif

//--------------------------------------------------------------------+
// Common
//--------------------------------------------------------------------+

#define CFG_TUSB_MCU               OPT_MCU_STM32F4
#define CFG_TUSB_OS                OPT_OS_NONE
#define CFG_TUSB_RHPORT0_MODE      (OPT_MODE_DEVICE | OPT_MODE_FULL_SPEED)

#define CFG_TUD_ENDPOINT0_SIZE     64

//--------------------------------------------------------------------+
// CDC
//--------------------------------------------------------------------+

#define CFG_TUD_CDC                1
#define CFG_TUD_CDC_RX_BUFSIZE     64
#define CFG_TUD_CDC_TX_BUFSIZE     64

//--------------------------------------------------------------------+
// Audio (UAC1) – IN only
//--------------------------------------------------------------------+

#define CFG_TUD_AUDIO                          1

#define CFG_TUD_AUDIO_ENABLE_EP_IN             1
#define CFG_TUD_AUDIO_ENABLE_EP_OUT            0
#define CFG_TUD_AUDIO_ENABLE_FEEDBACK_EP       0

// -------- Audio Function 1 --------

#define CFG_TUD_AUDIO_FUNC_1_CTRL_BUF_SZ       64
#define CFG_TUD_AUDIO_FUNC_1_N_AS_INT          1

#define CFG_TUD_AUDIO_FUNC_1_FORMAT            AUDIO_FORMAT_TYPE_I
#define CFG_TUD_AUDIO_FUNC_1_SAMPLE_RATE       48000
#define CFG_TUD_AUDIO_FUNC_1_CHANNEL_COUNT     2
#define CFG_TUD_AUDIO_FUNC_1_RESOLUTION        16
#define CFG_TUD_SOF_CALLBACK 				   1  // Включить колбэк SOF

// 48 kHz × 2 канала × 2 байта = 192 байта на 1 мс
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SZ_MAX      192

// Программный буфер (минимум ×2 от пакета)
#define CFG_TUD_AUDIO_FUNC_1_EP_IN_SW_BUF_SZ   384

#ifdef __cplusplus
 }
#endif

#endif /* _TUSB_CONFIG_H_ */
