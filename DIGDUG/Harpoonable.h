#pragma once

class Harpoonable {
public:
    virtual void AttachHarpoon() = 0;
    virtual void DetachHarpoon() = 0;
    virtual void Inflate() = 0;
    virtual bool isHarpoonAttached() const = 0;
    virtual ~Harpoonable() = default;

    virtual int pumpState = 0; 
    virtual float pumpTimer = 0.0f;
    virtual const float PUMP_DURATION = 3.0f;
};