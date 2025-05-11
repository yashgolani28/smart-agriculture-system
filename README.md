# 🌱 Intelligent Smart Precision Agriculture System

An IoT and Machine Learning-based hydroponic farming solution that enables real-time environmental monitoring and automated irrigation. It uses ESP32, Firebase, ThingSpeak, and an LSTM model to improve water efficiency and crop yield.

---

## 📸 Demo

> https://github.com/user-attachments/assets/f51d34ea-2663-405d-8a9b-a75b516388c0

---

## 🚀 Features

- Real-time monitoring of pH, soil moisture, temperature, humidity, and light
- Automatic irrigation based on machine learning predictions
- Cloud data storage with Firebase
- Data visualization using ThingSpeak
- Mobile or web interface for manual control

---

## 🧰 Tech Stack

| Layer        | Technology                         |
|--------------|------------------------------------|
| Microcontroller | ESP32                            |
| Sensors      | DHT11, Soil Moisture, LDR, pH      |
| Cloud        | Firebase Realtime Database, ThingSpeak |
| Machine Learning | LSTM using TensorFlow/Keras    |
| Languages    | C++ (Arduino), Python              |

---

## 📂 Folder Structure

```

smart-agriculture-system/
├── firmware/             # ESP32 Arduino code
├── model/                # LSTM training and evaluation
├── firebase\_setup/       # Firebase config and credentials
├── docs/                 # Diagrams and final report
└── README.md

````

---

## ⚙️ Setup Instructions

1. **Clone this repo**:
   ```bash
   git clone https://github.com/yashgolani28/smart-agriculture-system.git
   cd smart-agriculture-system

2. **Firmware Setup**:

   * Install the Arduino IDE
   * Add ESP32 board support in Board Manager
   * Open `firmware/main.ino` and upload it to ESP32

3. **Firebase Configuration**:

   * Set up a Firebase project
   * Get your API credentials and insert them in the code

4. **ThingSpeak Configuration**:

   * Create a channel and add the API key in `firmware/main.ino`

5. **ML Model Execution**:

   * Navigate to `model/`
   * Run the training script:

     ```bash
     python train_lstm.py
     ```

---

## 📊 System Architecture

![image](https://github.com/user-attachments/assets/11563928-8b6d-4af4-b9dc-f745394707ad)

---

## 🧪 Sample Data Flow

* Sensor reads → Sent to ESP32 → Pushed to Firebase + ThingSpeak
* LSTM model predicts irrigation need based on time-series data
* If condition met, pump activated via relay

---

## Prototype

<div align="center">
  <img src="https://github.com/user-attachments/assets/fb9c9b3c-c3db-4039-bc6a-690acd916fa5" alt="Smart Agri System" width="400"/>
</div>

---

## 🧑‍💻 Contributors

* Akshat Singh
* Bibi Mehnoor
* Himonish Gupta
* Yash Golani

**Guide**: Dr. Rajesh Bisane, Symbiosis Institute of Technology

---

## 📬 Contact

Feel free to open an issue or contact us via LinkedIn or GitHub for collaboration!

```


