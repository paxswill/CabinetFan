EESchema Schematic File Version 4
EELAYER 30 0
EELAYER END
$Descr User 5039 4000
encoding utf-8
Sheet 1 1
Title "Cabinet Fan"
Date "2020-09-09"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L CabinetFanSymbols:Cabinet_Barrel_Jack J1
U 1 1 5F5A46D0
P 3800 600
F 0 "J1" H 3570 650 50  0000 R CNN
F 1 "Barrel_Jack" H 3570 559 50  0000 R CNN
F 2 "" H 3850 560 50  0001 C CNN
F 3 "~" H 3850 560 50  0001 C CNN
	1    3800 600 
	-1   0    0    -1  
$EndComp
Wire Wire Line
	1900 1300 2100 1300
Wire Wire Line
	2100 1200 1900 1200
$Comp
L power:GND #PWR05
U 1 1 5F5AD617
P 2100 1300
F 0 "#PWR05" H 2100 1050 50  0001 C CNN
F 1 "GND" H 2250 1250 50  0000 C CNN
F 2 "" H 2100 1300 50  0001 C CNN
F 3 "" H 2100 1300 50  0001 C CNN
	1    2100 1300
	1    0    0    -1  
$EndComp
$Comp
L CabinetFanSymbols:ItsyBitsy5v U2
U 1 1 5F5A2F39
P 800 1150
F 0 "U2" H 925 1365 50  0000 C CNN
F 1 "ItsyBitsy5v" H 925 1274 50  0000 C CNN
F 2 "" H 800 650 50  0001 C CNN
F 3 "" H 800 650 50  0001 C CNN
	1    800  1150
	1    0    0    -1  
$EndComp
$Comp
L power:+5V #PWR03
U 1 1 5F5E54D9
P 250 1300
F 0 "#PWR03" H 250 1150 50  0001 C CNN
F 1 "+5V" H 100 1350 50  0000 C CNN
F 2 "" H 250 1300 50  0001 C CNN
F 3 "" H 250 1300 50  0001 C CNN
	1    250  1300
	1    0    0    -1  
$EndComp
Wire Wire Line
	400  1300 250  1300
Text Label 2100 1900 0    50   ~ 0
FAN_CONTROL
NoConn ~ 400  1200
NoConn ~ 400  1400
NoConn ~ 400  1500
NoConn ~ 400  1600
NoConn ~ 400  1700
NoConn ~ 400  1800
NoConn ~ 400  1900
NoConn ~ 400  2000
NoConn ~ 400  2100
NoConn ~ 400  2200
NoConn ~ 400  2300
NoConn ~ 400  2400
NoConn ~ 400  2500
NoConn ~ 4300 1700
$Comp
L power:GND #PWR02
U 1 1 5F5AE8FC
P 3800 2200
F 0 "#PWR02" H 3800 1950 50  0001 C CNN
F 1 "GND" H 3950 2200 50  0000 C CNN
F 2 "" H 3800 2200 50  0001 C CNN
F 3 "" H 3800 2200 50  0001 C CNN
	1    3800 2200
	1    0    0    -1  
$EndComp
$Comp
L Sensor_Temperature:TMP36xS U1
U 1 1 5F5A67DF
P 3800 1700
F 0 "U1" H 3256 1746 50  0000 R CNN
F 1 "TMP36" H 3256 1655 50  0000 R CNN
F 2 "Package_SO:SOIC-8_3.9x4.9mm_P1.27mm" H 3256 1609 50  0001 R CNN
F 3 "https://www.analog.com/media/en/technical-documentation/data-sheets/TMP35_36_37.pdf" H 3800 1700 50  0001 C CNN
	1    3800 1700
	-1   0    0    -1  
$EndComp
Connection ~ 3400 3350
Wire Wire Line
	4250 3550 4350 3550
Wire Wire Line
	3400 3350 3550 3350
Text Label 4250 3550 2    50   ~ 0
FAN_CONTROL
Connection ~ 4200 3350
Wire Wire Line
	4200 3350 4350 3350
Wire Wire Line
	4200 3050 4200 3000
Wire Wire Line
	4200 3250 4200 3350
Wire Wire Line
	4050 3350 4200 3350
Wire Wire Line
	3400 3650 3400 3700
$Comp
L power:GND #PWR08
U 1 1 5F614EE5
P 3400 3700
F 0 "#PWR08" H 3400 3450 50  0001 C CNN
F 1 "GND" H 3405 3527 50  0000 C CNN
F 2 "" H 3400 3700 50  0001 C CNN
F 3 "" H 3400 3700 50  0001 C CNN
	1    3400 3700
	1    0    0    -1  
$EndComp
Wire Wire Line
	3400 3450 3400 3350
Wire Wire Line
	3300 3350 3400 3350
Text Label 3200 1700 2    50   ~ 0
TEMP_INPUT
Text Label 3300 3350 2    50   ~ 0
TACH_INPUT
Wire Wire Line
	3850 3350 3750 3350
$Comp
L Motor:Fan_4pin M1
U 1 1 5F5A5B55
P 4650 3450
F 0 "M1" V 4945 3500 50  0000 C CNN
F 1 "Fan_4pin" V 4854 3500 50  0000 C CNN
F 2 "" H 4650 3460 50  0001 C CNN
F 3 "http://www.formfactors.org/developer%5Cspecs%5Crev1_2_public.pdf" H 4650 3460 50  0001 C CNN
	1    4650 3450
	1    0    0    -1  
$EndComp
$Comp
L power:+12V #PWR09
U 1 1 5F5FB4EE
P 4200 3000
F 0 "#PWR09" H 4200 2850 50  0001 C CNN
F 1 "+12V" H 4215 3173 50  0000 C CNN
F 2 "" H 4200 3000 50  0001 C CNN
F 3 "" H 4200 3000 50  0001 C CNN
	1    4200 3000
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R4
U 1 1 5F5FC321
P 4200 3150
F 0 "R4" H 4259 3196 50  0000 L CNN
F 1 "10k" H 4259 3105 50  0000 L CNN
F 2 "" H 4200 3150 50  0001 C CNN
F 3 "~" H 4200 3150 50  0001 C CNN
	1    4200 3150
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R1
U 1 1 5F5FFED5
P 3400 3550
F 0 "R1" H 3459 3596 50  0000 L CNN
F 1 "15k" H 3459 3505 50  0000 L CNN
F 2 "" H 3400 3550 50  0001 C CNN
F 3 "~" H 3400 3550 50  0001 C CNN
	1    3400 3550
	1    0    0    -1  
$EndComp
$Comp
L Device:R_Small R2
U 1 1 5F5FF7BB
P 3650 3350
F 0 "R2" V 3454 3350 50  0000 C CNN
F 1 "330" V 3545 3350 50  0000 C CNN
F 2 "" H 3650 3350 50  0001 C CNN
F 3 "~" H 3650 3350 50  0001 C CNN
	1    3650 3350
	0    1    1    0   
$EndComp
$Comp
L Device:R_Small R3
U 1 1 5F5FDCC5
P 3950 3350
F 0 "R3" V 3754 3350 50  0000 C CNN
F 1 "10k" V 3845 3350 50  0000 C CNN
F 2 "" H 3950 3350 50  0001 C CNN
F 3 "~" H 3950 3350 50  0001 C CNN
	1    3950 3350
	0    1    1    0   
$EndComp
$Comp
L power:GND #PWR011
U 1 1 5F5B0358
P 4650 3700
F 0 "#PWR011" H 4650 3450 50  0001 C CNN
F 1 "GND" H 4655 3527 50  0000 C CNN
F 2 "" H 4650 3700 50  0001 C CNN
F 3 "" H 4650 3700 50  0001 C CNN
	1    4650 3700
	-1   0    0    -1  
$EndComp
$Comp
L power:+12V #PWR010
U 1 1 5F5AF659
P 4650 3000
F 0 "#PWR010" H 4650 2850 50  0001 C CNN
F 1 "+12V" H 4665 3173 50  0000 C CNN
F 2 "" H 4650 3000 50  0001 C CNN
F 3 "" H 4650 3000 50  0001 C CNN
	1    4650 3000
	-1   0    0    -1  
$EndComp
$Comp
L power:+5V #PWR01
U 1 1 5F5BDC42
P 3800 1200
F 0 "#PWR01" H 3800 1050 50  0001 C CNN
F 1 "+5V" H 3900 1200 50  0000 C CNN
F 2 "" H 3800 1200 50  0001 C CNN
F 3 "" H 3800 1200 50  0001 C CNN
	1    3800 1200
	1    0    0    -1  
$EndComp
Wire Wire Line
	3500 700  3400 700 
Wire Wire Line
	3400 500  3500 500 
$Comp
L power:GND #PWR07
U 1 1 5F5AC458
P 3400 700
F 0 "#PWR07" H 3400 450 50  0001 C CNN
F 1 "GND" H 3350 600 50  0000 R CNN
F 2 "" H 3400 700 50  0001 C CNN
F 3 "" H 3400 700 50  0001 C CNN
	1    3400 700 
	1    0    0    -1  
$EndComp
NoConn ~ 1900 1500
NoConn ~ 1900 1400
NoConn ~ 1900 1700
NoConn ~ 1900 1800
NoConn ~ 1900 2000
NoConn ~ 1900 2100
NoConn ~ 1900 2200
NoConn ~ 1900 2300
NoConn ~ 1900 2400
Wire Wire Line
	4650 3650 4650 3700
Wire Wire Line
	4650 3000 4650 3150
Text Notes 150  500  0    50   ~ 0
TACH_INPUT must be connected to an external interrupt pin.
Text Notes 150  350  0    50   ~ 0
FAN_CONTROL must be connected to a PWM pin that is\nconnected to any of Timer/Counter1, 3, or 4 (meaning pins\nD5, D6, D9, D10, D11, or D13 are acceptable).
Wire Wire Line
	3300 1700 3200 1700
Text Notes 150  700  0    50   ~ 0
TEMP_INPUT must be connected to an analog input pin.
Text Label 2100 1600 0    50   ~ 0
TEMP_INPUT
Wire Wire Line
	2100 1600 1900 1600
Wire Wire Line
	2100 1900 1900 1900
Wire Wire Line
	1900 2500 2100 2500
Text Notes 3650 3750 0    30   ~ 0
The fan control signal is a 5V,\n25kHz PWM signal.
Text Notes 2750 3050 0    30   ~ 0
The tachometer signal is an open-collector, two pulse\nper revolution square wave that needs to be pulled up\nto 12V. The Noctua datasheet also specifies a maximum\ncurrent of no more than 5mA. In addition, the Arduino\nhas a maximum of 5V for its inputs. To satisfy these\nconstraints, there's a pull up to 12V (R4), followed by a\nvoltage divider to bring the 12V down to 5V.\nOther resistor values can be used as long as they satisfy\nthose constraints. I picked these values as I already had\nthese resistors on hand.
NoConn ~ 1350 2950
NoConn ~ 1250 2950
NoConn ~ 1150 2950
NoConn ~ 1050 2950
NoConn ~ 950  2950
Wire Wire Line
	3800 1200 3800 1300
Wire Wire Line
	3800 2100 3800 2200
$Comp
L power:+12V #PWR06
U 1 1 5F5AACDA
P 3400 500
F 0 "#PWR06" H 3400 350 50  0001 C CNN
F 1 "+12V" H 3350 600 50  0000 R CNN
F 2 "" H 3400 500 50  0001 C CNN
F 3 "" H 3400 500 50  0001 C CNN
	1    3400 500 
	1    0    0    -1  
$EndComp
$Comp
L power:+12V #PWR04
U 1 1 5F5AC8D8
P 2100 1200
F 0 "#PWR04" H 2100 1050 50  0001 C CNN
F 1 "+12V" H 2250 1250 50  0000 C CNN
F 2 "" H 2100 1200 50  0001 C CNN
F 3 "" H 2100 1200 50  0001 C CNN
	1    2100 1200
	1    0    0    -1  
$EndComp
Text Label 2100 2500 0    50   ~ 0
TACH_INPUT
$EndSCHEMATC
