Note: TI's grlib, usblib and inc directory are not included in the git package for jel. This is done
to ensure compliance with the license conditions for those libraries.

They are required for building the jel for tiva targets. To build the jel for a tiva target, simply
add the grlib, usblib and inc folder contents as they are included in TI's tivaware package directly
to the existing empty grlib, usblib and inc folders.

As of the time of this writing, the jel is designed to support tivaware 2.1.4.178 only.
