// cSpell:disable
#ifndef MY_SECRETS_H
#define MY_SECRETS_H

// Copy this file to `secrets.h` in the sketch folder, and replace the dummy
// defined values with those needed for your environment. Do not edit this
// (the template) file. Copy it, then edit the copy. This block, and other
// comments can optionally be removed from the resulting `secrets.h` file.

// Standard information needed to connect to your wireless access point
#define WIFI_SSID "your_access_point_name"
#define WIFI_PASSWORD "password_for_your_ap"

/* SMTP server sign in credentials */
#define AUTHOR_EMAIL "email_address_or_user_name_as_needed"
#define AUTHOR_PASSWORD "password_for_sending_email_through_server"

// email addesses and names to use for notification. If you want more or less
// than 3, the code will need to be adjusted to match as well.
#define EMAIL_TARGET "destination_address@some_domain.tld"
#define EMAIL_NAME "name to use with destination"
#define EMAIL_TARGET2 "another_address@some_domain.tld"
#define EMAIL_NAME2 "name to use with another destination"
#define SMS_TARGET "phoneNumber@sms.gateway.tld"
#define SMS_NAME "name to use with sms target"

// specify the URL of the time server (or pool) you want to use
// pool.ntp.org should work fine in most cases
#define NTP_SERVER "pool.ntp.org"
// local timezone offset from UTC in seconds when daylight savings time is NOT
// in effect. This could be either positive or negative, depending on the local
// timezone. https://en.wikipedia.org/wiki/List_of_UTC_time_offsets is one
// source of offset information.
#define UTC_OFFSET_SEC (3600*hours_offset)
// #define UTC_OFFSET_SEC (-3600*hours_offset)
// offset (seconds) from above when daylight savings is in effect. 0 when DST
// is not used for the local timezone, 3600 for a 1 hour offset
#define DAYLIGHT_OFFSET_SEC 0

#endif
// cSpell:enable
