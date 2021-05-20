/**
 * Exploration of sending email notificcations from esp32
 *
 * This also (first) sets the esp32 clock from NTP servers
 */

// mail_send.h includes secrets.h, which is not included in the project
// repository. To set up your own sketch folder, copy template_secrets.h to
// secrets.h, then fill in the details for your local environment in the copied
// file. In case the sketch folder is being reshared, do not put any of the
// private information in template_secrets.h, and do not share the secrets.h
// file. For git repositories, that can be excluded by having a .gitignore file
// (in the sketch or project folder) that hides secrets.h from `git add`
// commands.
#include "mail_send.h"

const char* ntpServer = NTP_SERVER;
const long  utcOffset_sec = UTC_OFFSET_SEC;
const int   daylightOffset_sec = DAYLIGHT_OFFSET_SEC;

void setup()
{
  Serial.begin(115200);
  delay(500);
  Serial.println("Starting monitor");
  wifiConnect();

  // initialize and get the time
  configTime(utcOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  if (0 == freshLocalTime()) {
    useTime("Watering report as of %B %d %Y %H:%M:%S");
  }
  const char *msg_subject = (const char *)&buffer;
//  const char * msg_subject = "newer subject";
  const char * msg_body = "report body with pump info";
  sendReport(msg_subject, msg_body);
  Serial.println(msg_subject);

  //disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop()
{
  delay(10000);
  printLocalTime();
}
