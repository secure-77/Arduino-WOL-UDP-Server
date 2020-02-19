
// wifi settings
#define SSID "Wifi SSID"
#define PASSWORD "Wifi Password"
IPAddress ip(192,168,1,5);   
IPAddress gateway(192,168,1,1);   
IPAddress subnet(255,255,255,0); 

// Target Domain, local WOL IP, MAC and Broadcast Addres
const char* dynDNS = "yourDomain.dyndns.org";
IPAddress serverIP (192, 168, 1, 10);
byte mac[] = {0x2C, 0xFD, 0xA1, 0xB8, 0x9B, 0x2D};
byte bc[] = {192, 168, 1, 255};

// email settings
const char* smtp_server = "smtp_server";
const int   smtp_port = 465;
const char* smtp_login = "smtp_login";
const char* smtp_password = "smtp_password";
const int   smtp_timeout = 5000;
const bool  smtp_ssl = true;
const char* mail_from = "mail_from";
const char* mail_to = "mail_to";

// The server accepts connections on this port
#define PORT 5444

// code tries
int code_tries = 3;

// Delay between WOL sending
byte senddelay = 10;

// wake up retries
const int retryCount = 3;
