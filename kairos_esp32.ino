#include <Arduino.h>
#include <U8g2lib.h>
#include <MUIU8g2.h>
#include "button.h"
#include "icons.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#define prog_frame_x 55
#define prog_frame_y 54
#define prog_frame_w 60
#define prog_frame_h 4

#define prog_x 56
#define prog_y 55
#define prog_w 58
#define prog_h 2

U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/6, /* dc=*/5, /* reset=*/4);

MUIU8G2 mui;

uint8_t is_redraw = 1;

//Graphics
const uint8_t workFrameCount = 22;
const uint8_t breakFrameCount = 19;
uint8_t frame = 0;
uint8_t flash = 0;

#define selectPin 11
#define prevPin 2
#define nextPin 0

ButtonHandler selectb(selectPin);
ButtonHandler prev(prevPin);
ButtonHandler next(nextPin);

#include <BlockNot.h>

BlockNot iconTimer(50);
BlockNot timeTimer(100);
BlockNot flashTimer(500);

#include <WiFi.h>
#include <HTTPClient.h>

/*const char* ssid = "Milton2G";
const char* password = "dashwifi2g";*/

const char* ssid = "MJ";
const char* password = "mihir1909";

const char* serverName = "https://kairosapi.vercel.app/add_log";

//Time
uint8_t initStatus = 0; //0=Splash, 1=Start, 2=Started, 3=Paused
uint8_t work = 2;
uint8_t shortv = 2;
uint8_t longv = 5;
uint8_t laps = 2;
uint8_t breakToggle = 0; //0=Work, 1=Break
int totalSeconds = 0;
int secondsCounter = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;
uint8_t totalLaps = 0;
uint8_t lapCount = 0;

void setTime(uint8_t type /*0=Work, 1=ShortBreak, 2=LongBreak*/) {
  uint8_t tmp;
  switch(type) {
    case 0: 
      tmp = work;
      break;
    case 1:
      tmp = shortv;
      break;
    case 2:
      tmp = longv;
      break;
  }
  minutes = tmp;
  secondsCounter = tmp * 60;
  totalSeconds = tmp * 60;
}

uint8_t mui_hrule(mui_t *ui, uint8_t msg) {
  if (msg == MUIF_MSG_DRAW) {
    u8g2.drawHLine(0, mui_get_y(ui), u8g2.getDisplayWidth() - 20);
  }
  return 0;
}

uint8_t mui_yes(mui_t *ui, uint8_t msg) {
  if (msg == MUIF_MSG_DRAW) {
    mui.leaveForm();
    initStatus = 0;
    is_redraw = 0;
    setTime(0);
    seconds = 0;
    frame = 0;
    u8g2.clearBuffer();
    u8g2.drawStr(20, 10, "You studied for");
    int minutes = totalLaps*work;
    u8g2.setCursor(22,25);
    u8g2.print(minutes);
    if(minutes < 60) {
      u8g2.drawStr(32, 25, "Mins./ ");
    } else {
      minutes = minutes/60;
      u8g2.drawStr(30, 25, "Hours/ ");
    }
    u8g2.setCursor(70,25);
    u8g2.print(totalLaps);
    u8g2.drawStr(80, 25, "Laps");
    u8g2.sendBuffer();
    if(WiFi.status()== WL_CONNECTED) {
      HTTPClient http;
      http.addHeader("Content-Type", "application/json");
      String serverPath = String(serverName) + "?duration=" + String(minutes) + "&sessions=" + String(totalLaps) + "&token=TPGUW3KE0V";
      http.begin(serverPath);
      int httpResponseCode = http.POST("");
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println(httpResponseCode);
        Serial.println(response);
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    }
  }
  return 0;
}
uint8_t mui_no(mui_t *ui, uint8_t msg) {
  if (msg == MUIF_MSG_DRAW) {
    mui.leaveForm();
    is_redraw = 1;
    iconTimer.start();
    timeTimer.start();
  }
  return 0;
}

const muif_t muif_list[] = {
  MUIF_U8G2_FONT_STYLE(0, u8g2_font_spleen6x12_mr),
  MUIF_U8G2_FONT_STYLE(1, u8g2_font_spleen6x12_mr),
  MUIF_U8G2_FONT_STYLE(2, u8g2_font_unifont_t_86),

  MUIF_RO("HR", mui_hrule),
  MUIF_RO("CY", mui_yes),
  MUIF_RO("CN", mui_no),
  MUIF_U8G2_LABEL(),
  MUIF_RO("GP", mui_u8g2_goto_data),
  MUIF_BUTTON("GC", mui_u8g2_goto_form_w1_pi),
  MUIF_BUTTON("EX", mui_u8g2_btn_exit_wm_fi),

  MUIF_U8G2_U8_MIN_MAX("NV", &work, 0, 60, mui_u8g2_u8_min_max_wm_mud_pi),
  MUIF_U8G2_U8_MIN_MAX("NW", &shortv, 0, 60, mui_u8g2_u8_min_max_wm_mud_pi),
  MUIF_U8G2_U8_MIN_MAX("NX", &longv, 0, 60, mui_u8g2_u8_min_max_wm_mud_pi),
  MUIF_U8G2_U8_MIN_MAX("NY", &laps, 0, 9, mui_u8g2_u8_min_max_wm_mud_pi),
  MUIF_EXECUTE_ON_SELECT_BUTTON("GO", mui_u8g2_btn_goto_wm_fi)
};

const fds_t fds_data[] =

  MUI_FORM(1)
    MUI_STYLE(1)
      MUI_LABEL(5, 8, "Pomo")
        MUI_STYLE(0)
          MUI_XY("HR", 0, 11)
            MUI_DATA("GP",
                     MUI_2 "Configure")
              MUI_XYA("GC", 5, 24, 0)
                MUI_STYLE(2)
                  MUI_XYT("EX", 120, 15, "⭠")

                    MUI_FORM(2)
                      MUI_STYLE(1)
                        MUI_LABEL(5, 8, "Time")
                          MUI_XY("HR", 0, 11)
                            MUI_STYLE(0)
                              MUI_LABEL(5, 23, "Work:")
                                MUI_LABEL(5, 36, "Short break:")
                                  MUI_LABEL(5, 49, "Long break:")
                                    MUI_LABEL(5, 64, "Laps:")
                                      MUI_XY("NV", 80, 23)
                                        MUI_XY("NW", 80, 36)
                                          MUI_XY("NX", 80, 49)
                                            MUI_XY("NY", 80, 64)
                                              MUI_STYLE(2)
                                                MUI_XYAT("GO", 120, 15, 1, "⭠")

                                                  MUI_FORM(3)
                                                    MUI_STYLE(1)
                                                    MUI_LABEL(5, 8, "Are you sure you want")
                                                    MUI_LABEL(5, 24, "to end the session?")
                                                    MUI_XY("HR", 0,30)
                                                    MUI_DATA("GP",
                                                      MUI_4 "Yes|"
                                                      MUI_5 "No")
                                                    MUI_XYA("GC", 5, 45, 0)
                                                    MUI_XYA("GC", 5, 57, 1)
                                                        
                                                  MUI_FORM(4)
                                                    MUI_XY("CY", 0, 11)

                                                  MUI_FORM(5)
                                                    MUI_XY("CN", 0, 11);

void setup() {
  Serial.begin(115200);
  Serial.println(MOSI);
  Serial.println(SCK);
  delay(1000);
  WiFi.begin(ssid, password);
  u8g2.begin();
    u8g2.setBitmapMode(1);
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 10, "Connection init");
  u8g2.sendBuffer();
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  u8g2.clearBuffer();
  drawSplash();
  mui.begin(u8g2, fds_data, muif_list, sizeof(muif_list) / sizeof(muif_t));
  setTime(0);
  iconTimer.stop();
  timeTimer.stop();
  flashTimer.stop();
  is_redraw = 1;
}

void loop() {
  int b = selectb.checkButton();
  int c = prev.checkButton();
  int d = next.checkButton();
  if (b == 2) {
    if (initStatus < 2) {
      mui.gotoForm(1, 0);
      is_redraw = 1;
    } else {
      iconTimer.stop();
      timeTimer.stop();
      mui.gotoForm(3, 0);
      is_redraw = 1;
    }
  }
  if (mui.isFormActive()) {
    if (is_redraw) {
      u8g2.firstPage();
      do {
        mui.draw();
      } while (u8g2.nextPage());
      is_redraw = 0;
    }

    if (b == 1) {
      mui.sendSelect();
      is_redraw = 1;
    }

    if (c == 1) {
      mui.prevField();
      is_redraw = 1;
    }

    if (d == 1) {
      mui.nextField();
      is_redraw = 1;
    }

  } else {
    if (initStatus == 0 && is_redraw == 1) {
      u8g2.clearBuffer();
      drawSplash();
    } else if (initStatus == 1 && is_redraw == 1) {
      u8g2.clearBuffer();
      updateIcon(false);
      updateTime(false);
      is_redraw = 0;
    }

    if (b == 1) {
      if (initStatus == 0) {
        initStatus = 1;
        is_redraw = 1;
      } else {
        is_redraw = 0;
        if (initStatus != 2) {
          u8g2.clearBuffer();
          iconTimer.start();
          timeTimer.start();
          initStatus = 2;
        } else {
          iconTimer.stop();
          timeTimer.stop();
          u8g2.setDrawColor(0);
          u8g2.drawBox(30, 10, 78, 40);
          u8g2.setDrawColor(1);
          u8g2.setFont(u8g2_font_spleen6x12_mr);
          u8g2.drawButtonUTF8(65, 35, U8G2_BTN_SHADOW1 | U8G2_BTN_HCENTER | U8G2_BTN_BW2, 34, 2, 5, "Paused");
          u8g2.sendBuffer();
          initStatus = 3;
        }
      }
    }
  }

  if (iconTimer.TRIGGERED) {
    updateIcon(true);
  }

  if (timeTimer.TRIGGERED) {
    updateTime(true);
  }

  if (flashTimer.TRIGGERED) {
    flashDisplay();
  }
}

void drawSplash() {
  u8g2.setDrawColor(1);
  u8g2.drawRBox(10,10,105,40,5);
  u8g2.drawRBox(20,38,10,15,3);
  u8g2.drawRBox(95,38,10,15,3);
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_profont29_mr);
  u8g2.drawStr(15, 40, "Kairos");
  String ipString = "Connected to " + WiFi.localIP().toString();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setDrawColor(1);
  u8g2.drawStr(0, 65, ipString.c_str());
  /*u8g2.setFont(u8g2_font_crox5tb_tr);
  u8g2.setCursor(10, 30);
  u8g2.print(F("Pomo"));*/
  u8g2.sendBuffer();
}

void updateIcon(bool loop) {
  if((breakToggle == 0 && frame == workFrameCount) || (breakToggle == 1 && frame == breakFrameCount)) {
    frame = 0;
    iconTimer.addTime(2000);
  }
  if (frame == 1) {
    iconTimer.setDuration(50);
  }
  u8g2.setDrawColor(0);
  u8g2.drawBox(0, 0, 50, 50);
  u8g2.setDrawColor(1);
  if (breakToggle == 0) {
    u8g2.drawXBMP(15, 20, 24, 24, hourglass[frame]);
  } else {
    u8g2.drawXBMP(15, 17, 24, 24, cup[frame]);
  }
  u8g2.drawFrame(prog_frame_x, prog_frame_y, prog_frame_w, prog_frame_h);
  u8g2.sendBuffer();
  if (loop) {
    frame++;
  }
}

void updateTime(bool loop) {
  if (initStatus < 2) {
    setTime(0);
  }
  u8g2.setFont(u8g2_font_spleen6x12_mr);
  if (breakToggle == 0) {
    u8g2.setCursor(15, 60);
    u8g2.print(F("Work"));
  } else {
    u8g2.setCursor(15, 60);
    u8g2.print(F("Break"));
  }
  u8g2.setDrawColor(0);
  u8g2.setFont(u8g2_font_crox5tb_tr);
  u8g2.drawBox(55, 20, 60, 25);
  u8g2.drawBox(prog_x, prog_y, prog_w, prog_h);
  u8g2.setDrawColor(1);
  double per = double(secondsCounter) / double(totalSeconds);
  u8g2.drawBox(prog_x, prog_y, int(double(prog_w) * per), prog_h);
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", minutes, seconds);
  u8g2.drawStr(55, 40, timeStr);
  u8g2.sendBuffer();
  if (loop == true) {
    if (seconds == 0) {
      if (minutes <= 0) {
        iconTimer.stop();
        timeTimer.stop();
        flashTimer.start();
      } else {
        minutes--;
        seconds = 59;
      }
    } else {
      seconds--;
      secondsCounter--;
    }
  }
}

void flashDisplay() {
  if (flash == 0 || flash == 2) {
    u8g2.clear();
    flash++;
  } else if (flash == 1) {
    updateIcon(false);
    updateTime(false);
    flash++;
  } else if (flash == 3) {
    flash = 0;
    if(breakToggle == 0) {
      totalLaps++;
      lapCount++;
      setTime(1);
      breakToggle = 1;
    } else {
      setTime(0);
      breakToggle = 0;
    }
    if(lapCount == laps) {
      setTime(2);
      breakToggle = 1;
      lapCount = 0;
    }
    flashTimer.stop();
    iconTimer.start();
    timeTimer.start();
  }
}

/*void loop() {
  //drawSplash();
  int result = next.checkButton();

  if (result == 1) {
    Serial.println("Single click detected");
  } else if (result == 2) {
    Serial.println("Long click detected");
  }
}*/
