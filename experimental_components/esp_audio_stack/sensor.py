"""Sensor platform for ESP Audio Stack diagnostics."""

import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
import esphome.final_validate as fv
from esphome.const import ENTITY_CATEGORY_DIAGNOSTIC

from . import CONF_ESP_AUDIO_STACK_ID, ESPAudioStack, esp_audio_stack_ns

CONF_SLOT = "slot"
CONF_TDM_SLOT_LEVELS = "tdm_slot_levels"
CONF_STD_SLOT_LEVELS = "std_slot_levels"

TdmSlotLevelSensor = esp_audio_stack_ns.class_(
    "TdmSlotLevelSensor",
    sensor.Sensor,
    cg.PollingComponent,
    cg.Parented.template(ESPAudioStack),
)


def _slot_level_schema(slot_validator):
    return (
        sensor.sensor_schema(
            TdmSlotLevelSensor,
            unit_of_measurement="dBFS",
            accuracy_decimals=1,
            entity_category=ENTITY_CATEGORY_DIAGNOSTIC,
            icon="mdi:microphone-message",
        )
        .extend(cv.polling_component_schema("500ms"))
        .extend({cv.Required(CONF_SLOT): slot_validator})
    )


CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ESP_AUDIO_STACK_ID): cv.use_id(ESPAudioStack),
        cv.Optional(CONF_TDM_SLOT_LEVELS): cv.ensure_list(
            _slot_level_schema(cv.int_range(min=0, max=7))
        ),
        cv.Optional(CONF_STD_SLOT_LEVELS): cv.ensure_list(
            _slot_level_schema(cv.one_of("left", "right", lower=True))
        ),
    }
)


def _validate_slot_bus(config):
    stacks = fv.full_config.get().get("esp_audio_stack", [])
    if isinstance(stacks, dict):
        stacks = [stacks]
    parent = next(p for p in stacks if p["id"] == config[CONF_ESP_AUDIO_STACK_ID])
    tdm = parent.get("use_tdm_reference", False) or "tdm_mic_slots" in parent
    if config.get(CONF_STD_SLOT_LEVELS) and (tdm or parent.get("rx_slot_mode") != "stereo"):
        raise cv.Invalid("std_slot_levels requires standard I2S rx_slot_mode: stereo")
    if config.get(CONF_TDM_SLOT_LEVELS):
        if not tdm:
            raise cv.Invalid("tdm_slot_levels requires a TDM bus")
        for slot in config[CONF_TDM_SLOT_LEVELS]:
            if slot[CONF_SLOT] >= parent.get("tdm_total_slots", 4):
                raise cv.Invalid("TDM level sensor slot is outside tdm_total_slots")
    return config


FINAL_VALIDATE_SCHEMA = _validate_slot_bus


async def to_code(config):
    parent = await cg.get_variable(config[CONF_ESP_AUDIO_STACK_ID])

    for key in (CONF_TDM_SLOT_LEVELS, CONF_STD_SLOT_LEVELS):
        entries = config.get(key, [])
        if entries:
            cg.add_define("USE_ESP_AUDIO_STACK_SLOT_LEVELS")
        for conf in entries:
            slot = conf[CONF_SLOT]
            if key == CONF_STD_SLOT_LEVELS:
                slot = {"left": 0, "right": 1}[slot]
            cg.add(parent.set_tdm_slot_level_sensor_enabled(slot, True))
            var = await sensor.new_sensor(conf)
            await cg.register_component(var, conf)
            cg.add(var.set_parent(parent))
            cg.add(var.set_slot(slot))
