#include "display_renderer.h"

#include <WiFi.h>

namespace {

constexpr int kOledLayoutStandard = 0;
constexpr int kOledLayoutSetpointOnly = 1;
constexpr int kOledLayoutSetpointInternal = 2;
constexpr int kOledLayoutSetpointInternalRelay = 3;
constexpr int kOledLayoutInternalExternal = 4;

}  // namespace

void renderDisplay(DisplayRenderInput input) {
  const String emptyNotice;
  const String& noticeText = input.oledNoticeText ? *input.oledNoticeText : emptyNotice;

  input.u8g2.clearBuffer();
  input.u8g2.setFont(u8g2_font_5x7_tf);

  if (input.provisioningMode) {
    char l1[24];
    char l2[24];
    char l3[24];
    char l4[24];
    snprintf(l1, sizeof(l1), "SSID");
    snprintf(l2, sizeof(l2), "%s", input.apSsid);
    snprintf(l3, sizeof(l3), "Configure WiFi");
    snprintf(l4, sizeof(l4), "at 192.168.4.1");
    input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 8, l1);
    input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 18, l2);
    input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 28, l3);
    input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 38, l4);
    input.u8g2.sendBuffer();
    return;
  }

  if (input.staInfoDisplayUntilMs != 0 && input.nowMs < input.staInfoDisplayUntilMs) {
    input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 8, WiFi.localIP().toString().c_str());
    input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 18, "snowleopard.");
    input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 28, "local");
    input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 38, "connected");
    input.u8g2.sendBuffer();
    return;
  }

  if (input.oledNoticeUntilMs != 0 && input.nowMs <= input.oledNoticeUntilMs && noticeText.length() > 0) {
    input.u8g2.setFont(u8g2_font_10x20_tf);
    const int width = static_cast<int>(input.u8g2.getStrWidth(noticeText.c_str()));
    const int x = max(0, (72 - width) / 2);
    input.u8g2.drawStr(x, input.oledYOffset + 28, noticeText.c_str());
    input.u8g2.sendBuffer();
    return;
  }

  if (!input.haveSensorReading) {
    input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 8, "Booting...");
    input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 18, "Reading sensors");
    input.u8g2.sendBuffer();
    return;
  }

  if (input.setpointFocusUntilMs != 0 && input.nowMs <= input.setpointFocusUntilMs) {
    const int setpointDisplay = static_cast<int>(roundf(input.tempToDisplayFn(input.setTempC)));
    char valueLine[24];
    snprintf(valueLine, sizeof(valueLine), "%d %s", setpointDisplay, input.tempUnitTextFn());

    input.u8g2.setFont(u8g2_font_6x10_tf);
    input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 9, "Setpoint");

    input.u8g2.setFont(u8g2_font_10x20_tf);
    const int width = static_cast<int>(input.u8g2.getStrWidth(valueLine));
    const int x = max(0, (72 - width) / 2);
    input.u8g2.drawStr(x, input.oledYOffset + 36, valueLine);
    input.u8g2.sendBuffer();
    return;
  }

  if (input.tempAlarmBeepActive || (input.tempAlarmTestUntilMs != 0 && input.nowMs <= input.tempAlarmTestUntilMs)) {
    const int setpointDisplay = static_cast<int>(roundf(input.tempToDisplayFn(input.setTempC)));
    const int currentDisplay = static_cast<int>(roundf(input.tempToDisplayFn(input.internalTempC)));

    char setLine[24];
    char curLine[24];
    snprintf(setLine, sizeof(setLine), "Set %d", setpointDisplay);
    snprintf(curLine, sizeof(curLine), "Int %d", currentDisplay);

    input.u8g2.setFont(u8g2_font_10x20_tf);
    const int setWidth = static_cast<int>(input.u8g2.getStrWidth(setLine));
    const int curWidth = static_cast<int>(input.u8g2.getStrWidth(curLine));
    const int setX = max(0, (72 - setWidth) / 2);
    const int curX = max(0, (72 - curWidth) / 2);
    input.u8g2.drawStr(setX, input.oledYOffset + 18, setLine);
    input.u8g2.drawStr(curX, input.oledYOffset + 38, curLine);
    input.u8g2.sendBuffer();
    return;
  }

  char l1[24];
  char l2[24];
  char l3[24];
  char l4[24];

  const float internalDisplayTemp = input.tempToDisplayFn(input.internalTempC);
  const float externalDisplayTemp = input.tempToDisplayFn(input.externalTempC);
  const float setDisplayTemp = input.tempToDisplayFn(input.setTempC);

  if (isnan(setDisplayTemp)) {
    snprintf(l1, sizeof(l1), "SET  -- %s", input.tempUnitTextFn());
  } else {
    snprintf(l1, sizeof(l1), "SET  %3d %s", static_cast<int>(roundf(setDisplayTemp)), input.tempUnitTextFn());
  }

  if (isnan(internalDisplayTemp)) {
    snprintf(l2, sizeof(l2), "IN:  --.- %s", input.tempUnitTextFn());
  } else {
    snprintf(l2, sizeof(l2), "IN:  %4.1f %s", internalDisplayTemp, input.tempUnitTextFn());
  }

  if (isnan(externalDisplayTemp)) {
    snprintf(l3, sizeof(l3), "EXT: --.- %s", input.tempUnitTextFn());
  } else {
    snprintf(l3, sizeof(l3), "EXT: %4.1f %s", externalDisplayTemp, input.tempUnitTextFn());
  }

  snprintf(l4, sizeof(l4), "%s", input.relayStateText);

  switch (input.oledLayoutMode) {
    case kOledLayoutSetpointOnly: {
      char setLine[24];
      if (isnan(setDisplayTemp)) {
        snprintf(setLine, sizeof(setLine), "Set --");
      } else {
        snprintf(setLine, sizeof(setLine), "Set %d", static_cast<int>(roundf(setDisplayTemp)));
      }
      input.u8g2.setFont(u8g2_font_10x20_tf);
      const int width = static_cast<int>(input.u8g2.getStrWidth(setLine));
      const int x = max(0, (72 - width) / 2);
      input.u8g2.drawStr(x, input.oledYOffset + 30, setLine);
      input.u8g2.setFont(u8g2_font_6x10_tf);
      input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 40, input.tempUnitTextFn());
      break;
    }

    case kOledLayoutSetpointInternal: {
      char setLine[24];
      char inLine[24];
      if (isnan(setDisplayTemp)) {
        snprintf(setLine, sizeof(setLine), "Set --");
      } else {
        snprintf(setLine, sizeof(setLine), "Set %d", static_cast<int>(roundf(setDisplayTemp)));
      }
      if (isnan(internalDisplayTemp)) {
        snprintf(inLine, sizeof(inLine), "Int --");
      } else {
        snprintf(inLine, sizeof(inLine), "Int %d", static_cast<int>(roundf(internalDisplayTemp)));
      }
      input.u8g2.setFont(u8g2_font_10x20_tf);
      const int setWidth = static_cast<int>(input.u8g2.getStrWidth(setLine));
      const int inWidth = static_cast<int>(input.u8g2.getStrWidth(inLine));
      input.u8g2.drawStr(max(0, (72 - setWidth) / 2), input.oledYOffset + 18, setLine);
      input.u8g2.drawStr(max(0, (72 - inWidth) / 2), input.oledYOffset + 38, inLine);
      break;
    }

    case kOledLayoutSetpointInternalRelay: {
      char setLine[24];
      char inLine[24];
      char relayLine[24];
      if (isnan(setDisplayTemp)) {
        snprintf(setLine, sizeof(setLine), "Set --");
      } else {
        snprintf(setLine, sizeof(setLine), "Set %d", static_cast<int>(roundf(setDisplayTemp)));
      }
      if (isnan(internalDisplayTemp)) {
        snprintf(inLine, sizeof(inLine), "Int --");
      } else {
        snprintf(inLine, sizeof(inLine), "Int %d", static_cast<int>(roundf(internalDisplayTemp)));
      }
      snprintf(relayLine, sizeof(relayLine), "%s", input.relayOn ? "ON" : "OFF");
      input.u8g2.setFont(u8g2_font_6x10_tf);
      input.u8g2.drawStr(max(0, (72 - static_cast<int>(input.u8g2.getStrWidth(setLine))) / 2), input.oledYOffset + 10, setLine);
      input.u8g2.drawStr(max(0, (72 - static_cast<int>(input.u8g2.getStrWidth(inLine))) / 2), input.oledYOffset + 22, inLine);
      input.u8g2.setFont(u8g2_font_9x15B_tf);
      input.u8g2.drawStr(max(0, (72 - static_cast<int>(input.u8g2.getStrWidth(relayLine))) / 2), input.oledYOffset + 40, relayLine);
      break;
    }

    case kOledLayoutInternalExternal: {
      char inLine[24];
      char outLine[24];
      if (isnan(internalDisplayTemp)) {
        snprintf(inLine, sizeof(inLine), "Int --");
      } else {
        snprintf(inLine, sizeof(inLine), "Int %d", static_cast<int>(roundf(internalDisplayTemp)));
      }
      if (isnan(externalDisplayTemp)) {
        snprintf(outLine, sizeof(outLine), "Ext --");
      } else {
        snprintf(outLine, sizeof(outLine), "Ext %d", static_cast<int>(roundf(externalDisplayTemp)));
      }
      input.u8g2.setFont(u8g2_font_10x20_tf);
      const int inWidth = static_cast<int>(input.u8g2.getStrWidth(inLine));
      const int outWidth = static_cast<int>(input.u8g2.getStrWidth(outLine));
      input.u8g2.drawStr(max(0, (72 - inWidth) / 2), input.oledYOffset + 18, inLine);
      input.u8g2.drawStr(max(0, (72 - outWidth) / 2), input.oledYOffset + 38, outLine);
      break;
    }

    case kOledLayoutStandard:
    default:
      input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 8, l1);
      input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 18, l2);
      input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 28, l3);
      input.u8g2.drawStr(input.oledXOffset, input.oledYOffset + 38, l4);
      break;
  }

  input.u8g2.sendBuffer();
}
