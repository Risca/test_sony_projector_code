#include "projector.h"
#include "serial.h"

#define START_CODE ((char)0xA9)
#define END_CODE   ((char)0x9A)

static byte ParseStateResponse(char buf[8])
{
  if (buf[0] == START_CODE &&
      buf[1] == 0x01 &&
      buf[2] == 0x02 &&
      buf[3] == 0x02 &&
      buf[4] == 0x00 &&
      buf[6] == (buf[5] | 0x03) &&
      buf[7] == END_CODE)
  {
    return buf[5];
  }
  return 0;
}

byte GetProjectorState()
{
  char buf[8] = { START_CODE, 0x01, 0x02, 0x01, 0x00, 0x00, 0x03, END_CODE };
  Serial0.write(buf, 8);
  size_t rv = Serial0.readBytes(buf, 8);
  if (rv == 8) {
  	return ParseStateResponse(buf);
  }
  return 0;
}

void TurnOnProjector()
{
  Serial0.print("Turning ON projector... ");
  Serial0.println("done.");
}

void TurnOffProjector()
{
  Serial0.print("Turning OFF projector... ");
  Serial0.println("done.");
}

