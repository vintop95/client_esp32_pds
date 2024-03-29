# Client ESP32 - Progetto Programmazione di Sistema 2017/18
### Gruppo
- Gianluca D'Alleo
- Salvatore Di Cara
- Giorgio Pizzuto
- Vincenzo Topazio

**Server ESP32**: https://github.com/vintop95/server_esp32_pds

### PRIMA DI TUTTO Errori noti
Problemi con l'installazione di python:  
https://stackoverflow.com/questions/52077195/msys2-installing-a-python-package-fails

***ERROR*** A stack overflow in task Tmr Svc has been detected.  
STACK OVERFLOW quando tenti di inviare i pacchetti: risolvere raddoppiando  
```CONFIG_FREERTOS_TIMER_TASK_STACK_DEPTH``` da 2048 a 4096  
https://www.esp32.com/viewtopic.php?t=1459

## Configurazione ambiente di sviluppo
* **Driver**: CP210x_Universal_Windows_Driver.rar contiene i driver necessari per la corretta configurazione della interfaccia di comunicazione tra esp32 e Windows (la mia interfaccia risulta essere COM3)  
SCARICARE I DRIVER DALLA CARTELLA /docs SULLA REPO  
* **MSYS32**  
	1) O riscaricare l'intero framework (da Get Started) per usufruire degli aggiornamenti  
	2) O copiare la cartella già pronta  
* **2) DOWNLOAD MSYS32 (direttorio msys32) GIÀ PRONTO**:
	- Copiare la cartella in ```C:```
	- Avviare ```mingw32.exe``` la prima volta e modificare in ```C:\msys32\home``` la cartella "vince" con il nome della cartella utente appena generata
	- Modificare ```C:\msys32\etc\profile.d\export_idf_path.sh``` sostituendo il path con il tuo nome utente (al posto di ```vince```)
* **1) Tutorial base**: https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html  
	- ATTENZIONE, saltare la parte di configurazione di msys32 nel tutorial e andare direttamente alla configurazione del progetto ```hello-world```  
	SE SI VUOLE PROSEGUIRE CON L'INSTALLAZIONE IN MODO CLASSICO, USARE pacman PER INSTALLARE PYTHON (Come Giuseppe può spiegare)
	- Prima di effettuare ```make menuconfig```, fai ```make clean``` per eliminare i vecchi riferimenti
	- Fai ```make menuconfig``` e nell'interfaccia seleziona ```Serial flasher config``` e modifica la porta in ```COMn``` (vedi in gestione dispositivi dopo installazione driver)
* **IDE**: Nonostante sia consigliato l'utilizzo di Eclipse, consiglio l'uso di Visual Studio Code poiché offre un terminale integrato semplice da gestire e inoltre offre l'IntelliSense facile da impostare
* **Tutorial VS CODE**:
  * https://github.com/VirgiliaBeatrice/esp32-devenv-vscode/blob/master/tutorial.md
  * https://www.youtube.com/watch?v=VPgEc8FUiqI
  * ERRORE INCLUSIONE INTELLISENSE: https://github.com/Microsoft/vscode-cpptools/issues/743  
  	AGGIUNGERE RIGA ``` "C_Cpp.intelliSenseEngine": "Tag Parser" ``` A USER SETTINGS

### Configurazione Client
- ```make menuconfig``` e:
	- modificare interfaccia USB della scheda
	- Attivare gestione errori c++
	- Personalizzare i parametri dell'app NON DA ```make menuconfig``` ma modificando ```params.h```
- Per convenzione la porta di ascolto del server è 7856

### Configurazione Server
Ci sono due modi per testare il client con un server:
1. usa netcat, permette di inviare e ricevere da riga di comando i messaggi nella socket TCP come telnet:  
nella repo è presente la cartella netcat-1.11, fare come segue:  
Copia nc.exe in Windows o Windows\system32
	apri linea di comando
crea server:
	nc -L -p nPorta
client:
	nc ip nPorta 
2. parte server: https://github.com/vintop95/server_esp32_pds
3. usa il server di prova: https://github.com/vintop95/server_esp_test


### Procedura compilazione da terminale
1. apri ```mingw32.exe```
2. ```cd /d(lettera disco)/[percorso_proj]/esp32_pds```  
2a. (opzionale) ```make menuconfig```
3. ```make flash``` (oppure ```make app``` per compilare e non flashare)  
4. ```make monitor``` (oppure usare PuTTy)

## Documentazioni
* **DOCUMENTAZIONI**:
  * ESP-IDF: https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/wifi/esp_wifi.html
  * ESP-wifi.h: https://github.com/espressif/esp-idf/blob/17ac4ba/components/esp32/include/esp_wifi.h
* **TEORIA STRUTTURA PKTS**:  
  * ESP_PROMISCOUS_HEADER_PKT: https://github.com/espressif/esp-idf/issues/1751
  * STRUTTURA_PKT_probe_req: https://mrncciew.com/2014/10/27/cwap-802-11-probe-requestresponse/
  * 802.11_FRAME_FORMAT: https://mrncciew.com/2013/04/24/802-11-frame-format/
  * 802.11_HEADER_SEQ_NUM: https://mrncciew.com/2014/11/01/cwap-mac-header-sequence-control/
  * 802.11_FRAMING_IN_DETAIL: http://osnet.cs.nchu.edu.tw/powpoint/seminar/802.11/802.11Framing%20in%20Detail.pdf
  * 802.11_MAC_FRAME_FORMAT_2: https://witestlab.poly.edu/blog/802-11-wireless-lan-2/
* **WIFI SNIFFER DEMOs**:
  * ESP_sniffer_DEMO: https://github.com/lpodkalicki/blog/blob/master/esp32/016_wifi_sniffer/main/main.c
  * ESP_SNIFFER_WITH_STRUCTURES: https://www.carvesystems.com/blog/advanced-esp-part-one-simple-sniffer/
  * ESP_SNIFFER_WITH_STRUCTURES_2: http://blog.podkalicki.com/esp32-wifi-sniffer/
  * ESP_LUCADENTELLA_DEMOs: https://github.com/lucadentella/esp32-tutorial
* **DOCUMENTAZIONI LIBRERIE AGGIUNTIVE**:
  * ESP_C++_LIBRARIES: https://github.com/nkolban/esp32-snippets/tree/master/cpp_utils
  * JSON_NLOHMANN: https://github.com/nlohmann/json
  * JSON_NLOHMANN_ARRAY: https://nlohmann.github.io/json/classnlohmann_1_1basic__json_aa80485befaffcadaa39965494e0b4d2e.html#aa80485befaffcadaa39965494e0b4d2e
* **TUTORIALS 4 DUMMIES**:  
  * HELLO_WORLD_ESP32_EXPLAINED: https://exploreembedded.com/wiki/Hello_World_with_ESP32_Explained
  * USE_MEMBER_FUNCS_WITH_CALLBACKS: https://www.codeproject.com/Articles/4817/How-to-use-SetTimer-with-callback-to-a-non-static
  * UNIX_CLIENT_WITH_SELECT: https://gist.github.com/Alexey-N-Chernyshov/4634731
  * UNIX_CLIENT_2: https://www.binarytides.com/code-a-simple-socket-client-class-in-c/
  
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
 ┌────────┐        ┌────────┐  
 │ Sender │ <----> │ Record │ 
 └────────┘        └────────┘
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
	- "DATA [jSON dell'array di Record raccolti nel periodo]": messaggio contenente i Record raccolti  
	- "END": chiudi la connessione
	Server:  
	- "OK": comunicazione ricevuta correttamente  
	- "ERR": errore di ricezione  
	
Ogni listenPeriod la sezione del sistema operativo FreeRTOS dedicata al timer chiamerà una callback contenente la funzione che gestisce l'invio dei Record accumulati durante il listenPeriod  
Sembra più comodo avviare una nuova connessione TCP ogni listenPeriod anziché avviarla una volta sola e lasciarla attiva, ispirandosi al principio di protocollo stateless HTML  
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

```WiFi.h```  
Classe che semplifica l'interfaccia offerta dal sistema ESP32 per la gestione del WiFi  
Errore 'err_t' does not name a type: https://github.com/espressif/esp-idf/issues/2392
aggiungere a dns.h
```
#if ESP_DNS 
	#include "lwip/err.h" 
#endif
```

```WiFiEventHandler.h```  
Classe di supporto a ```WiFi.h``` per la definizione di callback da chiamare all'avvenire di certi eventi riguardanti l'interfaccia WiFi

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
EDIT: rimosso poiché contiene configurazioni locali, lasciata una copia


# Ulteriori informazioni
### Videolezione del progetto
LEZIONE 24 2017/18

### Shortcuts for rapid compilation
- F4: flash app
- F5: build app
- F8: clean app
- F12: menuconfig
