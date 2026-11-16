/**
 * @file    update_api.hpp
 * @brief   HTTP routes to update the firmware over the air.
 * @author  Lucas Pichon
 * @date    2026-11-16
 */

#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <Update.h>
#include <functional>

#include "./pages/wifi_update_page.hpp"


// ============================================================
// UpdateApi class definition
// ============================================================

class UpdateApi
{
    // ── Private members ───────────────────────────────────────────────────
    private:
        std::function<void()> onFinished;  // Called once the upload has been answered

        void handleResult(WebServer& server);
        void handleUpload(WebServer& server);

    // ── Public interface ────────────────────────────────────────────────────
    public:

        /**
         * @brief Set the callback called after the update response has been sent.
         */
        void setOnFinished(std::function<void()> cb);

        /**
         * @brief Register the update page and the firmware upload on /update.
         */
        void install(WebServer& server);
};


// ============================================================
// Implementations of UpdateApi methods
// ============================================================

inline void UpdateApi::setOnFinished(std::function<void()> cb)
{
    onFinished = cb;
}

inline void UpdateApi::install(WebServer& server)
{
    server.on("/update", HTTP_GET,  [&server]() { server.send(200, "text/html", WIFI_UPDATE_PAGE); });
    server.on("/update", HTTP_POST, [this, &server]() { handleResult(server); },
                                    [this, &server]() { handleUpload(server); });
}

inline void UpdateApi::handleResult(WebServer& server)
{
    bool success = !Update.hasError();
    server.send(success ? 200 : 500, "text/plain", success ? "OK" : "Update failed");

    if (onFinished) onFinished();
}

inline void UpdateApi::handleUpload(WebServer& server)
{
    HTTPUpload& upload = server.upload();

    if (upload.status == UPLOAD_FILE_START)
    {
        Serial.printf("OTA: %s\n", upload.filename.c_str());
        if (!Update.begin(UPDATE_SIZE_UNKNOWN))
        {
            Serial.println("OTA: begin failed");
        }
    }
    else if (upload.status == UPLOAD_FILE_WRITE)
    {
        if (Update.write(upload.buf, upload.currentSize) != upload.currentSize)
        {
            Serial.println("OTA: write failed");
        }
    }
    else if (upload.status == UPLOAD_FILE_END)
    {
        if (Update.end(true))
        {
            Serial.printf("OTA: success (%u bytes)\n", upload.totalSize);
        }
        else
        {
            Serial.println("OTA: end failed");
        }
    }
}
