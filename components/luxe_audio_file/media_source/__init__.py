import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import media_source
from esphome.components.audio_file.media_source import AudioFileMediaSource
from esphome.const import CONF_SOURCE

DEPENDENCIES = ["audio_file"]

luxe_audio_file_ns = cg.esphome_ns.namespace("luxe_audio_file")
CooperativeFileSource = luxe_audio_file_ns.class_(
    "CooperativeFileSource", cg.Component, media_source.MediaSource
)

CONFIG_SCHEMA = media_source.media_source_schema(CooperativeFileSource).extend(
    {cv.Required(CONF_SOURCE): cv.use_id(AudioFileMediaSource)}
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config["id"])
    await cg.register_component(var, config)
    await media_source.register_media_source(var, config)
    source = await cg.get_variable(config[CONF_SOURCE])
    cg.add(var.set_source(source))
