/*
             LUFA Library
     Copyright (C) Dean Camera, 2014.

  dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
  Copyright 2014  Dean Camera (dean [at] fourwalledcubicle [dot] com)

  Permission to use, copy, modify, distribute, and sell this
  software and its documentation for any purpose is hereby granted
  without fee, provided that the above copyright notice appear in
  all copies and that both that the copyright notice and this
  permission notice and warranty disclaimer appear in supporting
  documentation, and that the name of the author not be used in
  advertising or publicity pertaining to distribution of the
  software without specific, written prior permission.

  The author disclaims all warranties with regard to this
  software, including all implied warranties of merchantability
  and fitness.  In no event shall the author be liable for any
  special, indirect or consequential damages or any damages
  whatsoever resulting from loss of use, data or profits, whether
  in an action of contract, negligence or other tortious action,
  arising out of or in connection with the use or performance of
  this software.
*/

/** \file
 *
 *  Main source file for the MIDI demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "MIDI.h"

/** LUFA MIDI Class driver interface configuration and state information. This structure is
 *  passed to all MIDI Class driver functions, so that multiple instances of the same class
 *  within a device can be differentiated from one another.
 */
USB_ClassInfo_MIDI_Device_t Keyboard_MIDI_Interface =
  {
    .Config =
      {
        .StreamingInterfaceNumber = INTERFACE_ID_AudioStream,
        .DataINEndpoint           =
          {
            .Address          = MIDI_STREAM_IN_EPADDR,
            .Size             = MIDI_STREAM_EPSIZE,
            .Banks            = 1,
          },
        .DataOUTEndpoint          =
          {
            .Address          = MIDI_STREAM_OUT_EPADDR,
            .Size             = MIDI_STREAM_EPSIZE,
            .Banks            = 1,
          },
      },
  };


/** Main program entry point. This routine contains the overall program flow, including initial
 *  setup of all components and the main program loop.
 */
int main(void)
{

  SetupHardware();

  LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
  GlobalInterruptEnable();

  uint8_t state = 0;
  for (;;) {
    uint8_t MIDICommand = 0;
    uint8_t MIDIPitch;
    uint8_t r = PIND & 1;

    if (r != state) {
      MIDICommand = r ? MIDI_COMMAND_NOTE_ON : MIDI_COMMAND_NOTE_OFF;
      MIDIPitch   = 0x3B;
      MIDI_EventPacket_t MIDIEvent = (MIDI_EventPacket_t)
      {
        .Event       = MIDI_EVENT(0, MIDICommand),

        .Data1       = MIDICommand | MIDI_CHANNEL(1),
        .Data2       = MIDIPitch,
        .Data3       = MIDI_STANDARD_VELOCITY,
      };

      state = r;


      MIDI_Device_SendEventPacket(&Keyboard_MIDI_Interface, &MIDIEvent);
      MIDI_Device_Flush(&Keyboard_MIDI_Interface);
    }

    MIDI_Device_USBTask(&Keyboard_MIDI_Interface);
    USB_USBTask();
  }
}

/** Configures the board hardware and chip peripherals for the demo's functionality. */
void SetupHardware(void)
{
  /* Disable watchdog if enabled by bootloader/fuses */
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  /* Disable clock division */
  clock_prescale_set(clock_div_1);

  DDRD = 0;
  USB_Init();
}



/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
  LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
  LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
  bool ConfigSuccess = true;

  ConfigSuccess &= MIDI_Device_ConfigureEndpoints(&Keyboard_MIDI_Interface);

  LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
  MIDI_Device_ProcessControlRequest(&Keyboard_MIDI_Interface);
}

