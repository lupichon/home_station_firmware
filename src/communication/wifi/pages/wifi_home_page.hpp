/**
 * @file    wifi_home_page.hpp
 * @brief   HTML home page with navigation to config, update and monitoring pages.
 * @author  Lucas Pichon
 * @date    2026-08-24
 */

#pragma once

#include <Arduino.h>

// ============================================================
// HTML page
// ============================================================

static const char WIFI_HOME_PAGE[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">

    <title>HomeStation</title>

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
            max-width: 420px;
            text-align: center;
        }

        h1 {
            font-size: 1.4rem;
            font-weight: 600;
            margin-bottom: 0.25rem;
        }

        .subtitle {
            color: #94a3b8;
            font-size: 0.875rem;
            margin-bottom: 2rem;
        }

        .nav-link {
            display: flex;
            align-items: center;
            gap: 0.6rem;
            width: 100%;
            padding: 0.75rem 1rem;
            margin-bottom: 0.75rem;
            background: #0f172a;
            border: 1px solid #475569;
            border-radius: 8px;
            color: #e2e8f0;
            font-size: 1rem;
            font-weight: 500;
            text-decoration: none;
            transition: border-color 0.2s, background 0.2s;
        }

        .nav-link:hover {
            border-color: #6366f1;
            background: #1e1b4b;
        }

        .nav-link .icon {
            font-size: 1.1rem;
        }

        .nav-link.primary {
            background: #6366f1;
            border-color: #6366f1;
        }

        .nav-link.primary:hover {
            background: #4f46e5;
            border-color: #4f46e5;
        }

        .footer {
            margin-top: 1.5rem;
            text-align: center;
            font-size: 0.7rem;
            color: #475569;
        }
    </style>
</head>

<body>

<div class="card">

    <h1>&#127968; HomeStation</h1>
    <p class="subtitle">Access point portal</p>

    <a class="nav-link primary" href="/monitoring">
        <span class="icon">&#128202;</span> Monitoring
    </a>

    <a class="nav-link" href="/config">
        <span class="icon">&#9881;&#65039;</span> Configuration
    </a>

    <a class="nav-link" href="/update">
        <span class="icon">&#11014;&#65039;</span> Firmware update
    </a>

    <div class="footer" id="footer">HomeStation Access Point</div>

</div>

<script>
fetch('/api/config')
    .then(r => r.json())
    .then(config => {
        const footer = document.getElementById('footer');
        if (config.firmwareVersion) {
            footer.textContent = 'HomeStation Access Point — v' + config.firmwareVersion;
        }
    })
    .catch(() => {});
</script>

</body>
</html>
)rawhtml";