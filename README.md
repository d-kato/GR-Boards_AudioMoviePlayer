# GR-Boards_AudioMoviePlayer
This is a sample program that works on GR-PEACH or GR-LYCHEE.  

## Overview
This sample playback audio or video files in the storage root folder. When USER_BUTTON0 is pressed, the next file is playbacked.  

For the mov file, the following information was consulted.  
http://yunit.techblog.jp/archives/69555254.html


## Requirements

### GR-PEACH
* [GR-PEACH](https://os.mbed.com/platforms/Renesas-GR-PEACH/)
* [GR-PEACH AUDIO CAMERA Shield](https://os.mbed.com/teams/Renesas/wiki/Audio_Camera-shield)
* [GR-PEACH 4.3 inch LCD Shield](https://os.mbed.com/teams/Renesas/wiki/LCD-shield#gr-peach-4-3-inch-lcd-shield)
* Audio speaker
* USB memory or microSD card

### GR-LYCHEE
* [GR-LYCHEE](https://os.mbed.com/platforms/Renesas-GR-LYCHEE/)
* 4.3 inch LCD (40pin)
* Audio speaker
* USB memory or SD card


### LCD selection
To select the LCD, add `` lcd-type`` to `` mbed_app.json``.  
```json
{
    "config": {
        "camera":{
            "help": "0:disable 1:enable",
            "value": "0"
        },
        "lcd":{
            "help": "0:disable 1:enable",
            "value": "1"
        },
        "lcd-type":{
            "help": "Please see mbed-gr-libs/README.md",
            "value": "GR_PEACH_4_3INCH_SHIELD"
        },
        "usb-host-ch":{
            "help": "(for GR-PEACH) 0:ch0 1:ch1",
            "value": "1"
        },
        "audio-camera-shield":{
            "help": "(for GR-PEACH) 0:not use 1:use",
            "value": "1"
        }
    }
}
```

| lcd-type "value"        | Description                        |
|:------------------------|:-----------------------------------|
| GR_PEACH_4_3INCH_SHIELD | GR-PEACH 4.3 inch LCD Shield       |
| GR_PEACH_7_1INCH_SHIELD | GR-PEACH 7.1 inch LCD Shield       |
| GR_PEACH_RSK_TFT        | GR-PEACH LCD for RSK board         |
| GR_PEACH_DISPLAY_SHIELD | GR-PEACH Display Shield            |
| GR_LYCHEE_LCD           | GR-LYHCEE TF043HV001A0 etc.(40pin) |

When LCD is not selected, the following is set.  
* GR-PEACH : GR_PEACH_4_3INCH_SHIELD  
* GR-LYCHEE : GR_LYCHEE_LCD  

## Supported format

### Audio

|                         |                                               |
|:------------------------|:----------------------------------------------|
| Format                  | Wav file (RIFF format) ".wav"                 |
| Channel                 | 2ch                                           |
| Frequencies             | 32kHz, 44.1kHz, 48kHz, 88.2kHz and 96kHz      |
| Quantization bit rate	  | 16bits, 24bits and 32bits                     |


### Movie

|                         |                                               |
|:------------------------|:----------------------------------------------|
| Format                  | QuickTime ".mov"                              |
| Video codec             | Motion Jpeg 480x270 less than 30fps           |
| Audio codec             | Linear PCM 16bit little-endian 44100Hz Stereo |

When using [ffmpeg](https://ffmpeg.org/download.html). You can convert with the following command.  The frame rate can be changed with ``fps = XX``.  
```
$ ffmpeg -i <INPUT_FILE> -ar 44100 -ac 2 -acodec pcm_s16le -s 480x270 -vcodec mjpeg -q:v 3 -movflags faststart -threads 4 -vf fps=30 <OUTPUT_FILE>.mov
```
