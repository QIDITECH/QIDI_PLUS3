<p align="center"><img src="other/QIDI.png" height="240" alt="QIDI's logo" /></p>
<p align="center"><a href="/LICENSE"><img alt="GPL-V3.0 License" src="other/qidi.svg"></a></p>

# Document Instructions
QIDISmart3 is a server-side software that interacts with the screen in the smart3 model system. This document contains our source code, and we provide two ways to update it: one is to download the packaged file to a USB drive and insert it into the machine for updating, and the other is to compile and update it through source code.  
***Please note that manual updates may affect normal after-sales service, so it is best to automatically update through the machine with packaged files.***  
QIDI provides a packaged version file in the version bar next to it. Please download the compressed package file starting with SMART.

## Detailed update process
#### Packaged files
1. Prepare a blank named USB drive.Please ensure that the device name of the USB drive is empty
<p align="left"><img src="other/blankname.png" height="360" alt="sample"></p>
2. Select the latest version in the version release bar next to it, download the compressed file package starting with SMART and extract it locally.
3. Place the files in the USB drive, such as
<p align="left"><img src="other/sample.png" height="240" alt="sample"></p>

4. Insert the USB drive into the machine's USB interface, and an update prompt will appear on the version information interface. Click the update button to restart according to the prompt.


#### Compile
1. Connect machines to the network and connect through SSH
2. Log in as root.The password is `makerbase`
3. After logging in, enter the following code block
```shell
cd /root
rm -rf xindi
git clone https://github.com/QIDITECH/QIDI_SMART3.git
```
4. Code cloning may take some time, please be patient. If there are certificate issues, please confirm if the system time is correct. After the code cloning is completed, enter the following code block
```shell
mv QIDI_SMART3 xindi
cd /root/xindi/build
cmake ..
make
```
5. The complete code compilation also takes some time, patiently wait for the compilation to complete, shut down and wait for 20 seconds before starting.

## Report Issues and Make Suggestions

You can contact [After-Sales Service](https://qidi3d.com/pages/warranty-policy-after-sales-support) to report issues and make suggestions.

## Others

The 3D printers of QIDI are based on Klipper.On the basic of Klipper open source project, we have made some modifications to it's source code to meet some of the user's needs.At the same time, we have also made modifications to Moonraker, so that the screens we set can correspond to the operations on the page.
Thanks to the developers and maintainers of these open source projects.Please consider using or supporting these powerful projects.

 Software |  QIDI edition
 ----|----
**Klipper** | **[link](https://github.com/QIDITECH/klipper)**
**Moonraker** | **[link](https://github.com/QIDITECH/moonrake)**









  
