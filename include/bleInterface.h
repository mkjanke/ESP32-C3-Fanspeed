#ifndef BLESERVER_H
#define BLESERVER_H

class bleInterface {
  public:
    void begin();
    void update();
    void stopAdvertising();
};

extern bool deviceConnected;

#endif //BLESERVER_H