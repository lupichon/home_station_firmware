/**
 * @file    wifi_update_page.hpp
 * @brief   HTML page for uploading a new firmware binary (OTA update).
 * @author  Lucas Pichon
 * @date    2026-08-24
 */

#pragma once

#include <Arduino.h>

// ============================================================
// HTML page
// ============================================================

static const char WIFI_UPDATE_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">

    <title>HomeStation - Firmware Update</title>

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
            max-width: 480px;
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

        .dropzone {
            border: 2px dashed #475569;
            border-radius: 8px;
            padding: 1.75rem 1rem;
            text-align: center;
            cursor: pointer;
            transition: border-color 0.2s, background 0.2s;
        }

        .dropzone:hover,
        .dropzone.dragover {
            border-color: #6366f1;
            background: #1e1b4b;
        }

        .dropzone .icon {
            font-size: 1.8rem;
            display: block;
            margin-bottom: 0.5rem;
        }

        .dropzone .filename {
            margin-top: 0.6rem;
            font-size: 0.85rem;
            color: #a5b4fc;
            word-break: break-all;
        }

        .hint {
            font-size: 0.75rem;
            color: #64748b;
            margin-top: 0.4rem;
        }

        input[type="file"] {
            display: none;
        }

        .progress-wrap {
            margin-top: 1.25rem;
            display: none;
        }

        .progress-bar-bg {
            width: 100%;
            height: 10px;
            background: #0f172a;
            border: 1px solid #475569;
            border-radius: 6px;
            overflow: hidden;
        }

        .progress-bar-fill {
            height: 100%;
            width: 0%;
            background: #6366f1;
            transition: width 0.15s ease-out;
        }

        .progress-label {
            margin-top: 0.5rem;
            text-align: center;
            font-size: 0.8rem;
            color: #94a3b8;
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

        .upload-button {
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

        .upload-button:hover {
            background: #4f46e5;
        }

        .upload-button:disabled {
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

        .back-link {
            display: block;
            margin-top: 1.25rem;
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

    <h1>&#11014;&#65039; HomeStation</h1>
    <p class="subtitle">Firmware update</p>

    <div class="section">

        <label class="dropzone" id="dropzone" for="fileInput">
            <span class="icon">&#128190;</span>
            <span id="dropzoneText">Click to select a .bin firmware file</span>
            <div class="filename" id="filename"></div>
        </label>
        <input type="file" id="fileInput" accept=".bin">
        <p class="hint">Only upload firmware built for this device. An invalid file can brick it.</p>

        <div class="progress-wrap" id="progressWrap">
            <div class="progress-bar-bg">
                <div class="progress-bar-fill" id="progressFill"></div>
            </div>
            <div class="progress-label" id="progressLabel">0%</div>
        </div>

    </div>

    <div class="reboot-notice">
        &#x21BA;&nbsp; The device will reboot automatically once the update completes.
    </div>

    <button
        type="button"
        class="upload-button"
        id="uploadButton"
        onclick="upload()"
        disabled
    >
        Upload &amp; Reboot
    </button>

    <div class="alert" id="alert"></div>

    <a class="back-link" href="/">&larr; Back to home</a>

    <div class="footer">HomeStation Firmware Update</div>

</div>

<script>

    // ========================================================
    // File selection
    // ========================================================

    let selectedFile = null;

    const fileInput     = document.getElementById('fileInput');
    const dropzone       = document.getElementById('dropzone');
    const dropzoneText   = document.getElementById('dropzoneText');
    const filenameEl     = document.getElementById('filename');
    const uploadButton   = document.getElementById('uploadButton');
    const progressWrap   = document.getElementById('progressWrap');
    const progressFill   = document.getElementById('progressFill');
    const progressLabel  = document.getElementById('progressLabel');

    fileInput.addEventListener('change', () => {
        if (fileInput.files.length > 0) {
            setFile(fileInput.files[0]);
        }
    });

    dropzone.addEventListener('dragover', (e) => {
        e.preventDefault();
        dropzone.classList.add('dragover');
    });

    dropzone.addEventListener('dragleave', () => {
        dropzone.classList.remove('dragover');
    });

    dropzone.addEventListener('drop', (e) => {
        e.preventDefault();
        dropzone.classList.remove('dragover');
        if (e.dataTransfer.files.length > 0) {
            setFile(e.dataTransfer.files[0]);
        }
    });

    function setFile(file) {
        selectedFile = file;
        filenameEl.textContent = file.name + ' (' + formatBytes(file.size) + ')';
        dropzoneText.textContent = 'Selected file:';
        uploadButton.disabled = false;
        hideAlert();
    }

    function formatBytes(bytes) {
        if (bytes < 1024) return bytes + ' B';
        if (bytes < 1024 * 1024) return (bytes / 1024).toFixed(1) + ' KB';
        return (bytes / (1024 * 1024)).toFixed(2) + ' MB';
    }


    // ========================================================
    // Alert helpers
    // ========================================================

    function showAlert(message, type) {
        const el = document.getElementById('alert');
        el.textContent = message;
        el.className = 'alert ' + type;
        el.style.display = 'block';
    }

    function hideAlert() {
        document.getElementById('alert').style.display = 'none';
    }


    // ========================================================
    // Upload
    // ========================================================

    function upload() {
        if (!selectedFile) return;

        uploadButton.disabled = true;
        uploadButton.textContent = 'Uploading...';
        hideAlert();
        progressWrap.style.display = 'block';
        progressFill.style.width = '0%';
        progressLabel.textContent = '0%';

        const formData = new FormData();
        formData.append('firmware', selectedFile);

        const xhr = new XMLHttpRequest();

        xhr.upload.addEventListener('progress', (e) => {
            if (e.lengthComputable) {
                const percent = Math.round((e.loaded / e.total) * 100);
                progressFill.style.width = percent + '%';
                progressLabel.textContent = percent + '%';
            }
        });

        xhr.addEventListener('load', () => {
            if (xhr.status === 200) {
                progressFill.style.width = '100%';
                progressLabel.textContent = '100%';
                uploadButton.textContent = 'Rebooting... (Refresh the page when the LED is not white)';
                showAlert('Firmware uploaded successfully. The device is rebooting.', 'success');
            } else {
                uploadButton.disabled = false;
                uploadButton.textContent = 'Upload & Reboot';
                showAlert('Update failed: ' + xhr.responseText, 'error');
            }
        });

        xhr.addEventListener('error', () => {
            uploadButton.disabled = false;
            uploadButton.textContent = 'Upload & Reboot';
            showAlert('Network error during upload.', 'error');
        });

        xhr.open('POST', '/update');
        xhr.send(formData);
    }

</script>

</body>
</html>
)rawhtml";