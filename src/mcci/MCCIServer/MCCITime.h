#pragma once

#include "MCCITypes.h"

// we may define time in several ways.
class CMCCITime
{
  public:
    CMCCITime() {}
    virtual ~CMCCITime() {}
    
    virtual MCCI_TIME_T now() const = 0;
};

// FIXME: will reflect real-time clock for production
class CMCCITimeReal : CMCCITime
{
  public:
    CMCCITimeReal() : CMCCITime() {}
    ~CMCCITimeReal() {}

    // FIXME: need time implementation
    virtual MCCI_TIME_T now() const { throw string("NOT IMPLEMENTED YET"); }
    
};

// a fake-able clock for debugging purposes
class CMCCITimeFake : CMCCITime
{
  private:
    MCCI_TIME_T fake_now;
    
  public:
    CMCCITimeFake() : CMCCITime() { this->fake_now = 0; }
    ~CMCCITimeFake() {}

    virtual MCCI_TIME_T now() const { return this->fake_now; }

    void set_now(MCCI_TIME_T v) { this->fake_now = v; }
        
};
