//-------------------------------------------------------------------------------------------------
//
// Copyright © 2001, Intel Corporation . Other brands and names may be claimed as the property of others. 
//
//
// CPU Counting Utility
// Date   : 10/30/2001
// Version: 1.4
// 
//
//
// File Name: CPUCount.cpp
//
// Note: 1) LogicalNum = Number of logical processors per PHYSICAL PROCESSOR.  If you want to count
//       the total number of logical processors, multiply this number with the total number of 
//       physical processors (PhysicalNum)
//
//       2) To detect whether hyper-threading is enabled or not is to see how many logical ID exist 
//       per single physical ID in APIC
//
//       3) For systems that don't support hyper-threading like AMD or PIII and below. the variable
//       LogicalNum will be set to 1 (which means number of logical processors equals to number of
//       physical processors.)
//    
//       4) Hyper-threading cannot be detected when application cannot access all processors in 
//       the system. The number of physical processors will be set to 255.  Make sure to enable ALL 
//       physical processors at startup of windows, and applications calling this function, CPUCount,
//       are NOT restricted to run on any particular logical or physical processors(can run on ALL
//       processors.)
// 
//       5) Windows currently can handle up to 32 processors. 
//
//
//-------------------------------------------------------------------------------------------------

#include "FirstSwgClientSetup.h"


#define HT_BIT             0x10000000     // EDX[28]  Bit 28 is set if HT is supported
#define FAMILY_ID          0x0F00         // EAX[11:8] Bit 8-11 contains family processor ID.
#define PENTIUM4_ID        0x0F00         
#define EXT_FAMILY_ID      0x0F00000      // EAX[23:20] Bit 20-23 contains extended family processor ID
#define NUM_LOGICAL_BITS   0x00FF0000     // EBX[23:16] Bit 16-23 in ebx contains the number of logical
                                          // processors per physical processor when execute cpuid with 
                                          // eax set to 1

#define INITIAL_APIC_ID_BITS  0xFF000000  // EBX[31:24] Bits 24-31 (8 bits) return the 8-bit unique 
                                          // initial APIC ID for the processor this code is running on.
                                          // Default value = 0xff if HT is not supported


// Status Flag
#define HT_NOT_CAPABLE           0
#define HT_ENABLED               1
#define HT_DISABLED              2
#define HT_SUPPORTED_NOT_ENABLED 3
#define HT_CANNOT_DETECT         4

unsigned int  HTSupported(void);
unsigned char LogicalProcPerPhysicalProc(void);
unsigned char GetAPIC_ID(void);
unsigned char CPUCount(unsigned char *,
					   unsigned char *);


#include <windows.h>
#include <stdio.h>



#if 0
void main(void)
{

   unsigned char LogicalNum   = 0,  // Number of logical CPU per ONE PHYSICAL CPU
                 PhysicalNum  = 0,  // Total number of physical processor

				 HTStatusFlag = 0;  


   printf("CPU Counting Utility\n");
   printf("Version 1.4\n");
   printf("Copyright (C) 2001 Intel Corporation.  All Rights Reserved\n\n");

   HTStatusFlag = CPUCount(&LogicalNum, &PhysicalNum);

   switch(HTStatusFlag)
   {
   case HT_NOT_CAPABLE:
	   printf("Hyper-threading technology not capable\n");
	   break;

   case HT_DISABLED:
	   printf("Hyper-threading technology disabled\n");
	   break;

   case HT_ENABLED:
	   printf("Hyper-threading technology enabled\n");
	   break;

   case HT_SUPPORTED_NOT_ENABLED:
	   printf("Hyper-threading technology capable but not enabled\n");
	   break;

   case HT_CANNOT_DETECT:
	   printf("Hyper-threading technology cannot be detected\n");
	   break;


   }

   printf("Number of logical processors per physical processor: %d\n", LogicalNum);
   
   if (PhysicalNum != (unsigned char)-1)
      printf("Number of physical processors: %d\n", PhysicalNum);
   else
   {
	  printf("Can't determine number of physical processors\n");
      printf("Make sure to enable ALL processors\n");
   }

   printf("\n\nPress Enter To Continue\n");
   getchar();
}
#endif


unsigned int HTSupported(void)
{
   

	unsigned int Regedx      = 0,
		         Regeax      = 0,
		         VendorId[3] = {0, 0, 0};

	__try    // Verify cpuid instruction is supported
	{
		__asm
		{
			xor eax, eax          // call cpuid with eax = 0
        	cpuid                 // Get vendor id string
			mov VendorId, ebx
			mov VendorId + 4, edx
			mov VendorId + 8, ecx
			
			mov eax, 1            // call cpuid with eax = 1
			cpuid
			mov Regeax, eax      // eax contains family processor type
			mov Regedx, edx      // edx has info about the availability of hyper-Threading
 
		}
	}

	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return(0);                   // cpuid is unavailable
	}

    if (((Regeax & FAMILY_ID) ==  PENTIUM4_ID) || 
		(Regeax & EXT_FAMILY_ID))
	  if (VendorId[0] == 'uneG')
		if (VendorId[1] == 'Ieni')
			if (VendorId[2] == 'letn')
				return(Regedx & HT_BIT);    // Genuine Intel with hyper-Threading technology

	return 0;    // Not genuine Intel processor
  
}


unsigned char LogicalProcPerPhysicalProc(void)
{

	unsigned int Regebx = 0;
	if (!HTSupported()) return (unsigned char) 1;  // HT not supported
	                                               // Logical processor = 1
	__asm
	{
		mov eax, 1
		cpuid
		mov Regebx, ebx
	}

	return (unsigned char) ((Regebx & NUM_LOGICAL_BITS) >> 16);

}


unsigned char GetAPIC_ID(void)
{

	unsigned int Regebx = 0;
	if (!HTSupported()) return (unsigned char) -1;  // HT not supported
	                                                // Logical processor = 1
	__asm
	{
		mov eax, 1
		cpuid
		mov Regebx, ebx
	}

	return (unsigned char) ((Regebx & INITIAL_APIC_ID_BITS) >> 24);

}


unsigned char CPUCount(unsigned char *LogicalNum,
					   unsigned char *PhysicalNum)
{
	unsigned char StatusFlag  = 0;
    SYSTEM_INFO info;


    *PhysicalNum = 0;
	*LogicalNum  = 0;
    info.dwNumberOfProcessors = 0;
    GetSystemInfo (&info);

	// Number of physical processors in a non-Intel system
	// or in a 32-bit Intel system with Hyper-Threading technology disabled
    *PhysicalNum = (unsigned char) info.dwNumberOfProcessors;  

    if (HTSupported())
	{
		unsigned char HT_Enabled = 0;

        *LogicalNum= LogicalProcPerPhysicalProc();

		if (*LogicalNum >= 1)    // >1 Doesn't mean HT is enabled in the BIOS
			                     // 
		{
            HANDLE hCurrentProcessHandle;
			DWORD  dwProcessAffinity;
			DWORD  dwSystemAffinity;
			DWORD  dwAffinityMask;

			// Calculate the appropriate  shifts and mask based on the 
			// number of logical processors.

			unsigned char i = 1,
				          PHY_ID_MASK  = 0xFF,
			              PHY_ID_SHIFT = 0;

			while (i < *LogicalNum)
			{
				i *= 2;
 	            PHY_ID_MASK  <<= 1;
	            PHY_ID_SHIFT++;

			}
			
			hCurrentProcessHandle = GetCurrentProcess();
			GetProcessAffinityMask(hCurrentProcessHandle, &dwProcessAffinity,
				                                          &dwSystemAffinity);

			// Check if available process affinity mask is equal to the
			// available system affinity mask
            if (dwProcessAffinity != dwSystemAffinity)
			{
                StatusFlag = HT_CANNOT_DETECT;
				*PhysicalNum = (unsigned char)-1;
				return StatusFlag;
			}
			   dwAffinityMask = 1;
			   while (dwAffinityMask != 0 && dwAffinityMask <= dwProcessAffinity)
			   {
			  	  // Check if this CPU is available
				  if (dwAffinityMask & dwProcessAffinity)
				  {
                     if (SetProcessAffinityMask(hCurrentProcessHandle,
						                        dwAffinityMask))
					 {
						 unsigned char APIC_ID,
							           LOG_ID,
									   PHY_ID;

						 Sleep(0); // Give OS time to switch CPU

                         APIC_ID = GetAPIC_ID();
						 LOG_ID  = static_cast<unsigned char>(APIC_ID & ~PHY_ID_MASK);
						 PHY_ID  = static_cast<unsigned char>(APIC_ID >> PHY_ID_SHIFT);
 
     					 if (LOG_ID != 0)  HT_Enabled = 1;

					 }

				  }

				  dwAffinityMask = dwAffinityMask << 1;

			   }
             
			// Reset the processor affinity
			 SetProcessAffinityMask(hCurrentProcessHandle, dwProcessAffinity);

            
			if (*LogicalNum == 1)  // Normal P4 : HT is disabled in hardware
			   	StatusFlag = HT_DISABLED;

			else
				if (HT_Enabled)
				{
                     // Total physical processors in a Hyper-Threading enabled system.
		             *PhysicalNum = static_cast<unsigned char>((*PhysicalNum) / (*LogicalNum));
			   	     StatusFlag = HT_ENABLED;
				}
				else StatusFlag = HT_SUPPORTED_NOT_ENABLED;

		}

	}
	else
	{
		// Processors do not have Hyper-Threading technology
		StatusFlag = HT_NOT_CAPABLE;
        *LogicalNum = 1;
        
	}
	return StatusFlag;
}




