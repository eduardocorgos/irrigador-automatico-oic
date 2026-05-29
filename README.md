# 🌿 Monitoramento e Irrigação Inteligente de Hortas Urbanas (IoT)
**Autor:** Eduardo Henrique Corgos

Este repositório contém o código-fonte, a documentação de hardware e as especificações de comunicação de um protótipo de Internet das Coisas (IoT) focado em Cidades Inteligentes (ODS 11). O sistema realiza o monitoramento contínuo da umidade do solo e automatiza a irrigação de áreas verdes urbanas, com envio de telemetria em tempo real para a nuvem.

---

## i) Descrição do Funcionamento e Uso (Como Reproduzir)
O sistema funciona de forma autônoma através de ciclos de leitura e atuação. Um sensor capacitivo mede a umidade do solo e envia o sinal analógico para o microcontrolador ESP32. O ESP32 converte esse valor para uma escala percentual (0% a 100%). Se a umidade cair abaixo do limite crítico estabelecido (30%), o microcontrolador aciona um módulo relé que liga uma mini bomba d'água, irrigando a planta até que a umidade volte a um nível seguro. Paralelamente, os dados são enviados para um *dashboard* na nuvem para monitoramento remoto.

**Passo a passo para reprodução:**
1. Realize a montagem do hardware conforme o esquemático detalhado na seção (iii).
2. Crie uma conta na plataforma [Ubidots STEM](https://ubidots.com/stem/) e gere o seu `TOKEN` de API.
3. Instale a IDE do Arduino ou a extensão PlatformIO no VS Code.
4. Instale as bibliotecas `WiFi.h` (nativa do ESP32) e `PubSubClient` (de Nick O'Leary).
5. No código-fonte, insira o nome e a palavra-passe da sua rede Wi-Fi local (`ssid` e `password`) e o seu `TOKEN` do Ubidots.
6. Compile e faça o upload do código para a placa ESP32.

---

## ii) Software Desenvolvido e Documentação de Código
O firmware foi desenvolvido em C/C++ utilizando o paradigma de programação estruturada com a biblioteca `PubSubClient` para gerenciar a fila de mensagens. 

O ficheiro principal (ex: `main.ino`) contém a lógica de leitura analógica do sensor, conversão de escalas (função `map`), controle digital do atuador (relé) e formatação de *payloads* JSON para publicação.

## iii) Descrição do Hardware Utilizado
A camada física foi dimensionada para garantir durabilidade no contacto com a terra e segurança no acionamento da carga.

* **Plataforma de Desenvolvimento:** NodeMCU ESP32. SoC (System on a Chip) dual-core de 3.3V com Wi-Fi nativo e conectores GPIO com conversores analógico-digitais (ADC) integrados.
* **Sensor (Entrada):** Sensor de Humidade de Solo Capacitivo v1.2. Não possui contactos metálicos expostos, prevenindo corrosão. Mede a humidade por variação do dielétrico e fornece saída analógica.
* **Atuador (Saída):** Mini Bomba de água submersível DC 5V (caudal de 80 a 120 L/H).
* **Controlo de Potência:** Módulo Relé 5V (1 Canal). Atua como chave eletromecânica para isolar o circuito de 3.3V do ESP32 do circuito de potência de 5V da bomba.
* **Acessórios:** Protoboard de 400 furos, jumpers fêmea-fêmea e macho-macho, mangueira de silicone (diâmetro interno 6mm) e fonte de alimentação externa 5V. As caixas de acomodação podem ser impressas em 3D utilizando material PLA ou PETG para resistência térmica.

**Mapeamento de Pinos (Pinout):**
| Componente | Pino do Componente | Conexão no ESP32 |
| :--- | :--- | :--- |
| **Sensor Capacitivo** | VCC / GND / AOUT | 3V3 / GND / GPIO 34 (ADC) |
| **Módulo Relé** | VCC / GND / IN | 5V (VIN) / GND / GPIO 26 |
| **Bomba de água (5V)** | Positivo / Negativo | Contacto NO do Relé / GND da Fonte |

---

## iv) Interfaces, Protocolos e Módulos de Comunicação
A topologia de comunicação prescinde de *shields* externos, pois toda a negociação de rede ocorre dentro do módulo embarcado da placa principal.

* **Módulo de Comunicação Físico:** Chip ESP-WROOM-32 integrado, operando na interface Wi-Fi (padrão 802.11 b/g/n) na faixa de frequência de 2.4 GHz.
* **Protocolo de Rede:** O dispositivo utiliza a pilha TCP/IP padrão para se ligar à infraestrutura de encaminhamento local e aceder à WAN (Internet).
* **Protocolo de Mensageria:** Para o tráfego de dados na camada de aplicação, utiliza-se o protocolo **MQTT (Message Queuing Telemetry Transport)**, estabelecendo ligações persistentes e leves (*overhead* reduzido) otimizadas para M2M (Machine-to-Machine) e IoT. O encapsulamento dos dados (*payload*) ocorre estritamente em formato JSON (`{"variavel": valor}`).

---

## v) Comunicação e Controlo via Internet (TCP/IP) e MQTT
Conforme os requisitos do projeto, a solução está totalmente integrada à Internet. 

A arquitetura MQTT implementada opera no modelo *Publish/Subscribe*. O microcontrolador atua exclusivamente como cliente *Publisher*. A cada ciclo de leitura do código (5000 ms), o ESP32 abre um socket TCP/IP através da rede sem fios e autentica-se no servidor (Broker) na nuvem por meio de um *Token* criptográfico. 

O Broker escolhido é a plataforma **Ubidots** (alojado no endereço `industrial.api.ubidots.com` operando na porta padrão não encriptada `1883`). O dispositivo publica as mensagens num "tópico" específico (ex: `/v1.6/devices/esp32`). A plataforma na nuvem recebe o JSON através deste tópico, armazena o histórico da base de dados e renderiza os *dashboards* em tempo real, permitindo a auditoria global do status da humidade e do acionamento da bomba via internet, de qualquer parte do mundo.