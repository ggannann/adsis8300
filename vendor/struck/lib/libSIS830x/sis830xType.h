#ifndef SIS830XTYPE_H_INCLUDED
#define SIS830XTYPE_H_INCLUDED

typedef enum{
    UNKNOWN,
    SIS8300,
    SIS8300L,
    SIS8300L2
}moduleType;

typedef struct _SIS830X_VERSION{
    int major;
    int minor;
}SIS830X_VERSION, *PSIS830X_VERSION;

typedef struct _SIS830X_DEV_STRUCT{
    bool open;
    int fp;
    moduleType type;
}SIS830X_DEVICE, *PSIS830X_DEVICE;

typedef struct _si5338a_Reg_Data{
   unsigned char Reg_Addr;
   unsigned char Reg_Val;
   unsigned char Reg_Mask;
}si5338a_Reg_Data;

#endif // SIS830XTYPE_H_INCLUDED
