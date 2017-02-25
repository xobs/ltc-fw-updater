Love-to-Code Firmware Updater
=============================

A specialized LtC program for updating LtC devices.


Background
----------

The Love-to-Code boards allow for about 10 kilobytes of data to be programmed,
because they house about 22 kilobytes of operating system data, including
the bootloader.  Ordinarily, the operating system is programmed once, and
never touched again.

Ordinarily.

Sometimes things go wrong, and you need to be able to update in the field.

This loadable program is entirely self-contained.  It unhooks the running
operating system, overwrites the boot vectors to point to itself, and sets
about loading its own program in.  Once the operating system has been
replaced, it resets the boot vectors, erases itself, and reboots into the new
operating system.


Entrypoints
===========

There are two entrypoints into the system.

1. The first entrypoint is the Esplanade_Main entrypoint.  This is the
location that we reach when control is handed off by the LtC bootloader.  This
entrypoint disables interrupts, performs some rudimentary checks, and then
overwrites Page 0 to point to itself.  That way, if the board is rebooted, the
update will continue.

It also updates Page 1 with the ordinary Flash Control Block that is
required to allow a Kinetis to boot.

Because the LtC Operating System is handing control to the bootloader, clocks
and peripherals are already set up.

2. The second entrypoint is entered via system reset.  In this mode,
everything is gated and the chip is running off of the internal oscillator.
We first need to calibrate the clock and ungate everything we're interested
in.  We also need to set up pin muxes and copy .data to RAM.

Then we can enter the normal updater loop.

After the program has been flashed, we verify the MD5 sum, and then jump to
code in RAM that erases the updater.  That way the new OS won't try to boot
the updater again.

Finally, we hit a breakpoint.  If a debugger is attached, we'll enter it
here, and if not the chip will reset into the new OS.
