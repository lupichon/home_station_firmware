/**
 * @file    wifi_monitoring_page.hpp
 * @brief   HTML page displaying live sensor measurements, refreshed every second.
 * @author  Lucas Pichon
 * @date    2026-08-24
 */

#pragma once

#include <Arduino.h>

// ============================================================
// HTML page
// ============================================================

static const char WIFI_MONITORING_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">

    <title>HomeStation - Monitoring</title>

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

        table {
            width: 100%;
            border-collapse: collapse;
        }

        td {
            padding: 0.65rem 0.25rem;
            border-bottom: 1px solid #334155;
            font-size: 0.9rem;
        }

        tr:last-child td {
            border-bottom: none;
        }

        td.key {
            color: #94a3b8;
        }

        td.value {
            text-align: right;
            font-weight: 600;
            color: #e2e8f0;
            font-variant-numeric: tabular-nums;
        }

        td.value.active {
            color: #f59e0b;
        }

        td.value.na {
            color: #475569;
            font-weight: 400;
        }

        .status-row {
            margin-top: 1.25rem;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 0.4rem;
            font-size: 0.75rem;
            color: #64748b;
        }

        .status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background: #475569;
            transition: background 0.2s;
        }

        .status-dot.ok {
            background: #22c55e;
        }

        .status-dot.error {
            background: #ef4444;
        }

        .back-link {
            display: block;
            margin-top: 1.5rem;
            text-align: center;
            font-size: 0.85rem;
            color: #818cf8;
            text-decoration: none;
        }

        .back-link:hover {
            color: #a5b4fc;
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

    <h1>&#128202; Monitoring</h1>
    <p class="subtitle">Live sensor data, refreshed every second</p>

    <table id="dataTable">
        <tr><td class="key">Temperature</td><td class="value" id="v-temperature">&mdash;</td></tr>
        <tr><td class="key">Humidity</td><td class="value" id="v-humidity">&mdash;</td></tr>
        <tr><td class="key">Pressure</td><td class="value" id="v-pressure">&mdash;</td></tr>
        <tr><td class="key">Luminosity</td><td class="value" id="v-luminosity">&mdash;</td></tr>
        <tr><td class="key">CO2</td><td class="value" id="v-co2">&mdash;</td></tr>
        <tr><td class="key">Gas (raw)</td><td class="value" id="v-gasRaw">&mdash;</td></tr>
        <tr><td class="key">VOC index</td><td class="value" id="v-vocIndex">&mdash;</td></tr>
        <tr><td class="key">NOx index</td><td class="value" id="v-noxIndex">&mdash;</td></tr>
        <tr><td class="key">Motion</td><td class="value" id="v-motion">&mdash;</td></tr>
        <tr><td class="key">Sound</td><td class="value" id="v-sound">&mdash;</td></tr>
        <tr><td class="key">Obstacle</td><td class="value" id="v-obstacle">&mdash;</td></tr>
        <tr><td class="key">Vibration</td><td class="value" id="v-vibration">&mdash;</td></tr>
    </table>

    <div class="status-row">
        <span class="status-dot" id="statusDot"></span>
        <span id="statusText">Loading...</span>
    </div>

    <a class="back-link" href="/">&larr; Back to home</a>

    <div class="footer">HomeStation Access Point</div>

</div>

<script>

    // ========================================================
    // Field formatting
    // ========================================================

    function setFloat(id, value, unit) {
        const el = document.getElementById(id);
        if (value === null || value === undefined) {
            el.textContent = 'N/A';
            el.className = 'value na';
        } else {
            el.textContent = value.toFixed(2) + ' ' + unit;
            el.className = 'value';
        }
    }

    function setInt(id, value, unit) {
        const el = document.getElementById(id);
        el.textContent = value + ' ' + unit;
        el.className = 'value';
    }

    function setBool(id, value) {
        const el = document.getElementById(id);
        el.textContent = value ? 'Detected' : 'Clear';
        el.className = 'value' + (value ? ' active' : '');
    }


    // ========================================================
    // Refresh live measurement
    // ========================================================

    function renderData(json) {
        setFloat('v-temperature', json.temperature, '&deg;C'.replace('&deg;','\u00b0'));
        setFloat('v-humidity',    json.humidity,    '%');
        setFloat('v-pressure',    json.pressure,    'hPa');
        setFloat('v-luminosity',  json.luminosity,  'lx');
        setInt('v-co2',           json.co2,         'ppm');
        setInt('v-gasRaw',        json.gasRaw,      '');
        setInt('v-vocIndex',      json.vocIndex,    '');
        setInt('v-noxIndex',      json.noxIndex,    '');
        setBool('v-motion',       !!json.motion);
        setBool('v-sound',        !!json.sound);
        setBool('v-obstacle',     !!json.obstacle);
        setBool('v-vibration',    !!json.vibration);
    }

    function setStatus(ok, message) {
        const dot = document.getElementById('statusDot');
        const text = document.getElementById('statusText');
        dot.className = 'status-dot ' + (ok ? 'ok' : 'error');
        text.textContent = message;
    }

    function refresh() {
        fetch('/api/measurement')
            .then(r => { if (!r.ok) throw new Error(); return r.json(); })
            .then(json => {
                renderData(json);
                setStatus(true, 'Updated ' + new Date().toLocaleTimeString());
            })
            .catch(() => setStatus(false, 'Connection error'));
    }

    refresh();
    setInterval(refresh, 1000);

</script>

</body>
</html>
)rawhtml";