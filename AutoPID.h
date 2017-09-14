#ifndef AUTOPID_H
#define AUTOPID_H
#include <Arduino.h>

class AutoPID {

  public:

    AutoPID(double *input, double *setpoint, double *output, double outputMin, double outputMax,
            double Kp, double Ki, double Kd);

    void setGains(double Kp, double Ki, double Kd);
    void setBangBang(double bangOn, double bangOff);
    void setBangBang(double bangRange);
    void setOutputRange(double outputMin, double outputMax);
    void setTimeStep(unsigned long timeStep);

    bool atSetPoint(double threshold);

    void run();
    void stop();
    void reset();

  private:
    double _Kp, _Ki, _Kd;
    double _integral, _previousError;
    double _bangOn, _bangOff;
    double *_input, *_setpoint, *_output;
    double _outputMin, _outputMax;
    unsigned long _timeStep, _lastStep;
    bool _stopped;

};//class AutoPID

class AutoPIDRelay : public AutoPID {
  public:

    AutoPIDRelay(double *input, double *setpoint, bool *relayState, double pulseWidth, double Kp, double Ki, double Kd)
      : AutoPID(input, setpoint, &_pulseValue, 0, 1.0, Kp, Ki, Kd) {
      _relayState = relayState;
      _pulseWidth = pulseWidth;
    };

    void run();

    double getPulseValue();

  private:
    bool * _relayState;
    unsigned long _pulseWidth, _lastPulseTime;
    double _pulseValue;
};//class AutoPIDRelay

#endif
