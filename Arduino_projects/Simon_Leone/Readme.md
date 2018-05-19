# Simon Leone - Arduino program

This is intended to run on Adafruit ItsyBisty 32u4 5V 16MHz.

## Simon Pocket Electronic Schematic

Original Simon Pocket have 4 quarter-circle colored push buttons,
three round button ("DERNIERE", "DEPART", "LA PLUS LONGUE"),
and two switches ("JEU", "DIFFICULTE").

"JEU" switch have 2 connecting jumpers, the first for powering the electronic
from the 9V battery, the second to choose the game.

The later is connected with all other buttons in a 3x4 matrix.

In the 80's LED were not so common, so Simon Pocket is equipped with small
incandescent 3V light bulb. A spare one is even package inside the case;
Since the drawing current of such bulb is too much for a common micro-controller,
and to avoid the use of a second power supply (like the 2 AA battery in the
  original game) and the use of and external mosfet driver, I change those bulbs
  for some LED.

There is also a small piezo buzzer, which can be easily wired to a micro controller.

[include PCB image with connection here]

## Arduino code

Basically, the code scans the buttons matrix on each loop and check for incoming
message from USB port.

The matrix rows pins are set as INPUT and we use the internal 20k pull-up resistor.
This avoid the need of external current limiting resistor.

There is also a debounce mechanism which can be easily tweak by changing a constant in `constants.h`.

## Pin mapping

See `constants.h` and annotated PCB schematic.

## Key code used

### Codes sent by Simon

| Button          | Code |
|-----------------|------|
| Red             |  16  |
| Green           |  17  |
| Yellow          |  18  |
| Blue            |  19  |
| La plus longue  |  32  |
| Derniere        |  33  |
| Arret           |  0   |
| Jeu 1           |  1   |
| Jeu 2           |  35  |
| Jeu 3           |  34  |
| Difficulte 1    |  51  |
| Difficulte 2    |  48  |
| Difficulte 3    |  50  |
| Difficulte 4    |  49  |

### Codes received by Simon

| Action         | Code |
|----------------|------|
| Red Led On     |  10  |
| Red Led Off    |  11  |
| Red Tone       |  12  |
| Green Led On   |  20  |
| Green Led Off  |  21  |
| Green Tone     |  22  |
| Yellow Led On  |  30  |
| Yellow Led Off |  31  |
| Yellow Tone    |  32  |
| Blue Led On    |  40  |
| Blue Led Off   |  41  |
| Blue Tone      |  42  |
| Lose Tone      |  50  |
| Win Tone       |  51  |
| Tone Off       |  52  |
