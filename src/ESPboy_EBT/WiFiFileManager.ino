//very basic WiFi file server by shiru8bit, designed to use in my own projects
//upload/download/delete files in LittleFS via web browser
//WTFPL

#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <LittleFS.h>
#include <uri/UriBraces.h>
#include <LittleFS.h>

#define APSSID "ESPboy"
#define APHOST "espboy"
#define APPSK  "87654321"

const char *ssid = APSSID;
const char *password = APPSK;
const char *host = APHOST;

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;


//a simple going back html for the file upload and delete requests

const char html_back_P[] PROGMEM = "<html><head><meta http-equiv=\"refresh\" content=\"1; URL=/\" /></head><body></body><html>";

File file_upload;



void WiFiFileManager()
{
  timer1_disable();

  screen_clear();
  set_font_color(0x0f);
  set_back_color(0x00);
  put_str(0, 0, "WIFI INIT...");
  screen_update();

  WiFi.mode(WIFI_AP);
  WiFi.forceSleepWake();

  serverSetup();

  screen_clear();
  set_font_color(0x0c);
  set_back_color(0x00);
  put_str(0, 0, "WIFI XCHG MODE");
  set_font_color(0x07);
  put_str(0, 2, "SSID:");
  set_font_color(0x0f);
  put_str(0, 3, ssid);
  set_font_color(0x07);
  put_str(0, 5, "PASS:");
  set_font_color(0x0f);
  put_str(0, 6, password);
  set_font_color(0x07);
  put_str(0, 8, "HTTP:");
  set_font_color(0x0f);
  put_str(0, 9, "192.168.4.1");
  set_font_color(0x0e);
  put_str(0, 11, "CONNECT WIFI AP");
  put_str(0, 12, "USE WEB BROWSER");
  put_str(0, 13, "TO MANAGE FILES");
  set_font_color(0x02);
  put_str(0, 15, "PRESS TO REBOOT");
  screen_update();

  Serial.print(F("FreeHeap:"));
  Serial.println(ESP.getFreeHeap());

  while (1) {
    serverLoop();
    if (myESPboy.getKeys()) ESP.reset();
    delay(100);
  }
}



void handleRoot()
{
  Serial.println("root requested, generating index");

  myESPboy.myLED.setRGB(0, 255, 0);
  myESPboy.myLED.on();

  FSInfo fs_info;

  LittleFS.info(fs_info);

  Dir dir = LittleFS.openDir("/");

  int file_cnt = 0;

  while (dir.next()) ++file_cnt;

  int free_kb = fs_info.usedBytes / 1024;
  int total_kb = fs_info.totalBytes / 1024;

  String output;

  output = "<html><head>";
  output += "<title>ESPboy file manager</title>";
  output += "<meta http-equiv=\"Cache-Control\" content=\"no-cache, no-store, must-revalidate\" />";
  output += "<meta http-equiv=\"Pragma\" content=\"no-cache\" />";
  output += "<meta http-equiv=\"Expires\" content=\"0\" />";
  output += "</head><body>";
  output += "<script src=\"https://unpkg.com/jszip@3.7.1/dist/jszip.min.js\" type=\"text/javascript\"></script>";
  output += "<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/2.1.3/jquery.min.js\"></script>";
  output += "<script>";

  output += "function createBlob(data) {";
  output += "return new Blob([data], { type: \"text/plain\" }); }";

  output += "function saveAs(content, fileName) {";
  output += "const a = document.createElement(\"a\");";
  output += "const isBlob = content.toString().indexOf(\"Blob\") > -1;";
  output += "let url = content;";
  output += "if (isBlob) { url = window.URL.createObjectURL(content); }";
  output += "a.href = url; a.download = fileName; a.click();";
  output += "if (isBlob) { window.URL.revokeObjectURL(url); }}";

  output += "async function zipfiles() {";
  output += "const zip = new JSZip();";

  dir.rewind();

  while (dir.next())
  {
    output += "zip.file(\"";
    output += dir.fileName();
    output += "\", $.get(\"dl/";
    output += dir.fileName();
    output += "\"));";
  }

  output += "const content = await zip.generateAsync({ type: \"blob\" });";
  output += "saveAs(content, \"backup.zip\");";
  output += "document.getElementById('bb').firstChild.data = \"Backup\"; }";

  output += "function reftimeout() { setTimeout(function(){window.location.reload();},100); }";
  output += "function rmconfirm() { return confirm('Sure?'); }";
  output += "function dobackup() { document.getElementById('bb').firstChild.data = \"Wait a while...\"; zipfiles(); }";
  output += "</script>";

  output += "<h2>ESPboy file manager</h2>";
  output += "<b>Upload a file:</b><br/><br/>";
  output += "<form action=\"up\" method=\"post\" enctype=\"multipart/form-data\" onsubmit=\"reftimeout()\">";
  output += "<input type=\"file\" name=\"file\" id=\"file\" /><input type=\"submit\" name=\"submit\" value=\"OK\" />";
  output += "</form><br/><b>";

  output += file_cnt;
  output += " files on the device (";
  output += free_kb;
  output += " of ";
  output += total_kb;
  output += "KB used):</b>";

  output += "<br/><br/>";
  output += "<button onclick=\"window.location.reload();\">Refresh</button>";
  output += "&nbsp;";
  output += "<button onclick=\"dobackup();\" id=\"bb\">Backup</button>";
  output += "<br/><br/>";
  output += "<table cellpadding=\"8\">";

  dir.rewind();

  int file_num = 1;

  while (dir.next())
  {
    output += "<tr><td>";
    output += file_num;
    output += "</td><td>";
    output += dir.fileName();
    output += "</td><td>(";
    output += dir.fileSize();
    output += " bytes)</td><td><a href=\"dl\\";
    output += dir.fileName();
    output += "\"";
    output += dir.fileName();
    output += "\">download</a></td><td><a href=\"rm\\";
    output += dir.fileName();
    output += "\"";
    output += dir.fileName();
    output += "\" onclick=\"return rmconfirm()\">delete</a></td></tr>";

    ++file_num;
  }

  output += "</body></html>";

  server.send(200, "text/html", output);
  output = String();

  myESPboy.myLED.off();
}



void handleFileDelete()
{
  myESPboy.myLED.setRGB(255, 0, 0);
  myESPboy.myLED.on();

  String path = server.pathArg(0);

  Serial.println("delete requested for " + path);

  if (!LittleFS.exists(path))
  {
    server.send(404, "text/html", "404");
  }
  else
  {
    LittleFS.remove(path);

    server.send_P(301, "text/html", html_back_P);
  }

  path = String();

  myESPboy.myLED.off();
}



String urldecode(String url) //needed for filenames with spaces
{
  String output = "";
  int ptr = 0;

  while (ptr < url.length())
  {
    char c = url.charAt(ptr++);

    switch (c)
    {
      case '+':
        {
          output += ' ';
        }
        break;

      case '%':
        {
          char d = 0;
          for (int j = 0; j < 2; ++j)
          {
            char h = url.charAt(ptr++);

            char n = 0;
            if (h >= '0' && h <= '9') n = h - '0';
            if (h >= 'a' && h <= 'f') n = h - 'a' + 10;
            if (h >= 'A' && h <= 'F') n = h - 'F' + 10;

            d = (d << 4) | n;
          }
          output += d;
        }
        break;

      default:
        {
          output += c;
        }
    }
  }

  return output;
}



void handleFileDownload()
{
  myESPboy.myLED.setRGB(255, 255, 0);
  myESPboy.myLED.on();

  String path = urldecode(server.pathArg(0));

  Serial.println("download requested for " + path);

  if (!LittleFS.exists(path))
  {
    server.send(404, "text/html", "404");
  }
  else
  {
    File f = LittleFS.open(path, "r");

    if (f)
    {
      int file_size = f.size();

      server.setContentLength(CONTENT_LENGTH_UNKNOWN);
      server.send(200, "application/octet-stream", "");
      bool first = true;

      while (file_size > 0)
      {
        unsigned char buf[1024];

        int block_size = file_size;
        if (block_size > sizeof(buf)) block_size = sizeof(buf);

        f.readBytes((char*)buf, block_size);

        server.sendContent((const char*)buf, block_size);

        file_size -= block_size;
      }

      f.close();
    }
    else
    {
      server.send(500, "text/html", "500");
    }
  }

  path = String();

  myESPboy.myLED.off();
}



void handleFileUpload()
{
  myESPboy.myLED.setRGB(0, 0, 255);
  myESPboy.myLED.on();

  HTTPUpload& upload = server.upload();

  switch (upload.status)
  {
    case UPLOAD_FILE_START:
      {
        String filename = upload.filename;

        Serial.println("upload requested for " + filename);

        if (!filename.startsWith("/")) filename = "/" + filename;

        file_upload = LittleFS.open(filename, "w");
        filename = String();
      }
      break;

    case UPLOAD_FILE_WRITE:
      {
        if (file_upload) file_upload.write(upload.buf, upload.currentSize);
      }
      break;

    case UPLOAD_FILE_END:
      {
        if (file_upload) file_upload.close();
      }
      break;
  }

  myESPboy.myLED.off();
}



void serverSetup()
{
  Serial.println();
  Serial.print(F("Configuring access point..."));
  WiFi.softAP(ssid, password);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print(F("AP IP address: "));
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.on(UriBraces("/rm/{}"), handleFileDelete);
  server.on(UriBraces("/dl/{}"), handleFileDownload);
  server.on("/up", HTTP_POST, []() {
    server.send_P(200, "text/plain", html_back_P);
  }, handleFileUpload);
  MDNS.begin(host);
  httpUpdater.setup(&server);
  server.begin();
  MDNS.addService("http", "tcp", 80);
  delay(50);
  Serial.println(F("HTTP server started"));
}

void serverLoop()
{
  server.handleClient();
}
