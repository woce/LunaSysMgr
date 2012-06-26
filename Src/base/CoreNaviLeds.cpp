/* @@@LICENSE
*
*      Copyright (c) 2009-2012 Hewlett-Packard Development Company, L.P.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */




#include "Common.h"

#include "CoreNaviLeds.h"
#include "DeviceInfo.h"
#include "Settings.h"

CoreNaviLeds* CoreNaviLeds::m_instance = NULL;

#if !defined(TARGET_DESKTOP)
CoreNaviLeds::CoreNaviLeds() :
    m_device(NULL)
    ,m_config(NULL)
	,m_lightbarEnabled(!DeviceInfo::instance()->coreNaviButton() && Settings::LunaSettings()->lightbarEnabled)
{

	hal_error_t error = HAL_ERROR_SUCCESS;
	error = hal_device_open(HAL_DEVICE_LED_CONTROLLER, "Default", &m_device);

    if (error != HAL_ERROR_SUCCESS)
        g_critical("Could not open device led controller");

    memset(&m_effect, 0, sizeof(m_effect));
}

CoreNaviLeds::~CoreNaviLeds()
{
	hal_error_t error = HAL_ERROR_SUCCESS;
    error = hal_device_close(m_device);

    if (error != HAL_ERROR_SUCCESS)
        g_critical("Could not close device led controller");

}

void CoreNaviLeds::configureParameters(int n, ...)
{
    va_list ap;

    va_start(ap, n);

    hal_error_t error = HAL_ERROR_SUCCESS;

    for (int i = 0; i < n/2; i++)
    {
        led_controller_parameter_type_t parameter_type = (led_controller_parameter_type_t) va_arg(ap, int);
        int value = va_arg(ap, int);

        error = hal_led_controller_core_configuration_set_param(m_config, 
                parameter_type, value);
        if (error != HAL_ERROR_SUCCESS)
            g_critical("could not configureParameter for CoreNavi effect!");
    }

    va_end(ap);

}

void CoreNaviLeds::finalizeAndExecute()
{
    hal_error_t error = HAL_ERROR_SUCCESS;
    hal_led_controller_core_configuration_finalize(m_config);

    if (error != HAL_ERROR_SUCCESS)
        g_critical("Could not finalize CoreNavi effect!");

    m_effect.core_configuration = m_config;

	error = hal_led_controller_execute_effect(m_device, m_effect);
    if (error != HAL_ERROR_SUCCESS)
        g_critical("Could not execute CoreNavi effect!");
}

void CoreNaviLeds::initializeEffect(led_controller_effect_type_t effect_type, int led)
{
    hal_error_t error = HAL_ERROR_SUCCESS;

    if (m_config != NULL)
    {
        error = hal_led_controller_core_configuration_release(m_config);

        if (error != HAL_ERROR_SUCCESS)
            g_critical("Could not release CoreNavi effect configuration");
    }

    error = hal_led_controller_core_configuration_create(effect_type,
        &m_config);

    m_effect.required.effect = effect_type;
	m_effect.required.led = (led_controller_led_t)led;

    if (error != HAL_ERROR_SUCCESS)
        g_critical("Could not create CoreNavi effect configuration");
}

CoreNaviLeds* CoreNaviLeds::instance (void)
{
    if (NULL == CoreNaviLeds::m_instance)
        CoreNaviLeds::m_instance = new CoreNaviLeds();
    return CoreNaviLeds::m_instance;
}

int CoreNaviLeds::Left() 
{
    return HAL_LED_CONTROLLER_LEFT_LED;
}

int CoreNaviLeds::Right() 
{
    return HAL_LED_CONTROLLER_RIGHT_LED;
}

int CoreNaviLeds::Center() 
{
    return HAL_LED_CONTROLLER_CENTER_LED;
}

int CoreNaviLeds::All() 
{
    return Left() | Right() | Center();
}
#if 0
void CoreNaviLeds::ledBlink(int led) 
{
}
#endif

/* Fade functions */
void CoreNaviLeds::ledFade(int led, int brightness, int cFadeIn, int cFadeOut, int sFadeIn, int sFadeOut) 
{
    initializeEffect(HAL_LED_CONTROLLER_EFFECT_LED_FADE, led);

    configureParameters( 10,
                        HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness,
                        HAL_LED_CONTROLLER_CORE_EFFECT_CENTER_FADE_IN, cFadeIn,
                        HAL_LED_CONTROLLER_CORE_EFFECT_CENTER_FADE_OUT, cFadeOut,
                        HAL_LED_CONTROLLER_CORE_EFFECT_SIDE_FADE_IN, sFadeIn,
                        HAL_LED_CONTROLLER_CORE_EFFECT_SIDE_FADE_OUT, sFadeOut);


}

void CoreNaviLeds::ledFullFade(int brightness, int firstIn,int firstOut,int secondIn, int secondOut,int thirdOut,bool left)
{
    led_controller_effect_direction_t direction = HAL_LED_CONTROLLER_DIRECTION_RIGHT;
    if (left) direction = HAL_LED_CONTROLLER_DIRECTION_LEFT;


    initializeEffect(HAL_LED_CONTROLLER_EFFECT_FULL_FADE, HAL_LED_CONTROLLER_CORE_LEDS);

    configureParameters( 14,
                        HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness,
                        HAL_LED_CONTROLLER_CORE_EFFECT_FIRST_IN, firstIn,
                        HAL_LED_CONTROLLER_CORE_EFFECT_FIRST_OUT, firstOut,
                        HAL_LED_CONTROLLER_CORE_EFFECT_SECOND_IN, secondIn,
                        HAL_LED_CONTROLLER_CORE_EFFECT_SECOND_OUT, secondOut,
                        HAL_LED_CONTROLLER_CORE_EFFECT_THIRD_OUT, thirdOut,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DIRECTION, direction);

    finalizeAndExecute();
                        
}


/* Pulses the selected led, waiting for specified delay periods */
void CoreNaviLeds::ledPulsate(int led, int brightness, int startDelay, int FadeIn, int FadeOut, int FadeDelay, int RepeatDelay, int repeat) 
{
    initializeEffect(HAL_LED_CONTROLLER_EFFECT_LED_PULSATE, led);

    configureParameters( 14,
                        HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness,
                        HAL_LED_CONTROLLER_CORE_EFFECT_START_DELAY, startDelay,
                        HAL_LED_CONTROLLER_CORE_EFFECT_FADE_IN, FadeIn,
                        HAL_LED_CONTROLLER_CORE_EFFECT_FADE_OUT, FadeOut,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DURATION, FadeDelay,
                        HAL_LED_CONTROLLER_CORE_EFFECT_REPEAT_DELAY, RepeatDelay,
                        HAL_LED_CONTROLLER_CORE_EFFECT_REPEAT, repeat);

    finalizeAndExecute();
}

/* Blink twice then sleep */
void CoreNaviLeds::ledDoublePulse(int led, int brightness, int pulseRamp, int pulseDuration, int pulseDelay, int repeatDelay, int repeat) 
{
    initializeEffect(HAL_LED_CONTROLLER_EFFECT_LED_DOUBLE_PULSE, led);

    configureParameters( 12,
                        HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness,
                        HAL_LED_CONTROLLER_CORE_EFFECT_PULSE_RAMP, pulseRamp,
                        HAL_LED_CONTROLLER_CORE_EFFECT_PULSE_DURATION, pulseDuration,
                        HAL_LED_CONTROLLER_CORE_EFFECT_PULSE_DELAY, pulseDelay,
                        HAL_LED_CONTROLLER_CORE_EFFECT_REPEAT_DELAY, repeatDelay,
                        HAL_LED_CONTROLLER_CORE_EFFECT_REPEAT, repeat);

    finalizeAndExecute();
}

/* Ramps the PWM value to brightness in time. */
void CoreNaviLeds::ledRampTo(int led, int brightness, int time) 
{
    initializeEffect(HAL_LED_CONTROLLER_EFFECT_LED_RAMP_TO, led);

    configureParameters( 4,
                        HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DURATION, time);

    finalizeAndExecute();

}

/* Sets the PWM value to brightness */
void CoreNaviLeds::ledSet (int led, int brightness) 
{

    initializeEffect(HAL_LED_CONTROLLER_EFFECT_LED_SET, led);

    configureParameters(2, HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness);

    finalizeAndExecute();
}

/* Waterdrop functions */
void CoreNaviLeds::ledWaterdrop(int brightness, int cFadeIn, int cFadeOut, int sFadeIn, int sFadeOut, bool reverse) 
{
    led_controller_effect_direction_t direction = HAL_LED_CONTROLLER_DIRECTION_FORWARD;
    if (reverse) direction = HAL_LED_CONTROLLER_DIRECTION_REVERSE;

    initializeEffect(HAL_LED_CONTROLLER_EFFECT_WATERDROP, HAL_LED_CONTROLLER_CORE_LEDS);

    configureParameters( 12,
                        HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness,
                        HAL_LED_CONTROLLER_CORE_EFFECT_CENTER_FADE_IN, cFadeIn,
                        HAL_LED_CONTROLLER_CORE_EFFECT_CENTER_FADE_OUT, cFadeOut,
                        HAL_LED_CONTROLLER_CORE_EFFECT_SIDE_FADE_IN, sFadeIn,
                        HAL_LED_CONTROLLER_CORE_EFFECT_SIDE_FADE_OUT, sFadeOut,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DIRECTION, direction);

    finalizeAndExecute();

}

void CoreNaviLeds::ledSeesaw(int brightness, int fadeTime, bool reverse)
{
    if (!m_lightbarEnabled)
        return;

    led_controller_effect_direction_t direction = HAL_LED_CONTROLLER_DIRECTION_FORWARD;
    if (reverse) direction = HAL_LED_CONTROLLER_DIRECTION_REVERSE;

    initializeEffect(HAL_LED_CONTROLLER_EFFECT_SEESAW, HAL_LED_CONTROLLER_CORE_LEDS);

    configureParameters( 6,
                        HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DURATION, fadeTime,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DIRECTION, direction);

    finalizeAndExecute();
}

void CoreNaviLeds::ledLightbarSwipe (int brightness, int fadeTime, bool goLeft)
{
    if (!m_lightbarEnabled)
        return;

    led_controller_effect_direction_t direction = HAL_LED_CONTROLLER_DIRECTION_RIGHT;
    if (goLeft) direction = HAL_LED_CONTROLLER_DIRECTION_LEFT;


    initializeEffect(HAL_LED_CONTROLLER_EFFECT_LIGHTBAR_SWIPE, HAL_LED_CONTROLLER_CORE_LEDS);

    configureParameters( 6,
                        HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DURATION, fadeTime,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DIRECTION, direction);

    finalizeAndExecute();
}

void CoreNaviLeds::ledLightbarFullSwipe (int brightness, int fadeTime, bool goLeft)
{
    if (!m_lightbarEnabled)
        return;

    led_controller_effect_direction_t direction = HAL_LED_CONTROLLER_DIRECTION_RIGHT;
    if (goLeft) direction = HAL_LED_CONTROLLER_DIRECTION_LEFT;

    initializeEffect(HAL_LED_CONTROLLER_EFFECT_LIGHTBAR_FULL_SWIPE, HAL_LED_CONTROLLER_CORE_LEDS);

    configureParameters( 6,
                        HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DURATION, fadeTime,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DIRECTION, direction);

    finalizeAndExecute();

}

void CoreNaviLeds::ledSwipe (int brightness, int fadeTime, bool goLeft)
{
    if (!m_lightbarEnabled)
        return;

    led_controller_effect_direction_t direction = HAL_LED_CONTROLLER_DIRECTION_RIGHT;
    if (goLeft) direction = HAL_LED_CONTROLLER_DIRECTION_LEFT;

    initializeEffect(HAL_LED_CONTROLLER_EFFECT_SWIPE, HAL_LED_CONTROLLER_CORE_LEDS);

    configureParameters( 6,
                        HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DURATION, fadeTime,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DIRECTION, direction);

    finalizeAndExecute();
}

void CoreNaviLeds::ledFullSwipe (int brightness, int fadeTime, bool goLeft)
{
    if (!m_lightbarEnabled)
        return;

    led_controller_effect_direction_t direction = HAL_LED_CONTROLLER_DIRECTION_RIGHT;
    if (goLeft) direction = HAL_LED_CONTROLLER_DIRECTION_LEFT;

    initializeEffect(HAL_LED_CONTROLLER_EFFECT_FULL_SWIPE, HAL_LED_CONTROLLER_CORE_LEDS);

    configureParameters(6,
                        HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DURATION, fadeTime,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DIRECTION, direction);

    finalizeAndExecute();
}

void CoreNaviLeds::ledFadeOff (int brightness, int fadeTime, bool goLeft)
{
    if (!m_lightbarEnabled)
        return;

    led_controller_effect_direction_t direction = HAL_LED_CONTROLLER_DIRECTION_RIGHT;
    if (goLeft) direction = HAL_LED_CONTROLLER_DIRECTION_LEFT;

    initializeEffect(HAL_LED_CONTROLLER_EFFECT_FADE_OFF, HAL_LED_CONTROLLER_CORE_LEDS);

    configureParameters(6,
                        HAL_LED_CONTROLLER_CORE_EFFECT_BRIGHTNESS, brightness,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DURATION, fadeTime,
                        HAL_LED_CONTROLLER_CORE_EFFECT_DIRECTION, direction);

    finalizeAndExecute();
}


void CoreNaviLeds::stopAll() 
{
    hal_led_controller_stop(m_device, (led_controller_led_t)(Left() | Right() | Center()));
}

void CoreNaviLeds::stopAll(int led) 
{
    hal_led_controller_stop(m_device, (led_controller_led_t)led);
}

#else

int CoreNaviLeds::Left() { return 0; }
int CoreNaviLeds::Right() { return 0; }
int CoreNaviLeds::Center() { return 0; }
int CoreNaviLeds::All() { return 0; }
void CoreNaviLeds::stopAll() {}
void CoreNaviLeds::stopAll(int led) {}
void CoreNaviLeds::ledFade(int led,
     int brightness,
     int cFadeIn,
     int cFadeOut,
     int sFadeIn,
     int sFadeOut) {}
void CoreNaviLeds::ledPulsate(int led,
     int brightness,
     int startDelay,
     int FadeIn,
     int FadeOut,
     int FadeOutDelay,
     int RepeatDelay,
     int repeat) {}
void CoreNaviLeds::ledDoublePulse(int led,
     int brightness,
     int pulseRamp,
     int pulseDur,
     int pulseDelay,
     int RepeatDelay,
     int repeat) {}

void CoreNaviLeds::ledFullFade(int brightness,
     int firstIn,
    int firstOut,
     int secondIn,
     int secondOut,
     int thirdOut,
     bool left) {}
void CoreNaviLeds::ledSet (int led,
     int brightness) {}
void CoreNaviLeds::ledRampTo(int led,
     int brightness,
     int time) {}
void CoreNaviLeds::ledWaterdrop(int brightness,
     int cFadeIn,
     int cFadeOut,
     int sFadeIn,
     int sFadeOut,
    bool reverse) {}
void CoreNaviLeds::ledLightbarSwipe (int brightness,
     int fadeTime,
     bool goLeft) {}
void CoreNaviLeds::ledLightbarFullSwipe (int brightness,
     int fadeTime,
     bool goLeft) {}
void CoreNaviLeds::ledSwipe (int brightness,
     int fadeTime,
     bool goLeft) {}
void CoreNaviLeds::ledFullSwipe (int brightness,
     int fadeTime,
     bool goLeft) {}
void CoreNaviLeds::ledSeesaw(int brightness,
     int fadeTime,
     bool reverse) {}
void CoreNaviLeds::ledFadeOff (int brightness,
     int fadeTime,
     bool goLeft) {}

CoreNaviLeds* CoreNaviLeds::instance (void) { return CoreNaviLeds::m_instance; }

CoreNaviLeds::CoreNaviLeds() {}
CoreNaviLeds::~CoreNaviLeds() {}

#endif
