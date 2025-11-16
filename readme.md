# ASG-Loetkit-2025

[Löt-Video](https://www.youtube.com/watch?v=-um5J8tv-0Q)

[Demo-Video](https://www.youtube.com/watch?v=5mWFSo3QfBs)

Dieses Projekt ist eine kleine DMX-leuchte, die 9 LEDs enthält und individuell ansteuern kann.

Es wurde speziell dazu entwickelt, einen Einstieg in das SMD-Löten zu bieten. 
Dazu werden zwei der ICs vorgelötet, der Rest der Komponenten wurden dazu ausgewählt, so einfach wie möglich zu löten zu sein.

## Beschreibung der Schaltung

Das Herzstück des Gerätes ist der CH32V003 Microcontroller. 
Dieser empfängt DMX-Signale entweder von einem USB-Serial chip (FT232RL), oder von einem MAX485-Transciever, der die DMX-Signale übersetzt.
Über einen Schalter kann zwischen den beiden Signalquellen umgeschaltet werden. 

Der Microcontroller liest die gewünschte DMX-Adresse von dem DIP-Schalter aus, und entscheidet basierend auf dem eingestellten Kanal-Modus, welche RGB-Farbwerte die LEDs haben sollen.

Die LEDs sind vom Typ WS2812. Das sind einzeln steuerbare RGB-LEDs die häufig in PCs oder anderen bunt leuchtenden dingen verwendet werden. Alle LEDs werden seriell über einen einzigen Pin am 
Microcontroller angesteuert, und formen eine Kette wo das vorherige LED die Daten an die nächste weitergibt.

Der Microcontroller kann einen kleinen push-schalter auslesen, um durch die Modi durchzuwechseln. Eine kleine rote LED kann dazu verwendet werden um das Vorhandensein eines DMX-Signals zu zeigen.

## Verwendung

### Stromversorgung

Das Gerät wird über den USB-Port mit 5V Strom versorgt. 

### DMX-Signal

Als Signalquelle stehen zwei Möglichkeiten zur Auswahl:

- DMX: Die beiden 3-pin Steckverbinder oben rechts sind die DMX in / out ports. Sie verhalten sich wie die Üblichen 3-pin XLR-buchsen, sind aber kleiner und günstiger. Der Schalter muss dafür in der rechten Stellung sein.
- USB: Wenn der FT232-chip installiert ist, dann kann das Gerät auch an einen Computer direkt angeschlossen werden. Dafür verwenden wir die Software `QLC+`, die diesen Chip nativ unterstützt. Der Schalter muss dafür in der linken Stellung sein.

**Anzeigen des DMX-Signals:**

Wenn der Push-schalter gedrückt ist wärend das Gerät eingesteckt wird, dann blinkt die rote LED D10 kurz, wenn ein neues DMX-Datenpaket empfangen wurde. 

### Auswahl eines Kanal-Moduses

Über einen Klick des Push-schalters wird durch die Kanal-Modi gewechselt. Standardmäßig ist das Gerät im 4-Kanal-Modus. Solange der Knopf gedrückt ist wird ein Muster auf den LEDs gezeigt, der den Modus symbolisiert. 

Die verfügbaren Modi sind wie folgt:

**4-Kanal-Modus**

Klassisches RGB + Dimmer

|Channel|Function|
|-------|--------|
|1|R|
|2|G|
|3|B|
|4|Dimmer|

**27-Kanal-Modus**

R, G, B für jede der 9 LEDs

|Channel|Function|
|-------|--------|
|3*n+1|R für Dn|
|3*n+2|G für Dn|
|3*n+3|B für Dn|

(wobei n = 0-8 für die 9 LEDs)

**28-Kanal-Modus**

R, G, B für jede der 9 LEDs, und ein Dimmer der alle gleichzeitig dimmt.

|Channel|Function|
|-------|--------|
|1|Dimmer|
|3*n+2|R für Dn|
|3*n+3|G für Dn|
|3*n+4|B für Dn|

(wobei n = 0-8 für die 9 LEDs)


**36-Kanal-Modus**

R, G, B und Dimmer für jede der 9 LEDs

|Channel|Function|
|-------|--------|
|4*n+1|R für Dn|
|4*n+2|G für Dn|
|4*n+3|B für Dn|
|4*n+4|Dimmer für Dn|

**HSV-Modus**

3 Kanäle, die Farbe wird dabei im [HSV-Farbraum](https://de.wikipedia.org/wiki/HSV-Farbraum) eingestellt. Kanal 3 ist dabei Helligkeit, die wie der Dimmer agiert.

|Channel|Function|
|-------|--------|
|1|Hue (Farbton)|
|2|Saturation (Sättigung)|
|3|Value (Dimmer)|

**3-Kanal-Modus**

Klassisches RGB

|Channel|Function|
|-------|--------|
|1|R|
|2|G|
|3|B|

### Demo-Modus

Wenn man den Auswahlknopf mehr als 10 sekunden hält, dann wechselt das Gerät in einen eingebauten Demo-Modus.
Alle eingehenden DMX-Signale werden dabei ignoriert.





