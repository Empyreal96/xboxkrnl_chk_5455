# xboxkrnl.exe build 5445 XDK CHK
My attempt at comparing the 5455 XDK kernel against the 4400 tree, NOT COMPILABLE CODE (Mainly psudocode with sections filled in)

Ongoing learning project for myself, the psudocode is partially modified in refence with the leaked 4400 tree, I have very little C and C++ knowledge so expect errors.. 


### Notice:
This is **NOT** compilable source code, despite some functions being filled in, this would need extensive work to make buildable.. 

There will be parts where I have no clue, look for comments `// TODO` for sections I have not been able to rewrite


### Example:

```c

VOID main(
		IN PUCHAR LoadOptions,
		IN const UCHAR* CryptKeys
		)

{
	///
	/// Find the load options and convert to boot flags for the kernel
	///
  UCHAR buf[64]; // [esp+0h] [ebp-40h] BYREF
  strncpy(buf, LoadOptions, 64); //64 = 0x40u?
  buf[63] = '\0';
  _strupr((buf);
  if ( strstr(buf, "SHADOW") )
    XboxBootFlags |= XBOX_BOOTFLAG_SHADOW; //1u
  if ( strstr(buf, "/HDBOOT") )
    XboxBootFlags |= XBOX_BOOTFLAG_HDBOOT; //2u
  if ( strstr(buf, "/CDBOOT") )
    XboxBootFlags |= XBOX_BOOTFLAG_CDBOOT; //4u
  if ( strstr(buf, "/DBBOOT") )
    XboxBootFlags |= XBOX_BOOTFLAG_DASHBOARDBOOT; //0x80;

///
/// Match the saved keys
///

// TO-DO
//  *(_DWORD *)XboxEEPROMKey = *(_DWORD *)CryptKeys;
//  *(_DWORD *)&XboxEEPROMKey[4] = *((_DWORD *)CryptKeys + 1);
//  *(_DWORD *)&XboxEEPROMKey[8] = *((_DWORD *)CryptKeys + 2);
//  *(_DWORD *)&XboxEEPROMKey[12] = *((_DWORD *)CryptKeys + 3);
//  *(_DWORD *)XboxCERTKey = *((_DWORD *)CryptKeys + 4);
//  *(_DWORD *)&XboxCERTKey[4] = *((_DWORD *)CryptKeys + 5);
//  *(_DWORD *)&XboxCERTKey[8] = *((_DWORD *)CryptKeys + 6);
//  *(_DWORD *)&XboxCERTKey[12] = *((_DWORD *)CryptKeys + 7);
//  memset((void *)0x80400000, 0, 0x6000u);
  
  *XboxEEPROMKey = *CryptKeys;
  *&XboxEEPROMKey[4] = *(CryptKeys + 1);
  *&XboxEEPROMKey[8] = *(CryptKeys + 2);
  *&XboxEEPROMKey[12] = *(CryptKeys + 3);
  *XboxCERTKey = *(CryptKeys + 4);
  *&XboxCERTKey[4] = *(CryptKeys + 5);
  *&XboxCERTKey[8] = *(CryptKeys + 6);
  *&XboxCERTKey[12] = *(CryptKeys + 7);
  memset(0x80400000, 0, 0x6000u);
  
  ///
  /// Init system
  ///
  KiSystemStartup();
}
```

