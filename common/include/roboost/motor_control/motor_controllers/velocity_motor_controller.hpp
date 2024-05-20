/**
 * @file velocity_motor_controller.hpp
 * @brief Definition of MotorController, which sets the control output directly to the motor driver with encoder feedback and PID control.
 * @version 0.2
 * @date 2024-05-17
 */

#ifndef VELOCITY_MOTOR_CONTROLLER_HPP
#define VELOCITY_MOTOR_CONTROLLER_HPP

#include <roboost/motor_control/encoders/encoder.hpp>
#include <roboost/motor_control/motor_controllers/motor_controller.hpp>
#include <roboost/utils/callback_scheduler.hpp>
#include <roboost/utils/controllers.hpp>
#include <roboost/utils/filters.hpp>

namespace roboost
{
    namespace motor_control
    {
        /**
         * @brief Implementation of MotorController, which sets the control output directly to the motor driver with encoder feedback and PID control.
         *
         * @tparam MotorDriverType The type of the motor driver.
         * @tparam EncoderType The type of the encoder.
         * @tparam ControllerType The type of the PID controller.
         * @tparam InputFilterType The type of the input filter.
         * @tparam OutputFilterType The type of the output filter.
         * @tparam RateLimitingFilterType The type of the rate limiting filter.
         */
        template <typename MotorDriverType, typename EncoderType, typename ControllerType, typename InputFilterType, typename OutputFilterType, typename RateLimitingFilterType>
        class VelocityController
            : public MotorControllerBase<VelocityController<MotorDriverType, EncoderType, ControllerType, InputFilterType, OutputFilterType, RateLimitingFilterType>, MotorDriverType>
        {
        public:
            /**
             * @brief Construct a new VelocityController object.
             *
             * @param motor_driver The motor driver to control.
             * @param encoder The encoder to read the rotation speed from.
             * @param pid_controller The PID controller to use.
             * @param input_filter The filter to use for the input.
             * @param output_filter The filter to use for the output.
             * @param rate_limiting_filter The rate limiting filter to use.
             * @param deadband_threshold The deadband threshold.
             * @param minimum_output The minimum output.
             */
            VelocityController(MotorDriverType& motor_driver, EncoderType& encoder, ControllerType& pid_controller, InputFilterType& input_filter, OutputFilterType& output_filter,
                               RateLimitingFilterType& rate_limiting_filter, int32_t deadband_threshold, int32_t minimum_output)
                : MotorControllerBase<VelocityController<MotorDriverType, EncoderType, ControllerType, InputFilterType, OutputFilterType, RateLimitingFilterType>, MotorDriverType>(motor_driver),
                  encoder_(encoder), pid_(pid_controller), input_filter_(input_filter), output_filter_(output_filter), rate_limiting_filter_(rate_limiting_filter),
                  deadband_threshold_(deadband_threshold), minimum_output_(minimum_output), current_setpoint_(0)
            {
            }

            /**
             * @brief Set the rotation speed of the motor.
             *
             * @param desired_rotation_speed The desired rotation speed in ticks/s.
             */
            void set_target(float desired_rotation_speed)
            {
                encoder_.update();
                float input = encoder_.get_velocity_radians_per_second(); // in ticks/s
                input = input_filter_.update(input);

                current_setpoint_ = rate_limiting_filter_.update(desired_rotation_speed, input);

                float output = pid_.update(current_setpoint_, input);
                output = output_filter_.update(output);

                // Adjust output for deadband and minimum output
                if (abs(output) < deadband_threshold_)
                {
                    output = 0;
                }
                else if (abs(output) < minimum_output_)
                {
                    output = minimum_output_ * (output / abs(output));
                }

                // map float output (-1, 1) to int32_t motor_control_value (-PWM_RESOLUTION, PWM_RESOLUTION)
                int32_t motor_control_value = static_cast<int32_t>(output * PWM_RESOLUTION);

                this->motor_driver_.set_motor_control(motor_control_value);
            }

            int64_t get_measurement() const { return encoder_.get_velocity_radians_per_second(); }

            int64_t get_setpoint() const { return current_setpoint_; }

        private:
            EncoderType& encoder_;
            ControllerType& pid_;
            InputFilterType& input_filter_;
            OutputFilterType& output_filter_;
            RateLimitingFilterType& rate_limiting_filter_;
            int32_t deadband_threshold_;
            int32_t minimum_output_;
            int64_t current_setpoint_;
        };

    } // namespace motor_control
} // namespace roboost

#endif // VELOCITY_MOTOR_CONTROLLER_HPP
