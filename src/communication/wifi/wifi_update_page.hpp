/**
 * @file    wifi_update_page.hpp
 * @brief   HTML page for OTA firmware update.
 * @author  Lucas Pichon
 * @date    2026-09-25
 */

#pragma once

#include <Arduino.h>

static const char WIFI_UPDATE_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>HomeStation - OTA Update</title>
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

        .back-link {
            display: inline-block;
            margin-bottom: 1.5rem;
            color: #6366f1;
            font-size: 0.875rem;
            text-decoration: none;
        }

        .back-link:hover {
            color: #a5b4fc;
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

        input[type=file] {
            width: 100%;
            padding: 0.6rem 0.75rem;
            background: #0f172a;
            border: 1px solid #475569;
            border-radius: 8px;
            color: #e2e8f0;
            font-size: 0.875rem;
        }

        .upload-button {
            width: 100%;
            margin-top: 0.5rem;
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

        .upload-button:hover {
            background: #4f46e5;
        }

        .upload-button:disabled {
            background: #475569;
            cursor: wait;
        }

        .progress-container {
            margin-top: 1.25rem;
            display: none;
        }

        .progress-label {
            font-size: 0.75rem;
            color: #94a3b8;
            margin-bottom: 0.4rem;
        }

        progress {
            width: 100%;
            height: 8px;
            border-radius: 4px;
            appearance: none;
            background: #0f172a;
            border: 1px solid #475569;
        }

        progress::-webkit-progress-bar {
            background: #0f172a;
            border-radius: 4px;
        }

        progress::-webkit-progress-value {
            background: #6366f1;
            border-radius: 4px;
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

        .warning-notice {
            margin-top: 1.5rem;
            padding: 0.75rem 1rem;
            border-radius: 8px;
            background: #1c1500;
            border: 1px solid #854d0e;
            color: #fde68a;
            font-size: 0.8rem;
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

    <a class="back-link" href="/">&#8592; Back to configuration</a>

    <h1>&#x1F4E6; OTA Update</h1>
    <p class="subtitle">Upload a new firmware binary to update the device.</p>

    <div class="field">
        <label for="file">Firmware file (.bin)</label>
        <input type="file" id="file" accept=".bin">
    </div>

    <button class="upload-button" id="uploadButton" onclick="upload()">
        Upload &amp; Reboot
    </button>

    <div class="progress-container" id="progressContainer">
        <p class="progress-label" id="progressLabel">Uploading... 0%</p>
        <progress id="progress" value="0" max="100"></progress>
    </div>

    <div class="alert" id="alert"></div>

    <div class="warning-notice">
        &#9888;&nbsp; Do not disconnect from the WiFi AP during the update.
        The device will reboot automatically when the update is complete.
    </div>

    <div class="footer">HomeStation OTA Update</div>

</div>

<script>
    function upload() {
        const file = document.getElementById('file').files[0];
        if (!file) {
            showAlert('Please select a firmware file.', 'error');
            return;
        }

        if (!file.name.endsWith('.bin')) {
            showAlert('Invalid file. Please select a .bin file.', 'error');
            return;
        }

        const button = document.getElementById('uploadButton');
        const progressContainer = document.getElementById('progressContainer');
        const progressBar = document.getElementById('progress');
        const progressLabel = document.getElementById('progressLabel');

        button.disabled = true;
        button.textContent = 'Uploading...';
        progressContainer.style.display = 'block';

        const formData = new FormData();
        formData.append('firmware', file);

        const xhr = new XMLHttpRequest();
        xhr.open('POST', '/update');

        xhr.upload.onprogress = e => {
            if (e.lengthComputable) {
                const pct = Math.round((e.loaded / e.total) * 100);
                progressBar.value = pct;
                progressLabel.textContent = 'Uploading... ' + pct + '%';
            }
        };

        xhr.onload = () => {
            if (xhr.status === 200) {
                progressBar.value = 100;
                progressLabel.textContent = 'Upload complete!';
                button.textContent = 'Rebooting...';
                showAlert('Update successful! The device is rebooting.', 'success');
            } else {
                showAlert('Update failed: ' + xhr.responseText, 'error');
                button.disabled = false;
                button.textContent = 'Upload & Reboot';
            }
        };

        xhr.onerror = () => {
            showAlert('Network error during upload.', 'error');
            button.disabled = false;
            button.textContent = 'Upload & Reboot';
        };

        xhr.send(formData);
    }

    function showAlert(message, type) {
        const el = document.getElementById('alert');
        el.textContent = message;
        el.className = 'alert ' + type;
        el.style.display = 'block';
    }
</script>
</body>
</html>
)rawhtml";