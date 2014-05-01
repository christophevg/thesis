  static time_t next_measurement = 0;
  if(next_measurement == 0) { next_measurement = clock_get_millis(); }

  if( clock_get_millis() < next_measurement) { return; }
  next_measurement += 5000;

  avr_set_bit(STATUS_LED_PORT,    // turn on the green status led
              STATUS_LED_PIN);

  uint16_t reading;               // the 16-bit reading from the ADC
  uint8_t  values[2];             // the bytes containing the reading in bytes

  // read light sensor
  reading = avr_adc_read(LIGHT_SENSOR_PIN);
  values[0] = (reading >> 8);
  values[1] = reading;
  
  _log("light reading: %02x %02x\n", values[0], values[1]);

  // and send it to the coordinator through the mesh
  mesh_send(address, DESTINATION, 2, values);

  avr_clear_bit(STATUS_LED_PORT,  // turn off the green status led
                STATUS_LED_PIN);
