import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, i2c, sensor
from esphome.const import CONF_ID, ENTITY_CATEGORY_DIAGNOSTIC

DEPENDENCIES = ["i2c"]
AUTO_LOAD = ["sensor", "binary_sensor"]

ns = cg.esphome_ns.namespace("luxe_ip5306")
LuxeIP5306 = ns.class_("LuxeIP5306", cg.PollingComponent, i2c.I2CDevice)

CONFIG_SCHEMA = (
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(LuxeIP5306),
            cv.Required("available"): binary_sensor.binary_sensor_schema(
                device_class="connectivity", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Required("charging"): binary_sensor.binary_sensor_schema(
                device_class="battery_charging"
            ),
            cv.Required("full"): binary_sensor.binary_sensor_schema(),
            cv.Required("charge_allowed"): binary_sensor.binary_sensor_schema(
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Required("level"): sensor.sensor_schema(
                unit_of_measurement="%", accuracy_decimals=0, device_class="battery",
                state_class="measurement", entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
            cv.Required("current_limit"): sensor.sensor_schema(
                unit_of_measurement="mA", accuracy_decimals=0,
                entity_category=ENTITY_CATEGORY_DIAGNOSTIC
            ),
        }
    )
    .extend(cv.polling_component_schema("10s"))
    .extend(i2c.i2c_device_schema(0x75))
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await i2c.register_i2c_device(var, config)
    for key in ("available", "charging", "full", "charge_allowed"):
        entity = await binary_sensor.new_binary_sensor(config[key])
        cg.add(getattr(var, f"set_{key}")(entity))
    for key in ("level", "current_limit"):
        entity = await sensor.new_sensor(config[key])
        cg.add(getattr(var, f"set_{key}")(entity))

