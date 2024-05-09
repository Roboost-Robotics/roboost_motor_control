/**
 * @file motor_controller.cpp
 * @author Jakob Friedl (friedl.jak@gmail.com)
 * @brief This file contains the implementation of a PID motor controller.
 * @version 0.1
 * @date 2023-07-06
 *
 * @copyright Copyright (c) 2023
 *
 */

#include <roboost/motor_control/motor_controllers/velocity_motor_controller.hpp>
#include <roboost/utils/comparisons.hpp>

using namespace roboost::motor_control;
using namespace roboost::controllers;
using namespace roboost::filters;

VelocityController::VelocityController(MotorDriver& motor_driver, Encoder& encoder, PIDController& pid, Filter& input_filter, Filter& output_filter)
    : MotorController(motor_driver), encoder_(encoder), pid_(pid), input_filter_(input_filter), output_filter_(output_filter)
{
}

void VelocityController::set_target(double desired_rotation_speed)
{
    encoder_.update();

    double input = encoder_.get_velocity();

    input = input_filter_.update(input);

    double output = pid_.update(desired_rotation_speed, input);

    output = output_filter_.update(output);

    motor_driver_.set_motor_control(output);
}

double VelocityController::get_measurement() const { return encoder_.get_velocity(); }