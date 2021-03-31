import leds
import htmlcolor
import buttons
import display
import utime
import bme680
import math

def AirQualityLED(gas_quality, numLEDs):
    
    if (gas_quality < 50):
        QualityColor = htmlcolor.GREEN
    elif (gas_quality < 100):
        QualityColor = htmlcolor.YELLOW
    elif (gas_quality < 150):
        QualityColor = htmlcolor.ORANGE
    elif (gas_quality < 200):
        QualityColor = htmlcolor.RED
    elif (gas_quality < 250):
        QualityColor = htmlcolor.CRIMSON
    else:
        QualityColor = htmlcolor.PURPLE
    LEDcount=round(gas_quality/200*numLEDs)
    leds.clear()
    for i in range(0,10-LEDcount):
        leds.set(i,QualityColor)
    leds.update()


bme680.init()
cont = True
numLEDs=10

while cont:
    temperature, humidity, pressure, resistance = bme680.get_data()
    gas_quality = math.log(resistance) + 0.04* math.log(resistance)/humidity #https://forums.pimoroni.com/t/bme680-observed-gas-ohms-readings/6608/17
    disp = display.open()
    disp.clear()
    disp.print("{:2.2f} °C".format(temperature), fg=[0,255,255], bg=[0,0,0], posx=0, posy=1)
    disp.print("{:2.2f} %r.h.".format(humidity), fg=[0,255,255], bg=[0,0,0], posx=0, posy=20)
    disp.print("{:2.0f} hPa".format(pressure), fg=[0,255,255], bg=[0,0,0], posx=0, posy=40)
    #disp.print("{:2.0f} gas.".format(resistance), fg=[0,255,255], bg=[0,0,0], posx=0, posy=60)
    disp.print("{:2.0f} gas".format(gas_quality), fg=[0,255,255], bg=[0,0,0], posx=0, posy=60)
    disp.update()
    #
    AirQualityLED(gas_quality, numLEDs)
    utime.sleep(0.5)