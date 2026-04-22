/****************************************************************************
** Copyright (C) 2020 MikroElektronika d.o.o.
** Contact: https://www.mikroe.com/contact
**
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
** OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
** DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT
** OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
**  USE OR OTHER DEALINGS IN THE SOFTWARE.
****************************************************************************/

/*!
 * @file stepper21.c
 * @brief Stepper 21 Click Driver.
 */

#include "stepper21.h"
#include "stepper21_wrap.h"
#include "sl_sleeptimer.h"
#include "sl_udelay.h"

#if 0
/**
 * @brief Set delay for controlling motor speed.
 * @details This function sets a delay between toggling step pin.
 * @param[in] speed_macro : Speed macro for selecting how long the delay will be.
 * @return Nothing.
 */
static void stepper21_speed_delay ( uint8_t speed_macro );

void stepper21_cfg_setup ( stepper21_cfg_t *cfg ) 
{
    // Communication gpio pins
    cfg->scl = HAL_PIN_NC;
    cfg->sda = HAL_PIN_NC;

    // Additional gpio pins
    cfg->dir = HAL_PIN_NC;
    cfg->rst = HAL_PIN_NC;
    cfg->step = HAL_PIN_NC;
    cfg->int_pin = HAL_PIN_NC;

    cfg->i2c_speed   = I2C_MASTER_SPEED_STANDARD;
    cfg->i2c_address = STEPPER21_DEVICE_ADDRESS_A1A0_00;
}
#endif
err_t stepper21_init ( stepper21_t *ctx ) 
{
    // i2c Setup - nothing probably...
    
    // GPIO, a bit useless for many of them...
    CMU_ClockEnable(cmuClock_GPIO, true);
    GPIO_PinModeSet(ctx->dir.port, ctx->dir.pin, gpioModePushPull, 0);
    GPIO_PinModeSet(ctx->rst.port, ctx->rst.pin, gpioModeWiredAndPullUp, 1);
#if 0
    GPIO_PinModeSet(ctx->step.port, ctx->step.pin, gpioModePushPull, 0);
#endif
    GPIO_PinModeSet(ctx->int_pin.port, ctx->int_pin.pin, gpioModeDisabled, 0);

    // Default config
    return stepper21_default_cfg ( ctx ) ;
}

err_t stepper21_default_cfg ( stepper21_t *ctx ) 
{
    err_t error_flag = STEPPER21_OK;
    stepper21_reset_device ( ctx );
    error_flag |= stepper21_enable_device ( ctx );
    stepper21_set_direction ( ctx, STEPPER21_DIR_CCW );
    // Configure FAULT, DECAY, and HOME pins as INPUT, others as OUTPUT
    error_flag |= stepper21_write_register ( ctx, STEPPER21_REG_CONFIG, STEPPER21_DEFAULT_CONFIG );
    error_flag |= stepper21_set_sleep_pin ( ctx, STEPPER21_PIN_STATE_HIGH );
    error_flag |= stepper21_set_decay_pin ( ctx, STEPPER21_PIN_STATE_OPEN );
    error_flag |= stepper21_set_step_mode ( ctx, STEPPER21_MODE_FULL_STEP );
    // PWM pin configuration
    sl_pwm_set_duty_cycle(ctx->pwm, 50);

    ctx->state = IDLE;

    return error_flag | verify_pwm_frequency(ctx) ;
}

err_t stepper21_write_register ( stepper21_t *ctx, uint8_t reg, uint8_t data_in )
{
    uint8_t data_buf[ 2 ] = { 0 };
    data_buf[ 0 ] = reg;
    data_buf[ 1 ] = data_in;
    return i2c_master_write( ctx, data_buf, 2 );
}

err_t stepper21_read_register ( stepper21_t *ctx, uint8_t reg, uint8_t *data_out )
{
    return i2c_master_write_then_read( ctx, &reg, 1, data_out, 1 );
}

err_t stepper21_get_step_mode ( stepper21_t *ctx, uint8_t *mode )
{
    uint8_t reg_data = 0;
    if ( STEPPER21_ERROR == stepper21_read_register ( ctx, STEPPER21_REG_OUTPUT, &reg_data ) )
    {
        return STEPPER21_ERROR;
    }
    *mode = ( reg_data >> 5 ) & STEPPER21_MODE_MASK;
    return STEPPER21_OK;
}

err_t stepper21_set_step_mode ( stepper21_t *ctx, uint8_t mode )
{
    uint8_t reg_data = 0;
    if ( mode > STEPPER21_MODE_1_OVER_32 )
    {
        return STEPPER21_ERROR;
    }
    if ( STEPPER21_ERROR == stepper21_read_register ( ctx, STEPPER21_REG_OUTPUT, &reg_data ) )
    {
        return STEPPER21_ERROR;
    }
    if ( mode == ( ( reg_data >> 5 ) & STEPPER21_MODE_MASK ) )
    {
        return STEPPER21_OK;
    }
    reg_data &= ~( STEPPER21_MODE_MASK << 5 );
    reg_data |= ( mode << 5 );
    return stepper21_write_register ( ctx, STEPPER21_REG_OUTPUT, reg_data );
}

err_t stepper21_get_sleep_pin ( stepper21_t *ctx, uint8_t *state )
{
    uint8_t reg_data = 0;
    if ( STEPPER21_ERROR == stepper21_read_register ( ctx, STEPPER21_REG_OUTPUT, &reg_data ) )
    {
        return STEPPER21_ERROR;
    }
    *state = ( reg_data & STEPPER21_PIN_SLEEP );
    return STEPPER21_OK;
}

err_t stepper21_set_sleep_pin ( stepper21_t *ctx, uint8_t state )
{
    uint8_t reg_data = 0;
    if ( state > STEPPER21_PIN_STATE_HIGH )
    {
        return STEPPER21_ERROR;
    }
    if ( STEPPER21_ERROR == stepper21_read_register ( ctx, STEPPER21_REG_OUTPUT, &reg_data ) )
    {
        return STEPPER21_ERROR;
    }
    if ( state == ( reg_data & STEPPER21_PIN_SLEEP ) )
    {
        return STEPPER21_OK;
    }
    reg_data &= ~STEPPER21_PIN_SLEEP;
    reg_data |= state;
    return stepper21_write_register ( ctx, STEPPER21_REG_OUTPUT, reg_data );
}

err_t stepper21_get_fault_pin ( stepper21_t *ctx, uint8_t *state )
{
    uint8_t reg_data = 0;
    if ( STEPPER21_ERROR == stepper21_read_register ( ctx, STEPPER21_REG_INPUT, &reg_data ) )
    {
        return STEPPER21_ERROR;
    }
    *state = ( reg_data & STEPPER21_PIN_FAULT ) >> 1;
    return STEPPER21_OK;
}

err_t stepper21_get_decay_pin ( stepper21_t *ctx, uint8_t *state )
{
    uint8_t reg_data = 0;
    if ( STEPPER21_ERROR == stepper21_read_register ( ctx, STEPPER21_REG_CONFIG, &reg_data ) )
    {
        return STEPPER21_ERROR;
    }
    if ( reg_data & STEPPER21_PIN_DECAY )
    {
        *state = STEPPER21_PIN_STATE_OPEN;
    }
    else
    {
        if ( STEPPER21_ERROR == stepper21_read_register ( ctx, STEPPER21_REG_OUTPUT, &reg_data ) )
        {
            return STEPPER21_ERROR;
        }
        *state = ( reg_data & STEPPER21_PIN_DECAY ) >> 2;
    }
    return STEPPER21_OK;
}

err_t stepper21_set_decay_pin ( stepper21_t *ctx, uint8_t state )
{
    uint8_t reg_data = 0;
    if ( state > STEPPER21_PIN_STATE_OPEN )
    {
        return STEPPER21_ERROR;
    }
    
    // Check if decay pin is configured as input (state open) or output (state low/high)
    if ( STEPPER21_ERROR == stepper21_read_register ( ctx, STEPPER21_REG_CONFIG, &reg_data ) )
    {
        return STEPPER21_ERROR;
    }
    
    // Check and configure the pin as input for open state
    if ( STEPPER21_PIN_STATE_OPEN == state)
    {
        if ( STEPPER21_PIN_DECAY == ( reg_data & STEPPER21_PIN_DECAY ) )
        {
            return STEPPER21_OK;
        }
        else
        {
            reg_data |= STEPPER21_PIN_DECAY;
            return stepper21_write_register ( ctx, STEPPER21_REG_CONFIG, reg_data );
        }
    }
    
    // Check and configure the pin as output for low or high states
    if ( STEPPER21_PIN_DECAY == ( reg_data & STEPPER21_PIN_DECAY ) )
    {
        reg_data &= ~STEPPER21_PIN_DECAY;
        if ( STEPPER21_ERROR == stepper21_write_register ( ctx, STEPPER21_REG_CONFIG, reg_data ) )
        {
            return STEPPER21_ERROR;
        }
    }
    
    // Check and configure the pin output state
    if ( STEPPER21_ERROR == stepper21_read_register ( ctx, STEPPER21_REG_OUTPUT, &reg_data ) )
    {
        return STEPPER21_ERROR;
    }
    if ( state == ( ( reg_data & STEPPER21_PIN_DECAY ) >> 2 ) )
    {
        return STEPPER21_OK;
    }
    reg_data &= ~STEPPER21_PIN_DECAY;
    reg_data |= ( state << 2 );
    return stepper21_write_register ( ctx, STEPPER21_REG_OUTPUT, reg_data );
}

err_t stepper21_get_enable_pin ( stepper21_t *ctx, uint8_t *state )
{
    uint8_t reg_data = 0;
    if ( STEPPER21_ERROR == stepper21_read_register ( ctx, STEPPER21_REG_OUTPUT, &reg_data ) )
    {
        return STEPPER21_ERROR;
    }
    *state = ( reg_data & STEPPER21_PIN_ENABLE ) >> 3;
    return STEPPER21_OK;
}

err_t stepper21_set_enable_pin ( stepper21_t *ctx, uint8_t state )
{
    uint8_t reg_data = 0;
    if ( state > STEPPER21_PIN_STATE_HIGH )
    {
        return STEPPER21_ERROR;
    }
    if ( STEPPER21_ERROR == stepper21_read_register ( ctx, STEPPER21_REG_OUTPUT, &reg_data ) )
    {
        return STEPPER21_ERROR;
    }
    if ( state == ( reg_data & STEPPER21_PIN_ENABLE ) )
    {
        return STEPPER21_OK;
    }
    reg_data &= ~STEPPER21_PIN_ENABLE;
    reg_data |= ( state << 3 );
    return stepper21_write_register ( ctx, STEPPER21_REG_OUTPUT, reg_data );
}

err_t stepper21_get_home_pin ( stepper21_t *ctx, uint8_t *state )
{
    uint8_t reg_data = 0;
    if ( STEPPER21_ERROR == stepper21_read_register ( ctx, STEPPER21_REG_INPUT, &reg_data ) )
    {
        return STEPPER21_ERROR;
    }
    *state = ( reg_data & STEPPER21_PIN_HOME ) >> 4;
    return STEPPER21_OK;
}

#if 0
// More handy start / stop / step / speed inteface...
void stepper21_drive_motor ( stepper21_t *ctx, uint32_t steps, uint8_t speed)
{
    stepper21_enable_device ( ctx );
    for ( uint32_t cnt = 0; cnt < steps; cnt++ )
    {
        // TODO insert speed control based on PWM
        stepper21_set_step_pin ( ctx, STEPPER21_PIN_STATE_HIGH );
        //stepper21_speed_delay ( speed );
        stepper21_set_step_pin ( ctx, STEPPER21_PIN_STATE_LOW );
        //stepper21_speed_delay ( speed );
    }
    stepper21_disable_device ( ctx );
}
#else
/***************************************************************************//**
 * Sets the speed of the motor in RPM.
 ******************************************************************************/
err_t stepper21_set_speed(stepper21_t *ctx, uint32_t rpm)
{
  drv8825_state_t curr_state = ctx->state;

  if (rpm > DRV8825_STEPPER_MAX_RPM) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  err_t sc = stepper21_disable_device(ctx);

  if (sc != SL_STATUS_OK)
    return sc;

  sl_pwm_deinit(ctx->pwm);

  sl_pwm_config_t pwm_config = {
    .frequency = ( rpm * DRV8825_STEPPER_MOTOR_STEPS_PER_REV ) / 60,
    .polarity = PWM_ACTIVE_HIGH,
  };

  sl_pwm_init(ctx->pwm, &pwm_config);
  sl_pwm_set_duty_cycle(ctx->pwm, 50);

  sc = stepper21_enable_device(ctx);
    
  if (sc != SL_STATUS_OK)
    return sc;

  ctx->rpm = rpm;

  if (curr_state == RUNNING) {
    stepper21_start(ctx);
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Starts the motor with the configured speed and direction.
 ******************************************************************************/
err_t stepper21_start(stepper21_t *ctx)
{
  if (ctx->state != IDLE) {
    return SL_STATUS_INVALID_STATE;
  }

  if (ctx->rpm > DRV8825_STEPPER_MAX_RPM) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }

  sl_pwm_start(ctx->pwm);
  ctx->state = RUNNING;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Stops the motor.
 ******************************************************************************/
err_t stepper21_stop(stepper21_t *ctx)
{
  if (ctx->state != RUNNING) {
      return SL_STATUS_INVALID_STATE;
  }

  sl_pwm_stop(ctx->pwm);
  GPIO_PinOutClear(ctx->step.port, ctx->step.pin);
  ctx->state = IDLE;

  return SL_STATUS_OK;
}

/***************************************************************************//**
 * Moves the motor by a given number of steps. This is a non-blocking function.
 * The speed of the movement can be set with the a4988_stepper_start() function.
 ******************************************************************************/
err_t stepper21_step(stepper21_t *ctx,
                     uint32_t step_count)
{
  inline void stepper21_step_delay (uint32_t period_us)
  {
    // Use msec vs usec delay according to the order of magnitude of the delay
    if (period_us >= 1000U) {
      sl_sleeptimer_delay_millisecond((uint16_t)((period_us/1000.0f)+0.5f));
    } else {
      sl_udelay_wait(period_us);
    }
  }

  uint32_t half_period_us;

  if (ctx->state != IDLE) {
    return SL_STATUS_INVALID_STATE;
  }
  
  if ((ctx->rpm == 0U) || (ctx->rpm > DRV8825_STEPPER_MAX_RPM)) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }
 
  half_period_us = (uint32_t)(30000000UL / ((uint32_t)ctx->rpm * (uint32_t)DRV8825_STEPPER_MOTOR_STEPS_PER_REV));

  if (half_period_us <= 0U) {
    return SL_STATUS_INVALID_CONFIGURATION;
  }
  
  stepper21_set_step_pin ( ctx, STEPPER21_PIN_STATE_LOW );

  for ( uint32_t cnt = 0; cnt < step_count; cnt++ )
  {
    // TODO step control based on a timer...
    stepper21_set_step_pin ( ctx, STEPPER21_PIN_STATE_HIGH );
    stepper21_step_delay (half_period_us);

    stepper21_set_step_pin ( ctx, STEPPER21_PIN_STATE_LOW );
    stepper21_step_delay (half_period_us);
  }
  
  return SL_STATUS_OK;
}
#endif

err_t stepper21_enable_device ( stepper21_t *ctx )
{
    err_t sc = stepper21_set_enable_pin ( ctx, STEPPER21_PIN_STATE_LOW );

    if (sc == SL_STATUS_OK) {
        ctx->state = IDLE;
    }

    return sc;
}

err_t stepper21_disable_device ( stepper21_t *ctx )
{
    err_t sc;
    
    if (ctx->state == RUNNING) {
      sl_pwm_stop(ctx->pwm);
    }
    
    sc = stepper21_set_enable_pin ( ctx, STEPPER21_PIN_STATE_HIGH );

    if (sc == SL_STATUS_OK) {
        ctx->state = DISABLED;
    }

    return sc;
}

void stepper21_set_direction ( stepper21_t *ctx, uint8_t dir )
{
    if (ctx->direction == dir) {
        return /*SL_STATUS_OK*/;
    }

    //Stop stepper if it is running
    if (ctx->state == RUNNING) {
        stepper21_stop(ctx);
    }

    if (dir == STEPPER21_DIR_CCW) {
        digital_out_write ( &ctx->dir, STEPPER21_DIR_CCW);
    } else if (dir == STEPPER21_DIR_CW) {
        digital_out_write ( &ctx->dir, STEPPER21_DIR_CW);
    } else {
        return /*SL_STATUS_INVALID_PARAMETER*/;
    }

    ctx->direction = dir;
}

void stepper21_switch_direction ( stepper21_t *ctx )
{
    digital_out_toggle ( &ctx->dir );

    if (ctx->direction == STEPPER21_DIR_CCW)
        ctx->direction = STEPPER21_DIR_CW;
    else
        ctx->direction = STEPPER21_DIR_CCW;
}

void stepper21_reset_device ( stepper21_t *ctx )
{
    digital_out_low ( &ctx->rst );
    sl_sleeptimer_delay_millisecond(100);
    digital_out_high ( &ctx->rst );
    sl_sleeptimer_delay_millisecond(100);
}

void stepper21_set_rst_pin ( stepper21_t *ctx, uint8_t state )
{
    digital_out_write ( &ctx->rst, state );
}

uint8_t stepper21_get_int_pin ( stepper21_t *ctx )
{
    return digital_in_read ( &ctx->int_pin );
}

void stepper21_set_step_pin ( stepper21_t *ctx, uint8_t state )
{
    digital_out_write ( &ctx->step, state );
}

#if 0
// @giferrar: not needed when using pwm
// STEPPER21_SPEED_VERY_SLOW
static void stepper21_speed_delay ( uint8_t speed_macro )
{
    switch ( speed_macro )
    {
        case STEPPER21_SPEED_VERY_SLOW:
        {
            Delay_10ms( );
            break;
        }
        case STEPPER21_SPEED_SLOW:
        {
            Delay_5ms( );
            break;
        }
        case STEPPER21_SPEED_MEDIUM:
        {
            Delay_1ms( );
            Delay_1ms( );
            Delay_500us( );
            break;
        }
        case STEPPER21_SPEED_FAST:
        {
            Delay_1ms( );
            break;
        }
        case STEPPER21_SPEED_VERY_FAST:
        {
            Delay_500us( );
            break;
        }
        default:
        {
            Delay_1ms( );
            break;
        }
    }
}
#endif
// ------------------------------------------------------------------------- END
