# RASPIAUDIO museLuxe

[Français](#français) · [English](#english)

## Français

### Musique et assistant vocal sur la même Luxe

Ce firmware réunit **Music Assistant**, via Sendspin, et **Home Assistant Assist**
sur la RASPIAUDIO museLuxe à ESP32. Il fonctionne avec une mémoire flash de 4 Mo,
y compris sur les anciennes Luxe sans gestionnaire de batterie IP5306-I²C.

Pendant l'écoute musicale, un appui simultané sur **Volume + et Volume −**
met le groupe Music Assistant en pause et lance Assist. Après la réponse,
la musique reprend automatiquement à son niveau sonore précédent.
Au repos, Assist peut aussi être appelé avec **« Okay Nabu »**.

**Le wake word n'est pas actif pendant la musique dans cette version.**
Elle n'utilise ni AEC ni reconnaissance de « Stop » pendant la lecture.
Les commandes des boutons ne nécessitent aucune automation personnalisée dans HA.

### Première installation

1. Installer le firmware par USB. Le nouveau partitionnement impose une
   installation complète lors du premier passage depuis un ancien firmware ;
   une simple mise à jour OTA ne remplace pas la table des partitions.
   `luxe.bin` est l'image complète de 4 Mo, sans réglages Wi-Fi, à écrire à
   l'adresse `0x0`. `update_firmware.bin` est réservé aux mises à jour OTA.
2. Configurer un réseau Wi-Fi 2,4 GHz par **Improv Serial** en USB, ou rejoindre
   le point d'accès **Luxe-Setup**, mot de passe **12345678**, puis utiliser
   son portail de configuration.
3. Ajouter l'appareil **Raspiaudio Luxe** découvert dans Home Assistant et
   lui affecter un pipeline Assist avec reconnaissance et synthèse vocales.
   Dans Music Assistant, sélectionner le lecteur Sendspin correspondant à la Luxe.

Home Assistant et Music Assistant doivent être accessibles sur le réseau local.
Les services vocaux dépendent du pipeline choisi dans HA : le firmware ne les
remplace pas.

### Utilisation

| Commande | Action |
| --- | --- |
| « Okay Nabu », sans musique | Lancer Assist. |
| Volume + et Volume − ensemble | Lancer Assist, même pendant la musique. |
| Les deux boutons ensemble, ou Play/Pause, pendant Assist | Annuler et revenir au mode précédent. |
| Play/Pause hors Assist | Lecture / pause de Music Assistant. |
| Volume + ou Volume − seul | Régler le volume du groupe MA pendant la musique, sinon celui des réponses vocales. Réglage appliqué au relâchement. |

Après le petit son d'écoute, attendre la **LED cyan** avant de poser la question.
Si aucune parole ne commence dans les **8 secondes** d'écoute, retour automatique
au mode précédent. Ce délai ne limite pas une question déjà commencée.

### Réglages et indications

Dans HA, **Use wake word**, **Microphone Mute**, **Startup sound** et
**Listening sound** permettent de personnaliser le fonctionnement. Le bouton
**Assist** lance aussi une interaction. **Microphone Mute** bloque les interactions
vocales, y compris par les boutons, mais ne coupe pas la musique.

Les volumes musique et **Voice Answer** sont indépendants. Les sons de démarrage
et d'écoute suivent le volume et la sourdine de Voice Answer.

La LED indique : **bleu** = wake word prêt, **vert** = musique, **cyan** = écoute,
**magenta** = traitement, **jaune** = réponse, **orange** = transition,
**rouge** = HA indisponible. L'état est également lisible dans **Operating mode**.

La batterie dispose d'une estimation de charge et d'un seuil d'alerte réglable
**Low battery threshold** : la LED clignote en cas de batterie faible, quelle que
soit sa couleur. Sur les modèles équipés, l'IP5306-I²C est détecté automatiquement
et ajoute des informations de charge ainsi que le réglage **Allow charging**.

Les mises à jour suivantes peuvent être effectuées par **OTA ESPHome**, avec un
firmware compatible. La recherche automatique de nouvelles versions n'est pas
encore intégrée.

## English

### Music and voice assistant on one Luxe

This firmware combines **Music Assistant**, through Sendspin, and **Home Assistant
Assist** on the ESP32-based RASPIAUDIO museLuxe. It supports **4 MB flash**, including
older Luxe models without an IP5306-I²C battery controller.

While music is playing, press **Volume + and Volume − together** to pause the
Music Assistant group and start Assist. Music resumes automatically after the
answer, at its previous volume. When music is not playing, you can also say
**“Okay Nabu”**.

**Wake-word detection is disabled during music in this version.** It does not use
AEC or listen for “Stop” during playback. No custom HA automations are required
for the button controls.

### First-time setup

1. Install the firmware over USB. When migrating from an older firmware, the
   new partition layout requires a full installation: an ordinary OTA update
   does not replace the partition table.
   `luxe.bin` is the complete 4 MB image, without Wi-Fi settings, to be written
   at address `0x0`. `update_firmware.bin` is for OTA updates only.
2. Configure a 2.4 GHz Wi-Fi network using **Improv Serial** over USB, or connect
   to **Luxe-Setup**, password **12345678**, and use its configuration portal.
3. Add the discovered **Raspiaudio Luxe** device in Home Assistant and assign
   an Assist pipeline with speech recognition and text-to-speech. In Music
   Assistant, select the Luxe's corresponding Sendspin player.

Home Assistant and Music Assistant must be reachable on the local network.
Voice services are provided by the selected HA pipeline, not by the firmware.

### Everyday controls

| Control | Action |
| --- | --- |
| “Okay Nabu”, without music | Start Assist. |
| Volume + and Volume − together | Start Assist, including during music. |
| Both volume buttons together, or Play/Pause, during Assist | Cancel and return to the previous mode. |
| Play/Pause outside Assist | Toggle Music Assistant playback. |
| Either volume button alone | Adjust MA group volume during music; otherwise adjust voice-answer volume. Applied on release. |

Wait for the listening sound to finish and the **cyan LED** before speaking.
If no speech starts within **8 seconds** of listening, the previous mode resumes
automatically. This timeout does not limit a question already in progress.

### Settings and status

HA exposes **Use wake word**, **Microphone Mute**, **Startup sound**,
**Listening sound**, and an **Assist** button. Microphone Mute prevents voice
interactions, including button-triggered Assist, without muting music.
Music and **Voice Answer** have separate volumes; startup and listening sounds
follow Voice Answer volume and mute.

LED colours: **blue** = wake word ready, **green** = music, **cyan** = listening,
**magenta** = processing, **yellow** = answering, **orange** = transition,
**red** = HA unavailable. **Operating mode** also reports the current state.

Battery monitoring includes an estimated charge level and an adjustable
**Low battery threshold**, with LED blinking regardless of its current colour.
An optional IP5306-I²C is detected automatically, providing charging information
and the **Allow charging** control on supported models.

Subsequent compatible firmware updates can use **ESPHome OTA**. Automatic
release checking is not yet included.

## Compilation / Building from source

ESPHome **2026.10.1** est la version testée / is the tested version.
Conserver ensemble / Keep together:

```text
luxe.yaml
luxe_policy.h
luxe_partitions.csv
components/
assets/
```

```sh
esphome run luxe.yaml --device /dev/ttyUSB0
```

Adapter le port USB si nécessaire / Change the USB port if needed.
Le nom réseau est `muse-luxe` suivi du suffixe MAC ; après le changement de nom
du firmware, vérifier la découverte de l'appareil dans HA et MA.
The network name is `muse-luxe` with a MAC suffix; after the firmware name
change, check device discovery in HA and MA.

## Publication après validation / Publishing after validation

1. Compiler `luxe.yaml`.
2. Copier `.esphome/build/muse-luxe/build/firmware.ota.bin` vers
   `update_firmware.bin`.
3. Calculer `md5sum update_firmware.bin`.
4. Reporter la version et le MD5 dans `manifest_update.json`.
5. Régénérer également `luxe.bin` avec le nouveau firmware et la table des
   partitions, en conservant une taille de 4 Mo et des réglages vierges.

Keep the YAML version, OTA binary, complete installation image and manifest
in sync. The manifest targets `ESP32` and the `RASPIAUDIO/esphomeLuxe` repository.
It is provided for publication; the current YAML enables native ESPHome OTA
but does not configure automatic HTTP firmware updates.
