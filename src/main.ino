#include <WiFi.h>         //biblioteca para conexão wifi
#include <PubSubClient.h> //biblioteca para conexão MQTT
#include <DHT.h>          //biblioteca para Módulo DHT11/DHT22

// configurações para a rede wifi
const char *ssid = "Rede";
const char *password = "123456789";
// configurações do broker MQTT
const char *mqttServer = "broker.hivemq.com";  // Host do broker MQTT público
const int mqttPort = 1883;                     // Porta do broker MQTT público
const char *mqttTopic = "rayssa/lerSensor"; // Topico do broker MQTT público

//
#define DHTPIN 15         // pino onde o sensor DHT22 está conectado
#define DHTTYPE DHT11     // tipo do sensor
DHT dht(DHTPIN, DHTTYPE); // cria objeto 'dht' com o pino e tipo do sensor

#define LED_WIFI 2 // LED azul onboard do ESP32 (GPIO 2)

// criação dos objetos de rede e MQTT
WiFiClient espClient;           
PubSubClient client(espClient); 

unsigned long lastMsg = 0;  
const long interval = 2000; 

void setup_wifi()
{
    delay(10);
    Serial.println("Conectando ao WiFi...");
    WiFi.begin(ssid, password); 
    while (WiFi.status() != WL_CONNECTED)
    { 
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ WiFi conectado. IP: " + WiFi.localIP().toString());
    digitalWrite(LED_WIFI, HIGH); // acende o LED ao conectar no WiFi
}

// Função para reconectar ao Broker MQTT (Servidor), se desconectado.
void reconnect()
{
    String clientId = "Esp32_" + String(random(0xffff), HEX);
    while (!client.connected())
    {
        Serial.println("Tentando conectar ao MQTT...");
        if (client.connect(clientId.c_str()))
        { 
            Serial.println("Conectado ao Broker.");
            client.subscribe(mqttTopic, 0);
        }
        else
        {
            Serial.println("Falha ao conectar-se... Tentando novamente em 5s...");
            Serial.print("Código da Falha: ");
            Serial.println(client.state()); 
            delay(5000);                    
        }
    }
}

void setup()
{
    Serial.begin(115200);                   
    dht.begin();                            
    pinMode(LED_WIFI, OUTPUT);              // define LED como saída
    digitalWrite(LED_WIFI, LOW);            // começa desligado
    setup_wifi();                           
    client.setServer(mqttServer, mqttPort); 
}

void loop()
{
    // Verifica status da conexão WiFi e controla LED
    if (WiFi.status() == WL_CONNECTED)
    {
        digitalWrite(LED_WIFI, HIGH);
    }
    else
    {
        digitalWrite(LED_WIFI, LOW);
    }

    if (!client.connected())
    {                
        reconnect(); 
    }
    client.loop(); 

    unsigned long now = millis(); 
    if (now - lastMsg > interval)
    {                  
        lastMsg = now; 

        float h = dht.readHumidity();    
        float t = dht.readTemperature(); 

        if (isnan(h) || isnan(t))
        { 
            Serial.println("Erro ao ler Sensor DHT22.");
            return; 
        }

        String payload = "{";
        payload += "\"temperatura\": " + String(t, 1) + ","; 
        payload += "\"umidade\": " + String(h, 1);           
        payload += "}";

        Serial.print("Publicando: "); 
        Serial.println(payload);

        client.publish(mqttTopic, payload.c_str()); 
    }
}
