#include "projector.h"
#include "serial.h"

#define START_CODE ((char)0xA9)
#define ON_CODE    ((char)0x2E)
#define OFF_CODE   ((char)0x2F)
#define END_CODE   ((char)0x9A)

static ProjectorState ParseStateResponse(char buf[8])
{
  if (buf[0] == START_CODE &&
      buf[1] == 0x01 &&
      buf[2] == 0x02 &&
      buf[3] == 0x02 &&
      buf[4] == 0x00 &&
      buf[6] == (buf[5] | 0x03) &&
      buf[7] == END_CODE)
  {
    return static_cast<ProjectorState>(buf[5]);
  }
  return STATE_UNKNOWN;
}

static ProjectorState ChangePowerState(bool on)
{
  const ProjectorState wantedState = on ? STATE_POWER_ON : STATE_STANDBY;
  char buf[8] = {START_CODE, 0x17, (on ? ON_CODE : OFF_CODE), 0x00, 0x00, 0x00, 0x3F, END_CODE };
  ProjectorState state = GetProjectorState();
  if (state != wantedState) {
    if (Serial0.write(buf, 8) == 8) {
      do {
        state = GetProjectorState();
      } while (!(state == wantedState || state == STATE_UNKNOWN));
    }
  }
}

ProjectorState GetProjectorState()
{
  char buf[8] = { START_CODE, 0x01, 0x02, 0x01, 0x00, 0x00, 0x03, END_CODE };
  Serial0.write(buf, 8);
  size_t rv = Serial0.readBytes(buf, 8);
  if (rv == 8) {
  	return ParseStateResponse(buf);
  }
  return STATE_UNKNOWN;
}

void TurnOnProjector()
{
  Serial.print("Turning ON projector... ");
  const ProjectorState state = ChangePowerState(true);
  Serial.println(state == STATE_POWER_ON ? "done." : "error!");
}

void TurnOffProjector()
{
  Serial.print("Turning OFF projector... ");
  const ProjectorState state = ChangePowerState(false);
  Serial.println(state == STATE_STANDBY ? "done." : "error!");
}

