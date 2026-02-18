/*
 * audio_generator.h
 *
 * Author: R9OFG.RU
 *
 * Генератор тестового аудиосигнала для UAC1
 * 1 kHz complex tone (I/Q) at S9+20 level (-53 dBFS) with noise floor
 * When OFF: noise floor only (-96 dBFS)
 */

#ifndef AUDIO_GENERATOR_H_
#define AUDIO_GENERATOR_H_

#include <stdint.h>

typedef enum {
  AUDIO_GEN_STATE_OFF = 0,  // Только шум (-96 dBFS)
  AUDIO_GEN_STATE_ON  = 1   // Сигнал S9+20 + шум
} audio_gen_state_t;

void audio_generator_init(void);
void audio_generator_set_state(audio_gen_state_t state);
audio_gen_state_t audio_generator_get_state(void);
void audio_generator_fill_buffer(int16_t *buffer, uint32_t n_frames);

#endif
