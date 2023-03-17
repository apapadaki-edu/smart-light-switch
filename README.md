

The following block of code decides whether the user will have control of the device
or if the switch will work on auto mode.
```
void loop() {
  Blynk.run();
  
  // Important piece that controls the sensor
  if(!Blynk.connected()){
    //Turn on switch when light is low
    if (lightVal <= 150 && (int) now.Hour() < 17 ) {
        servo.write(130); // open switch
    }else {
        servo.write(0);// close switch
    }
  }else {
    if (appState == 1){
      servo.write(130);
    }else{
      servo.write(0);
    }
  }
```

The luminosity read from the analog pin, along with the state of the switch (either being on or off, this depends on the angle of the servo motor; 0 degrees (closed), 130 degrees (open)), is published to the broker. 
with theme of "luminosity/SwitchPj3/dat" and in the form of "luminosity:servo_motor_angle";
-- The angles were decided, with my home switches in mind. Since I could not alter the switch in my house from withing I used a servo motor the pushes the already installed switch up or down. --

```
    /*read the light measurement every 10 minutes*/
  if (currentTime-previousTime >= lightReadInterval){
      lightVal = readLightVal(lightInputPin);
      Serial.println(servo.read());
      formatNumber(lightVal, servo.read());
      client.publish("luminocity/SwitchPj3/dat", lightString);
      previousTime = currentTime;
  } 
}
```
