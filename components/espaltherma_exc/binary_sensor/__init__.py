import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor
from esphome.helpers import sanitize, snake_case
from esphome.const import CONF_ID
from .. import CONF_COMP_ID, ESPAlthermaComponent

from .. import espaltherma_ns

# Example configuration entry
# - platform: espaltherma_exc
#   name: "Thermostat#1"
#   id: thermostat1
#   max_retries: 3
#   retry_interval_ms: 250

RepeatingBinarySensor = espaltherma_ns.class_(
    "RepeatingBinarySensor",
    binary_sensor.BinarySensor,
    cg.PollingComponent,
)

CONF_MAX_RETRIES =          "max_retries"
CONF_RETRY_INTERVAL =       "retry_interval_ms"

CONFIG_SCHEMA = cv.All(
    binary_sensor.binary_sensor_schema(RepeatingBinarySensor)
    .extend(
        {
            cv.GenerateID(): cv.declare_id(RepeatingBinarySensor),
            cv.GenerateID(CONF_COMP_ID): cv.use_id(ESPAlthermaComponent),
            cv.Optional(CONF_MAX_RETRIES, default=3): cv.positive_int,
            cv.Optional(CONF_RETRY_INTERVAL, default=1000): cv.positive_int,
        }
    ),
)

# s. also esphome\components\modbus_controller\binary_sensor\__init__.py
async def to_code(config):
  sens = cg.new_Pvariable(
      config[CONF_ID],
      config[CONF_MAX_RETRIES],
      config[CONF_RETRY_INTERVAL],
  )

  print("binary_sensor.to_code" + sens)

  await cg.register_component(sens, config)
  await binary_sensor.register_binary_sensor(sens, config)

  # take unique name from YAML file wiederanlaufverzoegerung instead of wiederanlaufverz_gerung
  unique_id = str(sens)
  cg.add(sens.set_object_id(sanitize(snake_case(unique_id))))

  paren = await cg.get_variable(config[CONF_COMP_ID])
  cg.add(paren.do_register_binary_sensor(sens))
