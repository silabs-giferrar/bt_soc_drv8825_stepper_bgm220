/***************************************************************************//**
 * @file
 * @brief Core application logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include "sl_bt_api.h"
#include "sl_main_init.h"
#include "app_assert.h"
#include "app.h"
//
#include "sl_cli.h"
#include "sl_cli_handles.h"
#include "sl_iostream.h"
#include "sl_iostream_handles.h"
#include "sl_simple_button_instances.h"
//#include "sl_pwm_instances.h"
#include "stepper21.h"
#include "gatt_db.h"
#include "sl_i2cspm_mikroe_config.h"
#include "sl_pwm_instances.h"

//
// The advertising set handle allocated from Bluetooth stack.
static uint8_t advertising_set_handle = 0xff;
// Pretty much a default configuration... 
static stepper21_t stepper21 = {
  {DRV8825_STEPPER_DIR_PORT, DRV8825_STEPPER_DIR_PIN},
  {DRV8825_STEPPER_RST_PORT, DRV8825_STEPPER_RST_PIN},
  {DRV8825_STEPPER_STEP_PORT, DRV8825_STEPPER_STEP_PIN},
  &sl_pwm_stepper,            /* PWM instance */
  DRV8825_STEPPER_MAX_RPM,    /* rpm          */
  DISABLED,                   /* status       */
  STEPPER21_DIR_CCW,          /* direction    */
  {DRV8825_STEPPER_INT_PORT, DRV8825_STEPPER_INT_PIN},
  SL_I2CSPM_MIKROE_PERIPHERAL,
  STEPPER21_DEVICE_ADDRESS_A1A0_00
};

void app_cli_stepper_set_rpm(sl_cli_command_arg_t *arguments)
{
  sl_status_t sc;
  uint32_t argument_value;

  argument_value = sl_cli_get_argument_uint32(arguments, 0);
  sc = stepper21_set_speed(&stepper21, argument_value);
  if (sc != SL_STATUS_OK) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"),
                       "Error: %u\r\n",
                       (unsigned int)sc);
    return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"),
                     "Speed was set to: %u \r\n",
                     (unsigned int)argument_value);
}

void app_cli_stepper_set_dir(sl_cli_command_arg_t *arguments)
{
#if 0
  sl_status_t sc;
#endif
  uint8_t argument_value;

  argument_value = sl_cli_get_argument_uint8(arguments, 0);
#if 0
  sc = 
#endif
  argument_value = argument_value ? STEPPER21_DIR_CW : STEPPER21_DIR_CCW;
  stepper21_set_direction ( &stepper21, argument_value );
#if 0
  if (sc != SL_STATUS_OK) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"),
                       "Error: %u\r\n",
                       (unsigned int)sc);
    return;
  }
#endif
  sl_iostream_printf(sl_iostream_get_handle("vcom"),
                     "Direction was set to: %u \r\n",
                     (unsigned int)argument_value);
}

void app_cli_stepper_step(sl_cli_command_arg_t *arguments)
{
  sl_status_t sc;
  uint32_t argument_value;

  argument_value = sl_cli_get_argument_uint32(arguments, 0);
  sc = stepper21_step( &stepper21, argument_value );
  if (sc != SL_STATUS_OK) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"),
                       "Error: %u\r\n",
                       (unsigned int)sc);
    return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"),
                     "Stepper is making %u steps\r\n",
                     (unsigned int)argument_value);
}

void app_cli_stepper_start(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t sc;

  sc = stepper21_start( &stepper21 );
  if (sc != SL_STATUS_OK) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"),
                       "Error: %u\r\n",
                       (unsigned int)sc);
    return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"), "Start motor\r\n");
}

void app_cli_stepper_stop(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t sc;

  sc = stepper21_stop( &stepper21 );
  if (sc != SL_STATUS_OK) {
      sl_iostream_printf(sl_iostream_get_handle("vcom"),
                         "Error: %u\r\n",
                         (unsigned int)sc);
      return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"), "Stop motor\r\n");
}

void app_cli_stepper_enable(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t sc;

  sc = stepper21_enable_device ( &stepper21 );
  if (sc != SL_STATUS_OK) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"),
                       "Error: %u\r\n",
                       (unsigned int)sc);
    return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"),
                     "Enable motor drive\r\n");
}

void app_cli_stepper_disable(sl_cli_command_arg_t *arguments)
{
  (void)arguments;
  sl_status_t sc;

  sc = stepper21_disable_device ( &stepper21 );
  if (sc != SL_STATUS_OK) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"),
                       "Error: %u\r\n",
                       (unsigned int)sc);
    return;
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"), "Disable motor drive\r\n");
}

// Create command details for the commands. The macro SL_CLI_UNIT_SEPARATOR can be
// used to format the help text for multiple arguments.
static const sl_cli_command_info_t cmd__set_rpm = \
  SL_CLI_COMMAND(app_cli_stepper_set_rpm,
                 "Set speed (RPM)",
                 "Speed in rpm\r",
                 {sl_cli_arg_uint32, sl_cli_arg_end, });

static const sl_cli_command_info_t cmd__set_dir = \
  SL_CLI_COMMAND(app_cli_stepper_set_dir,
                 "Set direction",
                 "0: ccw, 1: cw\r",
                 {sl_cli_arg_uint8, sl_cli_arg_end, });

static const sl_cli_command_info_t cmd__step = \
  SL_CLI_COMMAND(app_cli_stepper_step,
                 "Make steps with the motor",
                 "Step count\r",
                 {sl_cli_arg_uint32, sl_cli_arg_end, });

static const sl_cli_command_info_t cmd__start = \
  SL_CLI_COMMAND(app_cli_stepper_start,
                 "Start stepper motor\r\n",
                 "",
                 {sl_cli_arg_end, });

static const sl_cli_command_info_t cmd__stop = \
  SL_CLI_COMMAND(app_cli_stepper_stop,
                 "Stop stepper motor\r\n",
                 "",
                 {sl_cli_arg_end, });

static const sl_cli_command_info_t cmd__enable = \
  SL_CLI_COMMAND(app_cli_stepper_enable,
                 "Enable stepper motor\r\n",
                 "",
                 {sl_cli_arg_end, });

static const sl_cli_command_info_t cmd__disable = \
  SL_CLI_COMMAND(app_cli_stepper_disable,
                 "Disable stepper motor\r\n",
                 "",
                 {sl_cli_arg_end, });

// Create the array of commands
static sl_cli_command_entry_t app_cli_stepper_command_table[] = {
  { "set_rpm", &cmd__set_rpm, false },
  { "set_dir", &cmd__set_dir, false },
  { "step", &cmd__step, false },
  { "start",  &cmd__start, false },
  { "stop",  &cmd__stop, false },
  { "enable",  &cmd__enable, false },
  { "disable",  &cmd__disable, false },
  { NULL, NULL, false },
};

// Create the command group at the top level
static sl_cli_command_group_t app_cli_stepper_command_group = {
  { NULL },
  false,
  app_cli_stepper_command_table
};


// Application Init.
void app_init(void)
{
  /////////////////////////////////////////////////////////////////////////////
  // Put your additional application init code here!                         //
  // This is called once during start-up.                                    //
  /////////////////////////////////////////////////////////////////////////////
  if (stepper21_init( &stepper21 )) {
    sl_iostream_printf(sl_iostream_get_handle("vcom"), "Error: Default configuration.\n\r");
  }
  sl_iostream_printf(sl_iostream_get_handle("vcom"), "app_init DONE\n\r");
  stepper21_set_speed(&stepper21, BLE_MOTOR_INIT_RPMs);
  sl_cli_command_add_command_group(sl_cli_inst_handle,
                                   &app_cli_stepper_command_group);
}

// Application Process Action.
void app_process_action(void)
{
  if (app_is_process_required()) {
    /////////////////////////////////////////////////////////////////////////////
    // Put your additional application code here!                              //
    // This is will run each time app_proceed() is called.                     //
    // Do not call blocking functions from here!                               //
    /////////////////////////////////////////////////////////////////////////////
    //
  }
}

/**************************************************************************//**
 * Bluetooth stack event handler.
 * This overrides the default weak implementation.
 *
 * @param[in] evt Event coming from the Bluetooth stack.
 *****************************************************************************/
void sl_bt_on_event(sl_bt_msg_t *evt)
{
  sl_status_t sc;

  switch (SL_BT_MSG_ID(evt->header)) {
    // -------------------------------
    // This event indicates the device has started and the radio is ready.
    // Do not call any stack command before receiving this boot event!
    case sl_bt_evt_system_boot_id:
      // Create an advertising set.
      sc = sl_bt_advertiser_create_set(&advertising_set_handle);
      app_assert_status(sc);

      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Set advertising interval to 100ms.
      sc = sl_bt_advertiser_set_timing(
        advertising_set_handle,
        160, // min. adv. interval (milliseconds * 1.6)
        160, // max. adv. interval (milliseconds * 1.6)
        0,   // adv. duration
        0);  // max. num. adv. events
      app_assert_status(sc);
      // Start advertising and enable connections.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);
      break;

    // -------------------------------
    // This event indicates that a new connection was opened.
    case sl_bt_evt_connection_opened_id:
      break;

    // -------------------------------
    // This event indicates that a connection was closed.
    case sl_bt_evt_connection_closed_id:
      // Generate data for advertising
      sc = sl_bt_legacy_advertiser_generate_data(advertising_set_handle,
                                                 sl_bt_advertiser_general_discoverable);
      app_assert_status(sc);

      // Restart advertising after client has disconnected.
      sc = sl_bt_legacy_advertiser_start(advertising_set_handle,
                                         sl_bt_legacy_advertiser_connectable);
      app_assert_status(sc);
      break;

    ///////////////////////////////////////////////////////////////////////////
    // Add additional event handlers here as your application requires!      //
    ///////////////////////////////////////////////////////////////////////////
    //
    ///////////////////////////////////////////////////////////////////////////
    // This event indicates that the value of an attribute in the local GATT
    // database was changed by a remote GATT client.
    ///////////////////////////////////////////////////////////////////////////
    case sl_bt_evt_gatt_server_attribute_value_id:
      // Value of the gattdb_Shade_Ctrl characteristic was changed.
      if (gattdb_shade_control == evt->data.evt_gatt_server_attribute_value.attribute) {
          uint8_t data_recv;
          size_t data_recv_len;

          // Read characteristic value.
          sc = sl_bt_gatt_server_read_attribute_value(gattdb_shade_control,
                                                      0,
                                                      sizeof(data_recv),
                                                      &data_recv_len,
                                                      &data_recv);
          app_assert_status(sc);

          // Change the MOTOR state status.
          if ( data_recv == BLE_MOTOR_IDLE ) {
            // Disable the driver stage
            sc = stepper21_disable_device ( &stepper21 );
            app_assert_status(sc);
          } else if (data_recv == BLE_MOTOR_DOWN || data_recv == BLE_MOTOR_UP) {
            // Enable the driver
            sc = stepper21_enable_device ( &stepper21 );  
            app_assert_status(sc);
            //
            if (data_recv == BLE_MOTOR_UP)
              // Set DIR CCW
              stepper21_set_direction ( &stepper21, STEPPER21_DIR_CCW );
            else
              // Set DIR CW
              stepper21_set_direction ( &stepper21, STEPPER21_DIR_CW );
            //
            // Start the motor
            sc = stepper21_start ( &stepper21 );
            app_assert_status(sc);
          }
      }

    // -------------------------------
    // Default event handler.
    default:
      break;
  }
}

// Emergency button: if the motor is enabled then a button press disables it.
// The next button press will enable it again.
void sl_button_on_change(const sl_button_t *handle)
{
  if (sl_button_get_state(handle) == SL_SIMPLE_BUTTON_PRESSED) {
    if (stepper21.state == DISABLED) {
      stepper21_enable_device ( &stepper21 );
    } else {
      stepper21_disable_device ( &stepper21 );
    }
  }
}