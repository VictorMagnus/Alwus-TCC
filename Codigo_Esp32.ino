#include < RTClib.h > // importa a biblioteca para o esp 32 se comunicar com o módulo de relógio 
#include < WiFi.h >                    //importa biblioteca para conectar esp32 com wifi
#include < IOXhop_FirebaseESP32.h >    //importa biblioteca para esp32 se comunicar com firebase
#include < ArduinoJson.h >             //importa biblioteca para colocar informação no formato json, utilizado no firebase (intalar versão 5 para não ocorrer conflitos futuros)
#include < string >
#define WIFI_SSID "Internet"                  // Definir o nome da rede wifi 
#define WIFI_PASSWORD "xxxxxxxxx"             // Definir a senha da rede wifi 
#define FIREBASE_HOST "https://alwus-b6978-default-rtdb.firebaseio.com"    // Colocar o link do banco de dados 
#define FIREBASE_AUTH "xxxxxxxxxxxxxxxxxxxxxxx"   // Colocar a senha do banco de dados
#define led 2
#define botao 23
RTC_DS3231 rtc;
int horaAlimentacao1, minutoAlimentacao1; // Definindo as variáveis da primeira alimentação como inteiras
int horaAlimentacao2, minutoAlimentacao2; // Definindo as variáveis da segunda alimentação como inteiras
int horaAtual, minutoAtual; // Variáveis que armazenaram o horário e minutos atuais
int porcao1, porcao2; // Variaveis que armazenaram a porção de ração de cada alimentação
int racao1, racao2; // variável que controla se o sistema alimentou ou não, e para não dar conflito da saída ficar ligando e desligando
int tempo1; // variavel para controlar o tempo de acionamento da primeira alimentação 
int tempo2; // variavel para controlar o tempo de acionamento da segunda alimentação
String estadobtn;
String estadoanterior = "off";

void setup() { // abaixo esta o setup para conexão do esp com o módulo de relógio
    Serial.begin(9600);
    Serial.println();
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);     //inicia comunicação com wifi com rede definica anteriormente  
    Serial.print("Conectando ao wifi");       //imprime "Conectando ao wifi"
    while (WiFi.status() != WL_CONNECTED)     //enquanto se conecta ao wifi fica colocando pontos
    {
        Serial.print(".");
        delay(300);
    }
    Serial.println();                         
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);   //inicia comunicação com firebase definido anteriormente
    Serial.println();          
    if (!rtc.begin()) {
        Serial.println("Não conectado ao módulo RTC3231");
        Serial.flush();
        while (1) delay(10);
    }
    if (rtc.lostPower()) {
        Serial.println("Vamos acertar a hora!");

        //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // descomentar a linha abaixo para setar a data e horário atual da programação
        //rtc.adjust(DateTime(2023, 3, 25, 15, 37, 0)); 
    }
    pinMode(led, OUTPUT);
    pinMode(botao, OUTPUT);
    // fim de definição de horário
    horaAtual = rtc.now().hour(); // Salva a hora atual em uma variável
    minutoAtual = rtc.now().minute(); // Salva o minuto atual em uma variável
}
void loop() {
    // definição do horário, porção e estado atraves do firebase    
    horaAlimentacao1 = (Firebase.getInt("/alimenta/1/hora")); // função get vai no banco de dados, lê a variável e salva em uma variavel
    minutoAlimentacao1 = (Firebase.getInt("/alimenta/1/minuto"));
    porcao1 = (Firebase.getInt("/alimenta/1/Porcao"));
    porcao2 = (Firebase.getInt("/alimenta/2/Porcao"));
    estadobtn = (Firebase.getString("rele/rele"));
    horaAlimentacao2 = (Firebase.getInt("/alimenta/2/hora")); 
    minutoAlimentacao2 = (Firebase.getInt("/alimenta/2/minuto"));
    // fim da definição

     // abaixo esta a condição de acionamento apartir do firebase
    if (estadobtn == "on") {
        digitalWrite(led, HIGH);
    } else {
        digitalWrite(led, LOW);
    }
    if (estadobtn == "off") {
        digitalWrite(led, LOW);
    } else {
        digitalWrite(led, HIGH);
    }
    digitalWrite(botao, HIGH);

    // armazenando o horário atual pelo modulo em uma variavel
    horaAtual = rtc.now().hour();
    minutoAtual = rtc.now().minute();
    Serial.print(tempo1);
    Serial.println();
    Serial.print(porcao1);
    Serial.println();

    // abaixo esta a condição de tempo de acionamento apartir da porção definida pelo usuário
    if (porcao1 <= 200) {
        tempo1 = 30000;
    } else if (porcao1 > 200 && porcao1 <= 300) {
        tempo1 = 40000;
    } else if (porcao1 > 300 && porcao1 <= 400) {
        tempo1 = 50000;
    } else if (porcao1 > 400) {
        tempo1 = 60000;
    }
    if (porcao2 <= 200) {
        tempo2 = 30000;
    } else if (porcao2 > 200 && porcao2 <= 300) {
        tempo2 = 40000;
    } else if (porcao2 > 300 && porcao2 <= 400) {
        tempo2 = 50000;
    } else if (porcao2 > 400) {
        tempo2 = 60000;
    }
    // Abaixo está a programação para habilitar a saída do esp 32
    if (horaAtual == horaAlimentacao1 && minutoAtual == minutoAlimentacao1 && racao1 == 0) {
        digitalWrite(led, HIGH);
        delay(tempo1);
        digitalWrite(led, LOW);
        racao1 = 1; // altera o estado da variável, assim ela não ligará novamente no loop do código    
    } else {
        digitalWrite(led, LOW);
    }
    if (horaAtual == horaAlimentacao2 && minutoAtual == minutoAlimentacao2 && racao2 == 0) {
        digitalWrite(led, HIGH);
        delay(tempo2);
        digitalWrite(led, LOW);
        racao2 = 1; // altera o estado da variável pra 1, assim ela não ligará novamente no loop do código    
    } else {
        digitalWrite(led, LOW);
    }
    // o if abaixo servirá para resetar as variáveis quando for meia noite e assim poderá ligar o sistema novamente
    if (horaAtual == 0 && minutoAtual == 0) {
        racao1 = 0;
        racao2 = 0;
    }
    delay(100);
}