
#ifndef BUZZER_H_
#define BUZZER_H_

#include "main.h"

/*
For more information on the implementation of how notes are mapped and relative frequency is calculated refer checkout
this wikipedia page: https://en.wikipedia.org/wiki/Piano_key_frequencies
*/

#define FREQUENCY(key_code) ((key_code) >= C3 ? (440.0f * pow(2.0f, ((key_code) - A4) / 12.0f)) : 0)

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

typedef enum { BUZZER_TYPE_ACTIVE, BUZZER_TYPE_PASSIVE } buzzer_type;
typedef enum { BUZZER_MODE_BLOCKING, BUZZER_MODE_NON_BLOCKING } buzzer_mode;

// Maps to piano keys
typedef enum {

    // Octave 3
    C3 = 28,
    C3_SHARP,
    D3_FLAT = C3_SHARP,
    D3,
    D3_SHARP,
    E3_FLAT = D3_SHARP,
    E3,
    F3,
    F3_SHARP,
    G3_FLAT = F3_SHARP,
    G3,
    G3_SHARP,
    A3_FLAT = G3_SHARP,
    A3,
    A3_SHARP,
    B3_FLAT = A3_SHARP,
    B3,

    // Octave 4
    C4,
    C4_SHARP,
    D4_FLAT = C4_SHARP,
    D4,
    D4_SHARP,
    E4_FLAT = D4_SHARP,
    E4,
    F4,
    F4_SHARP,
    G4_FLAT = F4_SHARP,
    G4,
    G4_SHARP,
    A4_FLAT = G4_SHARP,
    A4,
    A4_SHARP,
    B4_FLAT = A4_SHARP,
    B4,

    // Octave 5
    C5,
    C5_SHARP,
    D5_FLAT = C5_SHARP,
    D5,
    D5_SHARP,
    E5_FLAT = D5_SHARP,
    E5,
    F5,
    F5_SHARP,
    G5_FLAT = F5_SHARP,
    G5,
    G5_SHARP,
    A5_FLAT = G5_SHARP,
    A5,
    A5_SHARP,
    B5_FLAT = A5_SHARP,
    B5

} key_t;

// Models a note as a key code, and duration
typedef struct {
    uint32_t duration;
    key_t key;
} note;

// Models a melody as collection of notes a
typedef struct {
    note *notes;
    uint8_t length;
    uint8_t loop; // melody has to restart once its finished
} melody;

typedef struct {
    buzzer_mode mode;
    buzzer_type type;
    TIM_HandleTypeDef duration_timer;
    
    union {

        struct {
            GPIO_TypeDef gpio_port;
            uint32_t gpio_pin;
        } gpio_config;

        struct {
            TIM_HandleTypeDef pwm_timer;
            uint32_t tim_channel;
        } pwm_config;

    } config;

} buzzer_settings;

// built-in melodies
extern melody dragon_quest_melody;
extern melody recovery_melody;
extern melody start_melody;
extern melody error_melody;

void buzzer_melody_start(melody melody);
void buzzer_setup(buzzer_settings *settings);
void buzzer_setup_mode(buzzer_mode mode);
void buzzer_setup_type(buzzer_type type);
void buzzer_blocking_melody(void);
void buzzer_melody_stop(void);
void buzzer_note_play(note note);
void buzzer_IRQHandler(void);
void buzzer_set_PWM_frequency(uint32_t frequency);

#endif
