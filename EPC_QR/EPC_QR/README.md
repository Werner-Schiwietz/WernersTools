# EPC_QR

## Zweck

das Programm dient dazu, einen EPC-/GiroCode-QR-Code zu generieren,
der für SEPA-Überweisungen verwendet werden kann. 
Der QR-Code enthält alle notwendigen Informationen für eine Überweisung, wie Empfänger, IBAN, Betrag und Verwendungszweck.


## Funktionsweise

Es wird intern ein EPC Sepa-Datensatz erzeugt, der per QR-Code-Generator in einen QR-Code umgewandelt wird.

## Verwendung

der QR-Code kann über die Zwischenablage in ein Textverarbeitungsprogramm, eine E-Mail oder sonstiges eingefügt werden.

## Voraussetzungen

- Windows
- Visual Studio mit C++-Desktopentwicklung zum Erstellen und verändern der Windows-Desktopanwendung

## Gespeicherte Kontodaten

Bis zu fünf zuletzt verwendete Datensätze mit Empfängername, IBAN, BIC, Betrag und Verwendungszweck werden gespeichert. Über die Auswahlliste „Zuletzt“ kann ein gespeicherter Datensatz geladen werden. Geänderte Daten werden beim Schließen gespeichert; ein gleicher IBAN-Eintrag wird aktualisiert und an den Anfang der Liste gesetzt. Die Datei `Data\account.dat` neben der Programmdatei wird bevorzugt; wenn dieser Ordner nicht beschreibbar ist, verwendet das Programm `%LOCALAPPDATA%\EPC_QR\account.dat`. Das Referenzfeld wird nicht gespeichert. Fehlt noch eine gespeicherte Datei, werden zunächst die eingebauten Beispieldaten angezeigt.

Die Datei enthält per Windows-DPAPI verschlüsselte Daten. Sie können normalerweise nur vom selben Windows-Benutzer auf demselben Computer entschlüsselt werden. Wird die Datei neben der EXE auf einen anderen Computer kopiert, stehen die gespeicherten Angaben dort daher in der Regel nicht zur Verfügung.

## BIC-Ermittlung

Für deutsche IBANs kann die BIC anhand der Bankleitzahl aus der lokalen Datei `Data\blz-bic-de.csv` ermittelt werden; ist der Programmordner nicht beschreibbar, liegt sie unter `%LOCALAPPDATA%\EPC_QR\blz-bic-de.csv`. Eine vorhandene ältere `blz-bic.csv` wird beim Start übernommen. Über „BIC-Tabelle DE“ wird die aktuelle CSV-Datei von der Deutschen Bundesbank geladen und lokal in eine BLZ/BIC/Banknamen-Zuordnung umgewandelt. Der ermittelte Bankname wird als Tooltip angezeigt, wenn der Mauszeiger über der IBAN steht. Es wird keine IBAN an einen Internetdienst gesendet. Die Datenbank deckt nur deutsche Bankleitzahlen ab; für andere Länder bleibt die BIC manuell einzugeben.

Quelle und Update: [Download der Bankleitzahlendateien bei der Deutschen Bundesbank](https://www.bundesbank.de/de/aufgaben/unbarer-zahlungsverkehr/serviceangebot/bankleitzahlen/download-bankleitzahlen-602592). Die öffentliche Datei wird ohne Gewähr bereitgestellt und ist keine vollständige BIC-Datenbank für den gesamten SEPA-Raum. Die Zuordnung ist daher ein Vorschlag und muss vor einer Überweisung geprüft werden. Die Aktualisierung setzt eine Internetverbindung voraus.

## Hinweise

keine Gewährleistung für die Richtigkeit der Daten, die in den QR-Code eingegeben werden. Bitte überprüfen Sie die Daten vor der Verwendung.
Es gilt die MIT-Lizenz.