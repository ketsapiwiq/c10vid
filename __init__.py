import bme680
import leds
import time
import display
# import color


colors = {'lime': [0, 255, 0], 'green': [0, 128, 0], 'yellow': [255, 255, 0], 'orange': [
    255, 128, 0], 'red': [255, 0, 0], 'purple': [128, 0, 128], 'brown': [128, 0, 0]}


gs = 160
all_colors = [((i >> 2) * gs, (i >> 1 & 1) * gs, (i & 1) * gs) for i in range(1, 8)]

_rand = 123456789
def rand():
    global _rand
    _rand = (1103515245 * _rand + 12345) & 0xFFFFFF
    return _rand


def iaq_color(iaq):
    if (iaq < 50):
        return colors['lime']
    if (iaq < 100):
        return colors['green']
    if (iaq < 150):
        return colors['yellow']
    if (iaq < 200):
        return colors['orange']
    if (iaq < 250):
        return colors['red']
    if (iaq <= 350):
        return colors['purple']
    if (iaq > 350):
        return colors['brown']

def iaq_string(iaq):
    if (iaq < 50):
        return "Excellent"
    if (iaq < 100):
        return "Good"
    if (iaq < 150):
        return "OK"
    if (iaq < 200):
        return "Ventilate?"
    if (iaq < 250):
        return "Ventilate!"
    if (iaq <= 350):
        return "Dangerous"
    if (iaq > 350):
        return "Leave!"


def main():

    disp = display.open()
    disp.clear().update()
    leds.clear()
    # leds.set_powersave()

    with bme680.Bme680() as environment:
        while True:
            data = environment.get_data()

            disp.clear()

            if(data.iaq_accuracy >= 2):
                disp.print("IAQ: " + str(data.iaq))
                disp.print(iaq_string(data.iaq), posy=20)
                disp.print("CO2: " +
                       str(round(data.eco2, 1)), posy=40)
                
                if(data.iaq_accuracy == 2):
                    disp.print("calibrating...", posy=60)

                leds.set_all([iaq_color(data.iaq)]*15)
                time.sleep(10)

            else:
                disp.print("calibrating...", posy=20)
                disp.update()
                oldest_time = time.time()
                latest_time = oldest_time
                while (latest_time - oldest_time < 10):
                    leds.set(rand() % 11, all_colors[rand() % len(all_colors)])
                    leds.set_rocket(rand() % 3, rand() % 32)
                    time.sleep_us(1)  # Feed watch doge
                    latest_time = time.time()

            disp.update()


if __name__ == "__main__":
    main()

    # # *****
    # # disp.print(str(brightness), posx=105)

    # # disp.rect(xs=150, ys=10, xe=158, ye=80, col=rectangleColor, filled=True, size=5)
    # valueX, valueY = check_arrow(arrowPosition)
    # print_arrow(valueX, valueY)

    # if show_led_bar:
    #     min_led, max_led = check_led_bar(arrowPosition)
    #     led_bar(env[arrowPosition], min_led, max_led)
    # else:
    #     leds.clear()
    #     leds.set_powersave(True)

    # disp.update()
    # for x in range(0,20):
    #     utime.sleep(0.05)

    #     if buttons.read(buttons.BOTTOM_LEFT):
    #         arrowPosition -= 1
    #         arrowPosition = arrowPosition % 4
    #         break

    #     if buttons.read(buttons.BOTTOM_RIGHT):
    #         arrowPosition += 1
    #         arrowPosition = arrowPosition % 4
    #         break

    #     if buttons.read(buttons.TOP_RIGHT):
    #         if show_led_bar:
    #             show_led_bar = False
    #             break
    #         else:
    #             show_led_bar = True
    #             break
