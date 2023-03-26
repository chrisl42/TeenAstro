#include "MotionControl.h"


class MotorDriver
{
public:
  TMC5160Stepper *drvP = NULL;
  MotionControl *mcP = NULL;
  unsigned int gear;
  unsigned int stepRot;
  byte micro = 4;
  bool reverse = false;
  bool silent = true;
  unsigned int highCurr=100; //in mA
  unsigned int lowCurr=100; //in mA
  SemaphoreHandle_t mutex = 0;
  int CSPin;

  // generic init - call this one first
  void init(int cspin, SemaphoreHandle_t mtx)
  {
    mutex = mtx;

    CSPin = cspin;
    drvP = new TMC5160Stepper(CSPin);
    // Initialize 5160 through SPI
    drvP->begin();  
    drvP->reset();
    drvP->push();
    drvP->TPOWERDOWN(255);
    drvP->tbl(2);
    drvP->toff(5);
    drvP->hstrt(5);
    drvP->hend(3);
    drvP->en_pwm_mode(silent);
    drvP->pwm_autoscale(silent);
    drvP->TPWMTHRS(64);
    drvP->intpol(1);
    setCurrent(lowCurr);
    setMicrostep(micro);
  }

  // StepDir constructor
  void initStepDir(int DirPin, int StepPin, void (*isrP)(), unsigned timerId)
  {
    pinMode(DirPin, OUTPUT);
    digitalWrite(DirPin, LOW);

    pinMode(StepPin, OUTPUT);
    digitalWrite(StepPin, LOW);

    pinMode(CSPin, OUTPUT);
    digitalWrite(CSPin, HIGH);

    drvP->dedge(true);		// enable double edge step impulse 

    // StepDir driver
    mcP = new StepDir();
    mcP->initStepDir(DirPin, StepPin, isrP, timerId);
  }

  // 5160 MotionController constructor (SPI only)
  void initMc5160(SemaphoreHandle_t mtx)
  {
    // MotionControl driver
    mcP = new Mc5160();
    mcP->initMc5160(drvP, mtx);
  }

  void setCurrent(unsigned int val)
  {
    if (mutex)
      xSemaphoreTake(mutex, portMAX_DELAY);
    drvP->rms_current(val / sqrt(2));
    if (mutex)
      xSemaphoreGive(mutex);
  }
  
  void setMicrostep(int mu)
  {
    int val = (int)pow(2., mu);
    if (mutex)
      xSemaphoreTake(mutex, portMAX_DELAY);
    drvP->microsteps(val);   
    if (mutex)
      xSemaphoreGive(mutex);
  }

  
  
  void setCurrentPos(long p)
  {
    mcP->setCurrentPos(reverse ? -p:p);
  }
  void setTargetPos(long p)
  {
    mcP->setTargetPos(reverse ? -p:p);
  }
  void setVmax(double v)
  {
    mcP->setVmax(v);
  }
  void setAmax(long a)
  {
    mcP->setAmax(a);
  }
  long getCurrentPos()
  {
    long p = mcP->getCurrentPos();
    return (reverse ? -p:p); 
  }
  long getTargetPos()
  {
    long p = mcP->getTargetPos(); 
    return (reverse ? -p:p); 
  }
  double getSpeed()
  {
    double v = mcP->getSpeed(); 
    return (reverse ? -v:v); 
  }
  bool positionReached()
  {
    bool p = mcP->positionReached(); 
    return p;
  }
  bool isMoving()
  {
    bool p = mcP->isMoving(); 
    return p;
  }
  void adjustSpeed(double percent)
  {
    mcP->adjustSpeed(percent); 
  }
  void abort()
  {
    mcP->abort(); 
  }
  void resetAbort()
  {
    mcP->resetAbort(); 
  }


};