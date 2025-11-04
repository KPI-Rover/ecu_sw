#include "motors_manager.h"
#include "cmsis_os.h"
#include "ulog.h"

extern TIM_HandleTypeDef htim1;

#define MOTOR_TIMER_PERIOD_MS   50u
#define MOTOR_MANAGER_MAX_MOTORS 4

static osTimerId_t motor_timer_handle;
static Motor_HandleTypeDef motors[MOTOR_MANAGER_MAX_MOTORS];
static uint8_t motors_count = 0;
static void motor_timer_callback(void *argument);


static Motor_HandleTypeDef* get_motor_by_id(uint8_t id) {
    if (id >= motors_count) return NULL;
    return &motors[id];
}

static void register_motor(const Motor_HandleTypeDef* src, uint8_t id) {
    if (id >= MOTOR_MANAGER_MAX_MOTORS) {
        ULOG_ERROR("register_motor: id %u out of range", id);
        return;
    }
    motors[id] = *src;
    if (id >= motors_count) motors_count = id + 1;
}



void MotorManager_Init(void) {

    Motor_HandleTypeDef m0;
    m0.IN1_port = GPIOC;
    m0.IN1_pin  = GPIO_PIN_4;
    m0.IN2_port = GPIOC;
    m0.IN2_pin  = GPIO_PIN_5;
    m0.htim_pwm = &htim1;
    m0.pwm_channel = TIM_CHANNEL_1;
    m0.state = MOTOR_STATE_STOP;
    m0.substate = MOTOR_SUBSTATE_NONE;
    m0.state_entry_time_ms = HAL_GetTick();
    m0.target_speed = 0;
    m0.current_speed = 0;

    register_motor(&m0, 0);
    DriverMotors_Init(&motors[0]);


    Motor_HandleTypeDef m1;
    m1.IN1_port = GPIOE;
    m1.IN1_pin  = GPIO_PIN_7;
    m1.IN2_port = GPIOE;
    m1.IN2_pin  = GPIO_PIN_8;
    m1.htim_pwm = &htim1;
    m1.pwm_channel = TIM_CHANNEL_2;
    m1.state = MOTOR_STATE_STOP;
    m1.substate = MOTOR_SUBSTATE_NONE;
    m1.state_entry_time_ms = HAL_GetTick();
    m1.target_speed = 0;
    m1.current_speed = 0;

    register_motor(&m1, 1);
    DriverMotors_Init(&motors[1]);



    const osTimerAttr_t timer_attrs = {
        .name = "Motor_Timer"
    };

    motor_timer_handle = osTimerNew(motor_timer_callback, osTimerPeriodic, NULL, &timer_attrs);
    if (!motor_timer_handle) {
        ULOG_ERROR("Failed to create Motor timer");
        osThreadExit();
    }

    osDelay(10);

    if (osTimerStart(motor_timer_handle, MOTOR_TIMER_PERIOD_MS) != osOK) {
        ULOG_ERROR("Failed to start Motor timer");
        osThreadExit();
    }
}

void MotorManager_Task(void* argument) {
    MotorManager_Init();
    while(1) {

        MotorManager_MoveForward(100);
        ULOG_INFO("MoveForward (all registered)");
        osDelay(4000);

        MotorManager_Stop();
        osDelay(1000);

        MotorManager_MoveBackward(55);
        ULOG_INFO("MoveBackward (all registered)");
        osDelay(3000);

        MotorManager_MoveForwardMotor(0,100);
        ULOG_INFO("MoveForward (one registered)");
        osDelay(2000);

        MotorManager_MoveForwardMotor(1,100);
		ULOG_INFO("MoveForward (one registered)");
		osDelay(2000);

        MotorManager_Stop();
        osDelay(1000);

    }
}

static void motor_timer_callback(void *argument) {
    for (uint8_t i = 0; i < motors_count; ++i) {
        DriverMotors_TimerTask(&motors[i]);
    }
}

void MotorManager_MoveForward(uint8_t speed) {
    for (uint8_t i = 0; i < motors_count; ++i) {
        if (motors[i].substate == MOTOR_SUBSTATE_DECEL) {
            ULOG_INFO("Motor %u: Can't move forward while decelerating (skipped)", i);
            continue;
        }
        DriverMotors_Forward(&motors[i]);
        motors[i].target_speed = speed;
        motors[i].current_speed = 0;
        motors[i].state = MOTOR_STATE_FORWARD;
        motors[i].substate = MOTOR_SUBSTATE_ACCEL;
        motors[i].state_entry_time_ms = HAL_GetTick();
    }
}

void MotorManager_MoveBackward(uint8_t speed) {
    for (uint8_t i = 0; i < motors_count; ++i) {
        if (motors[i].substate == MOTOR_SUBSTATE_DECEL) {
            ULOG_INFO("Motor %u: Can't move backward while decelerating (skipped)", i);
            continue;
        }
        DriverMotors_Backward(&motors[i]);
        motors[i].target_speed = speed;
        motors[i].current_speed = 0;
        motors[i].state = MOTOR_STATE_BACKWARD;
        motors[i].substate = MOTOR_SUBSTATE_ACCEL;
        motors[i].state_entry_time_ms = HAL_GetTick();
    }
}

void MotorManager_Stop(void) {
    for (uint8_t i = 0; i < motors_count; ++i) {
        if (motors[i].state == MOTOR_STATE_FORWARD || motors[i].state == MOTOR_STATE_BACKWARD) {
            motors[i].substate = MOTOR_SUBSTATE_DECEL;
        } else {
            DriverMotors_Stop(&motors[i]);
            motors[i].state = MOTOR_STATE_STOP;
            motors[i].substate = MOTOR_SUBSTATE_NONE;
        }
    }
}


void MotorManager_MoveForwardMotor(uint8_t id, uint8_t speed) {
    Motor_HandleTypeDef* m = get_motor_by_id(id);
    if (!m) {
        ULOG_ERROR("MoveForwardMotor: invalid id %u", id);
        return;
    }

    for (uint8_t i = 0; i < motors_count; ++i) {
        if (i != id) {
            Motor_HandleTypeDef* other = &motors[i];
            if (other->state == MOTOR_STATE_FORWARD || other->state == MOTOR_STATE_BACKWARD) {
                other->substate = MOTOR_SUBSTATE_DECEL;
            }
        }
    }

    if (m->substate == MOTOR_SUBSTATE_DECEL) {
        ULOG_INFO("Motor %u: Can't move forward while decelerating", id);
        return;
    }

    DriverMotors_Forward(m);
    m->target_speed = speed;
    m->current_speed = 0;
    m->state = MOTOR_STATE_FORWARD;
    m->substate = MOTOR_SUBSTATE_ACCEL;
    m->state_entry_time_ms = HAL_GetTick();

    ULOG_INFO("Motor %u: Move forward %u%%", id, speed);
}

void MotorManager_MoveBackwardMotor(uint8_t id, uint8_t speed) {
    Motor_HandleTypeDef* m = get_motor_by_id(id);
    if (!m) {
        ULOG_ERROR("MoveBackwardMotor: invalid id %u", id);
        return;
    }

    for (uint8_t i = 0; i < motors_count; ++i) {
        if (i != id) {
            Motor_HandleTypeDef* other = &motors[i];
            if (other->state == MOTOR_STATE_FORWARD || other->state == MOTOR_STATE_BACKWARD) {
                other->substate = MOTOR_SUBSTATE_DECEL;
            }
        }
    }

    if (m->substate == MOTOR_SUBSTATE_DECEL) {
        ULOG_INFO("Motor %u: Can't move backward while decelerating", id);
        return;
    }

    DriverMotors_Backward(m);
    m->target_speed = speed;
    m->current_speed = 0;
    m->state = MOTOR_STATE_BACKWARD;
    m->substate = MOTOR_SUBSTATE_ACCEL;
    m->state_entry_time_ms = HAL_GetTick();

    ULOG_INFO("Motor %u: Move backward %u%%", id, speed);
}

void MotorManager_StopMotor(uint8_t id) {
    Motor_HandleTypeDef* m = get_motor_by_id(id);
    if (!m) {
        ULOG_ERROR("StopMotor: invalid id %u", id);
        return;
    }

    if (m->state == MOTOR_STATE_FORWARD || m->state == MOTOR_STATE_BACKWARD) {
        m->substate = MOTOR_SUBSTATE_DECEL;
    } else {
        DriverMotors_Stop(m);
        m->state = MOTOR_STATE_STOP;
        m->substate = MOTOR_SUBSTATE_NONE;
    }

    ULOG_INFO("Motor %u: Stop", id);
}
