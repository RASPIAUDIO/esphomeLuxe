# Embedded UI sounds

The firmware embeds `startup_24k.flac`, generated from the lossless reference with
`sox startup.flac -C 8 startup_24k.flac rate -v 24000`.
It occupies 61,567 bytes and has 65,536 samples at 24,000 Hz, retaining the
original duration and pitch.
The normal speaker resampler converts its decoded audio to the 48 kHz output.
Only the local cues are resampled; music and voice answers are not changed.

`startup.flac` is a lossless conversion of the existing museLuxe startup sound:

https://github.com/RASPIAUDIO/esphomeLuxe/blob/192372ff59b560a74aa547dc26af609918fcc9d6/wav/sounds_timer_finished.wav

Conversion: `sox sounds_timer_finished.wav -C 8 startup.flac`.
48,000 Hz, 16-bit mono, 131,072 samples (about 2.73 seconds).
Original WAV: 262,188 bytes. FLAC: 89,806 bytes.

Decoded samples were compared byte-for-byte with the original PCM. SHA-256 of
the signed 16-bit little-endian PCM (not the file container):
`75de7a934f6fdcddb02e4c77f59d5fe65d0c48d50749db3a5fbd43ad62a47611`.

## Listening cue

`listening_24k.flac` is the Radio's user-approved “blup”, converted from the
prepared `../esphomeRadioFull/sounds/blup.wav` without changing duration or pitch:

```sh
sox ../esphomeRadioFull/sounds/blup.wav -C 8 assets/listening_24k.flac rate -v 24000
```

24,000 Hz, signed 16-bit mono, 23,462 samples (about 0.978 s), 13,116 bytes.
Prepared source WAV SHA-256:
`e9f07313b3b6902d51c5e6305df258ef1c66527393d200276a34e392bd2ef0c2`.
FLAC SHA-256:
`e2a60ff4c5122f99f41a27fabbfe96b5b25d555dab1ee00e12059299932a7322`.
The original MP3 was supplied by the user. Keep the existing Radio sound
provenance/redistribution information when distributing this recording.

Both cues play before microphone capture. The playback reservation includes
the queued start and the final buffer drain; the wake word is held throughout.
