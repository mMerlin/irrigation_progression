/**
 * This example will send plain text email.
 *
 * Created by K. Suwatchai (Mobizt)
 *
 * Email: suwatchai@outlook.com
 *
 * Github: https://github.com/mobizt/ESP-Mail-Client
 *
 * Copyright (c) 2021 mobizt
 *
*/

// To send to Gmail port 465 (SSL), less secure app option should be enabled.
// https://myaccount.google.com/lesssecureapps?pli=1

#include "mail_send.h"

struct tm timeinfo;
// strftime reference: https://mkssoftware.com/docs/man3/strftime.3.asp
char buffer [100] = "No data yet";

/** The smtp host name e.g. smtp.gmail.com for GMail or smtp.office365.com for
 * Outlook or smtp.mail.yahoo.com for yahoo mail, log in to your yahoo mail in
 * web browser and generate app password by going to
 * https://login.yahoo.com/account/security/app-passwords/add/confirm?src=noSrc
 * and using the app password as password with your yahoo mail account to login.
 * The google app password sign in is also available
 * https://support.google.com/mail/answer/185833?hl=en
*/
#define SMTP_HOST "smtp.gmail.com"

/** The smtp port e.g.
 * 25  or esp_mail_smtp_port_25
 * 465 or esp_mail_smtp_port_465
 * 587 or esp_mail_smtp_port_587
*/
#define SMTP_PORT 587

/* The SMTP Session object used for Email sending */
SMTPSession smtp;

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status);

void wifiConnect()
{
  Serial.printf("Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(200);
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void sendReport(const char * in_subject, const char * in_body)
{
  Serial.println("Sending report");

  /** Enable the debug via Serial port
   * none debug or 0
   * basic debug or 1
  */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the session config data */
  ESP_Mail_Session session;

  /** ########################################################
   * Some properties of SMTPSession data and parameters passed to the
   * SMTP_Message class methods expect a pointer to constant char
   * i.e. const char*.
   *
   * You may assign a string literal to those parameters and arguments
   * like this:
   *
   * session.login.user_domain = "mydomain.net";
   * session.login.user_domain = String("mydomain.net").c_str();
   *
   * or
   *
   * String domain = "mydomain.net";
   * session.login.user_domain = domain.c_str();
   *
   * And
   *
   * String name = "Jack " + String("dawson");
   * String email = "jack_dawson" + String(123) + "@mail.com";
   *
   * message.addRecipient(name.c_str(), email.c_str());
   *
   * message.addHeader(String("Message-ID: <abcde.fghij@gmail.com>").c_str());
   *
   * or
   *
   * String header = "Message-ID: <abcde.fghij@gmail.com>";
   * message.addHeader(header.c_str());
   *
   * ###########################################################
  */

  /* Set the session config */
  session.server.host_name = SMTP_HOST;
  session.server.port = SMTP_PORT;
  session.login.email = AUTHOR_EMAIL;
  session.login.password = AUTHOR_PASSWORD;
  session.login.user_domain = "watering.net";

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = "ESP Mail";
  message.sender.email = AUTHOR_EMAIL;
  message.subject = in_subject;
  message.addRecipient(EMAIL_NAME, EMAIL_TARGET);
  message.addRecipient(EMAIL_NAME2, EMAIL_TARGET2); // temporary
  message.addRecipient(SMS_NAME, SMS_TARGET);

  message.text.content = in_body;

  /** The Plain text message character set e.g.
   * us-ascii
   * utf-8
   * utf-7
   * The default value is utf-8
  */
  message.text.charSet = "us-ascii";

  /** The content transfer encoding e.g.
   * enc_7bit or "7bit" (not encoded)
   * enc_qp or "quoted-printable" (encoded)
   * enc_base64 or "base64" (encoded)
   * enc_binary or "binary" (not encoded)
   * enc_8bit or "8bit" (not encoded)
   * The default value is "7bit"
  */
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;

  /** The message priority
   * esp_mail_smtp_priority_high or 1
   * esp_mail_smtp_priority_normal or 3
   * esp_mail_smtp_priority_low or 5
   * The default value is esp_mail_smtp_priority_low
  */
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;

  /** The Delivery Status Notifications e.g.
   * esp_mail_smtp_notify_never
   * esp_mail_smtp_notify_success
   * esp_mail_smtp_notify_failure
   * esp_mail_smtp_notify_delay
   * The default value is esp_mail_smtp_notify_never
  */
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;

  /* Set the custom message header */
  if(0 == freshLocalTime()) {
    useTime("Message-ID: <a%Y%m%d%H%M%S@gmail.com>");
  }
  message.addHeader((const char *)&buffer);
//  message.addHeader("Message-ID: <abcde.fghij@gmail.com>");

  /* Connect to server with the session config */
  if (!smtp.connect(&session))
    return;

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message))
    Serial.println("Error sending Email, " + smtp.errorReason());
}

/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status)
{
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success())
  {
    Serial.println("----------------");
    Serial.printf("Message sent success: %d\n", status.completedCount());
    Serial.printf("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");
    struct tm dt;

    for (size_t i = 0; i < smtp.sendingResult.size(); i++)
    {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);
      localtime_r(&result.timesstamp, &dt);

      Serial.printf("Message No: %d\n", i + 1);
      Serial.printf("Status: %s\n", result.completed ? "success" : "failed");
      Serial.printf("Date/Time: %d/%d/%d %d:%d:%d\n", dt.tm_year + 1900, dt.tm_mon + 1, dt.tm_mday, dt.tm_hour, dt.tm_min, dt.tm_sec);
      Serial.printf("Recipient: %s\n", result.recipients);
      Serial.printf("Subject: %s\n", result.subject);
    }
    Serial.println("----------------\n");
  }
}

int freshLocalTime()
{
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain fresh time");
    return 1;
  }
  return 0;
}

void useTime(const char * fmtString)
{
  strftime(buffer, sizeof buffer, fmtString, &timeinfo);
}

void printLocalTime()
{
  if (0 == freshLocalTime()) {
    useTime("%A, %B %d %Y %H:%M:%S");
    Serial.println(buffer);
  }
//  struct tm timeinfo;
//  if(!getLocalTime(&timeinfo)){
//    Serial.println("Failed to obtain time");
//    return;
//  }
//  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}
