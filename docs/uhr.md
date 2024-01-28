# UHR Real-Time clock

The UHR card is a realtime clock card for the NDR-Klein Computer based on the E 050 chip. The date and time values can be set and read via a serial interface. The NDKC implementation used to bit-bang the clock and individual bits to set or retrieve the individual values which are BCD coded. The simulation follow this exact behaiviour. Currently it only supports setting or reading all bytes (date and time in total 7 bytes) at a time. As the NDR-Klein Computer  only uses this mode of operations, the modes to extract only individually adressed bytes from the E 050 chip, are not implemented.

## Features

1. The implementation uses the clock of the host system and is automatically initialized to the current date.
2. If the time is set to a different date as the current system date a difference is calculated and applied to the system clock. This ensures the accurarcy based on the system clock of the host computer (this may include NTP based clock synchronization). The timezone will be the same as the host computer and leap seconds are handled like on the host system.
3. Fully valid dates are in the range of 1^st^ of January 1970 and 31^st^ of December 2069. See Year 2000 problem below.

## Limitations

1. **YEAR 2000** problem: As the E 050 chip only uses 2 decimal places for the year we are limited to a range of 100 years (for which the day of week will be correctly calculated). In the simulation we use the mktime() function to make the time. As this function will only return valid dates for times after the UNIX epoch starting in 1^st^ of January 1970 we will interpret dates with a year value less the 70 as being dates in the 2000's. For year bytes lower te 70 we add 100 to create valid dates in the range from 1^st^ of January 2000 to 31^st^ of December 2069. This means that at time of writing, the clock will work correctly work for the current date as well as historical dates (like 27. of October 1984) which may have been possible with the real NDR-Klein Computer.
2. As discribed above, only the continuos mode to read or write all 7 bytes from the E 050 chip is implemented.

## Future Enhancements

1. None currently planed.

## References

1. Description on NDR-NKC.de (https://www.ndr-nkc.de/compo/io/uhr.htm)
2. Microeletronic Marin E 050 Datasheet (https://www.abc80.net/archive/luxor/misc_hw/datasheet/MEM-E050-16-data-sheet.pdf)
