import bme680
# import display, utime, vibra, os, bme680, color, buttons, leds

# led_bar_colours = [[255, 0, 255], [127, 0, 255], [0, 0, 255], [0, 128, 255], [0, 255, 255], [0, 255, 128], [0, 255, 0], [128, 255, 0], [255, 255, 0], [255, 128, 0], [255, 0, 0] ]

# bme680.init()
disp = display.open()

leds.clear()

for x in range(0, 11):
    leds.prep(10-x, led_bar_colours[x])

leds.update()

vibra.vibrate(50)
utime.sleep(0.25)
vibra.vibrate(50)
linecolor = color.Color(100, 100, 100) # grey
rectangleColor = color.Color(0, 255, 0)
arrowPosition = 0
show_led_bar = True


def print_arrow(peak_x, peak_y):
    disp.line(xs=peak_x, ys=peak_y, xe=peak_x+4, ye= peak_y-4, col=rectangleColor)
    disp.line(xs=peak_x, ys=peak_y, xe=peak_x+5, ye= peak_y+5, col=rectangleColor)

def check_arrow(arrow_position):
    if arrow_position == 0:
        return 150, 7
    if arrow_position == 1:
        return 150, 27
    if arrow_position == 2:
        return 150, 47
    if arrow_position == 3:
        return 150, 67

def led_bar(value, min_v, max_v):
    value = value - min_v
    max_v = max_v - min_v
    steps = max_v / 11.0
    current_step = value / steps
    current_step = int(current_step + 0.5)
    leds.clear()
    for x in range(0, current_step):
        leds.prep(10-x, led_bar_colours[x]) # 10 - 3 to reverse LEDs

    leds.update()


def check_led_bar(arrow_position):
    if arrow_position == 0:
        return 0, 40
    if arrow_position == 1:
        return 0, 100
    if arrow_position == 2:
        return 325, 1070
    if arrow_position == 3:
        return 8000, 550000


while True:
    env = bme680.get_data()
    #brightness = light_sensor.get_reading()
    disp.clear()
    disp.print(str(round(env[0], 1)) + " C")
    disp.line(xs=1, ys=17, xe=159, ye= 17, col=linecolor)
    disp.print(str(round(env[1], 1)) + " %", posy=20)
    disp.line(xs=1, ys=37, xe=159, ye= 37, col=linecolor)
    disp.print(str(round(env[2], 1)) + " hPa", posy=40)
    disp.line(xs=1, ys=57, xe=159, ye= 57, col=linecolor)
    disp.print(str(int(env[3])) + " Ohm", posy=60)
    #*****
    #disp.print(str(brightness), posx=105)

    #disp.rect(xs=150, ys=10, xe=158, ye=80, col=rectangleColor, filled=True, size=5)
    valueX, valueY = check_arrow(arrowPosition)
    print_arrow(valueX, valueY)

    if show_led_bar:
        min_led, max_led = check_led_bar(arrowPosition)
        led_bar(env[arrowPosition], min_led, max_led)
    else:
        leds.clear()
        leds.set_powersave(True) 

    disp.update()
    for x in range(0,20):
        utime.sleep(0.05)

        if buttons.read(buttons.BOTTOM_LEFT):
            arrowPosition -= 1
            arrowPosition = arrowPosition % 4
            break

        if buttons.read(buttons.BOTTOM_RIGHT):
            arrowPosition += 1
            arrowPosition = arrowPosition % 4
            break

        if buttons.read(buttons.TOP_RIGHT):
            if show_led_bar:
                show_led_bar = False
                break
            else:
                show_led_bar = True
                break