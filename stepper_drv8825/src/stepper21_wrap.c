#include "sl_i2cspm_mikroe_config.h"
#include "em_cmu.h"
#include "stepper21_wrap.h"
#include "stepper_config_drv8825.h"
#include "em_timer.h"

//////////////////////////////////////////////////////////////////////////////////////////
// Wrapper for GPIO (digital) handling
//////////////////////////////////////////////////////////////////////////////////////////
#include "em_gpio.h"

void digital_out_write ( digital_out_t * port_pin, uint8_t state )
{
    if (state)
        GPIO_PinOutSet(port_pin->port, port_pin->pin);
    else
        GPIO_PinOutClear(port_pin->port, port_pin->pin);
}

void digital_out_toggle ( digital_out_t * port_pin )
{
    GPIO_PinOutToggle(port_pin->port, port_pin->pin);
}

uint8_t digital_in_read (digital_out_t * port_pin)
{
    return GPIO_PinInGet(port_pin->port, port_pin->pin);
}

void digital_out_low ( digital_out_t * port_pin )
{
    digital_out_write ( port_pin, 0 );
}

void digital_out_high ( digital_out_t * port_pin )
{
    digital_out_write ( port_pin, 1 );
}

// ... 
//////////////////////////////////////////////////////////////////////////////////////////
// Wrapper for i2c smbus
//////////////////////////////////////////////////////////////////////////////////////////
#include "sl_i2cspm.h"

err_t i2c_master_write(stepper21_t *ctx, uint8_t *data, uint16_t len)
{
    I2C_TransferSeq_TypeDef seq;
    I2C_TransferReturn_TypeDef ret;

    seq.addr  = (uint16_t)(ctx->slave_address << 1);
    seq.flags = I2C_FLAG_WRITE;
    seq.buf[0].data = data;
    seq.buf[0].len  = len;

    ret = I2CSPM_Transfer(ctx->i2c, &seq);

    return (ret == i2cTransferDone) ? SL_STATUS_OK : SL_STATUS_TRANSMIT;
}

err_t i2c_master_write_then_read(stepper21_t *ctx,
                                 uint8_t *tx_data,
                                 uint16_t tx_len,
                                 uint8_t *rx_data,
                                 uint16_t rx_len)
{
    I2C_TransferSeq_TypeDef seq;
    I2C_TransferReturn_TypeDef ret;

    seq.addr  = (uint16_t)(ctx->slave_address << 1);
    seq.flags = I2C_FLAG_WRITE_READ;

    seq.buf[0].data = tx_data;
    seq.buf[0].len  = tx_len;

    seq.buf[1].data = rx_data;
    seq.buf[1].len  = rx_len;

    ret = I2CSPM_Transfer(ctx->i2c, &seq);

    return (ret == i2cTransferDone) ? SL_STATUS_OK : SL_STATUS_TRANSMIT;
}
//////////////////////////////////////////////////////////////////////////////////////////

static CMU_Clock_TypeDef get_timer_clock(TIMER_TypeDef *timer)
{
#if defined(_CMU_HFCLKSEL_MASK) || defined(_CMU_CMD_HFCLKSEL_MASK)
  CMU_Clock_TypeDef timer_clock = cmuClock_HF;
#elif defined(_CMU_SYSCLKCTRL_MASK)
  CMU_Clock_TypeDef timer_clock = cmuClock_SYSCLK;
#else
#error "Unknown root of clock tree"
#endif

  switch ((uint32_t)timer) {
#if defined(TIMER0_BASE)
    case TIMER0_BASE:
      timer_clock = cmuClock_TIMER0;
      break;
#endif
#if defined(TIMER1_BASE)
    case TIMER1_BASE:
      timer_clock = cmuClock_TIMER1;
      break;
#endif
#if defined(TIMER2_BASE)
    case TIMER2_BASE:
      timer_clock = cmuClock_TIMER2;
      break;
#endif
#if defined(TIMER3_BASE)
    case TIMER3_BASE:
      timer_clock = cmuClock_TIMER3;
      break;
#endif
#if defined(TIMER4_BASE)
    case TIMER4_BASE:
      timer_clock = cmuClock_TIMER4;
      break;
#endif
#if defined(TIMER5_BASE)
    case TIMER5_BASE:
      timer_clock = cmuClock_TIMER5;
      break;
#endif
#if defined(TIMER6_BASE)
    case TIMER6_BASE:
      timer_clock = cmuClock_TIMER6;
      break;
#endif
#if defined(WTIMER0_BASE)
    case WTIMER0_BASE:
      timer_clock = cmuClock_WTIMER0;
      break;
#endif
#if defined(WTIMER1_BASE)
    case WTIMER1_BASE:
      timer_clock = cmuClock_WTIMER1;
      break;
#endif
#if defined(WTIMER2_BASE)
    case WTIMER2_BASE:
      timer_clock = cmuClock_WTIMER2;
      break;
#endif
#if defined(WTIMER3_BASE)
    case WTIMER3_BASE:
      timer_clock = cmuClock_WTIMER3;
      break;
#endif
    default:
      EFM_ASSERT(0);
      break;
  }
  return timer_clock;
}

err_t verify_pwm_frequency(stepper21_t *ctx)
{
  uint32_t frequency = 0;

  // Verify actual configured frequency based on register values.
  frequency = (CMU_ClockFreqGet(get_timer_clock(ctx->pwm->timer)) /
              (TIMER_TopGet(ctx->pwm->timer)+1));

  // Store RPM value in the instance
  ctx->rpm = ( frequency * 60 ) / DRV8825_STEPPER_MOTOR_STEPS_PER_REV;

  return (ctx->rpm > DRV8825_STEPPER_MAX_RPM) ? SL_STATUS_INVALID_CONFIGURATION
                                             : SL_STATUS_OK;
}