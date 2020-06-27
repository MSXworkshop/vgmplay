// MsxUsbSlot.h: Header File for MSX USB SLOT

#ifndef __MSXUSBSLOT__H__
#define __MSXUSBSLOT__H__

#ifdef MSX_USB_SLOT
#ifdef WIN32

int msxusbslot_Open();
void msxusbslot_Close();
bool msxusbslot_IsOpen();
void msxusbslot_writeByte(int addr, int data);
void msxusbslot_writeOPLL(int reg, int data);
void msxusbslot_writeSCC(int addr, int data);

#endif
#endif
#endif
