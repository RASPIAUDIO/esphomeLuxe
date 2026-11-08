"""Experimental classic-ESP32 adapter for Espressif's aec_* API."""

import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components.esp32 import add_idf_component, get_esp32_variant
from esphome.components.esp_audio_stack import AudioProcessor
from esphome.const import CONF_ID, Framework

DEPENDENCIES = ["esp32", "esp_audio_stack", "psram"]
CONF_FILTER_LENGTH = "filter_length"
CONF_FILTER1_DIAGNOSTIC = "allow_filter1_diagnostic"
CONF_HEAP_DIAGNOSTICS = "heap_diagnostics"

def validate_classic(config):
    if get_esp32_variant() != "ESP32":
        raise cv.Invalid("luxe_aec is the classic ESP32 experiment, not the Radio S3 AEC")
    if config[CONF_FILTER_LENGTH] == 1 and not config[CONF_FILTER1_DIAGNOSTIC]:
        raise cv.Invalid("Filter 1 requires allow_filter1_diagnostic: true and USB crash capture")
    return config

ClassicAec = cg.esphome_ns.namespace("luxe_aec").class_(
    "ClassicAec", cg.Component, AudioProcessor
)
CONFIG_SCHEMA = cv.All(
    cv.only_with_framework(Framework.ESP_IDF),
    cv.Schema({
        cv.GenerateID(): cv.declare_id(ClassicAec),
        # Filter 1 is opt-in only, for a controlled reproduction with USB logs.
        cv.Optional(CONF_FILTER_LENGTH, default=2): cv.int_range(min=1, max=4),
        cv.Optional(CONF_FILTER1_DIAGNOSTIC, default=False): cv.boolean,
        cv.Optional(CONF_HEAP_DIAGNOSTICS, default=False): cv.boolean,
    }).extend(cv.COMPONENT_SCHEMA),
    validate_classic,
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_filter_length(config[CONF_FILTER_LENGTH]))
    cg.add(var.set_heap_diagnostics(config[CONF_HEAP_DIAGNOSTICS]))
    # Exact versions: classic-ESP32 uses aec_create/process, not afe_aec_*.
    add_idf_component(name="espressif/esp-sr", ref="2.6.0")
    add_idf_component(name="espressif/esp-dsp", ref="1.8.0")
    cg.add_define("USE_AUDIO_PROCESSOR")
