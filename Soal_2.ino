#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <DHT.h>

// Konfigurasi pin dan jenis sensor DHT
#define DHTPIN 2          // Pin data sensor DHT22
#define DHTTYPE DHT22     // Jenis sensor yang digunakan
#define LED_PIN 3         // Pin LED

DHT dht(DHTPIN, DHTTYPE);

// Pengaturan untuk Ethernet Shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Alamat MAC dari Ethernet Shield
IPAddress server(192, 168, 1, 100);                 // IP address dari MQTT broker

// Inisialisasi Ethernet client dan MQTT client
EthernetClient ethClient;
PubSubClient client(ethClient);

// Fungsi callback untuk menerima pesan dari MQTT
void callback(char* topic, byte* payload, unsigned int length) {
  // Memproses pesan yang diterima di sini jika diperlukan
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  // Memulai komunikasi serial
  Serial.begin(9600);

  // Memulai sensor DHT22
  dht.begin();

  // Memulai Ethernet Shield
  Ethernet.begin(mac);

  // Menunggu hingga koneksi jaringan berhasil
  while (Ethernet.localIP() == INADDR_NONE) {
    delay(1000);
    Serial.println("Menghubungkan ke jaringan..");
  }

  // Konfigurasi MQTT client
  client.setServer(server, 1883);
  client.setCallback(callback);

  // Menghubungkan ke broker MQTT
  while (!client.connected()) {
    Serial.println("Menghubungkan ke MQTT broker..");
    if (client.connect("ArduinoClient")) {
      Serial.println("Terhubung ke MQTT broker");
    } else {
      Serial.print("Gagal terhubung, ");
      Serial.print(client.state());
      Serial.println(" Coba lagi dalam 5 detik");
      delay(5000);
    }
  }
}

void loop() {
  // Pastikan terhubung ke MQTT broker
  if (!client.connected()) {
    while (!client.connected()) {
      Serial.println("Menghubungkan kembali ke MQTT broker...");
      if (client.connect("ArduinoClient")) {
        Serial.println("Terhubung ke MQTT broker");
      } else {
        Serial.print("Gagal terhubung, rc=");
        Serial.print(client.state());
        Serial.println(" Coba lagi dalam 5 detik");
        delay(5000);
      }
    }
  }
  client.loop();

  // Membaca data dari sensor DHT22
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  // Memeriksa apakah pembacaan sensor berhasil
  if (isnan(suhu) || isnan(kelembapan)) {
    Serial.println("Gagal membaca dari sensor DHT!");
    return;
  }

  // Menampilkan data ke serial monitor
  Serial.print("Suhu: ");
  Serial.print(suhu);
  Serial.print(" °C, Kelembapan: ");
  Serial.print(kelembapan);
  Serial.println(" %");

  // Mengirim data suhu ke MQTT broker
  String suhuString = String(suhu);
  client.publish("daq1/suhu", suhuString.c_str());

  // Mengirim data kelembapan ke MQTT broker
  String kelembapanString = String(kelembapan);
  client.publish("daq1/kelembapan", kelembapanString.c_str());

  // Memeriksa apakah suhu melebihi ambang batas
  if (suhu > 30.0) { // Ambang batas suhu 30°C
    digitalWrite(LED_PIN, HIGH); // Nyalakan LED jika suhu tinggi
    client.publish("daq1/alert", "Suhu tinggi terdeteksi!"); // Kirim peringatan melalui MQTT
  } else {
    digitalWrite(LED_PIN, LOW); // Matikan LED jika suhu normal
  }

  // Jeda sebelum membaca lagi
  delay(2000);
}
