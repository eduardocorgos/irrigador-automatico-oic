#include <WiFi.h>
#include <PubSubClient.h>

// ==========================================
// 1. CONFIGURAÇÕES DE REDE (Exclusivo Wokwi)
// ==========================================
const char* ssid = "Wokwi-GUEST";
const char* password = ""; 

// ==========================================
// 2. CONFIGURAÇÕES DA NUVEM - UBIDOTS
// ==========================================
const char* mqttBroker = "industrial.api.ubidots.com";
const char* mqttClientName = "HortaWokwi01"; // Nome interno do cliente
const char* ubidotsToken = " "; // <-- COLOQUE SEU TOKEN AQUI
const char* deviceLabel = "esp32";
const char* varUmidade = "umidade_solo";
const char* varBomba = "status_bomba"; // Nova variável para ver no Ubidots se a bomba ligou

// ==========================================
// 3. PINOS DO HARDWARE
// ==========================================
const int pinoSensor = 34; // Onde o Potenciômetro (sensor) está ligado
const int pinoRele = 26;   // Onde o Módulo Relé (bomba) está ligado

// ==========================================
// 4. CALIBRAÇÃO DO SIMULADOR
// ==========================================
// No Wokwi, o potenciômetro vai de 0 a 4095. 
// Vamos simular que 4095 é terra seca e 0 é terra encharcada.
const int valorSeco = 4095;
const int valorMolhado = 0;

WiFiClient espClient;
PubSubClient client(espClient);

// Função para conectar no Wi-Fi virtual
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Conectando-se à rede: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado com sucesso!");
}

// Função para conectar no Ubidots (MQTT)
void reconnect() {
  while (!client.connected()) {
    Serial.print("Tentando conectar ao Ubidots...");
    if (client.connect(mqttClientName, ubidotsToken, "")) {
      Serial.println("Conectado à nuvem!");
    } else {
      Serial.print("Falhou, erro código: ");
      Serial.print(client.state());
      Serial.println(" - Tentando novamente em 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Configura o pino do relé como SAÍDA de energia e garante que inicie desligado
  pinMode(pinoRele, OUTPUT);
  digitalWrite(pinoRele, LOW);

  setup_wifi();
  client.setServer(mqttBroker, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // 1. LER O SENSOR (Potenciômetro no simulador)
  int leituraBruta = analogRead(pinoSensor);
  int umidadePorcentagem = map(leituraBruta, valorSeco, valorMolhado, 0, 100);

  // Trava os limites entre 0 e 100%
  if(umidadePorcentagem < 0) umidadePorcentagem = 0;
  if(umidadePorcentagem > 100) umidadePorcentagem = 100;

  // 2. LÓGICA DE IRRIGAÇÃO (ATUADOR)
  int statusBomba = 0; // 0 = Desligada, 1 = Ligada
  
  if (umidadePorcentagem < 30) {
    digitalWrite(pinoRele, HIGH); // Aciona o relé (Liga a bomba)
    statusBomba = 1;
    Serial.print("[ALERTA] Umidade baixa! BOMBA LIGADA. | ");
  } else {
    digitalWrite(pinoRele, LOW);  // Desaciona o relé (Desliga a bomba)
    statusBomba = 0;
    Serial.print("[OK] Umidade normal. BOMBA DESLIGADA. | ");
  }

  // 3. MOSTRAR NO COMPUTADOR (Monitor Serial)
  Serial.print("Umidade: ");
  Serial.print(umidadePorcentagem);
  Serial.println("%");

  // 4. ENVIAR PARA O UBIDOTS (Nuvem)
  char topic[100];
  char payload[100];
  
  // Monta o caminho e a mensagem com as DUAS variáveis (umidade e status da bomba)
  sprintf(topic, "/v1.6/devices/%s", deviceLabel);
  sprintf(payload, "{\"%s\": %d, \"%s\": %d}", varUmidade, umidadePorcentagem, varBomba, statusBomba);

  client.publish(topic, payload);
  Serial.println("--> Dados de monitoramento enviados para a nuvem!");
  Serial.println("------------------------------------------------");

  // Aguarda 5 segundos antes do próximo ciclo no simulador
  delay(5000); 
}