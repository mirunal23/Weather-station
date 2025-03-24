#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <math.h>
#include <stdio.h>

// Configurare pini
#define pinNTC 0      // Pin pentru senzorul NTC
#define pinMQ135 1    // Pin pentru senzorul MQ135
#define pinMQ7 2      // Pin pentru senzorul MQ7 (CO)
#define pinDHT PD7    // Pin pentru senzorul DHT11 (umiditate)
#define led_verde PB0 // Pin pentru LED verde
#define led_rosu PB1  // Pin pentru LED rosu

// Parametri NTC
#define BCOEFFICIENT 3730.0      // Coeficientul B al termistorului
#define SERIES_RESISTOR 10000.0 // Rezistorul in serie cu NTC
#define TEMPERATURE_NOMINAL 298.15 // Temperatura nominala in Kelvin (25°C)

// Functie pentru initializarea ADC-ului
void ADC_Init() {
	ADMUX = (1 << REFS0); // Seteaza referinta AVcc pentru ADC
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1); // Activeaza ADC-ul si seteaza prescaler-ul la 64
}

// Functie pentru citirea valorii de la un canal ADC
uint16_t ADC_Read(unsigned char channel) {
	ADMUX = (ADMUX & 0xF8) | (channel & 0x07); // Seteaza canalul de citire
	ADCSRA |= (1 << ADSC); // Incepe conversia ADC
	while (!(ADCSRA & (1 << ADIF))); // Asteapta finalizarea conversiei
	uint16_t result = ADC; // Citeste rezultatul ADC
	return result; // Returneaza valoarea citita
}

// Functie pentru initializarea UART-ului
void UART_Init(unsigned int baud) {
	unsigned int ubrr = F_CPU / 16 / baud - 1; // Calculeaza valoarea UBRR
	UBRR0H = (ubrr >> 8); // Seteaza UBRR in H
	UBRR0L = ubrr;        // Seteaza UBRR in L
	UCSR0B = (1 << RXEN0) | (1 << TXEN0); // Activeaza transmiterea si receptia UART
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00); // Seteaza formatul: 8 biti de date, 1 bit de stop
}

// Functie pentru trimiterea unui caracter prin UART
void UART_Transmit(char data) {
	while (!(UCSR0A & (1 << UDRE0))); // Asteapta sa fie liber buffer-ul UART
	UDR0 = data; // Trimite caracterul
}

// Functie pentru trimiterea unui string prin UART
void UART_Print(const char* str) {
	while (*str) {
		UART_Transmit(*str++); // Trimite fiecare caracter din string
	}
}

// Functie pentru afisarea meniului pe UART
void showMenu() {
	UART_Print("\n~~~ MENIU ~~~\n");
	UART_Print("1: Citire temperatura, tensiune, curent si rezistent NTC\n");
	UART_Print("2: Citire calitate aer MQ135\n");
	UART_Print("3: Citire concentratie CO MQ7\n");
	UART_Print("4: Citire umiditate DHT11\n");
	UART_Print("Scrie 's' pentru a revedea meniul.\n");
	UART_Print("\nSelecteaza o optiune:\n");
}

// Functie pentru aprinderea LED-urilor in functie de conditii
void setLEDs(uint8_t verde, uint8_t rosu) {
	if (verde) PORTB |= (1 << led_verde); else PORTB &= ~(1 << led_verde);
	if (rosu) PORTB |= (1 << led_rosu); else PORTB &= ~(1 << led_rosu);
}

// Functie pentru afisarea unui numar float prin UART cu un numar specificat de zecimale
void UART_PrintFloat(double value, int decimals) {
	if (value < 0) {
		UART_Transmit('-'); // Afiseaza minus daca e numar negativ
		value = -value;
	}

	// Partea intreaga
	int integerPart = (int)value;

	// Partea fractionara, rotunjire
	double fractionalPart = (value - integerPart) * pow(10, decimals);
	int fractionalInt = (int)(fractionalPart + 0.5); // Rotunjire corecta

	// Afiseaza partea intreaga
	char buffer[16];
	sprintf(buffer, "%d", integerPart);
	UART_Print(buffer);

	UART_Transmit('.'); // Adauga punctul zecimal

	// Afiseaza partea fractionara
	for (int i = decimals - 1; i > 0 && fractionalInt < pow(10, i); i--) {
		UART_Transmit('0'); // Adauga zerouri daca e nevoie
	}
	sprintf(buffer, "%d", fractionalInt);
	UART_Print(buffer);
}

// Functie care preia comanda si executa optiunea
void handleOption(char cmd) {
	switch (cmd) {
		case '1': { // Citire temperatura NTC
			//  _delay_ms(500); // Asteapta 0.5 secunde pentru citire stabila
			double voltage_ntc = 5.0 * ADC_Read(pinNTC) / 1023 ; // Calcul tensionii de la NTC
			double current = ((5.0 - voltage_ntc) / SERIES_RESISTOR) * 1000.0; // Calcul curent
			double res_ntc = voltage_ntc / (current / 1000.0); // Calcul rezistenta NTC
			double temperature =( 1 / ((log(res_ntc / SERIES_RESISTOR)) / BCOEFFICIENT + (1 / TEMPERATURE_NOMINAL))) - 273.15; // Calcul temperatura in Celsius
			UART_Print("Temperatura NTC: ");
			UART_PrintFloat(temperature, 2); // Afiseaza temperatura cu 2 zecimale
			UART_Print(" C  ");
			UART_Print("Tensiunea: ");
			UART_PrintFloat(voltage_ntc, 2); // Afiseaza tensiunea
			UART_Print(" V  ");
			UART_Print("Curentul: ");
			UART_PrintFloat(current, 2); // Afiseaza curentul
			UART_Print(" mA  ");
			UART_Print("Rezistenta: ");
			UART_PrintFloat(res_ntc, 2); // Afiseaza rezistenta
			UART_Print(" ohmi\n");

			// Control LED in functie de temperatura
			setLEDs(temperature < 30.0, temperature >= 30.0);
			break;
		}

		case '2': { // Citire calitatea aerului cu MQ135
			// _delay_ms(500); // Asteapta 0.5 secunde pentru citire stabila
			double mq135Value = ADC_Read(pinMQ135); // Citeste valoarea de la MQ135
			double airQuality = mq135Value / 10000; // Calcul procent calitate aer
			UART_Print("Calitatea aerului (MQ135): ");
			UART_PrintFloat(airQuality, 3);
			UART_Print("%  ");
			UART_PrintFloat(mq135Value, 0); // Afiseaza valoarea in ppm
			UART_Print(" ppm\n");
			
			// Control LED in functie de calitatea aerului
			setLEDs(airQuality < 0.04, airQuality >= 0.04);
			break;
		}
		case '3': { // Citire concentratie CO cu MQ7
			// _delay_ms(500); // Asteapta 0.5 secunde pentru citire stabila
			double mq7Value = ADC_Read(pinMQ7); // Citeste valoarea de la MQ7
			double coConcentration = mq7Value / 10000 ; // Calcul concentratie CO
			UART_Print("Monoxid de carbon (MQ7): ");
			UART_PrintFloat(coConcentration, 3); // Afiseaza concentratia de CO
			UART_Print("%  ");
			UART_PrintFloat(mq7Value, 0); // Afiseaza valoarea in ppm
			UART_Print(" ppm\n");

			// Control LED in functie de concentratia de CO
			setLEDs(coConcentration < 0.02, coConcentration >= 0.02);
			break;
		}
		case '4': { // Citire umiditate cu DHT11
			// _delay_ms(500); // Asteapta 0.5 secunde pentru citire stabila
			double dhtValue = ADC_Read(pinDHT); // Citeste valoarea de la DHT11
			double humidity = dhtValue / 10.0; // Calcul umiditate
			UART_Print("Umiditatea din aer (DHT): ");
			UART_PrintFloat(humidity, 2); // Afiseaza umiditatea
			UART_Print("%\n");

			// Control LED in functie de umiditate
			setLEDs(humidity < 80.0, humidity >= 80.0);
			break;
		}
		default:
		break;
	}
}

char input;
uint8_t running;

void handleInput() {
	if (UCSR0A & (1 << RXC0)) {
		input = UDR0; // Citeste caracterul din UART
		
		if(input != '\n') { // Pentru a nu citi caracterul ENTER
			if (input == 's') {
				showMenu(); // Reafisam meniul
				running = 0;  // Resetam starea de rulare
				} else if (!running && input >= '1' && input <= '4') {
				// Comanda valida, incepem sa rulam optiunea
				running = 1;
				UART_Print("Optiunea selectata: ");
				UART_Transmit(input); // Afiseaza optiunea aleasa
				UART_Print("\n");
				} else {
				UART_Print("Comanda invalida\n"); // Mesaj de eroare
			}
		}
	}
}

int main(void) {
	// Initializari
	ADC_Init();  // Initializeaza ADC
	UART_Init(9600);  // Initializeaza UART cu baud rate 9600
	DDRB |= (1 << led_verde) | (1 << led_rosu); // Seteaza pin-urile pentru LED-uri ca iesiri

	showMenu();  // Afisam meniul la inceput

	while (1) {
		handleInput();

		// Executa comanda daca `running` este activ
		if (running) {
			handleOption(input); // Executa optiunea aleasa

			_delay_ms(500); // Asteapta 0.5 secunde pentru actualizare

			//Verifica daca s-a primit o comanda noua
			input = UDR0;
			if (input == 's') {
				running = 0; // Iesi din bucla de rulare
				showMenu(); // Reafiseaza meniul
			}
		}
	}
}
