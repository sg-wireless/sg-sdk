<!------------------------------------------------------------------------------
 ! @copyright Copyright (c) 2023-2024 SG Wireless - All Rights Reserved
 !
 ! Permission is hereby granted, free of charge, to any person obtaining a copy
 ! of this software and associated documentation files(the “Software”), to deal
 ! in the Software without restriction, including without limitation the rights
 ! to use,  copy,  modify,  merge, publish, distribute, sublicense, and/or sell
 ! copies  of  the  Software,  and  to  permit  persons to whom the Software is
 ! furnished to do so, subject to the following conditions:
 !
 ! The above copyright notice and this permission notice shall be included in
 ! all copies or substantial portions of the Software.
 !
 ! THE SOFTWARE IS PROVIDED “AS IS”,  WITHOUT WARRANTY OF ANY KIND,  EXPRESS OR
 ! IMPLIED,  INCLUDING BUT NOT LIMITED TO  THE  WARRANTIES  OF  MERCHANTABILITY
 ! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 ! AUTHORS  OR  COPYRIGHT  HOLDERS  BE  LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER
 ! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 ! OUT OF OR IN  CONNECTION WITH  THE SOFTWARE OR  THE USE OR OTHER DEALINGS IN
 ! THE SOFTWARE.
 !
 ! @author  Ahmed Sabry (SG Wireless)
 !
 ! @brief   Documentation file for Fuel-Gauge.
 !----------------------------------------------------------------------------->

# Fuel-Gauge ( BQ27421 )

<!------------------------------------------------------------------------------
 ! TOC
 !----------------------------------------------------------------------------->

## Contents

* [Introduction](#intro)
* [Fuel-Gauge Functions](#funcs)
* [Example](#example)

<!------------------------------------------------------------------------------
 ! Introduction
 !----------------------------------------------------------------------------->
<div id="intro"></div>

## Introduction

The Fuel-Gauge feature is supported for the board shield that have the
Fuel-Gauge module on it. If so, the Fuel-Gauge firmware interface component
will be activated and build with the final firmware image.

This interface component utilizes the open-source driver for this Fuel-Gauge
which can be found at this git repository
    [https://github.com/svcguy/lib-BQ27421/tree/master](
        https://github.com/svcguy/lib-BQ27421/tree/master)

<!------------------------------------------------------------------------------
 ! Fuel-Gauge Functions
 !----------------------------------------------------------------------------->
<div id="funcs"></div>

## Fuel-Gauge Functions

* `initialization`: the module will be initialized once it is imported.
  after its initialization, it could be deinitialized by calling
  `fuel_gauge.deinit()` and to initialize it again use `fuel_gauge.initialize()`

* `fuel_gauge.info()` it returns a tuple with all read information from the
  driver.

* `fuel_gauge.print()` it print all Fuel-Gauge info.

<!------------------------------------------------------------------------------
 ! Introduction
 !----------------------------------------------------------------------------->
<div id="example"></div>

## Example

```python
>>> import fuel_gauge

# deinit
>>> fuel_gauge.deinit()

# init again
>>> fuel_gauge.initialize()

# printing the info
>>> fuel_gauge.print()
Voltage               4200 mV
Current               62 mA
Temprature            30.10 degC
Charge State          100 %
Health State          91 %
Design Capacity       1200 mAh
Remaining Capacity    497 mAh
Full Charge Capacity  497 mAh
is critical           no 
is low                no 
is full               no 
is charging           yes
is discharging        no

# getting the info in a tuple for scripting:
>>> fuel_gauge.info()
    # the following tuple will return
    ( voltage_mV=4131,
    current_mA=0, 
    temp_degC=30.65, 
    charge_percent=100,
    health_percent=91, 
    designCapacity_mAh=1200, 
    remainingCapacity_mAh=497,
    fullChargeCapacity_mAh=497, 
    isCritical=False, 
    isLow=False, 
    isFull=False,
    isCharging=False,
    isDischarging=True)
```

<!--- end of file ------------------------------------------------------------->
