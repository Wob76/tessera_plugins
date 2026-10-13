"""Tessera's kegerator tile: read keg entities from Home Assistant."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import binary_sensor, number, sensor, smart_display, text_sensor
from esphome.const import CONF_ID

DEPENDENCIES = ["smart_display"]

CONF_KEG1_VOLUME = "keg1_volume"
CONF_KEG1_GLASSES = "keg1_glasses"
CONF_KEG1_POUR_RATE = "keg1_pour_rate"
CONF_KEG1_FLOW = "keg1_flow"
CONF_KEG1_LABEL = "keg1_label"
CONF_KEG1_CAPACITY = "keg1_capacity"
CONF_KEG2_VOLUME = "keg2_volume"
CONF_KEG2_GLASSES = "keg2_glasses"
CONF_KEG2_POUR_RATE = "keg2_pour_rate"
CONF_KEG2_FLOW = "keg2_flow"
CONF_KEG2_LABEL = "keg2_label"
CONF_KEG2_CAPACITY = "keg2_capacity"

keg_tap_ns = cg.esphome_ns.namespace("keg_tap")
KegTap = keg_tap_ns.class_("KegTap", cg.Component)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(KegTap),
        cv.Required(CONF_KEG1_VOLUME): cv.use_id(sensor.Sensor),
        cv.Required(CONF_KEG1_GLASSES): cv.use_id(sensor.Sensor),
        cv.Required(CONF_KEG1_POUR_RATE): cv.use_id(sensor.Sensor),
        cv.Required(CONF_KEG1_FLOW): cv.use_id(binary_sensor.BinarySensor),
        cv.Required(CONF_KEG1_LABEL): cv.use_id(text_sensor.TextSensor),
        cv.Required(CONF_KEG1_CAPACITY): cv.use_id(number.Number),
        cv.Required(CONF_KEG2_VOLUME): cv.use_id(sensor.Sensor),
        cv.Required(CONF_KEG2_GLASSES): cv.use_id(sensor.Sensor),
        cv.Required(CONF_KEG2_POUR_RATE): cv.use_id(sensor.Sensor),
        cv.Required(CONF_KEG2_FLOW): cv.use_id(binary_sensor.BinarySensor),
        cv.Required(CONF_KEG2_LABEL): cv.use_id(text_sensor.TextSensor),
        cv.Required(CONF_KEG2_CAPACITY): cv.use_id(number.Number),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await smart_display.register_plugin(var, __file__)

    cg.add(var.set_keg1_volume(await cg.get_variable(config[CONF_KEG1_VOLUME])))
    cg.add(var.set_keg1_glasses(await cg.get_variable(config[CONF_KEG1_GLASSES])))
    cg.add(var.set_keg1_pour_rate(await cg.get_variable(config[CONF_KEG1_POUR_RATE])))
    cg.add(var.set_keg1_flow(await cg.get_variable(config[CONF_KEG1_FLOW])))
    cg.add(var.set_keg1_label(await cg.get_variable(config[CONF_KEG1_LABEL])))
    cg.add(var.set_keg1_capacity(await cg.get_variable(config[CONF_KEG1_CAPACITY])))
    cg.add(var.set_keg2_volume(await cg.get_variable(config[CONF_KEG2_VOLUME])))
    cg.add(var.set_keg2_glasses(await cg.get_variable(config[CONF_KEG2_GLASSES])))
    cg.add(var.set_keg2_pour_rate(await cg.get_variable(config[CONF_KEG2_POUR_RATE])))
    cg.add(var.set_keg2_flow(await cg.get_variable(config[CONF_KEG2_FLOW])))
    cg.add(var.set_keg2_label(await cg.get_variable(config[CONF_KEG2_LABEL])))
    cg.add(var.set_keg2_capacity(await cg.get_variable(config[CONF_KEG2_CAPACITY])))
