## Simple TinyUSB project for USB Composite device with CDC-ACM and MSC

This project is based on the [TinyUSB](https://github.com/hathach/tinyusb.git) GitHub project.

Make sure that you have setup and run the example projects from the [VSCode Getting Started Template](https://github.com/alifsemi/alif_vscode-template) before working on this project.

By default this project is configured to build and run from TCM for M55-HE.

# CDC-ACM
Connect the PC to the SOC USB micro-USB port J2. When the application runs a TinyUSB COM part will be assigned on the PC. Check with COM Port and you can access the Port with any terminal client like Tera Term or Putty. Setup your port with 115200 baud, 8bit data, no parity and 1 Stop bit.

# MSC
Tiny USB MSC will show up as drive on the host PC.

