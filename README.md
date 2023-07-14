# ESP32-S3-Mini Personal Assistant

A compact, powerful, and intuitive personal assistant powered by Google's Speech-to-Text and Text-to-Speech APIs, and OpenAI's ChatGPT. Speak to it, and it responds!

## Overview
This project leverages a ESP32-S3-Mini microcontroller equipped with a microphone and speaker, creating a handy device capable of converting spoken questions to text, generating meaningful responses using ChatGPT, converting these responses back to speech, and audibly presenting them. This hands-free approach allows for easy interaction, akin to having your very own smart assistant!

## Hardware Requirements
* ESP32-S3-Mini
* Microphone module (for audio input)
* MAX98357A speaker amp
* Micro SD card (for data storage)
* Tactile Button

## Software Requirements
* Arduino IDE
* ESP32 Arduino Core
* Google Cloud account (for Google's Speech-to-Text and Text-to-Speech APIs)
* OpenAI account (for ChatGPT)

## Getting Started
1. Clone this repository into your local machine.
2. Open Arduino IDE and load the provided .ino file.
3. Ensure all required libraries mentioned in the code are installed.
4. Replace placeholder values for Google Cloud and OpenAI API keys in the code with your own.
5. Compile and upload the sketch to your ESP32-S3-Mini board.

## Operation
* Press the button to activate the device.
* Speak your query into the microphone. Your voice will be recorded and stored on the micro SD card.
* The device will convert your voice recording into text using Google's Speech-to-Text API.
* The transcribed text will be sent to ChatGPT to generate a meaningful response.
* The response from ChatGPT will be converted back into speech using Google's Text-to-Speech API.
* Finally, the synthesized speech will be played back to you through the speaker, answering your query!

## Note
Remember to ensure your device has stable access to the internet, as the APIs used in this project require online communication. Also, be aware of any potential costs associated with the usage of Google Cloud and OpenAI APIs.

## License
This project is licensed under the MIT License - see the LICENSE file for details.

## Contributions
Contributions to this project are welcome! Please create a pull request or raise an issue.

## Troubleshooting
If you experience any issues, please refer to the troubleshooting guide. If your problem persists, feel free to open an issue in this repository.

Enjoy your new ESP32-S3-Mini Personal Assistant!
