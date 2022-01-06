#ifndef BLESERVER_H
#define BLESERVER_H

class bleInterface {
  public:
    void begin();
    void updateFan();
    void updateUptime();
    void updateTemperature(float, float);
    void stopAdvertising();
};

extern bool deviceConnected;

#endif //BLESERVER_H
