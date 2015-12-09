struct float_value {
  String label;
  String field;
  float value;
  String UoM;
};

struct long_value {
  String label;
  String field;  
  long value;
  String UoM;
};

struct sensor_data_type {
  float_value pressure;
  
  float_value temperature_c;
  float_value temperature_f;
  
  float_value heat_index_c;
  float_value heat_index_f;

  long_value humidity;
};

