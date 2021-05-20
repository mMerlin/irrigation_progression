#ifndef MAIL_SEND_H
#define MAIL_SEND_H

#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <ESP_Mail_Client.h>
#include "time.h"
#include "secrets.h"

extern struct tm timeinfo;
extern char buffer [100];

void sendReport(const char *, const char *);
void smtpCallback(SMTP_Status);
void printLocalTime();
int freshLocalTime();
void useTime(const char *);
void wifiConnect();

#endif
