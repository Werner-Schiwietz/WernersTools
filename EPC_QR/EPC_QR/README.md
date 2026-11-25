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

## Hinweise

keine Gewährleistung für die Richtigkeit der Daten, die in den QR-Code eingegeben werden. Bitte überprüfen Sie die Daten vor der Verwendung.
Es gilt die MIT-Lizenz.