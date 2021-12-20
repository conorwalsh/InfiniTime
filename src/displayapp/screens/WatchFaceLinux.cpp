#include "displayapp/screens/WatchFaceLinux.h"

#include <date/date.h>
#include <lvgl/lvgl.h>
#include <cstdio>
#include "displayapp/screens/BatteryIcon.h"
#include "displayapp/screens/BleIcon.h"
#include "displayapp/screens/NotificationIcon.h"
#include "displayapp/screens/Symbols.h"
#include "components/battery/BatteryController.h"
#include "components/ble/BleController.h"
#include "components/ble/NotificationManager.h"
#include "components/heartrate/HeartRateController.h"
#include "components/motion/MotionController.h"
#include "components/settings/Settings.h"
using namespace Pinetime::Applications::Screens;

WatchFaceLinux::WatchFaceLinux(DisplayApp* app,
                                   Controllers::DateTime& dateTimeController,
                                   Controllers::Battery& batteryController,
                                   Controllers::Ble& bleController,
                                   Controllers::NotificationManager& notificatioManager,
                                   Controllers::Settings& settingsController,
                                   Controllers::HeartRateController& heartRateController,
                                   Controllers::MotionController& motionController)
  : Screen(app),
    currentDateTime {{}},
    dateTimeController {dateTimeController},
    batteryController {batteryController},
    bleController {bleController},
    notificatioManager {notificatioManager},
    settingsController {settingsController},
    heartRateController {heartRateController},
    motionController {motionController} {
  settingsController.SetClockFace(3);

  batteryIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_label_set_text(batteryIcon, Symbols::batteryFull);
  lv_obj_align(batteryIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);

  batteryPlug = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(batteryPlug, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDC3545));
  lv_label_set_text(batteryPlug, Symbols::plug);
  lv_obj_align(batteryPlug, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  bleIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bleIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x007BFF));
  lv_label_set_text(bleIcon, Symbols::bluetooth);
  lv_obj_align(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  notificationIcon = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(notificationIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x00FF00));
  lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(false));
  lv_obj_align(notificationIcon, nullptr, LV_ALIGN_IN_TOP_LEFT, 0, 0);

  topRoot = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(topRoot, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x007BFF));
  lv_label_set_text(topRoot, "root@watch:~$");
  lv_obj_align(topRoot, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 30);

  topCmd = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(topCmd, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
  lv_label_set_text(topCmd, "lswatch");
  lv_obj_align(topCmd, topRoot, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
  
  labelTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
  lv_label_set_text(labelTime, "[TIME] ");
  lv_obj_align(labelTime, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 5, 60);

  valueTime = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(valueTime, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x007BFF));
  lv_label_set_text(valueTime, "");
  lv_obj_align(valueTime, labelTime, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

  labelDate = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
  lv_label_set_text(labelDate, "[DATE] ");
  lv_obj_align(labelDate, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 5, 90);

  valueDate = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(valueDate, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
  lv_label_set_text(valueDate, "");
  lv_obj_align(valueDate, labelDate, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

  labelBatt = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelBatt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
  lv_label_set_text(labelBatt, "[BATT] [");
  lv_obj_align(labelBatt, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 5, 120);

  valueBatt = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(valueBatt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
  lv_label_set_text(valueBatt, "          ");
  lv_obj_align(valueBatt, labelBatt, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

  labelBatt2 = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelBatt2, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
  lv_label_set_text(labelBatt2, "]");
  lv_obj_align(labelBatt2, valueBatt, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

  labelStep = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelStep, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
  lv_label_set_text(labelStep, "[STEP] ");
  lv_obj_align(labelStep, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 5, 150);

  valueStep = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(valueStep, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
  lv_label_set_text(valueStep, "");
  lv_obj_align(valueStep, labelStep, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

  labelHr = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(labelHr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
  lv_label_set_text(labelHr, "[ HR ] ");
  lv_obj_align(labelHr, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 5, 180);

  valueHr = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(valueHr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDC3545));
  lv_label_set_text(valueHr, "");
  lv_obj_align(valueHr, labelHr, LV_ALIGN_OUT_RIGHT_MID, 0, 0);
  
  bottomRoot = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bottomRoot, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x007BFF));
  lv_label_set_text(bottomRoot, "root@watch:~$");
  lv_obj_align(bottomRoot, lv_scr_act(), LV_ALIGN_IN_TOP_LEFT, 0, 210);

  bottomCmd = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_style_local_text_color(bottomCmd, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFFFFF));
  lv_label_set_text(bottomCmd, "|");
  lv_obj_align(bottomCmd, bottomRoot, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

  backgroundLabel = lv_label_create(lv_scr_act(), nullptr);
  lv_obj_set_click(backgroundLabel, true);
  lv_label_set_long_mode(backgroundLabel, LV_LABEL_LONG_CROP);
  lv_obj_set_size(backgroundLabel, 240, 240);
  lv_obj_set_pos(backgroundLabel, 0, 0);
  lv_label_set_text(backgroundLabel, "");

  taskRefresh = lv_task_create(RefreshTaskCallback, LV_DISP_DEF_REFR_PERIOD, LV_TASK_PRIO_MID, this);
  Refresh();
}

WatchFaceLinux::~WatchFaceLinux() {
  lv_task_del(taskRefresh);
  lv_obj_clean(lv_scr_act());
}

void WatchFaceLinux::Refresh() {
  /* Display power icon if plugged in */
  powerPresent = batteryController.IsPowerPresent();
  if (powerPresent.IsUpdated()) {
    lv_label_set_text(batteryPlug, BatteryIcon::GetPlugIcon(powerPresent.Get()));
  }

  /* Update battery info if percentage changed */
  batteryPercentRemaining = batteryController.PercentRemaining();
  if (batteryPercentRemaining.IsUpdated()) {
    /* Battery Icon */
    auto batteryPercent = batteryPercentRemaining.Get();
    if (batteryPercent == 100) {
      lv_obj_set_style_local_text_color(batteryIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_GREEN);
    } else {
      lv_obj_set_style_local_text_color(batteryIcon, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    }
    lv_label_set_text(batteryIcon, BatteryIcon::GetBatteryIcon(batteryPercent));

    /* Battery Label */
    uint8_t hashes = (uint8_t)((batteryPercent / 10) + 0.5);
    switch (hashes){
      case 0:
        lv_label_set_text(valueBatt, "          ");
        break;
      case 1:
        lv_label_set_text(valueBatt, "#         ");
        lv_obj_set_style_local_text_color(valueBatt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDC3545));
        break;
      case 2:
        lv_label_set_text(valueBatt, "##        ");
        lv_obj_set_style_local_text_color(valueBatt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDC3545));
        break;
      case 3:
        lv_label_set_text(valueBatt, "###       ");
        lv_obj_set_style_local_text_color(valueBatt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDC3545));
        break;
      case 4:
        lv_label_set_text(valueBatt, "####      ");
        lv_obj_set_style_local_text_color(valueBatt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFC107));
        break;
      case 5:
        lv_label_set_text(valueBatt, "#####     ");
        lv_obj_set_style_local_text_color(valueBatt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFC107));
        break;
      case 6:
        lv_label_set_text(valueBatt, "######    ");
        lv_obj_set_style_local_text_color(valueBatt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xFFC107));
        break;
      case 7:
        lv_label_set_text(valueBatt, "#######   ");
        lv_obj_set_style_local_text_color(valueBatt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x28A745));
        break;
      case 8:
        lv_label_set_text(valueBatt, "########  ");
        lv_obj_set_style_local_text_color(valueBatt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x28A745));
        break;
      case 9:
        lv_label_set_text(valueBatt, "######### ");
        lv_obj_set_style_local_text_color(valueBatt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x28A745));
        break;
      case 10:
        lv_label_set_text(valueBatt, "##########");
        lv_obj_set_style_local_text_color(valueBatt, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x28A745));
        break;
    }
  }

  /* Show BlueTooth icon if connected */
  bleState = bleController.IsConnected();
  if (bleState.IsUpdated()) {
    lv_label_set_text(bleIcon, BleIcon::GetIcon(bleState.Get()));
  }

  lv_obj_align(batteryIcon, lv_scr_act(), LV_ALIGN_IN_TOP_RIGHT, 0, 0);
  lv_obj_align(batteryPlug, batteryIcon, LV_ALIGN_OUT_LEFT_MID, -5, 0);
  lv_obj_align(bleIcon, batteryPlug, LV_ALIGN_OUT_LEFT_MID, -5, 0);

  /* Show notification icon if new notification */
  notificationState = notificatioManager.AreNewNotificationsAvailable();
  if (notificationState.IsUpdated()) {
    lv_label_set_text(notificationIcon, NotificationIcon::GetIcon(notificationState.Get()));
  }

  /* Update tine related info */
  currentDateTime = dateTimeController.CurrentDateTime();
  if (currentDateTime.IsUpdated()) {
    auto newDateTime = currentDateTime.Get();

    auto dp = date::floor<date::days>(newDateTime);
    auto time = date::make_time(newDateTime - dp);
    auto yearMonthDay = date::year_month_day(dp);
    auto day = static_cast<unsigned>(yearMonthDay.day());

    int hour = time.hours().count();
    auto minute = time.minutes().count();
    auto seconds = time.seconds().count();

    char hoursChar[3];
    char minutesChar[3];
    char secondsChar[3];

    sprintf(hoursChar, "%02d", hour);
    sprintf(minutesChar, "%02d", static_cast<int>(minute));
    sprintf(secondsChar, "%02d", static_cast<int>(seconds));

    if (currentSec != seconds) {
      /* Blink cursor every second */
      if (seconds % 2 == 0) {
        lv_label_set_text(bottomCmd, "|");
      }
      else {
        lv_label_set_text(bottomCmd, "");
      }
      
      /* Update time label */
      lv_label_set_text_fmt(valueTime, "%s:%s:%s", hoursChar, minutesChar, secondsChar);
      currentSec = seconds;
    }

    /* Update date info */
    if (currentDay != day) {
      lv_label_set_text_fmt(valueDate, "%s %d %s", dateTimeController.DayOfWeekShortToString(), day, dateTimeController.MonthShortToString());
      currentDay = day;
    }
  }

  /* Update heart rate info if available */
  heartbeat = heartRateController.HeartRate();
  heartbeatRunning = heartRateController.State() != Controllers::HeartRateController::States::Stopped;
  if (heartbeat.IsUpdated() || heartbeatRunning.IsUpdated()) {
    if (heartbeatRunning.Get()) {
      lv_label_set_text_fmt(valueHr, "%d BPM", heartbeat.Get());
      lv_obj_set_style_local_text_color(valueHr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0xDC3545));
    } else {
      lv_label_set_text_static(valueHr, "N/A");
      lv_obj_set_style_local_text_color(valueHr, LV_LABEL_PART_MAIN, LV_STATE_DEFAULT, lv_color_hex(0x6C757D));
    }
  }

  /* Update step count if changed */
  stepCount = motionController.NbSteps();
  motionSensorOk = motionController.IsSensorOk();
  if (stepCount.IsUpdated() || motionSensorOk.IsUpdated()) {
    lv_label_set_text_fmt(valueStep, "%lu", stepCount.Get());
  }

}
