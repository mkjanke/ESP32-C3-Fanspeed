#ifndef BLEINTERFACE_H
#define BLEINTERFACE_H

class bleInterface {
 public:
  static bool deviceConnected;

  void begin();
  void updateFan();
  void updateUptime(char *);
  void updateTemperature(float, float);
  void stopAdvertising();
};

#endif  // BLEINTERFACE_H
