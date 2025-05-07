# Raspiaudio Muse Luxe Voice Satellite

[![Video Guide](https://img.youtube.com/vi/iLQuCf8FqXM/0.jpg)](https://www.youtube.com/watch?v=iLQuCf8FqXM)

Welcome to the Raspiaudio Muse Luxe Voice Satellite project! This guide will help you get started with your device, including setup instructions, source code information, and support options.

## Introducing the New Version: luxe_microWW (with esphome 2025.4.0)

Discover the enhancements in the latest release!

### Features:

- **Micro Wake Words**: Now supports multiple wake words like "okay_nabu".
- **Full interface with Home Assistant Assist**
- **MP3/AAC player**: Media Browser, My media,...

### Flashing Your Device

1. Open **Google Chrome**.
2. Navigate to [apps.raspiaudio.com](https://apps.raspiaudio.com).
3. Choose **Muse Luxe - Home Assistant** and follow the on-screen instructions for flashing and initialize your WiFi credentials

  **an alternative method** for Wifi init is available
  1. Connect to the device's access point with these credentials:
   - **SSID**: Raspiaudio-Luxe
   - **Password**: 12345678
   2. Access `192.168.4.1` in your browser to configure your home Wi-Fi settings.
### Interface
**Led**
1. Blue => waiting Wake Word
2. Green => Listening
3. Yellow => Answering
4. Orange => External player
   
**Buttons**
1. Plus => volume up
2. Minus => volume down
3. Center
- click => mute/unmute
- double click => stop external player
     
   
### Music Assistant
If you want to use it in good conditions you will have to change one parameter in Music Assistant.
1. Select your player's parameters (Raspiaudio Muse Luxe)
2. Select Open Settings => Avanced settings => Output codec to use for streaming audio to the player
3. There choose .wav
4. Save


### Source Code

Explore and contribute to the project on GitHub: [esphomeLuxe Repository](https://github.com/RASPIAUDIO/esphomeLuxe). For recompilation, ensure to perform a full build clean in ESPHome first.

### Forum & Support

Join our community for discussions and support: [Raspiaudio Forum Thread](https://forum.raspiaudio.com/t/muse-luxe-voice-assistant-now-possible/726/209)

### Step-by-Step Video Guide

For a comprehensive walkthrough, watch our [Step-by-Step Video Guide](https://youtu.be/QDDjXAWuk0E).

We hope you enjoy using your Raspiaudio Muse Luxe Voice Satellite! For any further assistance, feel free to reach out via the forum.
