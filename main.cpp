#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// =========================
// Wi-Fi Access Point config
// =========================
const char* AP_SSID = "SKIDO_SERVER";
const char* AP_PASSWORD = "12345678";

// =========================
// Hardware pin definitions
// =========================
const uint8_t DHT_PIN = 4;             // Connect DHT22 data pin here
const uint8_t FAN_PWM_PIN = 18;        // PWM output to fan driver/transistor
const uint8_t PWM_CHANNEL = 0;
const uint16_t PWM_FREQUENCY = 25000;
const uint8_t PWM_RESOLUTION = 8;      // 0 to 255
const uint8_t LCD_I2C_ADDRESS = 0x27;  // Change to 0x3F if your LCD uses that address
const uint8_t LCD_COLUMNS = 16;
const uint8_t LCD_ROWS = 2;

// =========================
// Sensor and server objects
// =========================
const uint8_t DHT_TYPE = DHT22;
DHT dht(DHT_PIN, DHT_TYPE);
WebServer server(80);
LiquidCrystal_I2C lcd(LCD_I2C_ADDRESS, LCD_COLUMNS, LCD_ROWS);

// =========================
// Global state
// =========================
float currentTemperature = 25.0f;   // Safe default shown at startup
int fanSpeedPercent = 0;            // 0 to 100%
bool autoMode = true;               // true = temperature control, false = manual slider

// ==========================================================
// Web page stored in flash memory using a raw string literal
// ==========================================================
const char WEBPAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>SKIDO Smart Fan</title>
  <style>
    :root {
      --bg-1: #081120;
      --bg-2: #0f1d33;
      --card: rgba(255, 255, 255, 0.12);
      --card-border: rgba(255, 255, 255, 0.18);
      --text-main: #f4f7fb;
      --text-soft: #b8c6dc;
      --accent: #3dd9b8;
      --accent-2: #4aa3ff;
      --warning: #ffb86b;
      --shadow: 0 20px 60px rgba(0, 0, 0, 0.35);
    }

    * {
      box-sizing: border-box;
      margin: 0;
      padding: 0;
      font-family: "Segoe UI", Tahoma, Geneva, Verdana, sans-serif;
    }

    body {
      min-height: 100vh;
      display: flex;
      align-items: center;
      justify-content: center;
      background:
        radial-gradient(circle at top left, rgba(74, 163, 255, 0.25), transparent 35%),
        radial-gradient(circle at bottom right, rgba(61, 217, 184, 0.20), transparent 30%),
        linear-gradient(135deg, var(--bg-1), var(--bg-2));
      color: var(--text-main);
      padding: 20px;
    }

    .dashboard {
      width: 100%;
      max-width: 820px;
      background: var(--card);
      border: 1px solid var(--card-border);
      border-radius: 24px;
      box-shadow: var(--shadow);
      backdrop-filter: blur(14px);
      padding: 28px;
    }

    .header {
      text-align: center;
      margin-bottom: 24px;
    }

    .header h1 {
      font-size: 2rem;
      font-weight: 700;
      letter-spacing: 0.5px;
      margin-bottom: 8px;
    }

    .header p {
      color: var(--text-soft);
      font-size: 0.98rem;
    }

    .grid {
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
      gap: 18px;
      margin-bottom: 22px;
    }

    .card,
    .control-card {
      background: rgba(255, 255, 255, 0.08);
      border: 1px solid rgba(255, 255, 255, 0.08);
      border-radius: 20px;
      padding: 22px;
    }

    .label {
      color: var(--text-soft);
      font-size: 0.92rem;
      margin-bottom: 10px;
    }

    .value {
      font-size: 2.4rem;
      font-weight: 700;
      line-height: 1.1;
    }

    .value small {
      font-size: 1.1rem;
      color: var(--text-soft);
      margin-left: 4px;
    }

    .control-header,
    .mode-row {
      display: flex;
      justify-content: space-between;
      align-items: center;
      gap: 12px;
    }

    .control-header {
      margin-bottom: 18px;
    }

    .mode-row {
      margin-bottom: 20px;
      flex-wrap: wrap;
    }

    .control-header h2,
    .mode-row h3 {
      font-size: 1.15rem;
      font-weight: 600;
    }

    .badge {
      background: rgba(61, 217, 184, 0.14);
      color: #9ef1de;
      border: 1px solid rgba(61, 217, 184, 0.28);
      border-radius: 999px;
      padding: 8px 14px;
      font-size: 0.9rem;
    }

    .mode-badge.manual {
      background: rgba(255, 184, 107, 0.14);
      color: #ffd9ab;
      border-color: rgba(255, 184, 107, 0.28);
    }

    .actions {
      display: flex;
      gap: 12px;
      flex-wrap: wrap;
    }

    button {
      border: none;
      border-radius: 12px;
      padding: 11px 16px;
      font-size: 0.95rem;
      font-weight: 600;
      cursor: pointer;
      transition: transform 0.15s ease, opacity 0.15s ease;
    }

    button:hover {
      transform: translateY(-1px);
      opacity: 0.95;
    }

    .auto-btn {
      background: linear-gradient(135deg, var(--accent), var(--accent-2));
      color: #04101e;
    }

    .manual-btn {
      background: rgba(255, 255, 255, 0.12);
      color: var(--text-main);
      border: 1px solid rgba(255, 255, 255, 0.15);
    }

    .slider-wrap {
      margin: 18px 0 12px;
    }

    input[type="range"] {
      width: 100%;
      appearance: none;
      height: 10px;
      border-radius: 999px;
      background: linear-gradient(90deg, var(--accent), var(--accent-2));
      outline: none;
    }

    input[type="range"]::-webkit-slider-thumb {
      appearance: none;
      width: 24px;
      height: 24px;
      border-radius: 50%;
      background: white;
      border: 3px solid var(--accent-2);
      cursor: pointer;
      box-shadow: 0 6px 18px rgba(0, 0, 0, 0.25);
    }

    input[type="range"]::-moz-range-thumb {
      width: 24px;
      height: 24px;
      border-radius: 50%;
      background: white;
      border: 3px solid var(--accent-2);
      cursor: pointer;
    }

    .footer-text {
      margin-top: 14px;
      color: var(--text-soft);
      font-size: 0.92rem;
      text-align: center;
    }

    .status {
      margin-top: 14px;
      text-align: center;
      color: #ffd8b0;
      min-height: 20px;
      font-size: 0.92rem;
    }

    @media (max-width: 600px) {
      .dashboard {
        padding: 20px;
      }

      .header h1 {
        font-size: 1.7rem;
      }

      .value {
        font-size: 2rem;
      }

      .control-header,
      .mode-row {
        flex-direction: column;
        align-items: flex-start;
      }
    }
  </style>
</head>
<body>
  <main class="dashboard">
    <section class="header">
      <h1>SKIDO Smart Fan</h1>
      <p>ESP32 temperature monitoring and fan speed control dashboard</p>
    </section>

    <section class="grid">
      <div class="card">
        <div class="label">Current Temperature</div>
        <div class="value" id="temperatureValue">25.0<small>&deg;C</small></div>
      </div>

      <div class="card">
        <div class="label">Fan Speed</div>
        <div class="value" id="fanSpeedValue">0<small>%</small></div>
      </div>
    </section>

    <section class="control-card">
      <div class="mode-row">
        <h3>Control Mode</h3>
        <div class="badge mode-badge" id="modeBadge">AUTO</div>
      </div>

      <div class="actions">
        <button class="auto-btn" id="autoButton">Enable Auto Mode</button>
      </div>

      <div class="control-header" style="margin-top: 22px;">
        <h2>Manual Fan Control</h2>
        <div class="badge" id="sliderBadge">0%</div>
      </div>

      <div class="slider-wrap">
        <input type="range" id="fanSlider" min="0" max="100" value="0">
      </div>

      <p class="footer-text">Auto mode follows temperature. Moving the slider switches to manual mode instantly.</p>
      <div class="status" id="statusText"></div>
    </section>
  </main>

  <script>
    const temperatureValue = document.getElementById('temperatureValue');
    const fanSpeedValue = document.getElementById('fanSpeedValue');
    const fanSlider = document.getElementById('fanSlider');
    const sliderBadge = document.getElementById('sliderBadge');
    const statusText = document.getElementById('statusText');
    const modeBadge = document.getElementById('modeBadge');
    const autoButton = document.getElementById('autoButton');

    let sendTimer = null;

    function updateSpeedDisplay(value) {
      fanSpeedValue.innerHTML = value + '<small>%</small>';
      sliderBadge.textContent = value + '%';
      fanSlider.value = value;
    }

    function updateModeDisplay(isAuto) {
      modeBadge.textContent = isAuto ? 'AUTO' : 'MANUAL';
      modeBadge.className = isAuto ? 'badge mode-badge' : 'badge mode-badge manual';
    }

    async function fetchTemperature() {
      try {
        const response = await fetch('/temperature');
        const text = await response.text();
        const parts = text.split(',');
        const value = parseFloat(parts[0]);
        const speed = parseInt(parts[1], 10);
        const mode = parts[2] ? parts[2].trim() : 'AUTO';

        if (!isNaN(value)) {
          temperatureValue.innerHTML = value.toFixed(1) + '<small>&deg;C</small>';
        }

        if (!isNaN(speed)) {
          updateSpeedDisplay(speed);
        }

        updateModeDisplay(mode === 'AUTO');
        statusText.textContent = 'Live data updated';
      } catch (error) {
        statusText.textContent = 'Temperature update failed';
      }
    }

    async function sendFanSpeed(value) {
      try {
        const response = await fetch('/setFan?speed=' + encodeURIComponent(value));
        const text = await response.text();
        updateModeDisplay(false);
        statusText.textContent = text;
      } catch (error) {
        statusText.textContent = 'Could not send fan speed to ESP32';
      }
    }

    async function enableAutoMode() {
      try {
        const response = await fetch('/auto');
        const text = await response.text();
        updateModeDisplay(true);
        statusText.textContent = text;
        fetchTemperature();
      } catch (error) {
        statusText.textContent = 'Could not enable auto mode';
      }
    }

    fanSlider.addEventListener('input', () => {
      const value = fanSlider.value;
      updateSpeedDisplay(value);

      clearTimeout(sendTimer);
      sendTimer = setTimeout(() => {
        sendFanSpeed(value);
      }, 150);
    });

    autoButton.addEventListener('click', enableAutoMode);

    updateModeDisplay(true);
    updateSpeedDisplay(fanSlider.value);
    fetchTemperature();
    setInterval(fetchTemperature, 2500);
  </script>
</body>
</html>
)rawliteral";

// =====================================
// Convert fan percentage into PWM value
// =====================================
void applyFanSpeed(int percent) {
  fanSpeedPercent = constrain(percent, 0, 100);

  // Map 0-100% speed into 0-255 duty cycle for 8-bit PWM
  int dutyCycle = map(fanSpeedPercent, 0, 100, 0, 255);
  ledcWrite(PWM_CHANNEL, dutyCycle);

  Serial.print("Fan speed set to: ");
  Serial.print(fanSpeedPercent);
  Serial.print("%, PWM duty: ");
  Serial.println(dutyCycle);
}

// ==========================================
// Automatic fan control based on temperature
// ==========================================
void applyAutoFanSpeed() {
  if (currentTemperature >= 10 && currentTemperature < 20) {
    applyFanSpeed(50);
  }
  else if (currentTemperature >= 20 && currentTemperature < 29) {
    applyFanSpeed(60);
  }
  else if (currentTemperature >= 29 && currentTemperature < 30) {
    applyFanSpeed(65);
  }
  else if (currentTemperature >= 30 && currentTemperature < 31) {
    applyFanSpeed(70);
  }
  else if (currentTemperature >= 31 && currentTemperature < 32) {
    applyFanSpeed(75);
  }
  else if (currentTemperature >= 32 && currentTemperature < 33) {
    applyFanSpeed(80);
  }
  else if (currentTemperature >= 33 && currentTemperature < 34) {
    applyFanSpeed(85);
  }
  else if (currentTemperature >= 34) {
    applyFanSpeed(100);
  }
  else {
    applyFanSpeed(0);
  }
}

// ======================================
// Show live temperature and fan speed on
// the I2C LCD screen
// ======================================
void updateLCD() {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Temp:");
  lcd.print(currentTemperature, 1);
  lcd.print((char)223);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Fan:");
  lcd.print(fanSpeedPercent);
  lcd.print("% ");
  lcd.print(autoMode ? "A" : "M");
}

// ==============================
// Read DHT22 temperature safely
// ==============================
void updateTemperatureReading() {
  float newTemperature = dht.readTemperature();

  if (isnan(newTemperature)) {
    Serial.println("Failed to read temperature from DHT22");
    return;
  }

  currentTemperature = newTemperature;

  // Apply automatic fan control when auto mode is active.
  if (autoMode) {
    applyAutoFanSpeed();
  }

  updateLCD();

  Serial.print("Temperature: ");
  Serial.print(currentTemperature, 1);
  Serial.print(" C | Fan: ");
  Serial.print(fanSpeedPercent);
  Serial.print("% | Mode: ");
  Serial.println(autoMode ? "AUTO" : "MANUAL");
}

// ==========================
// HTTP route: serve homepage
// ==========================
void handleRoot() {
  server.send(200, "text/html", WEBPAGE);
}

// ============================================
// HTTP route: send temperature, fan and mode
// ============================================
void handleTemperature() {
  String response = String(currentTemperature, 1) + "," + String(fanSpeedPercent) + "," + (autoMode ? "AUTO" : "MANUAL");
  server.send(200, "text/plain", response);
}

// ==================================
// HTTP route: update fan speed value
// ==================================
void handleSetFan() {
  if (!server.hasArg("speed")) {
    server.send(400, "text/plain", "Missing speed parameter");
    return;
  }

  int requestedSpeed = server.arg("speed").toInt();
  requestedSpeed = constrain(requestedSpeed, 0, 100);

  autoMode = false; // Manual slider overrides auto mode.
  applyFanSpeed(requestedSpeed);
  updateLCD();

  server.send(200, "text/plain", "Manual fan: " + String(fanSpeedPercent) + "%");
}

// ==================================
// HTTP route: switch back to auto mode
// ==================================
void handleAutoMode() {
  autoMode = true;
  applyAutoFanSpeed();
  updateLCD();
  server.send(200, "text/plain", "Auto mode enabled");
}

// ===============================
// HTTP route: handle unknown URLs
// ===============================
void handleNotFound() {
  server.send(404, "text/plain", "Page not found");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("Starting SKIDO Smart Fan...");

  // Start DHT22 sensor
  dht.begin();
  Serial.println("DHT22 initialized");

  // Start I2C LCD
  lcd.init();
  lcd.backlight();
  updateLCD();
  Serial.println("I2C LCD initialized");

  // Configure ESP32 PWM output for the fan
  ledcSetup(PWM_CHANNEL, PWM_FREQUENCY, PWM_RESOLUTION);
  ledcAttachPin(FAN_PWM_PIN, PWM_CHANNEL);
  applyFanSpeed(0);
  Serial.println("PWM output initialized");

  // Start ESP32 in Access Point mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASSWORD);

  IPAddress ipAddress = WiFi.softAPIP();
  Serial.print("Access Point started");
  Serial.print(" | SSID: ");
  Serial.print(AP_SSID);
  Serial.print(" | Password: ");
  Serial.println(AP_PASSWORD);
  Serial.print("AP IP address: ");
  Serial.println(ipAddress);

  // Read the first temperature so auto mode starts with real data.
  updateTemperatureReading();

  // Register web server routes
  server.on("/", handleRoot);
  server.on("/temperature", handleTemperature);
  server.on("/setFan", handleSetFan);
  server.on("/auto", handleAutoMode);
  server.onNotFound(handleNotFound);

  // Start server
  server.begin();
  Serial.println("Web server started on port 80");
}

void loop() {
  server.handleClient();

  static unsigned long lastRead = 0;

  if (millis() - lastRead > 2000) {
    updateTemperatureReading();
    lastRead = millis();
  }
}
