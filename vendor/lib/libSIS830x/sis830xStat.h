#ifndef SIS830XSTAT_H_INCLUDED
#define SIS830XSTAT_H_INCLUDED

#define API_RETURNCODE_START 0

typedef enum _SIS830X_STATUS{
    Stat830xSuccess = API_RETURNCODE_START, // 0
    Stat830xInvalidDeviceIndex,             // 1
    Stat830xNullArgument,                   // 2
    Stat830xAlreadyOpen,                    // 3
    Stat830xNotOpen,                        // 4
    Stat830xIoctlError,                     // 5
    Stat830xReadCallError,                  // 6
    Stat830xReadLenError,                   // 7
    Stat830xWriteCallError,                 // 8
    Stat830xWriteLenError,                  // 9
    Stat830xSpiBusy,                        // 10
    Stat830xFlashBusy,                      // 11
    Stat830xInvalidPath,                    // 12
    Stat830xFileTooLarge,                   // 13
    Stat830xMemAlloc,                       // 14
    Stat830xNotSupported,                   // 15
    Stat830xVerifyError,                    // 16
    Stat830xI2cBusy,                        // 17
    Stat830xI2cNack,                        // 18
    Stat830xSynthNoLock,                    // 19
}SIS830X_STATUS;

typedef char SIS830X_STATUS_TEXT[20];

#endif // SIS830XSTAT_H_INCLUDED
