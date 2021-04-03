import bme680
import leds
import time
import display
import vibra
import buttons


colors = {'lime': [0, 255, 0], 'green': [0, 128, 0], 'yellow': [255, 255, 0], 'orange': [
    255, 128, 0], 'red': [255, 0, 0], 'purple': [128, 0, 128], 'brown': [128, 0, 0], 'blue': [0, 0, 255]}

gs = 160
all_colors = [((i >> 2) * gs, (i >> 1 & 1) * gs, (i & 1) * gs)
              for i in range(1, 8)]

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
        return "Ventilate?"
    if (iaq < 200):
        return "Ventilate."
    if (iaq < 250):
        return "Ventilate!"
    if (iaq <= 350):
        return "Danger!"
    if (iaq > 350):
        return "Leave!"


def leds_set_bottom(co2_color):
    leds.set(11, co2_color)
    leds.set(12, co2_color)
    leds.set(13, co2_color)
    leds.set(14, co2_color)


def main():

    power_saving = False

    disp = display.open()
    disp.backlight(25)
    disp.clear().update()
    leds.clear()
    leds.set_powersave()
    leds.dim_top(1)
    leds.dim_bottom(1)

    with bme680.Bme680() as environment:
        while True:
            data = environment.get_data()

            disp.clear()

            # button toggle screen
            if buttons.read(buttons.TOP_RIGHT):
                power_saving = not power_saving

            if(data.iaq_accuracy >= 2):

                leds.set_all([iaq_color(data.iaq)]*11)

                # Set green rocket if under 600 ppm: no mask required
                if(data.eco2 < 600):
                    co2_text = "No masks required"
                    co2_color = colors['green']
                    leds.set_rocket(2, 31)
                    leds.set_rocket(1, 0)
                    leds.set_rocket(0, 0)

                # Set blue rocket if over 600 ppm: masks required
                elif(data.eco2 >= 600 and data.eco2 < 900):
                    co2_text = "Wear masks"
                    co2_color = colors['blue']
                    leds.set_rocket(0, 31)
                    leds.set_rocket(1, 0)
                    leds.set_rocket(2, 0)
                # Set yellow rocket if over 900 ppm: dangerous even with masks on
                else:
                    co2_text = "Masks could be not enough!"
                    co2_color = colors['orange']

                    leds.set_rocket(1, 31)
                    leds.set_rocket(2, 0)
                    leds.set_rocket(0, 0)
                #     vibra.vibrate(500)
                #     time.sleep(1)
                #     vibra.vibrate(500)
                #     time.sleep(1)
                #     vibra.vibrate(500)

                leds_set_bottom(co2_color)

                if(not power_saving):
                    disp.backlight(25)
                    disp.print("IAQ: " + str(data.iaq))
                    disp.print("CO2: " +
                               str(int(data.eco2)) + "ppm", posy=40)
                    if(data.iaq_accuracy == 3):
                        disp.print(iaq_string(data.iaq), posy=20,
                                   fg=iaq_color(data.iaq))
                        disp.print(co2_text, posy=65, fg=co2_color,
                                   font=display.FONT12)
                    else:
                        disp.print("(still calibrating...)",
                                   posy=60, font=display.FONT12)
                else:
                    disp.clear()
                    disp.backlight(0)

                disp.update()

                time.sleep(1)

            else:
                disp.print("Calibrating...", posy=0, font=display.FONT16)
                disp.print(
                    "Place the badge both in open-air then in a closed box with exhaled air for around 10min each.", posy=40, font=display.FONT8)

                disp.update()
                if(not power_saving):
                    oldest_time = time.time()
                    latest_time = oldest_time
                    while (latest_time - oldest_time < 1):
                        leds.set(rand() %
                                 11, all_colors[rand() % len(all_colors)])
                        time.sleep_ms(1)  # Feed watch doge
                        latest_time = time.time()

                else:
                    leds.set_all([[0, 0, 0]]*15)
                    time.sleep(1)


if __name__ == "__main__":
    main()
