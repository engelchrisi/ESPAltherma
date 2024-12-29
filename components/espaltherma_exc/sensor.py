import esphome.codegen as cg
from esphome.components import sensor
from esphome.helpers import sanitize, snake_case
from . import CONF_COMP_ID
import os
from .generate_sensors import SensorMetaData, read_sensor_metadata, parse_label_def
from .gen.sensor_config_schema import CONFIG_SCHEMA

#print(f'Loading {CONFIG_SCHEMA}')

async def to_code(config):
  print("sensor.to_code")
  var = await cg.get_variable(config[CONF_COMP_ID])
  print("<=== " + str(var))

  #labels_file = './common/external_components/espaltherma_exc/labelDefinitions.h'
  current_file_directory = os.path.dirname(os.path.abspath(__file__))
  labels_file = os.path.join(current_file_directory, 'labelDefinitions.h')
  sensor_limit = None
  sensor_data_list = read_sensor_metadata(labels_file, sensor_limit, "sensor")

  for sensor_data in sensor_data_list:
    unique_id = sensor_data.unique_id

    if unique_id in config:
      conf = config[unique_id]
      # print("Config: "+ str(conf))
      sens = await sensor.new_sensor(conf)

      # code in esphome's setup_entity always sets the sanitized name as object id:
      # add(var.set_object_id(sanitize(snake_case(config[CONF_NAME]))))
      # Therefore add a 2nd call overwriting this with our unique ID
      cg.add(sens.set_object_id(sanitize(snake_case(unique_id))))

      print("Sensor: "+ str(sens))
      cg.add(var.do_register_sensor(sens))


