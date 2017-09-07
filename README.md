# Record acoustic indices

Configure [Audiomoth](https://www.openacousticdevices.info/) by following these steps :    

1. Download [Timesetter](https://www.openacousticdevices.info/timesetter-app) and install, open it.   
2. Connect Audiomoth to your computer, and set the switch to "USB/OFF" (the middle one). You should see the red LED blinking for a while, then the green one will light up.
3. If Audiomoth is correctly recognised by your computer, the "Set time" button in Timesetter should be active now. Press it. The device is now configured.  
4. If you want to record the indices right away (e.g. for the sake of testing), switch to "DEFAULT", the device will start recording immediately.
5. Otherwise, switch to "CUSTOM" will wait until either 00:00AM or 12:00PM (UTC) to start recording. While waiting, the green LED will blink.  
  
If the time is not set, switching to "CUSTOM" will result in blinking of both LEDs, which means that you need to set the time.  
Once the time is set, the device will record the acoustic indices (ACI, H[t], CVR, FFT_SUM) computed every minute continuously in the same order. To stop recording, change the switch position back to "USB/OFF".  

The binary file is provided in the folder `audiomoth_indices`.

# Read acoustic indices, and draw false color images

Once you finished recording, you will get some .bin files. Use `readIndices.py` to read the result.  

The python script `readIndices.py` takes one argument, which should be specified as the name of the folder containing the .bin files created by the device. **This folder must be under the same directory** as `readIndices.py`.  

Run `python readIndices.py your-directory` to get the output, which will be created under *your-directory*.  

The output is a .pdf file, containing false color images for indices ACI, H[t] and CVR, and a RGB image using the above three indices in the same order. The name of the file is the timestamp of the time when the recording started. You can use [Timestamp Converter](https://www.epochconverter.com/hex) to convert it to a readable string.

Some example files and their outputs are provided in `testdir`.  
