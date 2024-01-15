<p align="center"><img src="other/QIDI.png" height="240" alt="QIDI's logo" /></p>
<p align="center"><a href="/LICENSE"><img alt="GPL-V3.0 License" src="other/qidi.svg"></a></p>

# Document Instructions

QIDI_PLUS3 is a server-side software that interacts with the screen in the Plus3 model system. This document contains our source code, and we provide the safer solution to update it: Download the packaged file to a USB drive and insert it into the machine for updating.</br>

QIDI provides a packaged version file in the version bar next to it. Please download the compressed package file starting with PLUS.  
We have provided multiple different versions of source code. Please select the branch you want to download, and the name of the branch is the corresponding version name.

## 4.2.13 Update content

<strong><ol>

<li>SOC Update: WiFi settings will be preserved after updates.</li>
<li>Material Handling: Improved logic for material feeding and retraction.</li>
<li>Heating Limit: Maximum chamber heating restricted to 65 degrees Celsius.</li>
<li>Factory Reset: Also clears web-based print records and accumulated time.</li>
<li>SSH Login: Added login info .</li>
</ol></strong>

## Detailed update process

#### Packaged files

Note that all updates can not be updated from higher versions

1. Select the latest version in the version release bar next to it, download the compressed file package starting with PLUS and extract it locally.<a href="https://github.com/QIDITECH/QIDI_PLUS3/releases">Jump link</a>

2. Place the files in the USB drive, such as

<p align="left"><img src="other/sample.png" height="240" alt="sample"></p>

3. Insert the USB drive into the machine's USB interface, and an update prompt will appear on the version information interface. Click the update button to restart according to the prompt.

## Report Issues and Make Suggestions

**_You can contact [After-Sales Service](https://qidi3d.com/pages/warranty-policy-after-sales-support) to report issues and make suggestions._**  
**_You can directly contact our after-sales team for any issues related to machine mechanics, slicing software, firmware, and various machine issues. They will reply to your questions within twelve hours._**

## Others

Different from the usual method of directly accessing the fluid page through an IP address, the QIDI version sets the default port number to 10088, so you need to add `:10088` after the machine's IP to access the fluid page</br>
The 3D printers of QIDI are based on Klipper.On the basic of Klipper open source project, we have made some modifications to it's source code to meet some of the user's needs.At the same time, we have also made modifications to Moonraker, so that the screens we set can correspond to the operations on the page.
Thanks to the developers and maintainers of these open source projects.Please consider using or supporting these powerful projects.

| Software      | QIDI edition                                                                     |
| ------------- | -------------------------------------------------------------------------------- |
| **Klipper**   | **[https://github.com/QIDITECH/klipper](https://github.com/QIDITECH/klipper)**   |
| **Moonraker** | **[https://github.com/QIDITECH/moonrake](https://github.com/QIDITECH/moonrake)** |
