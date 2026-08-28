/**
 * @file    wifi_config_page.hpp
 * @brief   HTML page for configuring WiFi settings.
 * @author  Lucas Pichon
 * @date    2026-08-24
 */

#pragma once

#include <Arduino.h>

// ============================================================
// HTML page 
// ============================================================

static const char WIFI_CONFIG_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">

    <title>HomeStation - Configuration</title>

    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
        }

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
            max-width: 520px;
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

        .section {
            margin-top: 1.75rem;
            padding-top: 1.5rem;
            border-top: 1px solid #334155;
        }

        .section:first-of-type {
            margin-top: 0;
            padding-top: 0;
            border-top: none;
        }

        .section-title {
            font-size: 1rem;
            font-weight: 600;
            color: #f1f5f9;
            margin-bottom: 1rem;
        }

        .field {
            margin-bottom: 1.25rem;
        }

        label {
            display: block;
            font-size: 0.875rem;
            font-weight: 500;
            color: #cbd5e1;
            margin-bottom: 0.4rem;
        }

        input {
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

        input:focus {
            border-color: #6366f1;
        }

        input.invalid {
            border-color: #ef4444;
        }

        .input-row {
            display: flex;
            gap: 0.5rem;
        }

        .input-row input {
            flex: 1;
        }

        .toggle-button {
            width: auto;
            padding: 0.6rem 0.75rem;
            background: #334155;
            color: #cbd5e1;
            border: 1px solid #475569;
            border-radius: 8px;
            font-size: 0.8rem;
            cursor: pointer;
        }

        .toggle-button:hover {
            background: #475569;
        }

        .hint {
            font-size: 0.75rem;
            color: #64748b;
            margin-top: 0.4rem;
        }

        .reboot-notice {
            margin-top: 1.5rem;
            padding: 0.75rem 1rem;
            border-radius: 8px;
            background: #1e1b4b;
            border: 1px solid #3730a3;
            color: #a5b4fc;
            font-size: 0.8rem;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        .save-button {
            width: 100%;
            margin-top: 1.75rem;
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

        .save-button:hover {
            background: #4f46e5;
        }

        .save-button:disabled {
            background: #475569;
            cursor: wait;
        }

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

        .footer {
            margin-top: 1rem;
            text-align: center;
            font-size: 0.7rem;
            color: #475569;
        }
    </style>
</head>

<body>

<div class="card">

    <h1>&#127968; HomeStation</h1>
    <p class="subtitle">Configuration portal</p>


    <!-- =====================================================
         TIME
         ===================================================== -->

    <div class="section">

        <div class="section-title">Time</div>

        <div class="field">
            <label for="utcOffset">UTC Offset</label>
            <input
                type="number"
                id="utcOffset"
                name="utcOffset"
                min="-12"
                max="14"
                step="1"
            >
            <p class="hint">Integer value between -12 and +14. Example: 2 for UTC+2.</p>
        </div>

    </div>


    <!-- =====================================================
         LORAWAN
         ===================================================== -->

    <div class="section">

        <div class="section-title">LoRaWAN</div>

        <div class="field">
            <label for="devEui">Dev EUI</label>
            <input
                type="text"
                id="devEui"
                name="devEui"
                maxlength="16"
                autocomplete="off"
                placeholder="0102030405060708"
            >
            <p class="hint">16 hexadecimal characters.</p>
        </div>

        <div class="field">
            <label for="appEui">App EUI</label>
            <input
                type="text"
                id="appEui"
                name="appEui"
                maxlength="16"
                autocomplete="off"
                placeholder="0102030405060708"
            >
            <p class="hint">16 hexadecimal characters.</p>
        </div>

        <div class="field">
            <label for="appKey">App Key</label>
            <div class="input-row">
                <input
                    type="password"
                    id="appKey"
                    name="appKey"
                    maxlength="32"
                    autocomplete="off"
                    placeholder="00112233445566778899AABBCCDDEEFF"
                >
                <button type="button" class="toggle-button" onclick="togglePassword('appKey', this)">Show</button>
            </div>
            <p class="hint">32 hexadecimal characters.</p>
        </div>

    </div>


    <!-- =====================================================
         BLUETOOTH
         ===================================================== -->

    <div class="section">

        <div class="section-title">Bluetooth</div>

        <div class="field">
            <label for="bleDeviceName">Device Name</label>
            <input
                type="text"
                id="bleDeviceName"
                name="bleDeviceName"
                maxlength="31"
                autocomplete="off"
                placeholder="HomeStation"
            >
        </div>

        <div class="field">
            <label for="serviceUUID">Service UUID</label>
            <input
                type="text"
                id="serviceUUID"
                name="serviceUUID"
                maxlength="36"
                autocomplete="off"
                placeholder="xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
            >
        </div>

        <div class="field">
            <label for="characteristicUUID">Characteristic UUID</label>
            <input
                type="text"
                id="characteristicUUID"
                name="characteristicUUID"
                maxlength="36"
                autocomplete="off"
                placeholder="xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
            >
        </div>

        <div class="field">
            <label for="timeSyncUUID">Time Sync Characteristic UUID</label>
            <input
                type="text"
                id="timeSyncUUID"
                name="timeSyncUUID"
                maxlength="36"
                autocomplete="off"
                placeholder="xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
            >
        </div>

        <div class="field">
            <label for="alarmTargetUUID">Alarm Target Characteristic UUID</label>
            <input
                type="text"
                id="alarmTargetUUID"
                name="alarmTargetUUID"
                maxlength="36"
                autocomplete="off"
                placeholder="xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx"
            >
        </div>

    </div>


    <!-- =====================================================
         WIFI
         ===================================================== -->

    <div class="section">

        <div class="section-title">WiFi Access Point</div>

        <div class="field">
            <label for="wifiApSSID">AP SSID</label>
            <input
                type="text"
                id="wifiApSSID"
                name="wifiApSSID"
                maxlength="32"
                autocomplete="off"
                placeholder="HomeStation"
            >
        </div>

        <div class="field">
            <label for="wifiApPassword">AP Password</label>
            <div class="input-row">
                <input
                    type="password"
                    id="wifiApPassword"
                    name="wifiApPassword"
                    maxlength="64"
                    autocomplete="off"
                    placeholder="Password"
                >
                <button type="button" class="toggle-button" onclick="togglePassword('wifiApPassword', this)">Show</button>
            </div>
        </div>

    </div>

    <!-- =====================================================
         Sensors
         ===================================================== -->

    <div class="section">
        <div class="section-title">Sensors</div>
        <div id="sensorsList"></div>
    </div>

    <!-- =====================================================
         COMMUNICATIONS
         ===================================================== -->
    <div class="section">
        <div class="section-title">Communications</div>

        <label>
            <input type="checkbox" id="commsBluetooth">
            Bluetooth
        </label>
        <br>
        <label>
            <input type="checkbox" id="commsLora">
            LoRaWAN
        </label>
        <br>
        <label>
            <input type="checkbox" id="commsWifi">
            WiFi (Config Portal)
        </label>
        <p class="hint" style="color:#f87171;">
            ⚠️ Disabling WiFi will make this configuration page unreachable after reboot.
            To restart it, press the button 5 seconds.
        </p>
    </div>

    <!-- =====================================================
         REBOOT NOTICE
         ===================================================== -->

    <div class="reboot-notice">
        &#x21BA;&nbsp; Saving will always reboot the device.
        Reconnect to the WiFi AP after reboot.
    </div>


    <!-- =====================================================
         SAVE
         ===================================================== -->

    <button
        type="button"
        class="save-button"
        id="saveButton"
        onclick="save()"
    >
        Save &amp; Reboot
    </button>

    <div class="alert" id="alert"></div>

    <div class="footer">HomeStation Configuration Portal</div>

</div>


<script>

    // ========================================================
    // Helpers
    // ========================================================

    function setValue(id, value) {
        const el = document.getElementById(id);
        if (el) el.value = value ?? '';
    }

    function togglePassword(id, button) {
        const input = document.getElementById(id);
        if (input.type === 'password') {
            input.type = 'text';
            button.textContent = 'Hide';
        } else {
            input.type = 'password';
            button.textContent = 'Show';
        }
    }

    function showAlert(message, type) {
        const el = document.getElementById('alert');
        el.textContent = message;
        el.className = 'alert ' + type;
        el.style.display = 'block';
        if (type === 'error') {
            setTimeout(() => { el.style.display = 'none'; }, 4000);
        }
    }

    function isHex(value, length) {
        return value.length === length && /^[0-9a-fA-F]+$/.test(value);
    }

    function markInvalid(id, invalid) {
        const el = document.getElementById(id);
        if (invalid) el.classList.add('invalid');
        else el.classList.remove('invalid');
    }


    // ========================================================
    // Load configuration
    // ========================================================

    function loadConfig() {
        fetch('/api/config')
            .then(r => { if (!r.ok) throw new Error(); return r.json(); })
            .then(config => {
                setValue('utcOffset',         config.utcOffset ?? 0);
                setValue('devEui',            config.devEui);
                setValue('appEui',            config.appEui);
                setValue('appKey',            config.appKey);
                setValue('bleDeviceName',     config.bleDeviceName);
                setValue('serviceUUID',       config.serviceUUID);
                setValue('characteristicUUID',config.characteristicUUID);
                setValue('timeSyncUUID',      config.timeSyncUUID);
                setValue('alarmTargetUUID',   config.alarmTargetUUID);
                setValue('wifiApSSID',        config.wifiApSSID);
                setValue('wifiApPassword',    config.wifiApPassword);
            })
            .catch(() => showAlert('Unable to load configuration.', 'error'));
    }


    // ========================================================
    // Validate configuration
    // ========================================================

    function validateConfig(config) {

        if (isNaN(config.utcOffset) || config.utcOffset < -12 || config.utcOffset > 14) {
            markInvalid('utcOffset', true);
            showAlert('Invalid UTC offset. Must be between -12 and +14.', 'error');
            return false;
        }
        markInvalid('utcOffset', false);

        if (!isHex(config.devEui, 16)) {
            markInvalid('devEui', true);
            showAlert('Invalid Dev EUI. Must be exactly 16 hexadecimal characters.', 'error');
            return false;
        }
        markInvalid('devEui', false);

        if (!isHex(config.appEui, 16)) {
            markInvalid('appEui', true);
            showAlert('Invalid App EUI. Must be exactly 16 hexadecimal characters.', 'error');
            return false;
        }
        markInvalid('appEui', false);

        if (!isHex(config.appKey, 32)) {
            markInvalid('appKey', true);
            showAlert('Invalid App Key. Must be exactly 32 hexadecimal characters.', 'error');
            return false;
        }
        markInvalid('appKey', false);

        if (config.bleDeviceName.length === 0) {
            markInvalid('bleDeviceName', true);
            showAlert('Bluetooth device name cannot be empty.', 'error');
            return false;
        }
        markInvalid('bleDeviceName', false);

        for (const field of ['serviceUUID', 'characteristicUUID', 'timeSyncUUID', 'alarmTargetUUID']) {
            if (config[field].length === 0) {
                markInvalid(field, true);
                showAlert('Bluetooth UUID fields cannot be empty.', 'error');
                return false;
            }
            markInvalid(field, false);
        }

        if (config.wifiApSSID.length === 0) {
            markInvalid('wifiApSSID', true);
            showAlert('WiFi AP SSID cannot be empty.', 'error');
            return false;
        }
        markInvalid('wifiApSSID', false);

        if (config.wifiApPassword.length === 0) {
            markInvalid('wifiApPassword', true);
            showAlert('WiFi AP password cannot be empty.', 'error');
            return false;
        }
        markInvalid('wifiApPassword', false);

        return true;
    }


    // ========================================================
    // Save configuration
    // ========================================================

    function save() {

        const saveButton = document.getElementById('saveButton');

        const config = {
            utcOffset:          parseInt(document.getElementById('utcOffset').value),
            devEui:             document.getElementById('devEui').value.trim(),
            appEui:             document.getElementById('appEui').value.trim(),
            appKey:             document.getElementById('appKey').value.trim(),
            bleDeviceName:      document.getElementById('bleDeviceName').value.trim(),
            serviceUUID:        document.getElementById('serviceUUID').value.trim(),
            characteristicUUID: document.getElementById('characteristicUUID').value.trim(),
            timeSyncUUID:       document.getElementById('timeSyncUUID').value.trim(),
            alarmTargetUUID:    document.getElementById('alarmTargetUUID').value.trim(),
            wifiApSSID:         document.getElementById('wifiApSSID').value.trim(),
            wifiApPassword:     document.getElementById('wifiApPassword').value,
            enabledSensorsMask: getSensorsMask(),
            enabledCommsMask:   getCommsMask()
        };

        if (!validateConfig(config)) return;

        saveButton.disabled = true;
        saveButton.textContent = 'Saving...';

        fetch('/api/config', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(config)
        })
        .then(r => { if (!r.ok) throw new Error(); return r.json(); })
        .then(result => {
            if (result.ok) {
                saveButton.textContent = 'Rebooting... (Refresh the page when the LED is not white)';
                showAlert(
                    'Configuration saved. The device is rebooting — reconnect to the WiFi AP in a few seconds.',
                    'success'
                );
            } else {
                showAlert('Error: ' + (result.error || 'unknown error'), 'error');
                saveButton.disabled = false;
                saveButton.textContent = 'Save & Reboot';
            }
        })
        .catch(() => {
            showAlert('Network error while saving configuration.', 'error');
            saveButton.disabled = false;
            saveButton.textContent = 'Save & Reboot';
        });
    }

    // ========================================================
    // Set sensors mask
    // ========================================================

    function buildSensorsList(names) {
        const container = document.getElementById('sensorsList');
        container.innerHTML = '';
        names.forEach((name, i) => {
            const label = document.createElement('label');
            const input = document.createElement('input');
            input.type = 'checkbox';
            input.id = 'sensor' + i;
            label.appendChild(input);
            label.appendChild(document.createTextNode(' ' + name));
            container.appendChild(label);
            container.appendChild(document.createElement('br'));
        });
    }

    function setSensorsMask(mask) {
        document.querySelectorAll('#sensorsList input[type=checkbox]')
            .forEach((cb, i) => { cb.checked = (mask & (1 << i)) !== 0; });
    }

    function setCommsMask(mask) {
        document.getElementById('commsBluetooth').checked = (mask & 0x01) !== 0;
        document.getElementById('commsLora').checked      = (mask & 0x02) !== 0;
        document.getElementById('commsWifi').checked      = (mask & 0x04) !== 0;
    }

    function getCommsMask() {
        let mask = 0;
        if (document.getElementById('commsBluetooth').checked) mask |= 0x01;
        if (document.getElementById('commsLora').checked)      mask |= 0x02;
        if (document.getElementById('commsWifi').checked)      mask |= 0x04;
        return mask;
    }

    function getSensorsMask() {
        let mask = 0;
        document.querySelectorAll('#sensorsList input[type=checkbox]')
            .forEach((cb, i) => { if (cb.checked) mask |= (1 << i); });
        return mask;
    }

    function loadAll() {
        fetch('/api/sensors')
            .then(r => { if (!r.ok) throw new Error(); return r.json(); })
            .then(names => {
                buildSensorsList(names);
                return fetch('/api/config');
            })
            .then(r => { if (!r.ok) throw new Error(); return r.json(); })
            .then(config => {
                setValue('utcOffset',          config.utcOffset ?? 0);
                setValue('devEui',             config.devEui);
                setValue('appEui',             config.appEui);
                setValue('appKey',             config.appKey);
                setValue('bleDeviceName',      config.bleDeviceName);
                setValue('serviceUUID',        config.serviceUUID);
                setValue('characteristicUUID', config.characteristicUUID);
                setValue('timeSyncUUID',       config.timeSyncUUID);
                setValue('alarmTargetUUID',    config.alarmTargetUUID);
                setValue('wifiApSSID',         config.wifiApSSID);
                setValue('wifiApPassword',     config.wifiApPassword);
                setSensorsMask(config.enabledSensorsMask ?? 0xFFFF);
                setCommsMask(config.enabledCommsMask ?? 0x07);
            })
            .catch(() => showAlert('Unable to load configuration.', 'error'));
    }

    // ========================================================
    // Load on open
    // ========================================================

    loadAll();

</script>

</body>
</html>
)rawhtml";