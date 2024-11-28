#include "buzzer.h"
#include <math.h>

static struct {
    buzzer_settings settings;
    melody melody;
    uint8_t note_index;
} state;

// Dragon Quest melody
note dragon_quest_melody_notes[] = {
    {375, G3},  {125, G3},  {500, C4}, {500, D4},       {500, E4},       {500, F4},  {500, G4},  {1000, C5}, {375, B4},
    {125, A4},  {750, A4},  {500, G4}, {250, F4_SHARP}, {250, F4_SHARP}, {250, A4},  {500, G4},  {1000, E4}, {375, E3},
    {125, E3},  {500, E3},  {500, E3}, {500, F3_SHARP}, {500, G3_SHARP}, {1250, A3}, {250, A3},  {250, B3},  {250, C4},
    {1250, D4}, {250, A3},  {250, A3}, {250, C4},       {500, C4},       {500, B3},  {500, A3},  {500, G3},  {1250, E4},
    {250, F4},  {250, E4},  {250, D4}, {1000, C4},      {500, A3},       {500, C4},  {1250, D4}, {250, E4},  {250, D4},
    {250, C4},  {1000, C4}, {500, B3}, {500, G3},       {1250, G4},      {250, E4},  {250, F4},  {250, G4},  {1250, A4},
    {250, A3},  {250, B3},  {250, C4}, {1000, F4},      {1000, E4},      {2000, C4},
};
melody dragon_quest_melody = {dragon_quest_melody_notes, 60, 0};

// Recovery melody
note recovery_notes[] = {{750, E3}, {750, C5}};
melody recovery_melody = {recovery_notes, 2, 1};

// Startup melody
note start_notes[] = {{100, C4}, {100, D4}, {100, E4}, {100, F4}, {100, G4}, {100, A4}, {100, B4}};
melody start_melody = {start_notes, 7, 1};

// Error melody
note error_notes[] = {{250, E3}, {250, 0}};
melody error_melody = {error_notes, 2, 1};

/*
TODO: calcolare la lunghezza della melodia in buzzer_melody_start in modo che non debba essere specificato alla definizione
della melodia per evitare overhead. valutare se toglierlo come parametro e metterlo nella libreria come variabile globale.
*/

void buzzer_setup(buzzer_settings *settings) { state.settings = *settings; }

void buzzer_setup_mode(buzzer_mode mode) { state.settings.mode = mode; }

void buzzer_setup_type(buzzer_type type) { state.settings.mode = type; }

// Starts to play a melody for a certain buzzer type
void buzzer_melody_start(melody melody) {
    state.melody = melody;
    state.note_index = 0;

    if (state.settings.mode == BUZZER_MODE_NON_BLOCKING) {

        if (state.settings.type == BUZZER_TYPE_PASSIVE) {
            HAL_TIM_PWM_Start(&state.settings.config.pwm_config.pwm_timer, state.settings.config.pwm_config.tim_channel);
        } else if (state.settings.type == BUZZER_TYPE_ACTIVE) {
            HAL_GPIO_TogglePin(&state.settings.config.gpio_config.gpio_port, state.settings.config.gpio_config.gpio_pin);
        }

        HAL_TIM_Base_Start_IT(&state.settings.duration_timer);
        HAL_TIM_Base_Start(&state.settings.duration_timer);

    } else if (state.settings.mode == BUZZER_MODE_BLOCKING) {
        buzzer_blocking_melody();
    }
}

// Makes the buzzer stop any melody
void buzzer_melody_stop(void) {

	if (state.settings.type == BUZZER_TYPE_PASSIVE) {
		HAL_TIM_PWM_Stop(&state.settings.config.pwm_config.pwm_timer, state.settings.config.pwm_config.tim_channel);
	} else if (state.settings.type == BUZZER_TYPE_ACTIVE) {
		HAL_GPIO_TogglePin(&state.settings.config.gpio_config.gpio_port, state.settings.config.gpio_config.gpio_pin);
	}

	HAL_TIM_Base_Stop_IT(&state.settings.duration_timer);
	HAL_TIM_Base_Stop(&state.settings.duration_timer);


}

// Makes the buzzer play a melody, blocking other activities until the melody ends
void buzzer_blocking_melody(void) {

	HAL_TIM_PWM_Start(&state.settings.config.pwm_config.pwm_timer, state.settings.config.pwm_config.tim_channel);

	while (state.note_index < state.melody.length || state.melody.loop) {

		note current_note = state.melody.notes[state.note_index];

		if (state.settings.type == BUZZER_TYPE_PASSIVE) {
			buzzer_set_PWM_frequency(FREQUENCY(current_note.key));
		}

		HAL_Delay(current_note.duration);

		state.note_index++;

		if (state.note_index >= state.melody.length && state.melody.loop)
			state.note_index = 0;

	}

    // After the melody, stop any PWM or GPIO toggling
    buzzer_melody_stop();
}

// Set the PWM frequency
void buzzer_set_PWM_frequency(uint32_t frequency) {
    if (frequency == 0) {
        // Turns off the PWM if the frequency is 0 (0% duty cycle)
        __HAL_TIM_SET_COMPARE(&state.settings.config.pwm_config.pwm_timer, state.settings.config.pwm_config.tim_channel, 0);
    } else {

    	uint32_t timer_clock = SystemCoreClock; // Frequency at witch the TIM2 is working (APB1)

		//uint32_t autoreload = __HAL_TIM_GET_AUTORELOAD(&state.settings.config.pwm_config.pwm_timer);
    	//uint32_t prescaler = (uint32_t)(timer_clock / ((frequency - 1) * autoreload)) - 1;
        //__HAL_TIM_SET_PRESCALER(&state.settings.config.pwm_config.pwm_timer, prescaler);

		//uint32_t prescaler = (timer_clock / (frequency * (autoreload + 1))) - 1; // Calculates and sets the prescaler to output the correct frequency

        uint32_t autoreload = timer_clock / ((frequency - 1));
    	__HAL_TIM_SET_AUTORELOAD(&state.settings.config.pwm_config.pwm_timer, autoreload);

        __HAL_TIM_SET_COMPARE(&state.settings.config.pwm_config.pwm_timer, state.settings.config.pwm_config.tim_channel, autoreload / 2); // Sets the CCR register to half of the autoreload register, 50% duty cycle
    }
}

// Handles the Interrupt Request of the furation timer
void buzzer_IRQHandler(void) {

	// If the buzzer is passive, we can setup the frequency for every note
	if (state.settings.type == BUZZER_TYPE_PASSIVE) {
		buzzer_set_PWM_frequency(FREQUENCY(state.melody.notes[state.note_index].key));
	}

	// Calculates and sets the duration
	uint32_t autoreload_value = (((float)state.melody.notes[state.note_index].duration / 1000.0f) *
            					 ((SystemCoreClock) / (state.settings.duration_timer.Instance->PSC + 1)));
	__HAL_TIM_SET_AUTORELOAD(&state.settings.duration_timer, autoreload_value);


	state.note_index++;

	// Se hai raggiunto la fine della melodia e loop è attivo, ricomincia
	if (state.note_index >= state.melody.length) {
		if (state.melody.loop) {
			state.note_index = 0; // Restarts the melody
		} else {
			buzzer_melody_stop(); // Stops the melody
			return;
		}
	}
	__HAL_TIM_CLEAR_IT(&state.settings.duration_timer, TIM_IT_UPDATE);
}
