Korte uitleg i2c zonder onnodige diepgang.

Eerst even wat dingetjes die kut zijn.
- Het uploaden van de master code naar de Atmega2560 is nogal een dingetje. Gert heeft ons hier een mail over gestuurd. Zorg in de arguments string wel dat de COM poort overeenkomt met de poort waarop de Atmega2560 is aangesloten.
- Bij het uploaden van de slave hex file naar de robot zorg dat hij op Atmega32 staat in AVR studio. 
- Als 1 van de 2 devices blijft hangen (omdat je loopt te resetten ofzo), dan moet je de code waarschijnlijk opnieuw uploaden naar beide devices.
- Gebruik voor het gemak 1 karakter voor het sturen van data. Ik moet hier nog wat conversie functies voor maken als we meer data moeten versturen, maar voor de rijfuncties is dit niet nodig.

Ik heb de (voor jullie) belangrijkste functie van i2c opgesplitst in 2 functies: i2c_write() en i2c_read(). Als je alleen een signaal naar de slave (RP6) wil sturen gebruik je i2c_write(). Als je een signaal wil sturen en vervolgens informatie terug wil krijgen gebruik je i2c_write() EN i2c_read(). Zo simpel is het.

De enige code die je in de master hoeft aan te passen is de USART interrupt routine die begint bij regel 60. Wat je hier moet doen spreekt voor zich. Of je hier alleen write of ook read wil gebruiken hangt af van wat je ermee wil, maar ik zie nu dat ik "write only" nog niet echt een specifieke functie heb gegeven. Gebruik voor nu write + read, ik zal dat als ik wakker ben wel even fixen en jullie op de hoogte brengen als ik het op github heb gezet.

In de slave code hoef je wat betreft i2c alleen de functie "i2cFunctions(char a)" aan te passen. Als je bijv. naar links wil gaan doe je iets als: TWDR = '<'; rijLinks(); break; // TWDR is wat de slave terugstuurt



Overigens had ik nog een idee hoe het in elkaar gezet kan worden:

void main(void) {
	sei();
	i2c_slave_init();
	de_rest_van_de_inits_die_de_motor_gebruikt();
	while(1) {
		stilstand van de motor
	}
	return 0;
}

Vervolgens worden de functies voor vooruit, achteruit, links, en rechts alleen gebruikt door i2cFunctions(), die aangeroepen wordt door de TWI interrupt. Hierdoor zou de robot moeten bewegen als je de knop ingedrukt houd en weer stoppen met bewegen als je em loslaat omdat hij dan weer terug in de while loop valt.