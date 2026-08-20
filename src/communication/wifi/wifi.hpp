#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "../communication.hpp"

// ============================================================
// HTML page (stored in flash)
// ============================================================

static const char WIFI_CONFIG_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HomeStation - Configuration</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
            background: #0f172a;
            color: #e2e8f0;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 1rem;
        }
        .card {
            background: #1e293b;
            border: 1px solid #334155;
            border-radius: 12px;
            padding: 2rem;
            width: 100%;
            max-width: 420px;
        }
        h1 {
            font-size: 1.4rem;
            font-weight: 600;
            margin-bottom: 0.25rem;
        }
        .subtitle {
            color: #94a3b8;
            font-size: 0.875rem;
            margin-bottom: 1.75rem;
        }
        label {
            display: block;
            font-size: 0.875rem;
            font-weight: 500;
            color: #cbd5e1;
            margin-bottom: 0.4rem;
        }
        input[type="number"] {
            width: 100%;
            padding: 0.6rem 0.75rem;
            background: #0f172a;
            border: 1px solid #475569;
            border-radius: 8px;
            color: #e2e8f0;
            font-size: 1rem;
            outline: none;
            transition: border-color 0.2s;
        }
        input[type="number"]:focus {
            border-color: #6366f1;
        }
        .hint {
            font-size: 0.75rem;
            color: #64748b;
            margin-top: 0.4rem;
            margin-bottom: 1.5rem;
        }
        button {
            width: 100%;
            padding: 0.7rem;
            background: #6366f1;
            color: white;
            border: none;
            border-radius: 8px;
            font-size: 1rem;
            font-weight: 500;
            cursor: pointer;
            transition: background 0.2s;
        }
        button:hover { background: #4f46e5; }
        .alert {
            margin-top: 1.25rem;
            padding: 0.75rem 1rem;
            border-radius: 8px;
            font-size: 0.875rem;
            display: none;
        }
        .alert.success {
            background: #052e16;
            border: 1px solid #166534;
            color: #86efac;
        }
        .alert.error {
            background: #450a0a;
            border: 1px solid #991b1b;
            color: #fca5a5;
        }
        .divider {
            border: none;
            border-top: 1px solid #334155;
            margin: 1.5rem 0;
        }
        .reboot-btn {
            background: #374151;
            margin-top: 0;
        }
        .reboot-btn:hover { background: #4b5563; }
    </style>
</head>
<body>
<div class="card">
    <h1>&#127968; HomeStation</h1>
    <p class="subtitle">Configuration portal</p>

    <label for="utcOffset">UTC Offset</label>
    <input type="number" id="utcOffset" name="utcOffset"
           min="-12" max="14" value="0" step="1">
    <p class="hint">Integer value between -12 and +14 (e.g. 2 for UTC+2)</p>

    <button onclick="save()">Save & Apply</button>
    <div class="alert" id="alert"></div>

    <hr class="divider">
    <button class="reboot-btn" onclick="reboot()">&#128260; Reboot device</button>
</div>

<script>
    // Load current value on page open
    fetch('/api/config')
        .then(r => r.json())
        .then(d => {
            document.getElementById('utcOffset').value = d.utcOffset ?? 0;
        });

    function showAlert(msg, type) {
        const el = document.getElementById('alert');
        el.textContent = msg;
        el.className = 'alert ' + type;
        el.style.display = 'block';
        setTimeout(() => el.style.display = 'none', 3500);
    }

    function save() {
        const val = parseInt(document.getElementById('utcOffset').value);
        if (isNaN(val) || val < -12 || val > 14) {
            showAlert('Invalid value. Must be between -12 and +14.', 'error');
            return;
        }
        fetch('/api/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify({ utcOffset: val })
        })
        .then(r => r.json())
        .then(d => {
            if (d.ok) showAlert('Saved successfully!', 'success');
            else      showAlert('Error saving: ' + (d.error || 'unknown'), 'error');
        })
        .catch(() => showAlert('Network error.', 'error'));
    }

    function reboot() {
        fetch('/api/reboot', { method: 'POST' })
            .then(() => showAlert('Rebooting…', 'success'))
            .catch(() => {});
    }
</script>
</body>
</html>
)rawhtml";


// ============================================================
// WiFiCommunication class
// ============================================================

class WiFiCommunication : public Communication
{
    private:
        String wifiApSSID;
        String wifiApPassword;
        WebServer server;

        // Pointer to the system clock's UTC offset so the GET endpoint can read it
        int8_t* utcOffsetRef;

        // Callback called when the user submits a new UTC offset via the web UI
        // Signature: void(int8_t newOffset)
        std::function<void(int8_t)> onUtcOffsetChanged;

        void handleRoot();
        void handleGetConfig();
        void handlePostConfig();
        void handleReboot();
        void handleNotFound();

    public:
        WiFiCommunication();
        void configure(const String& ssid, const String& password);
        bool begin() override;
        void loop();
        bool send(uint8_t* data, size_t size) override;
        void setUtcOffsetTarget(int8_t* ref, std::function<void(int8_t)> cb);
};


// ============================================================
// Constructor
// ============================================================

inline WiFiCommunication::WiFiCommunication()
    : server(80), 
      utcOffsetRef(nullptr), 
      Communication()
{
}

void WiFiCommunication::configure(const String& ssid, const String& password)
{
    wifiApSSID = ssid;
    wifiApPassword = password;
}

// ============================================================
// Initialization
// ============================================================

inline bool WiFiCommunication::begin()
{
    WiFi.mode(WIFI_AP);

    bool ok = WiFi.softAP(wifiApSSID.c_str(), wifiApPassword.c_str());
    if (!ok)
    {
        initialized = false;
        return false;
    }

    // Route registration
    server.on("/",           HTTP_GET,  [this]() { handleRoot();      });
    server.on("/api/config", HTTP_GET,  [this]() { handleGetConfig(); });
    server.on("/api/config", HTTP_POST, [this]() { handlePostConfig();});
    server.on("/api/reboot", HTTP_POST, [this]() { handleReboot();    });
    server.onNotFound(       [this]() { handleNotFound(); });

    server.begin();

    initialized = true;
    return true;
}


// ============================================================
// Loop (call every iteration)
// ============================================================

inline void WiFiCommunication::loop()
{
    if (!initialized) return;
    server.handleClient();
}


// ============================================================
// send() — not used for AP/config mode, required by base class
// ============================================================

inline bool WiFiCommunication::send(uint8_t* data, size_t size)
{
    return false;
}


// ============================================================
// UTC offset wiring
// ============================================================

inline void WiFiCommunication::setUtcOffsetTarget(int8_t* ref, std::function<void(int8_t)> cb)
{
    utcOffsetRef      = ref;
    onUtcOffsetChanged = cb;
}


// ============================================================
// HTTP handlers
// ============================================================

inline void WiFiCommunication::handleRoot()
{
    server.send_P(200, "text/html", WIFI_CONFIG_PAGE);
}

inline void WiFiCommunication::handleGetConfig()
{
    int8_t current = (utcOffsetRef != nullptr) ? *utcOffsetRef : 0;

    String json = "{\"utcOffset\":";
    json += String((int)current);
    json += "}";

    server.send(200, "application/json", json);
}

inline void WiFiCommunication::handlePostConfig()
{
    String body = server.arg("plain");

    // Minimal JSON parsing — look for "utcOffset":<value>
    int idx = body.indexOf("\"utcOffset\"");
    if (idx == -1)
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"missing utcOffset\"}");
        return;
    }

    int colon = body.indexOf(':', idx);
    if (colon == -1)
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"malformed JSON\"}");
        return;
    }

    String valStr = body.substring(colon + 1);
    valStr.trim();

    // Strip trailing } or ,
    valStr.replace("}", "");
    valStr.replace(",", "");
    valStr.trim();

    int val = valStr.toInt();
    if (val < -12 || val > 14)
    {
        server.send(400, "application/json", "{\"ok\":false,\"error\":\"out of range\"}");
        return;
    }

    int8_t newOffset = (int8_t)val;

    if (utcOffsetRef != nullptr)
    {
        *utcOffsetRef = newOffset;
    }

    if (onUtcOffsetChanged)
    {
        onUtcOffsetChanged(newOffset);
    }

    server.send(200, "application/json", "{\"ok\":true}");
}

inline void WiFiCommunication::handleReboot()
{
    server.send(200, "application/json", "{\"ok\":true}");
    delay(300);
    ESP.restart();
}

inline void WiFiCommunication::handleNotFound()
{
    server.send(404, "text/plain", "Not found");
}