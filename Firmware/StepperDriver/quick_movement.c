#include "stepper_control.h"
#include "quick_movement.h"
#include "app_ios_and_regs.h"
#include "hwbp_core.h"

extern AppRegs app_regs;

extern bool motor_is_running[MOTORS_QUANTITY];

/************************************************************************/
/* Global variables (old way)                                           */
/************************************************************************/
bool m1_quick_parameters_loaded = false;
bool m2_quick_parameters_loaded = false;

uint16_t m1_acc_number_of_steps;
uint16_t m2_acc_number_of_steps;

uint8_t m1_quick_count_down = 0;
uint8_t m2_quick_count_down = 0;

uint16_t m1_quick_relative_steps;
uint16_t m2_quick_relative_steps;

uint16_t m1_quick_timer_per;
uint16_t m2_quick_timer_per;
uint8_t m1_quick_state_ctrl;
uint8_t m2_quick_state_ctrl;
uint16_t m1_quick_stop_decreasing_interval;
uint16_t m2_quick_stop_decreasing_interval;
uint16_t m1_quick_start_increasing_interval;
uint16_t m2_quick_start_increasing_interval;

uint16_t m1_quick_acc_interval;
uint16_t m2_quick_acc_interval;
uint16_t m1_quick_step_interval;
uint16_t m2_quick_step_interval;


/************************************************************************/
/* Quick movement routines (old way)                                    */
/************************************************************************/
bool m1_quick_load_parameters (void)
{
	uint16_t motor1_positive_steps = (uint16_t)((app_regs.REG_RESERVED14 > 0) ? app_regs.REG_RESERVED14 : -app_regs.REG_RESERVED14);

	m1_quick_parameters_loaded = true;
	
	float m1_acc_number_of_steps_float = (float)(app_regs.REG_RESERVED10 - app_regs.REG_RESERVED8) / (float)app_regs.REG_RESERVED12;
	
	m1_acc_number_of_steps = m1_acc_number_of_steps_float - ((uint16_t)m1_acc_number_of_steps_float) > 0 ? (uint16_t)m1_acc_number_of_steps_float + 1 : (uint16_t)m1_acc_number_of_steps_float;
	
	m1_quick_timer_per = app_regs.REG_RESERVED10 + app_regs.REG_RESERVED12;
	m1_quick_step_interval = app_regs.REG_RESERVED8;
	m1_quick_state_ctrl = 0;
	m1_quick_stop_decreasing_interval = abs(app_regs.REG_RESERVED14) - m1_acc_number_of_steps;
	m1_quick_start_increasing_interval = m1_acc_number_of_steps;
	
	m1_quick_acc_interval = app_regs.REG_RESERVED12;
	
	if (m1_acc_number_of_steps <= 2)
	{
		return false;
	}
	
	if (m1_acc_number_of_steps * 2 < abs(app_regs.REG_RESERVED14))
	{
		//return false;
	}
	
	if (app_regs.REG_RESERVED12 < app_regs.REG_RESERVED8)
	{
		//return false;
	}
	
	return true;
}

bool m2_quick_load_parameters (void)
{
	uint16_t motor2_positive_steps = (uint16_t)((app_regs.REG_RESERVED15 > 0) ? app_regs.REG_RESERVED15 : -app_regs.REG_RESERVED15);

	m2_quick_parameters_loaded = true;
	
	float m2_acc_number_of_steps_float = (float)(app_regs.REG_RESERVED11 - app_regs.REG_RESERVED9) / (float)app_regs.REG_RESERVED13;
	
	m2_acc_number_of_steps = m2_acc_number_of_steps_float - ((uint16_t)m2_acc_number_of_steps_float) > 0 ? (uint16_t)m2_acc_number_of_steps_float + 1 : (uint16_t)m2_acc_number_of_steps_float;
	
	m2_quick_timer_per = app_regs.REG_RESERVED11 + app_regs.REG_RESERVED13;
	m2_quick_step_interval = app_regs.REG_RESERVED9;
	m2_quick_state_ctrl = 0;
	m2_quick_stop_decreasing_interval = abs(app_regs.REG_RESERVED15) - m2_acc_number_of_steps;
	m2_quick_start_increasing_interval = m2_acc_number_of_steps;
	
	m2_quick_acc_interval = app_regs.REG_RESERVED13;
	
	if (m2_acc_number_of_steps <= 2)
	{
		return false;
	}
	
	if (m2_acc_number_of_steps * 2 < abs(app_regs.REG_RESERVED15))
	{
		//return false;
	}
	
	if (app_regs.REG_RESERVED13 < app_regs.REG_RESERVED9)
	{
		//return false;
	}
	
	return true;
}

bool m1_quick_launch_movement (void)
{	
	if (read_DRIVE_ENABLE_M1)
	{
		return false;
	}
	
	if (m1_quick_parameters_loaded == false)
	{
		return false;
	}
	
	m1_quick_timer_per = app_regs.REG_RESERVED10 + app_regs.REG_RESERVED12;
	m1_quick_state_ctrl = 0;
	
	/* Only executes the movement if the motor is stopped */
	if_moving_stop_rotation(1);	
	
	if (app_regs.REG_RESERVED14 > 0)
	{
		set_DIR_M1;
	}
	else
	{
		clr_DIR_M1;
	}
	
	// 4 Stop motor if moving
	// 3 Wait 1 ms
	// 2 Star m1_start_quick_movement()
	// 1 Moving
	// 0 Stopped
	m1_quick_count_down = 4;
	
	m1_quick_relative_steps = abs(app_regs.REG_RESERVED14);
	
	return true;
}

bool m2_quick_launch_movement (void)
{
	if (read_DRIVE_ENABLE_M2)
	{
		return false;
	}
	
	if (m2_quick_parameters_loaded == false)
	{
		return false;
	}
	
	m2_quick_timer_per = app_regs.REG_RESERVED11 + app_regs.REG_RESERVED13;
	m2_quick_state_ctrl = 0;
	
	/* Only executes the movement if the motor is stopped */
	if_moving_stop_rotation(2);
	
	if (app_regs.REG_RESERVED15 > 0)
	{
		set_DIR_M2;
	}
	else
	{
		clr_DIR_M2;
	}
	
	// 4 Stop motor if moving
	// 3 Wait 1 ms
	// 2 Star m2_start_quick_movement()
	// 1 Moving
	// 0 Stopped
	m2_quick_count_down = 4;
	
	m2_quick_relative_steps = abs(app_regs.REG_RESERVED15);
	
	return true;
}

void m1_start_quick_movement (void)
{	
	if (read_DRIVE_ENABLE_M1)
	{
		return;
	}
	
	/* Start the generation of pulses */
	timer_type0_pwm(&TCD0, TIMER_PRESCALER_DIV64, (app_regs.REG_RESERVED10 - 1) >> 1, 2 >> 1, INT_LEVEL_MED, INT_LEVEL_MED);
	
	motor_is_running[1] = true;
	
	
	if (core_bool_is_visual_enabled())
	{
		set_LED_M1;
	}
}

void m2_start_quick_movement (void)
{
	if (read_DRIVE_ENABLE_M2)
	{
		return;
	}
	
	/* Start the generation of pulses */
	timer_type0_pwm(&TCE0, TIMER_PRESCALER_DIV64, (app_regs.REG_RESERVED11 - 1) >> 1, 2 >> 1, INT_LEVEL_MED, INT_LEVEL_MED);
	
	motor_is_running[2] = true;
	
	
	if (core_bool_is_visual_enabled())
	{
		set_LED_M2;
	}
}


/************************************************************************/
/* Global variables                                                     */
/************************************************************************/

// Check file Harp Motion Controller Plots - New Trapezoidal Speed Control.html

uint32_t m1_speed_start;
uint32_t m1_speed_limit;
uint16_t m1_acc;
uint16_t m1_move_pulses;

uint32_t m2_speed_start;
uint32_t m2_speed_limit;
uint16_t m2_acc;
uint16_t m2_move_pulses;

uint32_t m1_speed;
uint32_t m1_delay;
bool m1_accelerating;
uint16_t m1_pulses_to_accelerate;
uint16_t m1_short_move_pulses;

uint8_t m1_exec_ctrl;
bool m1_use_steps;
bool m1_use_single_step;

uint32_t m2_speed;
uint32_t m2_delay;
bool m2_accelerating;
uint16_t m2_pulses_to_accelerate;
uint16_t m2_short_move_pulses;

uint8_t m2_exec_ctrl;
bool m2_use_steps;
bool m2_use_single_step;

uint32_t m1_timer_limit;
uint32_t m2_timer_limit;

#define MINIMUM_US_BETWEEN_PULSES 16


/************************************************************************/
/* Quick movement routines                                              */
/************************************************************************/

// Check file Harp Motion Controller Plots - New Trapezoidal Speed Control.html

bool m1_recalc_internal_paramenters (void)
{
	m1_speed_start = 1000.0 * app_regs.REG_MOTOR1_QUICK_START_SPEED / app_regs.REG_MOTOR1_QUICK_PULSE_DISTANCE;
	m1_speed_limit = 1000.0 * app_regs.REG_MOTOR1_QUICK_NOMINAL_SPEED / app_regs.REG_MOTOR1_QUICK_PULSE_DISTANCE;
	m1_acc = 1024.0 * app_regs.REG_MOTOR1_QUICK_ACCELERATION / app_regs.REG_MOTOR1_QUICK_PULSE_DISTANCE;
	if (app_regs.REG_MOTOR1_QUICK_DISTANCE > 0)
		m1_move_pulses = (app_regs.REG_MOTOR1_QUICK_DISTANCE * 1000.0 / app_regs.REG_MOTOR1_QUICK_PULSE_DISTANCE);
	else
		m1_move_pulses = (app_regs.REG_MOTOR1_QUICK_DISTANCE * -1000.0 / app_regs.REG_MOTOR1_QUICK_PULSE_DISTANCE);
		
	
	
	float max_pulses_check = app_regs.REG_MOTOR1_QUICK_DISTANCE * 1000.0 / app_regs.REG_MOTOR1_QUICK_PULSE_DISTANCE;
		
	if (max_pulses_check > 65530 /* 2^16 */)
		/* Check for max uint16 because m1_move_pulses is a uint16 */
		return false;
		
	return true;
}

bool m2_recalc_internal_paramenters (void)
{
	m2_speed_start = 1000.0 * app_regs.REG_MOTOR2_QUICK_START_SPEED / app_regs.REG_MOTOR2_QUICK_PULSE_DISTANCE;
	m2_speed_limit = 1000.0 * app_regs.REG_MOTOR2_QUICK_NOMINAL_SPEED / app_regs.REG_MOTOR2_QUICK_PULSE_DISTANCE;
	m2_acc = 1024.0 * app_regs.REG_MOTOR2_QUICK_ACCELERATION / app_regs.REG_MOTOR2_QUICK_PULSE_DISTANCE;
	if (app_regs.REG_MOTOR2_QUICK_DISTANCE > 0)
		m2_move_pulses = (app_regs.REG_MOTOR2_QUICK_DISTANCE * 1000.0 / app_regs.REG_MOTOR2_QUICK_PULSE_DISTANCE);
	else
		m2_move_pulses = (app_regs.REG_MOTOR2_QUICK_DISTANCE * -1000.0 / app_regs.REG_MOTOR2_QUICK_PULSE_DISTANCE);
	
	
	
	float max_pulses_check = app_regs.REG_MOTOR2_QUICK_DISTANCE * 1000.0 / app_regs.REG_MOTOR2_QUICK_PULSE_DISTANCE;
	
	if (max_pulses_check > 65530 /* 2^16 */)
		/* Check for max uint16 because m1_move_pulses is a uint16 */
		return false;
	
	return true;
}

bool m1_update_internal_variables (void)
{
	m1_timer_limit = (uint16_t)(1000000.0/m1_speed_limit) >> 1; // Shift right 1 position because timer is 2us resolution
	
	if (m1_timer_limit < (MINIMUM_US_BETWEEN_PULSES >> 1))
		// m1_timer_limit = MINIMUM_US_BETWEEN_PULSES >> 1;	// Make sure time between pulses don't go below the minimum acceptable
		return false;
	
	m1_speed = m1_speed_start;
	m1_delay = 0;
	m1_accelerating = true;
	m1_pulses_to_accelerate = 0;
	m1_short_move_pulses = m1_move_pulses/2.0;
	
	m1_exec_ctrl = 9;
	m1_use_steps = false;
	m1_use_single_step = false;
	
	if (m1_move_pulses <= 4)
	{
		return false;
	}
	
	return true;
}

bool m2_update_internal_variables (void)
{
	m2_timer_limit = (uint16_t)(1000000.0/m2_speed_limit) >> 1; // Shift right 1 position because timer is 2us resolution
	
	if (m2_timer_limit < (MINIMUM_US_BETWEEN_PULSES >> 1))
		// m1_timer_limit = MINIMUM_US_BETWEEN_PULSES >> 1;	// Make sure time between pulses don't go below the minimum acceptable
		return false;
	
	m2_speed = m1_speed_start;
	m2_delay = 0;
	m2_accelerating = true;
	m2_pulses_to_accelerate = 0;
	m2_short_move_pulses = m2_move_pulses/2.0;
	
	m2_exec_ctrl = 9;
	m2_use_steps = false;
	m2_use_single_step = false;
	
	if (m2_move_pulses <= 4)
	{
		return false;
	}
	
	return true;
}

bool m1_launch_quick_movement (void)
{
	if (m1_update_internal_variables() == false)
	{
		return false;
	}
	
	if (read_DRIVE_ENABLE_M1)
	{
		return false;
	}
	
	m1_quick_timer_per = app_regs.REG_RESERVED10 + app_regs.REG_RESERVED12;
	m1_quick_state_ctrl = 0;
	
	/* Only executes the movement if the motor is stopped */
	if_moving_stop_rotation(1);
	
	if (app_regs.REG_MOTOR1_QUICK_DISTANCE > 0)
	{
		set_DIR_M1;
	}
	else
	{
		clr_DIR_M1;
	}
	
	// 4 Stop motor if moving
	// 3 Wait 1 ms
	// 2 Star m1_start_quick_movement()
	// 1 Moving
	// 0 Stopped
	m1_quick_count_down = 4;
	
	m1_quick_relative_steps = abs(m1_move_pulses);
	
	return true;
}

bool m2_launch_quick_movement (void)
{
	if (m2_update_internal_variables() == false)
	{
		return false;
	}
	
	if (read_DRIVE_ENABLE_M2)
	{
		return false;
	}
	
	m2_quick_timer_per = app_regs.REG_RESERVED11 + app_regs.REG_RESERVED13;
	m2_quick_state_ctrl = 0;
	
	/* Only executes the movement if the motor is stopped */
	if_moving_stop_rotation(2);
	
	if (app_regs.REG_MOTOR2_QUICK_DISTANCE > 0)
	{
		set_DIR_M2;
	}
	else
	{
		clr_DIR_M2;
	}
	
	// 4 Stop motor if moving
	// 3 Wait 1 ms
	// 2 Star m2_start_quick_movement()
	// 1 Moving
	// 0 Stopped
	m2_quick_count_down = 4;
	
	m2_quick_relative_steps = abs(m2_move_pulses);
	
	return true;
}

void m1_initiate_quick_movement (void)
{
	if (read_DRIVE_ENABLE_M1)
	{
		return;
	}
	
	/* Start the generation of pulses */
	m1_delay = (uint16_t)(1000000.0/m1_speed);
	
	timer_type0_pwm(&TCD0, TIMER_PRESCALER_DIV64, (m1_delay >> 1) - 1, 2 >> 1, INT_LEVEL_MED, INT_LEVEL_MED);
	
	if (0)//m1_delay < 6000 && m1_delay > 20)
	{
		app_regs.REG_MOTOR2_QUICK_DISTANCE = m1_delay;
		core_func_send_event(ADD_REG_MOTOR2_QUICK_DISTANCE, true);
	}
	
	motor_is_running[1] = true;
	
	
	if (core_bool_is_visual_enabled())
	{
		set_LED_M1;
	}
}

void m2_initiate_quick_movement (void)
{
	if (read_DRIVE_ENABLE_M2)
	{
		return;
	}
	
	/* Start the generation of pulses */
	m2_delay = (uint16_t)(1000000.0/m2_speed);
	
	timer_type0_pwm(&TCE0, TIMER_PRESCALER_DIV64, (m2_delay >> 1) - 1, 2 >> 1, INT_LEVEL_MED, INT_LEVEL_MED);
	
	if (0)//m2_delay < 6000 && m2_delay > 20)
	{
		app_regs.REG_MOTOR2_QUICK_DISTANCE = m2_delay;
		core_func_send_event(ADD_REG_MOTOR2_QUICK_DISTANCE, true);
	}
	
	motor_is_running[2] = true;
	
	
	if (core_bool_is_visual_enabled())
	{
		set_LED_M2;
	}
}
