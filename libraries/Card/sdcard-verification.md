Reading SD Card Registers
===============================================================================

This document details how to read card specific information from microSD cards
 using the Android SDK and an old HTC Eris mobile phone connected over USB.

* Author: Brian E Tovar
* Email:  betovar@leaflabs.com
* Date:   14 Oct 2012


Introduction and Objective
===============================================================================

One of the most important parts of testing an algorithm is knowing the expected
 output of the data. Obtaining card specific values (like the `cid` and `csd`) 
 from an independent source is critical to testing a method for retrieving 
 these values. 

The objective of this document is to provide a means to compare the card 
 specific information provided by the Wirish SD-Card library. As mentioned, 
 USB card readers cannot provide access to these values. Readers built into 
 motherboards can provide these values, but not with ease. This procedure 
 outlines the simplest, most through method available to me at the time. Other 
 methods do exist and might be easier for you.


Experimental
===============================================================================

Summary of steps taken to set up phone for reading cards:

1. [Download] [1] the Android SDK manager 
2. [Install] [2] the platform-tools
3. [Setup] [3] USB debugging with a udev rule for device
4. [Run] [4] `adb shell` from <sdk>/platform-tools/
5. [Cat] [5] register values from /sys/block/mmcblk*/device/

[1]: http://developer.android.com/sdk/index.html
[2]: http://developer.android.com/sdk/installing/adding-packages.html
[3]: http://developer.android.com/tools/device.html#setting-up
[4]: http://developer.android.com/tools/help/adb.html
[5]: http://stackoverflow.com/a/7197463

Results and Data
===============================================================================

These results are from five different cards of various sizes and speed classes.
 They are identified by their screen printed labeling (or lack thereof).

Unlabeled 1GB
-------------

* CID: 0x03534453553031478080baf50e009b8c
* CSD: 0x002600325f5983ae7efbcfff924040d6
* SCR: 0x0225000000000000

ATP 2GB
-------

* CID: 0x09415041462055441047b00419007b36
* CSD: 0x002f00325f5a83cf2db7ffbf96800068
* SCR: 0x0125000000000000

SanDisk 2GB
-----------

* CID: 0x03534453553032478000e22ef4009cfc
* CSD: 0x002600325f5a83aefefbcfff928040de
* SCR: 0x0225800000000000

Transcend 4GB
-------------

* CID: 0x035344535530344780158058b000c1cc
* CSD: 0x400e00325b5900001d8a7f800a4040b8
* SCR: 0x0235800100000000

SanDisk 8GB
-----------

* CID: 0x0353445355303847800204968f00c560
* CSD: 0x400e00325b5900003b377f800a4040ae
* SCR: 0x0235800100000000


Discussion
===============================================================================

Upon inspection of these values, the 4GB Transcend card is the only one to not
 provide a matching `cid` register after two successful attempts where the 
 card is ready. It is possible that these readings are using an immature 
 version of the library and suggests another reading in the future to verify 
 this inconsistency.


Conclusion
===============================================================================

For the most part, the algorithm and cmd line communication seem robust when 
 the card is ready. Therefore efforts should be made to make a reliable 
 `begin()` routine for getting the card into a ready state.