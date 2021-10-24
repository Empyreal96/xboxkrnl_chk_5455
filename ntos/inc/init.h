///
/// Header for xboxkrnl init
///
/// empyreal96

#ifndef _INIT_
#define _INIT_









VOID
KiSystemStartup(
    VOID
    );
	
	
#define XBOX_KEY_LENGTH 16

#if !defined(_XBOX_) // maybe _NTSYSTEM_
