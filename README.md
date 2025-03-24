# Weather-station
WEATHER STATION FOR MONITORING ENVIRONMENTAL CONDITIONS
Pentru realizarea stației meteo, am folosit un Arduino Uno bazat pe microcontrollerul ATmega328P. Fiecare senzor este conectat la un anumit pin al microcontrollerului pentru a colecta datele relevante:
1.1. TEMPERATURA DIN CAMERĂ – folosind un senzor NTC (Negative Temperature Coefficient). Acest senzor își schimbă rezistența în funcție de temperatură, ceea ce îl face ideal pentru măsurători de precizie (conectat la pinul analogic A0).
1.2. UMIDITATEA – cu ajutorul unui senzor DHT11 (Digital Humidity and Temperature). Acest senzor poate măsura atât umiditatea relativă, cât și temperatura, dar eu îl folosesc în principal pentru umiditate (conectat la pinul digital PD7).
1.3. CALITATEA AERULUI – prin intermediul senzorului MQ135, care este sensibil la diferiți poluanți din aer, cum ar fi amoniacul, dioxidul de carbon și alte substanțe chimice volatile (conectat la pinul analogic A1).
1.4. NIVELUL DE MONOXID DE CARBON – folosind senzorul MQ7, care este optimizat pentru detectarea concentrațiilor de monoxid de carbon, un gaz periculos și inodor (conectat la pinul analogic A2).
Am adăugat și două LED-uri pentru a oferi o indicație vizuală asupra stării mediului:
•
LED verde (conectat la pinul PB0) – Se aprinde când toate condițiile de mediu sunt în limite normale:
o
Temperatura este între -40°C și 30°C.
o
Umiditatea este până în 80%.
o
Calitatea aerului indicată de MQ135 este acceptabilă până în 0.04% (400ppm).
o
Concentrația de monoxid de carbon detectată de MQ7 este până în 0.02% (200ppm).
•
LED roșu (conectat la pinul PB1) – Se aprinde dacă vreuna dintre următoarele condiții este îndeplinită:
o
Temperatura este între 30°C și 125°C.
o
Umiditatea este prea mare (peste 80%).
o
MQ135 indică o calitate slabă a aerului (valori ridicate de poluanți).
o
MQ7 detectează concentrații periculoase de monoxid de carbon.
