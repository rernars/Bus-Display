#include <WiFi.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

/* ================= WIFI ================= */
const char* ssid     = "Eren";
const char* password = "Eren2004";

/* ================= TIME ================= */
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;     // adjust for your timezone
const int   daylightOffset_sec = 3600;

/* ================= MATRIX ================= */
#define PANEL_WIDTH 64
#define PANEL_HEIGHT 64  	// Panel height of 64 will required PIN_E to be defined.
#define PANELS_NUMBER 2 
#define PIN_E 21

#define PANE_WIDTH PANEL_WIDTH * PANELS_NUMBER
#define PANE_HEIGHT PANEL_HEIGHT

MatrixPanel_I2S_DMA* dma_display;

/* ================= ROUTE DEFINITIONS ================= */
struct BusRoute {
  const char* route;
  const char* destination;
};

/* EXACTLY your 8 combinations */
BusRoute routes[] = {
  {"14",  "Hubland"},
  {"14",  "Bahnhof"},
  {"114", "UniSport"},
  {"114", "Bahnhof"},
  {"214", "Bibliothk"},
  {"214", "Bahnhof"},
  {"29",  "TGZ"},
  {"29",  "Bahnhof"}
};

const int ROUTE_COUNT = sizeof(routes) / sizeof(routes[0]);

/* ================= BUS SCHEDULE TABLES ================= */
/* ---- REPLACE TIMES WITH REAL DATA ---- */

/* 14 → Am Hubland */
const char* bus14_hub_weekday[]  = {
"04:43",  
"05:12","05:40","05:53",
"06:40",
"07:03","07:22","07:33","07:43","07:59",
"08:19","08:39","08:59",
"09:19","09:39","09:59",
"10:19","10:39","10:59",
"11:19","11:39","11:59",
"12:19","12:39","12:59",
"13:19","13:25","13:39","13:59",
"14:19","14:39","14:59",
"15:19","15:39","15:59",
"16:19","16:39","16:59",
"17:04","17:19","17:34","17:49",
"18:04","18:19","18:39","18:59",
"19:19","19:33","19:53",
"20:13","20:33","20:53",
"21:13","21:43",
"22:13","22:43",
"23:23"
};
const char* bus14_hub_saturday[] = {
"00:23",
"01:23",
"05:40",
"06:29","06:59",
"07:29","07:59",
"08:26","08:46",
"09:06","09:26","09:46",
"10:06","10:26","10:46",
"11:06","11:26","11:46",
"12:06","12:26","12:46",
"13:06","13:27","13:47",
"14:07","14:27","14:47",
"15:07","15:27","15:47",
"16:07","16:27","16:47",
"17:07","17:54",
"18:14","18:54",
"19:13","19:53",
"20:13","20:53",
"21:13",
"22:13",
"23:23"
};
const char* bus14_hub_sunday[]   = {
"00:23",
"01:23",
"07:59",
"08:59",
"09:59",
"10:59",
"11:22","11:52",
"12:22","12:52",
"13:22","13:52",
"14:22","14:52",
"15:22","15:52",
"16:22","16:52",
"17:22","17:52",
"18:22","18:52",
"19:22","19:52",
"20:32",
"21:12",
"22:12",
"23:22"
};

/* 14 → Hauptbahnhof */
const char* bus14_hbf_weekday[]  = {
"05:03","05:33",
"06:07","06:19","06:45",
"07:05","07:18","07:28","07:32","07:45","07:49",
"08:10","08:28","08:48",
"09:08","09:28","09:48",
"10:08","10:28","10:48",
"11:08","11:28","11:48",
"12:08","12:28","12:48",
"13:08","13:28","13:48",
"14:08","14:28","14:48",
"15:08","15:28","15:48",
"16:08","16:28","16:48",
"17:03","17:18","17:33","17:48",
"18:03","18:18","18:33","18:48",
"19:08","19:27","19:47",
"20:04","20:34","20:54",
"21:14"
};
const char* bus14_hbf_saturday[] = {
"05:59",
"06:29","06:59",
"07:29","07:59",
"08:29","08:49",
"09:09","09:29","09:49",
"10:09","10:29","10:49",
"11:09","11:29","11:49",
"12:09","12:29","12:49",
"13:09","13:29","13:49",
"14:09","14:29","14:49",
"15:09","15:29","15:49",
"16:09","16:29","16:49",
"17:09","17:44",
"18:19","18:39",
"19:19","19:39",
"20:14","20:34","20:54"  
};
const char* bus14_hbf_sunday[]   = {
"07:29",
"08:29",
"09:29",
"10:29",
"11:24","11:49",
"12:19","12:49",
"13:19","13:49",
"14:19","14:49",
"15:19","15:49",
"16:19","16:49",
"17:19","17:49",
"18:19","18:49",
"19:19","19:49",
"20:24","20:54"  
};

/* 114 → Am Hubland */
const char* bus114_hub_weekday[] = {
"07:37","07:47","07:57",
"08:07","08:27","08:47",
"09:07","09:27","09:47",
"10:07","10:27","10:47",
"11:07","11:27","11:47",
"12:07","12:27","12:47",
"13:07","13:27","13:47",
"14:07","14:27","14:57",
"15:12","15:27","15:42","15:57",
"16:12","16:27","16:42","16:57",
"17:20","17:30",
"18:00","18:30",
"19:00","19:30",
"20:00","20:30"
};
const char* bus114_hub_saturday[] = {};
const char* bus114_hub_sunday[]   = {};

/* 114 → Hauptbahnhof */
const char* bus114_hbf_weekday[]  = {
  "07:33",
  "08:16","08:36","08:56",
  "09:16","09:36","09:56",
  "10:16","10:36","10:56",
  "11:16","11:36","11:56",
  "12:16","12:36","12:56",
  "13:16","13:36","13:56",
  "14:16","14:36","14:56",
  "15:16","15:36","15:56",
  "16:16","16:36","16:56",
  "17:11","17:26","17:41","17:56",
  "18:11","18:26","18:41","18:56",
  "19:18","19:38","19:58",
  "20:18","20:45",
  "21:15","21:45",
  "22:15",
  "23:15"
};
const char* bus114_hbf_saturday[] = {};
const char* bus114_hbf_sunday[]   = {};

/* 214 → Am Hubland */
const char* bus214_hub_weekday[] = {
"07:20","07:37","07:52","08:57",
"08:12","08:32","08:52",
"09:12","09:32","09:52",
"10:12","10:32","10:52",
"11:12","11:32","11:52",
"12:12","12:32","12:52",
"13:12","13:32","13:52",
"14:12","14:32","14:52",
"15:12","15:32","15:52",
"16:12"
};
const char* bus214_hub_saturday[] = {};
const char* bus214_hub_sunday[]   = {};

/* 214 → Hauptbahnhof */
const char* bus214_hbf_weekday[]  = {
  "07:17","07:33",
  "08:04","08:22","08:42",
  "09:02","09:22","09:42",
  "10:02","10:22","10:42",
  "11:02","11:22","11:42",
  "12:02","12:22","12:42",
  "13:02","13:22","13:42",
  "14:02","14:22","14:42",
  "15:02","15:22","15:42",
  "16:02","16:22","16:42",
  "17:02"
};
const char* bus214_hbf_saturday[] = {};
const char* bus214_hbf_sunday[]   = {};

/* 29 → Am Hubland */
const char* bus29_hub_weekday[] = {
"06:17","06:47",
"07:17","07:32","07:47",
"08:02","08:32",
"09:02","09:32",
"10:02","10:32",
"11:02","11:32",
"12:02","12:32",
"13:02","13:32",
"14:02","14:32",
"15:02","15:32",
"16:02","16:32",
"17:02","17:32","17:47",
"18:02","18:17","18:32",
"19:02","19:32",
"20:01","20:31",
"21:01","21:31",
"22:01","22:31",
"23:01"
};
const char* bus29_hub_saturday[] = {
"06:31",
"07:31",
"08:31",
"09:01","09:31",
"10:01","10:31",
"11:01","11:31",
"12:01","12:31",
"13:01","13:31",
"14:01","14:31",
"15:01","15:31",
"16:01","16:31",
"17:01","17:31",
"18:01","18:31",
"19:01","19:31",
"20:01","20:31",
"21:01","21:31",
"22:01","22:31",
"23:01",
"00:01"
};
const char* bus29_hub_sunday[]   = {
"06:31",
"07:31",
"08:31",
"09:31",
"10:31",
"11:31",
"12:31",
"13:31",
"14:31",
"15:31",
"16:31",
"17:31",
"18:31",
"19:31",
"20:31",
"21:31",
"22:31",
"00:01"
};

/* 29 → Hauptbahnhof */
const char* bus29_hbf_weekday[]  = {
  "06:12","06:42",
  "07:07","07:22","07:37","07:52",
  "08:07","08:22","08:52",
  "09:22","09:52",
  "10:22","10:52",
  "11:22","11:52",
  "12:22","12:52",
  "13:22","13:52",
  "14:22","14:52",
  "15:22","15:52",
  "16:22","16:37","16:52",
  "17:07","17:22","17:52",
  "18:07","18:22","18:52",
  "19:22","19:52",
  "20:22","20:52",
  "21:22",
  "22:22",
  "23:22",
  "00:22"
};
const char* bus29_hbf_saturday[] = {};
const char* bus29_hbf_sunday[]   = {};

/* ================= SCHEDULE REGISTRY ================= */
struct BusSchedule {
  const char** weekday;
  int weekdayCount;
  const char** saturday;
  int saturdayCount;
  const char** sunday;
  int sundayCount;
};

BusSchedule schedules[] = {
  {bus14_hub_weekday,  sizeof(bus14_hub_weekday)/sizeof(char*),
   bus14_hub_saturday, sizeof(bus14_hub_saturday)/sizeof(char*),
   bus14_hub_sunday,   sizeof(bus14_hub_sunday)/sizeof(char*)},

  {bus14_hbf_weekday,  sizeof(bus14_hbf_weekday)/sizeof(char*),
   bus14_hbf_saturday, sizeof(bus14_hbf_saturday)/sizeof(char*),
   bus14_hbf_sunday,   sizeof(bus14_hbf_sunday)/sizeof(char*)},

  {bus114_hub_weekday,  sizeof(bus114_hub_weekday)/sizeof(char*),
   bus114_hub_saturday, sizeof(bus114_hub_saturday)/sizeof(char*),
   bus114_hub_sunday,   sizeof(bus114_hub_sunday)/sizeof(char*)},

  {bus114_hbf_weekday,  sizeof(bus114_hbf_weekday)/sizeof(char*),
   bus114_hbf_saturday, sizeof(bus114_hbf_saturday)/sizeof(char*),
   bus114_hbf_sunday,   sizeof(bus114_hbf_sunday)/sizeof(char*)},

  {bus214_hub_weekday,  sizeof(bus214_hub_weekday)/sizeof(char*),
   bus214_hub_saturday, sizeof(bus214_hub_saturday)/sizeof(char*),
   bus214_hub_sunday,   sizeof(bus214_hub_sunday)/sizeof(char*)},

  {bus214_hbf_weekday,  sizeof(bus214_hbf_weekday)/sizeof(char*),
   bus214_hbf_saturday, sizeof(bus214_hbf_saturday)/sizeof(char*),
   bus214_hbf_sunday,   sizeof(bus214_hbf_sunday)/sizeof(char*)},

  {bus29_hub_weekday,  sizeof(bus29_hub_weekday)/sizeof(char*),
   bus29_hub_saturday, sizeof(bus29_hub_saturday)/sizeof(char*),
   bus29_hub_sunday,   sizeof(bus29_hub_sunday)/sizeof(char*)},

  {bus29_hbf_weekday,  sizeof(bus29_hbf_weekday)/sizeof(char*),
   bus29_hbf_saturday, sizeof(bus29_hbf_saturday)/sizeof(char*),
   bus29_hbf_sunday,   sizeof(bus29_hbf_sunday)/sizeof(char*)}
};

const int SCHEDULE_COUNT = sizeof(schedules) / sizeof(schedules[0]);

/* ================= UPCOMING STRUCT ================= */
struct UpcomingBus {
  int minutes;
  int index;
};

/* ================= UTILS ================= */
int timeToMinutes(const char* t) {
  return (t[0]-'0')*600 + (t[1]-'0')*60 +
         (t[3]-'0')*10  + (t[4]-'0');
}

/* ================= DISPLAY ================= */
void drawBuses(const std::vector<UpcomingBus>& list) {
  dma_display->clearScreen();
  dma_display->setTextSize(1);

  const int startX = 6;
  const int startY = 5;
  const int rowHeight = 12; // 8px font + 4px gap

  for (int i = 0; i < list.size() && i < 5; i++) {
    const BusRoute& r = routes[list[i].index];
    int y = startY + i * rowHeight;

    dma_display->setCursor(startX, y);

    /* BUS NUMBER — GREEN */
    dma_display->setTextColor(dma_display->color565(0, 255, 0));
    dma_display->printf("%-3s ", r.route);

    /* DESTINATION — WHITE */
    dma_display->setTextColor(dma_display->color565(255, 255, 255));
    dma_display->printf("%-8s ", r.destination);

    /* TIME — RED */
    dma_display->setTextColor(dma_display->color565(0, 255, 0));
    dma_display->printf("%2d min", list[i].minutes);
    
  }
}


/* ================= SETUP ================= */
void setup() {
  Serial.begin(115200);

  HUB75_I2S_CFG mxconfig;
  mxconfig.mx_height = PANEL_HEIGHT;      // we have 64 pix heigh panels
  mxconfig.chain_length = PANELS_NUMBER;  // we have 2 panels chained
  mxconfig.gpio.e = PIN_E;   
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  dma_display->begin();
  dma_display->setBrightness8(120);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

/* ================= LOOP ================= */
void loop() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return;

  int nowMin = timeinfo.tm_hour * 60 + timeinfo.tm_min;
  int wday = timeinfo.tm_wday; // 0=Sunday

  std::vector<UpcomingBus> upcoming;

  for (int i = 0; i < SCHEDULE_COUNT; i++) {
    const char** times;
    int count;

    if (wday == 0) {
      times = schedules[i].sunday;
      count = schedules[i].sundayCount;
    } else if (wday == 6) {
      times = schedules[i].saturday;
      count = schedules[i].saturdayCount;
    } else {
      times = schedules[i].weekday;
      count = schedules[i].weekdayCount;
    }

    for (int t = 0; t < count; t++) {
      int busMin = timeToMinutes(times[t]);
      if (busMin >= nowMin) {
        upcoming.push_back({busMin - nowMin, i});
      }
    }
  }

  std::sort(upcoming.begin(), upcoming.end(),
    [](const UpcomingBus& a, const UpcomingBus& b) {
      return a.minutes < b.minutes;
    });

  drawBuses(upcoming);
  delay(10000);
}
