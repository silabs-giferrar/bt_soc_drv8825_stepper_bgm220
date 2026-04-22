#ifndef STEPPER21_WRAP_H
#define STEPPER21_WRAP_H

#ifdef __cplusplus
extern "C"{
#endif

#include "sl_i2cspm_mikroe_config.h"
#include "sl_i2cspm.h"
#include "em_gpio.h"
#include "em_cmu.h"
#include "stepper21.h"


extern void digital_out_write ( digital_out_t * port_pin, uint8_t state );

extern void digital_out_toggle ( digital_out_t * port_pin );

extern uint8_t digital_in_read (digital_out_t * port_pin);

extern void digital_out_low ( digital_out_t * port_pin );

extern void digital_out_high ( digital_out_t * port_pin );

extern err_t i2c_master_write(stepper21_t *ctx, uint8_t *data, uint16_t len);

extern err_t i2c_master_write_then_read(stepper21_t *ctx, \
                                 uint8_t *tx_data, \
                                 uint16_t tx_len, \
                                 uint8_t *rx_data, \
                                 uint16_t rx_len);

extern err_t verify_pwm_frequency(stepper21_t *ctx);

#ifdef __cplusplus
}
#endif

#endif // STEPPER21_WRAP_H