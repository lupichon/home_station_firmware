/**
 * @file    wifi_status_page.hpp
 * @brief   HTML page displaying full system, sensors and communications status.
 * @author  Lucas Pichon
 * @date    2026-09-26
 */

#pragma once

#include <Arduino.h>

// ============================================================
// HTML page
// ============================================================

static const char WIFI_STATUS_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">

    <title>HomeStation - Status</title>

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
            padding: 1.5rem 1rem;
        }

        .page {
            max-width: 640px;
            margin: 0 auto;
        }

        h1 {
            font-size: 1.4rem;
            font-weight: 600;
            margin-bottom: 0.25rem;
            text-align: center;
        }

        .subtitle {
            color: #94a3b8;
            font-size: 0.875rem;
            margin-bottom: 1.5rem;
            text-align: center;
        }

        .card {
            background: #1e293b;
            border: 1px solid #334155;
            border-radius: 12px;
            padding: 1.5rem;
            margin-bottom: 1rem;
        }

        .card-title {
            font-size: 0.95rem;
            font-weight: 600;
            color: #f1f5f9;
            margin-bottom: 1rem;
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        table {
            width: 100%;
            border-collapse: collapse;
        }

        td {
            padding: 0.5rem 0.25rem;
            border-bottom: 1px solid #334155;
            font-size: 0.875rem;
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

        .dot {
            display: inline-block;
            width: 10px;
            height: 10px;
            border-radius: 50%;
            background: #475569;
            margin-right: 0.5rem;
            vertical-align: middle;
        }

        .dot.ok {
            background: #22c55e;
        }

        .dot.error {
            background: #ef4444;
        }

        .dot.warning {
            background: #f59e0b;
        }

        .row-inline {
            display: flex;
            align-items: center;
            justify-content: space-between;
            padding: 0.5rem 0.25rem;
            border-bottom: 1px solid #334155;
            font-size: 0.875rem;
        }

        .row-inline:last-child {
            border-bottom: none;
        }

        .row-label {
            display: flex;
            align-items: center;
        }

        .row-name {
            color: #e2e8f0;
        }

        .row-state {
            color: #94a3b8;
            font-size: 0.8rem;
        }

        .comm-block {
            padding: 0.75rem 0.25rem;
            border-bottom: 1px solid #334155;
        }

        .comm-block:last-child {
            border-bottom: none;
        }

        .comm-header {
            display: flex;
            align-items: center;
            justify-content: space-between;
            margin-bottom: 0.4rem;
        }

        .comm-name {
            font-weight: 600;
            font-size: 0.9rem;
        }

        .comm-details {
            color: #94a3b8;
            font-size: 0.78rem;
            display: flex;
            flex-wrap: wrap;
            gap: 0.75rem;
        }

        .refresh-row {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 0.4rem;
            font-size: 0.75rem;
            color: #64748b;
            margin: 1rem 0;
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
            margin-top: 1rem;
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

<div class="page">

    <h1>&#128202; System Status</h1>
    <p class="subtitle">Full diagnostic, refreshed every 2 seconds</p>

    <!-- System -->
    <div class="card">
        <div class="card-title">&#128421;&#65039; System</div>
        <table id="systemTable"></table>
    </div>

    <!-- Memory -->
    <div class="card">
        <div class="card-title">&#129504; Memory</div>
        <table id="memoryTable"></table>
    </div>

    <!-- Flash -->
    <div class="card">
        <div class="card-title">&#128190; Flash</div>
        <table id="flashTable"></table>
    </div>

    <!-- Clock -->
    <div class="card">
        <div class="card-title">&#128337; Clock</div>
        <table id="clockTable"></table>
    </div>

    <!-- Alarm -->
    <div class="card">
        <div class="card-title">&#128276; Alarm</div>
        <table id="alarmTable"></table>
    </div>

    <!-- Communications -->
    <div class="card">
        <div class="card-title">&#128225; Communications</div>
        <div id="commsList"></div>
    </div>

    <!-- Sensors -->
    <div class="card">
        <div class="card-title">&#127777;&#65039; Sensors</div>
        <div id="sensorsList"></div>
    </div>

    <div class="refresh-row">
        <span class="status-dot" id="statusDot"></span>
        <span id="statusText">Loading...</span>
    </div>

    <a class="back-link" href="/">&larr; Back to home</a>

    <div class="footer">HomeStation Access Point</div>

</div>

<script>

    // ========================================================
    // Formatting helpers
    // ========================================================

    function formatUptime(ms) {
        let totalSec = Math.floor(ms / 1000);
        const days = Math.floor(totalSec / 86400);
        totalSec %= 86400;
        const hours = Math.floor(totalSec / 3600);
        totalSec %= 3600;
        const minutes = Math.floor(totalSec / 60);
        const seconds = totalSec % 60;

        let parts = [];
        if (days > 0) parts.push(days + 'd');
        if (hours > 0 || days > 0) parts.push(hours + 'h');
        parts.push(minutes + 'm');
        parts.push(seconds + 's');
        return parts.join(' ');
    }

    function formatBytes(bytes) {
        if (bytes < 1024) return bytes + ' B';
        if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
        return (bytes / (1024 * 1024)).toFixed(2) + ' MB';
    }

    function formatEpoch(epoch) {
        if (!epoch || epoch === 0) return 'Not set';
        const d = new Date(epoch * 1000);
        return d.toLocaleString();
    }

    function addRow(table, key, value) {
        const row = table.insertRow();
        const keyCell = row.insertCell();
        const valueCell = row.insertCell();
        keyCell.className = 'key';
        valueCell.className = 'value';
        keyCell.textContent = key;
        valueCell.textContent = value;
    }


    // ========================================================
    // Section renderers
    // ========================================================

    function renderSystem(system) {
        const table = document.getElementById('systemTable');
        table.innerHTML = '';
        addRow(table, 'Uptime', formatUptime(system.uptimeMs));
        addRow(table, 'Firmware version', 'v' + system.firmwareVersion);
        addRow(table, 'Reset reason', system.resetReason);
        addRow(table, 'Chip model', system.chipModel);
        addRow(table, 'Chip revision', system.chipRevision);
        addRow(table, 'Chip cores', system.chipCores);
        addRow(table, 'CPU frequency', system.cpuFreqMHz + ' MHz');
        addRow(table, 'SDK version', system.sdkVersion);
    }

    function renderMemory(memory) {
        const table = document.getElementById('memoryTable');
        table.innerHTML = '';
        addRow(table, 'Free heap', formatBytes(memory.freeHeap));
        addRow(table, 'Total heap', formatBytes(memory.heapSize));
        addRow(table, 'Min free heap (ever)', formatBytes(memory.minFreeHeap));
        addRow(table, 'Max alloc block', formatBytes(memory.maxAllocHeap));
    }

    function renderFlash(flash) {
        const table = document.getElementById('flashTable');
        table.innerHTML = '';
        addRow(table, 'Flash chip size', formatBytes(flash.flashChipSize));
        addRow(table, 'Sketch size', formatBytes(flash.sketchSize));
        addRow(table, 'Free sketch space', formatBytes(flash.freeSketchSpace));
    }

    function renderClock(clock) {
        const table = document.getElementById('clockTable');
        table.innerHTML = '';
        addRow(table, 'Synchronized', clock.synchronized ? 'Yes' : 'No');
        if (clock.synchronized) {
            addRow(table, 'Synced since', clock.synchronizedSinceS + 's');
        }
    }

    function renderAlarm(alarm) {
        const table = document.getElementById('alarmTable');
        table.innerHTML = '';
        addRow(table, 'Armed', alarm.armed ? 'Yes' : 'No');
        addRow(table, 'Ringing', alarm.ringing ? 'Yes' : 'No');
        addRow(table, 'Target', formatEpoch(alarm.targetEpoch));
    }

    function renderSensors(sensors) {
        const container = document.getElementById('sensorsList');
        container.innerHTML = '';

        sensors.forEach(sensor => {
            const row = document.createElement('div');
            row.className = 'row-inline';

            const label = document.createElement('div');
            label.className = 'row-label';

            const dot = document.createElement('span');
            dot.className = 'dot ' + (sensor.lastReadOK ? 'ok' : 'error');

            const name = document.createElement('span');
            name.className = 'row-name';
            name.textContent = sensor.name;

            label.appendChild(dot);
            label.appendChild(name);

            const state = document.createElement('span');
            state.className = 'row-state';
            if (!sensor.enabled) {
                state.textContent = 'Disabled';
            } else if (!sensor.initialized) {
                state.textContent = 'Init failed';
            } else if (!sensor.lastReadOK) {
                state.textContent = 'Read error';
            } else {
                state.textContent = 'OK';
            }

            row.appendChild(label);
            row.appendChild(state);
            container.appendChild(row);
        });
    }

    function renderComms(comms) {
        const container = document.getElementById('commsList');
        container.innerHTML = '';

        // --- WiFi ---
        const wifiBlock = document.createElement('div');
        wifiBlock.className = 'comm-block';
        wifiBlock.innerHTML = `
            <div class="comm-header">
                <span class="comm-name"><span class="dot ${comms.wifi.enabled ? 'ok' : 'error'}"></span>WiFi</span>
            </div>
            <div class="comm-details">
                <span>Enabled: ${comms.wifi.enabled ? 'Yes' : 'No'}</span>
                <span>Clients: ${comms.wifi.clients}</span>
            </div>
        `;
        container.appendChild(wifiBlock);

        // --- Bluetooth ---
        const btOk = comms.bluetooth.enabled && comms.bluetooth.initialized && comms.bluetooth.lastOK;
        const btBlock = document.createElement('div');
        btBlock.className = 'comm-block';
        btBlock.innerHTML = `
            <div class="comm-header">
                <span class="comm-name"><span class="dot ${comms.bluetooth.enabled ? (btOk ? 'ok' : 'warning') : 'error'}"></span>Bluetooth</span>
            </div>
            <div class="comm-details">
                <span>Enabled: ${comms.bluetooth.enabled ? 'Yes' : 'No'}</span>
                <span>Initialized: ${comms.bluetooth.initialized ? 'Yes' : 'No'}</span>
                <span>Connected: ${comms.bluetooth.connected ? 'Yes' : 'No'}</span>
                <span>Last TX: ${comms.bluetooth.lastOK ? 'OK' : 'Failed'}</span>
            </div>
        `;
        container.appendChild(btBlock);

        // --- LoRaWAN ---
        const loraOk = comms.lorawan.enabled && comms.lorawan.joined && comms.lorawan.lastOK;
        const loraBlock = document.createElement('div');
        loraBlock.className = 'comm-block';
        loraBlock.innerHTML = `
            <div class="comm-header">
                <span class="comm-name"><span class="dot ${comms.lorawan.enabled ? (loraOk ? 'ok' : 'warning') : 'error'}"></span>LoRaWAN</span>
            </div>
            <div class="comm-details">
                <span>Enabled: ${comms.lorawan.enabled ? 'Yes' : 'No'}</span>
                <span>Initialized: ${comms.lorawan.initialized ? 'Yes' : 'No'}</span>
                <span>Joined: ${comms.lorawan.joined ? 'Yes' : 'No'}</span>
                <span>Last TX: ${comms.lorawan.lastOK ? 'OK' : 'Failed'}</span>
            </div>
        `;
        container.appendChild(loraBlock);
    }


    // ========================================================
    // Refresh
    // ========================================================

    function setStatus(ok, message) {
        const dot = document.getElementById('statusDot');
        const text = document.getElementById('statusText');
        dot.className = 'status-dot ' + (ok ? 'ok' : 'error');
        text.textContent = message;
    }

    function refresh() {
        fetch('/api/status')
            .then(r => { if (!r.ok) throw new Error(); return r.json(); })
            .then(data => {
                renderSystem(data.system);
                renderMemory(data.memory);
                renderFlash(data.flash);
                renderClock(data.clock);
                renderAlarm(data.alarm);
                renderComms(data.comms);
                renderSensors(data.sensors);
                setStatus(true, 'Updated ' + new Date().toLocaleTimeString());
            })
            .catch(() => setStatus(false, 'Connection error'));
    }

    refresh();
    setInterval(refresh, 2000);

</script>

</body>
</html>
)rawhtml";