Card Library
===============================================================================

A Wirish class for reading and writing to SD cards

* Author: Brian Tovar
* Email: betovar@leaflabs.com
* Modified: 15 Sep 2012


Overview
===============================================================================

The goal of this library is to provide a feature set of commands for SD cards. 
The project's first priority will be development of the SDIO peripheral on ARM
Cortex-M3 and M4 devices. Later, a FAT layer be developed on top of this SD 
class and possibly even a SPI based class with mirrored functionality.


How It Works
===============================================================================

Most of this work is based on this documentation:
* [Reference Manual for STM32F103ZE] [1]
* [SD Card Physical Layer Specification] [2]

The SDIO peripheral, like any other in `libmaple`, is first built around a C
 application programming interface (API). A Wirish/C++ API is then built on 
 top of that to handle many of the common requests that users require of SD 
 card communication. For instance, sending a command, parsing a response, and 
 piping data. Within this class are three levels of abtraction. First, the 
 basic rountines afore-mentioned, second, the intermediate methods that 
 perform certian specific tasks, and the highest level, the public methods 
 that the user or FAT interface will call.

Examples of such intermediate methods include:

* `getICR()`
* `getOCR()`
* `getCID()`
* `getCSD()`
* `newRCA()`
* `initialize()`
* `identify()`

Examples of such highest level methods include:

* `begin()`
* `end()`
* `read()`
* `write()`

The goal is to keep this public method list short and simple, while focusing 
on developing a robust underlying codebase. In the future, new rountines will 
be developed to provide access to advanced features, like password protection, 
quick erase, and SDIO features.


Status
------

Thus far, the status of this library has beento idenitfy a card and read it's 
card specific registers. This is known as the card initialization and 
identification process: it has been completed and in the testing phase. 
The `begin()` method provides this process. The `reset()` method is having 
trouble and therefore the `end()` method does not fully reboot the card.

To date, the only working parts are `initialization()` and `identification()` 
methods. This includes all the functions called from within these, like 
sending cmd8 and acmd41 for checking the interface conditions. Although these 
are working, they are not reliable. Sometimes the cards don't initialze 
properly and the causes are yet unknown. Trying to initialize the card again 
requires a manual reset of the entire board.


Testing (wip)
===============================================================================

Here is an example of a command on OSX that reads the one-thousandth block of 
an SD card mounted as disk4, and writes the result out to a file:

`dd if=/dev/diskN of=test.bin bs=512 count=1 iseek=1000`

CID Number
----------

The CID register, which includes data like the serial number and manufacture 
date, can be read when the card initializes. Verifying this information is not 
yet possible without a reliable independent source. It is only possible to 
read this register and the CSD register, which holds other useful card 
specific data, via an SD reader of the non-USB type, ie. one that is connected 
directly to a processor or a motherboard. The most common method currently is 
through the use of android cellphones, but other code exists for laptops and 
desktops with built-in SD card slots. So at this point, much of this 
"functionality" is circumstantial; meaning that the values make some sense 
(the manufacture date is current and in the past), but cannot be considered 
fully tested.


Reading and Writing Blocks (work in progress)
--------------------------

First up is reading any block on the card, later writing will be tested. This 
is being done to protect the cards from immature code in the off chance that 
something is irrevocably damaged. 

Data is transferred on one or four data lines. I will focus on reading with 
only one data line to start, check that the fifo is working with interrupts, 
and then expand. The ultimate goal is to have data transferred over DMA, but 
but a polling scheme might be implemented for a slow speed demo.



Conflicts (to be conmpleted)
===============================================================================

* peripheral gpio pins in use
* DMA2 Channel4 (specific to F1 line)
* card detection pin (interrupt based)


Proposals
===============================================================================

SDIO device
-----------

It has recently come to my attention that the C functions specific to the sdio 
peripheral, while modeled off the spi.h device, do not require a pointer to 
the SDIO device. This is because there is never a need for multiple sdio 
peripherals in a microcontroller. I am proposing that each function remove 
the argument in their call and simply reference the SDIO device globally.

mapleFAT
--------

High-density performance line devices have one SDIO peripheral and up to three
SPI peripherals. This document seeks to explain the design specifications for 
a FAT filesystem on such devices. It will focus (for the moment) on a 
high-level description of the classes involved and their inheritance schemes.

Inheritance diagram:

>         +-------------+                  +-------------+
>         | SDMode-FAT  |                  | SPIMode-FAT |
>         +-------------+                  +-------------+ high-level
>                |       \                /       |
>                |        \              /        |
>                |         \            /         |
>                |          +----------+          |
>                |          | mapleFAT |          |
>                |          +----------+ abstract |
>                |                                |
>                |                                |
>         +--------------+                 +-------------+
>         | HardwareSDIO |                 | HardwareSPI |
>         +--------------+                 +-------------+ low-level

This diagram depicts the five proposed classes. SDMode-FAT will inherit from
an SDIO specific hardware class _and_ the abstract mapleFAT class. The 
SPIMode-FAT class will do the same for it's hardware specific functions and 
both XXXMode-FAT classes will have the same public calls so users can simply 
change the object instantiation line in their code to switch modes (at compile 
time).

As mentioned, mapleFAT should have definitions for all it's user visible 
functions, but it's low-level IO calls (like read and write) should be purely 
virtual. This should ensure that HardwareXXX classes define these basic 
methods: eg. virtual mapleFAT::write(where, what) = 0.

At this point you might be wondering how to separate SD card commands from the 
FAT library. Well for one, HardwareSDIO should handle that by itself. The 
peripheral is designed to handle command and response formatting and parsing.
The SPI peripheral, on the other hand, does not. HardwareSPI is a simple 
wrapper for common settings and basic byte reads so the SPIMode-FAT will have 
to provide higher-level functionality for these calls. The ultimate goal of 
this class being block read and write functions.

This brings us to a design decision. Some commands available in SPI mode are 
not available in SD mode and vice versa. Furthermore, responses are not 
formatted the same way and some response bits are left off entirely. I'm 
proposing that the higher-level classes handle these cases.


References
===============================================================================

[1]: http://www.st.com/internet/com/TECHNICAL_RESOURCES/TECHNICAL_LITERATURE/REFERENCE_MANUAL/CD00171190.pdf

[2]: https://www.sdcard.org/downloads/pls/simplified_specs/Part_1_Physical_Layer_Simplified_Specification_Ver_3.01_Final_100518.pdf