# ESP32-S3 Digital Loudspeaker Management System

Proyek firmware ini berfungsi sebagai platform pengolahan audio digital untuk modul loudspeaker management berbasis ESP32-S3. Fokus utama adalah mengolah sinyal audio masuk menjadi output yang sudah difilter dan dikendalikan sesuai kebutuhan loudspeaker 3-way.

Fitur yang tersedia:
- Pengatur gain input/output
- Equalizer 10-band peaking filter
- Crossover 3-way (low / mid / high)
- Limiter untuk menjaga level output aman
- Pembalik fasa per kanal
- Struktur modul yang mudah dikembangkan untuk I2S / DAC / amplifier

## Diagram pipeline

Input stereo -> gain -> 10-band EQ -> crossover 3-way -> limiter -> phase inversion -> output gain

## Komponen yang didukung
- ESP32-S3 devkit / board dengan DAC/I2S
- Input audio analog via ADC atau codec digital
- Output audio ke DAC I2S atau amplifier class-D
- Pengaturan konfigurasi melalui Serial Monitor / Web UI / MQTT (dapat dikembangkan lanjut)

## Struktur folder
- `src/main.cpp` : setup dan demo runtime
- `src/audio_dsp.cpp` : implementasi DSP
- `include/audio_dsp.h` : deklarasi kelas DSP
- `include/config.h` : konfigurasi frekuensi, crossover, dan default parameter
- `platformio.ini` : konfigurasi build PlatformIO

## Quick start

1. Instal PlatformIO VS Code / CLI
2. Clone repositori ini
3. Buka folder proyek di VS Code
4. Build project:

```bash
pio run
```

5. Upload ke board ESP32-S3:

```bash
pio run -t upload
```

6. Buka Serial Monitor:

```bash
pio device monitor
```

## Konfigurasi default

- Sample rate: 48 kHz
- EQ: 10 band peaking filter
- Crossover low: 150 Hz
- Crossover mid low: 500 Hz
- Crossover mid high: 2500 Hz
- Limiter threshold: -1 dBFS
- Gain input/output: +0 dB

## Catatan pengembangan lanjutan

Fitur berikut dapat ditambahkan di tahap berikutnya:
- Web UI untuk mengatur parameter secara real-time
- Preset loudspeaker tuning (studio / live / club)
- Remote control via BLE / WiFi
- Logging level output ke LCD atau OLED
- Integrasi dengan DSP di luar CPU utama ESP32-S3

## Hardware contoh

- ESP32-S3 DevKitC-1
- PCM5102 / ES8388 / CODEC I2S
- DAC stereo 24-bit
- Power amplifier 2 channel + speaker 3-way
- Kabel input/output sesuai pin I2S dan ADC

## Lisensi

Proyek ini dibuat sebagai template dan dasar pengembangan loudspeaker management system. Anda dapat memodifikasi sesuai kebutuhan proyek Anda.
