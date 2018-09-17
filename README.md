# Client ESP32 - Progetto Programmazione di Sistema 2017/18
### Gruppo
- Gianluca D'Alleo
- Salvatore Di Cara
- Giorgio Pizzuto
- Vincenzo Topazio

## Configurazione ambiente di sviluppo
* Tutorial base: https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html
* IDE:
Nonostante sia consigliato l'utilizzo di Eclipse, consiglio l'uso di Visual Studio Code poiché offre un terminale integrato semplice da gestire e inoltre offre l'IntelliSense facile da impostare
* Tutorial:
  * https://github.com/VirgiliaBeatrice/esp32-devenv-vscode/blob/master/tutorial.md
  * https://www.youtube.com/watch?v=VPgEc8FUiqI

### Procedura compilazione da terminale
1. apri mingw32
2. cd ~/esp/hello_world  
2a. (opzionale) make menuconfig
3. make flash
4. make monitor (oppure usare PuTTy)

### Errori noti
Problemi con l'installazione di python:  
https://stackoverflow.com/questions/52077195/msys2-installing-a-python-package-fails

## Errori di incompatibilità da C a C++
Inizializzazione di strutture:
```wifi_config_t sta_config={.sta={.ssid="foo",.password="bar, (...) }};```
https://esp32.com/viewtopic.php?f=13&t=1317&p=5942&hilit=c+initializer#p5942 

Attivare gestione errori c++ da make menuconfig

Errore inclusione json.hpp (GIÀ RISOLTO)  
    non è possibile usare std::tostring  
	  aggiungere "CPPFLAGS += -D_GLIBCXX_USE_C99" in component.mk  
	  https://github.com/espressif/esp-idf/issues/1445
	
## Classi principali
Modello
```
 ┌────────┐  
 │ Server │  
 └────────┘  
     ^
     |	
 ┌────────┐       ┌────────┐  
 │ Sender │ ----> │ Record │ 
 └────────┘       └────────┘
     ^		   
     |
┌───────────┐  
│  Sniffer  │  
└───────────┘ 
```
 
 -------
### Server
Gestisce l'interfaccia di comunicazione con il server che ha il compito di elaborare i Record inviati  
L'invio avviene mediante il protocollo definito di seguito:  
	Client:  
	- "INIT [jSON dei dati di inizializzazione]": primo messaggio inviato al server per comunicare il nome del dispositivo esp32  
	- "DATA [jSON dei Record raccolti]": messaggio contenente i Record raccolti  
	Server:  
	- "OK": comunicazione ricevuta correttamente (per adesso solo dopo l'invio del messaggio INIT)  
	- "ERR": errore di ricezione  
	
Ogni listenPeriod la sezione del sistema operativo FreeRTOS dedicata al timer chiamerà una callback contenente la funzione che gestisce l'invio dei Record accumulati durante il listenPeriod  
Sembra più comodo avviare una nuova connessione TCP ogni listenPeriod anziché avviarla una volta sola e lasciarla attiva, ispirandosi al principio di protocollo stateless HTML  
Il protocollo è da perfezionare, in particolare:  
	- Considerare l'introduzione di un messaggio END per terminare la comunicazione  
	- Eventuali modifiche per il miglioramento delle prestazioni  
### Sender
Gestisce la raccolta e l'invio dei Record usando l'interfaccia offerta da Server 
### Record
Struttura che raccoglie le informazioni dei pacchetti ascoltati necessari per l'elaborazione

### Sniffer
Gestisce l'ascolto di pacchetti in modalità promiscua e usa l'interfaccia offerta da Sender per accumulare i Record da inviare al server

## Classi di supporto
```json.hpp```  
Classe che offre un'interfaccia semplice per la conversione di oggetti in jSON  
Documentazione: https://github.com/nlohmann/json

```kolban-ESP32```  
Manuale per ESP32, di particolare importanza la sezione delle librerie C++ aggiuntive da me incluse a partire da pagina 1167  
Le seguenti classi sono prelevate da https://github.com/nkolban/esp32-snippets/tree/master/cpp_utils  
(Alcune hanno subito una leggera modifica per migliorare l'integrazione con il progetto)

```Wifi.h```  
Classe che semplifica l'interfaccia offerta dal sistema ESP32 per la gestione del WiFi  
Errore 'err_t' does not name a type: https://github.com/espressif/esp-idf/issues/2392
aggiungere a dns.h
```
#if ESP_DNS 
	#include "lwip/err.h" 
#endif
```

```WifiEventHandler.h```  
Classe di supporto a ```Wifi.h``` per la definizione di callback da chiamare all'avvenire di certi eventi riguardanti l'interfaccia WiFi

```FreeRTOSTimer.h```  
Classe che semplifica l'interfaccia offerta dal sistema ESP32 per la gestione dei timer  
Necessaria per la definizione di una callback da chiamare alla scadenza del periodo definito  
Documentazione dell'interfaccia originale: https://www.freertos.org/FreeRTOS-timers-xTimerCreate.html  
Errore stack insufficiente: https://esp32.com/viewtopic.php?t=1459  

```FreeRTOS.h```  
Classe che semplifica l'interfaccia offerta dal sistema ESP32 per la gestione della sincronizzazione

```Task.h```  
Classe che semplifica l'interfaccia offerta dal sistema ESP32 per la gestione dei task  

```Socket.h```  
Classe che semplifica l'interfaccia offerta dal sistema ESP32 per la gestione dei Socket  
ATTENZIONE: non lo uso perché porta rogne

```tcp_test.cpp```  
File cpp contenente una funzione per testare una connessione TCP con un server web

## Altri file
```main/component.mk```  
Direttive per il make: contiene una direttiva per l'utilizzo di C++11

```main/Kconfig.projbuild```  
File che permette di definire nuovi parametri da settare mediante il menu chiamabile con ```make menuconfig```  
Ulteriori info: http://www.lucadentella.it/2017/01/23/esp32-7-menuconfig/

```build```  
Cartella creata automaticamente con il comando ```make flash```, non modificare

```.vscode/c_cpp_properties.json```  
File contenente i parametri per il corretto funzionamento di IntelliSense di Visual Studio Code

```.vscode/tasks.json```  
File contenente le definizioni dei "task" offerti da Visual Studio Code per la velocizzazione dei comandi di compilazione dell'applicazione ESP32

```sdkconfig```  
File contenente i parametri di configurazione impostabili con ```make menuconfig```



# Ulteriori informazioni
### Videolezione del progetto
https://video.polito.it/dl/4cd708b427c1386b1fcc74040f49c7db/5b97c9a9/2018/2018_02GRSOV_0219089/Programmazione_di_sistema_lez_24/Programmazione_di_sistema_lez_24.mp4

### Driver
CP210x_Universal_Windows_Driver.rar contiene i driver necessari per la corretta configurazione della interfaccia di comunicazione tra esp32 e Windows (la mia interfaccia risulta essere COM3)


### Shortcuts for rapid compilation
- F4: flash app
- F5: build app
- F8: clean app
- F12: menuconfig

### Configure the project

```
make menuconfig
```

* Set serial port under Serial Flasher Options.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
make flash: fa il build dell'applicazione e la flasha nella scheda
make monitor: avvia l'interfaccia seriale con la scheda
```

(To exit the serial monitor, type ``Ctrl-]`` (nel layout italiano ] corrisponde al tasto +) .)
