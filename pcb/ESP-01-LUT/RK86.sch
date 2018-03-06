EESchema Schematic File Version 2  date Вт 06 мар 2018 08:51:01
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:special
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:esp8266
LIBS:led
LIBS:pwr
LIBS:test
LIBS:RK86-cache
EELAYER 25  0
EELAYER END
$Descr A4 8283 11700
encoding utf-8
Sheet 1 1
Title ""
Date "6 mar 2018"
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
Wire Wire Line
	2700 2850 2350 2850
Connection ~ 3050 5550
Wire Wire Line
	3050 5650 3050 5450
Wire Wire Line
	6000 5100 6100 5100
Wire Wire Line
	6100 5100 6100 5300
Wire Wire Line
	3050 5650 3200 5650
Wire Wire Line
	3850 7700 4350 7700
Wire Wire Line
	4350 7500 3850 7500
Connection ~ 1450 2850
Wire Wire Line
	1350 2850 1550 2850
Connection ~ 2450 2850
Wire Wire Line
	3050 5450 3200 5450
Wire Wire Line
	7100 5100 6600 5100
Connection ~ 6600 5100
Wire Wire Line
	6000 5500 6100 5500
Wire Wire Line
	6100 5500 6100 5700
Connection ~ 6100 5500
Connection ~ 6100 5100
Wire Wire Line
	6600 5100 6600 5700
Connection ~ 6600 5300
Connection ~ 6600 5500
Connection ~ 6750 5100
Connection ~ 6950 5100
Wire Wire Line
	2350 2800 2350 2900
Connection ~ 2350 2850
Text GLabel 5000 2050 2    60   Input ~ 0
GND
$Comp
L CONN_7 J3
U 1 1 5A9E15B1
P 4650 1750
F 0 "J3" V 4620 1750 60  0000 C CNN
F 1 "miniDIN" V 4720 1750 60  0000 C CNN
	1    4650 1750
	-1   0    0    -1  
$EndComp
$Comp
L R R11
U 1 1 593E2273
P 6350 5700
F 0 "R11" V 6430 5700 50  0000 C CNN
F 1 "1.5k=" V 6350 5700 50  0000 C CNN
	1    6350 5700
	0    -1   -1   0   
$EndComp
$Comp
L R R2
U 1 1 593E2265
P 6350 5300
F 0 "R2" V 6430 5300 50  0000 C CNN
F 1 "470=" V 6350 5300 50  0000 C CNN
	1    6350 5300
	0    -1   -1   0   
$EndComp
Text GLabel 7100 5100 2    60   Input ~ 0
COMP_OUT
$Comp
L GND #PWR01
U 1 1 593E1EF8
P 6950 5600
F 0 "#PWR01" H 6950 5600 30  0001 C CNN
F 1 "GND" H 6950 5530 30  0001 C CNN
	1    6950 5600
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR02
U 1 1 593E1EF6
P 6750 5600
F 0 "#PWR02" H 6750 5600 30  0001 C CNN
F 1 "GND" H 6750 5530 30  0001 C CNN
	1    6750 5600
	1    0    0    -1  
$EndComp
$Comp
L R R4
U 1 1 593E1EF3
P 6950 5350
F 0 "R4" V 7030 5350 50  0000 C CNN
F 1 "150=" V 6950 5350 50  0000 C CNN
	1    6950 5350
	1    0    0    -1  
$EndComp
$Comp
L R R3
U 1 1 593E1EED
P 6750 5350
F 0 "R3" V 6830 5350 50  0000 C CNN
F 1 "150=" V 6750 5350 50  0000 C CNN
	1    6750 5350
	1    0    0    -1  
$EndComp
$Comp
L R R5
U 1 1 593E1EEA
P 6350 5500
F 0 "R5" V 6430 5500 50  0000 C CNN
F 1 "1k=" V 6350 5500 50  0000 C CNN
	1    6350 5500
	0    -1   -1   0   
$EndComp
Text GLabel 6000 5500 0    60   Input ~ 0
SYNC_OUT
$Comp
L R R1
U 1 1 593E1EE2
P 6350 5100
F 0 "R1" V 6430 5100 50  0000 C CNN
F 1 "470=" V 6350 5100 50  0000 C CNN
	1    6350 5100
	0    -1   -1   0   
$EndComp
Text GLabel 6000 5100 0    60   Input ~ 0
VIDEO_OUT
Text GLabel 4400 5550 2    60   Input ~ 0
VIDEO_OUT
Text GLabel 1850 5650 0    60   Input ~ 0
VIDEO
Text GLabel 1850 5450 0    60   Input ~ 0
SYNC_OUT
Text GLabel 3050 5100 2    60   Input ~ 0
SYNC_OUT
Text GLabel 1850 5000 0    60   Input ~ 0
SYNC
Text GLabel 1850 5200 0    60   Input ~ 0
VIDEO
$Comp
L 74HC00 U2
U 4 1 593E181B
P 2450 5550
F 0 "U2" H 2450 5600 60  0000 C CNN
F 1 "74HC00" H 2450 5450 60  0000 C CNN
	4    2450 5550
	1    0    0    -1  
$EndComp
$Comp
L 74HC00 U2
U 3 1 593E1818
P 2450 6000
F 0 "U2" H 2450 6050 60  0000 C CNN
F 1 "74HC00" H 2450 5900 60  0000 C CNN
	3    2450 6000
	1    0    0    -1  
$EndComp
$Comp
L 74HC00 U2
U 2 1 593E180A
P 3800 5550
F 0 "U2" H 3800 5600 60  0000 C CNN
F 1 "74HC00" H 3800 5450 60  0000 C CNN
	2    3800 5550
	1    0    0    -1  
$EndComp
$Comp
L 74HC00 U2
U 1 1 593E17FC
P 2450 5100
F 0 "U2" H 2450 5150 60  0000 C CNN
F 1 "74HC00" H 2450 5000 60  0000 C CNN
	1    2450 5100
	1    0    0    -1  
$EndComp
NoConn ~ 2300 2000
NoConn ~ 2300 1850
$Comp
L R R14
U 1 1 59394742
P 4100 8100
F 0 "R14" V 4180 8100 50  0000 C CNN
F 1 "510" V 4100 8100 50  0000 C CNN
	1    4100 8100
	0    -1   -1   0   
$EndComp
$Comp
L R R6
U 1 1 59394738
P 4100 7900
F 0 "R6" V 4180 7900 50  0000 C CNN
F 1 "510" V 4100 7900 50  0000 C CNN
	1    4100 7900
	0    -1   -1   0   
$EndComp
Text GLabel 2750 8200 2    60   Input ~ 0
Vcc
Text GLabel 2750 8400 2    60   Input ~ 0
Vcc
Text GLabel 2250 8200 0    60   Input ~ 0
RST
Text GLabel 2100 7700 0    60   Input ~ 0
RST
Text GLabel 2750 8800 2    60   Input ~ 0
Vcc
Text GLabel 2250 8800 0    60   Input ~ 0
GPIO2
Text GLabel 2750 8600 2    60   Input ~ 0
Vcc
Text GLabel 2250 8600 0    60   Input ~ 0
GPIO0
Text GLabel 2250 8400 0    60   Input ~ 0
CH_PD
Text GLabel 3850 8100 0    60   Input ~ 0
GPIO2
Text GLabel 3850 7900 0    60   Input ~ 0
GPIO0
Text GLabel 3850 7700 0    60   Input ~ 0
TXD
Text GLabel 3850 7500 0    60   Input ~ 0
RXD
Text GLabel 2900 7700 2    60   Input ~ 0
GPIO0
Text GLabel 2900 7600 2    60   Input ~ 0
GPIO2
Text GLabel 2100 7600 0    60   Input ~ 0
CH_PD
Text GLabel 2100 7800 0    60   Input ~ 0
Vcc
Text GLabel 2900 7500 2    60   Input ~ 0
GND
Text GLabel 2900 7800 2    60   Input ~ 0
RXD
Text GLabel 2100 7500 0    60   Input ~ 0
TXD
$Comp
L CONN_4X2 WIFI1
U 1 1 592C14A8
P 2500 7650
F 0 "WIFI1" H 2500 7900 50  0000 C CNN
F 1 "ESP-01" V 2500 7650 40  0000 C CNN
	1    2500 7650
	1    0    0    -1  
$EndComp
Text Notes 5000 8000 0    60   ~ 0
Уровни PS/2 - 5v.\nРезисторы 1k для защиты ESP.
Text GLabel 4350 8100 2    60   Input ~ 0
PS2_DATA
Text GLabel 4350 7900 2    60   Input ~ 0
PS2_CLK
$Comp
L R R7
U 1 1 59359C80
P 2500 8200
F 0 "R7" V 2580 8200 50  0000 C CNN
F 1 "22k" V 2500 8200 50  0000 C CNN
	1    2500 8200
	0    -1   -1   0   
$EndComp
Text GLabel 4350 7700 2    60   Input ~ 0
SYNC
Text Notes 4950 7550 0    60   ~ 0
RXD = I2S_DATA
Text GLabel 4350 7500 2    60   Input ~ 0
VIDEO
$Comp
L R R10
U 1 1 593598CB
P 2500 8800
F 0 "R10" V 2580 8800 50  0000 C CNN
F 1 "22k" V 2500 8800 50  0000 C CNN
	1    2500 8800
	0    -1   -1   0   
$EndComp
$Comp
L R R9
U 1 1 5935988D
P 2500 8600
F 0 "R9" V 2580 8600 50  0000 C CNN
F 1 "22k" V 2500 8600 50  0000 C CNN
	1    2500 8600
	0    -1   -1   0   
$EndComp
$Comp
L R R8
U 1 1 59359823
P 2500 8400
F 0 "R8" V 2580 8400 50  0000 C CNN
F 1 "22k" V 2500 8400 50  0000 C CNN
	1    2500 8400
	0    -1   -1   0   
$EndComp
Text GLabel 2700 2850 2    60   Input ~ 0
Vcc
$Comp
L GND #PWR03
U 1 1 593592DA
P 2450 3250
F 0 "#PWR03" H 2450 3250 30  0001 C CNN
F 1 "GND" H 2450 3180 30  0001 C CNN
	1    2450 3250
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR04
U 1 1 593592D8
P 1450 3250
F 0 "#PWR04" H 1450 3250 30  0001 C CNN
F 1 "GND" H 1450 3180 30  0001 C CNN
	1    1450 3250
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR05
U 1 1 593592D5
P 1950 3150
F 0 "#PWR05" H 1950 3150 30  0001 C CNN
F 1 "GND" H 1950 3080 30  0001 C CNN
	1    1950 3150
	1    0    0    -1  
$EndComp
Text GLabel 1350 2850 0    60   Input ~ 0
+5v
$Comp
L CP1 C2
U 1 1 593592BA
P 2450 3050
F 0 "C2" H 2500 3150 50  0000 L CNN
F 1 "220uF" H 2500 2950 50  0000 L CNN
	1    2450 3050
	1    0    0    -1  
$EndComp
$Comp
L CP1 C1
U 1 1 593592B7
P 1450 3050
F 0 "C1" H 1500 3150 50  0000 L CNN
F 1 "220uF" H 1500 2950 50  0000 L CNN
	1    1450 3050
	1    0    0    -1  
$EndComp
$Comp
L LM1117 U1
U 1 1 593592A8
P 1950 2850
F 0 "U1" H 2050 2650 60  0000 C CNN
F 1 "LM1117" H 1950 3050 60  0000 C CNN
	1    1950 2850
	1    0    0    -1  
$EndComp
Text Notes 2100 4050 0    60   ~ 0
При 0 на VIDEO, на выходе должно быть 0.3В (черный).\nПри 1 на VIDEO, на выходе должно быть 1.0В (белый).\nПри 0 на SYNC, выход должен зависеть от VIDEO.\nПри 1 на SYNC и 0 на VIDEO, на выходе должно быть 0.3В (черный).\nПри 1 на SYNC и 1 на VIDEO, на выходе должно быть 0.0В (синхронизация).\n\nТаким образом, VIDEO формирует точные интервалы для синхронизации и картинки,\nSYNC включается заранее до импульсов синхронизации и отключается после их.\nТак получается четкая синхронизация синхроимпульсов и картинки.
Text GLabel 1550 2100 0    60   Input ~ 0
GND
Text GLabel 1550 2000 0    60   Input ~ 0
GND
Text GLabel 2300 2100 2    60   Input ~ 0
GND
Text GLabel 1550 1850 0    60   Input ~ 0
+5v
Text GLabel 6250 1450 2    60   Input ~ 0
COMP_OUT
Text GLabel 6250 1650 2    60   Input ~ 0
GND
NoConn ~ 5000 1950
Text GLabel 5000 1850 2    60   Input ~ 0
PS2_CLK
Text GLabel 5000 1750 2    60   Input ~ 0
+5v
Text GLabel 5000 1650 2    60   Input ~ 0
GND
NoConn ~ 5000 1550
Text GLabel 5000 1450 2    60   Input ~ 0
PS2_DATA
$Comp
L CONN_2 J1
U 1 1 59358E4F
P 5900 1550
F 0 "J1" V 5850 1550 40  0000 C CNN
F 1 "RCA" V 5950 1550 40  0000 C CNN
	1    5900 1550
	-1   0    0    -1  
$EndComp
$Comp
L USB J2
U 1 1 593586E1
P 1950 1650
F 0 "J2" H 1900 2050 60  0000 C CNN
F 1 "USB-B" V 1700 1800 60  0000 C CNN
	1    1950 1650
	1    0    0    -1  
$EndComp
$EndSCHEMATC
