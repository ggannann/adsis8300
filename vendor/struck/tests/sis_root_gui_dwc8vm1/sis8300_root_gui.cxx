/***************************************************************************/
/*                                                                         */
/*  Filename: sis8300_root_gui.cxx                                         */
/*                                                                         */
/*  Hardware: SIS8300_V2                                                   */
/*  Firmware: V_1400                                                       */
/*                                                                         */
/*  Funktion: ADC Sampling                                                 */
/*                                                                         */
/*  Autor:                TH                                               */
/*  date:                 10.01.2012                                       */
/*  last modification:    15.02.2013                                       */
/*                                                                         */
/* ----------------------------------------------------------------------- */
/*                                                                         */
/*  SIS  Struck Innovative Systeme GmbH                                    */
/*                                                                         */
/*  Harksheider Str. 102A                                                  */
/*  22399 Hamburg                                                          */
/*                                                                         */
/*  Tel. +49 (0)40 60 87 305 0                                             */
/*  Fax  +49 (0)40 60 87 305 20                                            */
/*                                                                         */
/*  http://www.struck.de                                                   */
/*                                                                         */
/*  � 2013                                                                 */
/*                                                                         */
/***************************************************************************/

#define MAIN_WINDOW_WIDTH					360
#define MAIN_WINDOW_HIGH					1000
#define MAIN_WINDOW_POSTION_X				10
#define MAIN_WINDOW_POSTION_Y				20

#define SIS8300_TEST_WINDOW_WIDTH			350
#define SIS8300_TEST_WINDOW_HIGH			1005
#define SIS8300_TEST_WINDOW_POSTION_X		10
#define SIS8300_TEST_WINDOW_POSTION_Y		50


#define SIS8300_RAW_DATA_WINDOW_WIDTH			700
#define SIS8300_RAW_DATA_WINDOW_HIGH			490
#define SIS8300_RAW_DATA_WINDOW_POSTION_X		365
#define SIS8300_RAW_DATA_WINDOW_POSTION_Y		50

#define SIS8300_HISTOGRAM_WINDOW_WIDTH			800
#define SIS8300_HISTOGRAM_WINDOW_HIGH			1005
#define SIS8300_HISTOGRAM_WINDOW_POSTION_X		1070
#define SIS8300_HISTOGRAM_WINDOW_POSTION_Y		50

#define SIS8300_FFT_WINDOW_WIDTH				700
#define SIS8300_FFT_WINDOW_HIGH					490
#define SIS8300_FFT_WINDOW_POSTION_X			365
#define SIS8300_FFT_WINDOW_POSTION_Y			565


#define PCA9535ADDR     0x20
#define LTC2493ADDR     0x34

#include "sis830x.h"

#include "sis8300_defs.h"
#include "sis8300_reg.h"

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/fs.h>

#include <sys/ioctl.h>

#include <math.h>
//#include <complex.h>
#include <fftw3.h>


#include <TROOT.h>
#include <TApplication.h>
#include <TVirtualX.h>
#include <TVirtualPadEditor.h>
#include <TGResourcePool.h>
#include <TGListBox.h>
#include <TGListTree.h>
#include <TGFSContainer.h>
#include <TGClient.h>
#include <TGFrame.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TGCanvas.h>
#include <TGComboBox.h>
#include <TGTab.h>
#include <TGSlider.h>
#include <TGDoubleSlider.h>
#include <TGFileDialog.h>
#include <TGTextEdit.h>
#include <TGShutter.h>
#include <TGProgressBar.h>
#include <TGColorSelect.h>
#include <TRootEmbeddedCanvas.h>
#include <TCanvas.h>
#include <TColor.h>
#include <TH1.h>
#include <TH2.h>
#include <TRandom.h>
#include <TSystem.h>
#include <TSystemDirectory.h>
#include <TEnv.h>
#include <TFile.h>
#include <TKey.h>
#include <TGDockableFrame.h>
#include <TGFontDialog.h>
#include <TStyle.h>
#include <TPave.h>
#include <TPaveText.h>

#include "TGraph.h"
//#include "TMultiGraph.h"
#include "TLatex.h"

//#include "TVirtualFFT.h"
#include "TMath.h"
#include "TF1.h"


#define MAX_PRETRIGGER_DELAY 2046 //

//#define MAX_ROOT_PLOT_LENGTH 0x40000 // 262144
#define MAX_ROOT_PLOT_LENGTH 0x80000 // 524288
//#define MAX_ROOT_PLOT_LENGTH 0x100000 // 1.048.576
//#define MAX_ROOT_PLOT_LENGTH 0x200000 // 2.097.152

//#define ADC_BUFFER_LENGTH 0x1000000 // 16 Msamples / 32MByte
//#define ADC_BUFFER_LENGTH 0x400000 // 4 Msamples / 8MByte
#define ADC_BUFFER_LENGTH 0x100000 // 1 Msamples / 2MByte

#define DWC8VM1

Int_t gl_x[MAX_ROOT_PLOT_LENGTH] ;
Int_t gl_y[MAX_ROOT_PLOT_LENGTH] ;

float gl_float_x[MAX_ROOT_PLOT_LENGTH/2] ;
float gl_float_y[MAX_ROOT_PLOT_LENGTH/2] ;
float gl_float_y1[MAX_ROOT_PLOT_LENGTH/2] ;



unsigned int gl_uint_adc_buffer[ADC_BUFFER_LENGTH]; // 2MByte
unsigned short gl_ushort_adc1_buffer[ADC_BUFFER_LENGTH]; //
unsigned short gl_ushort_adc2_buffer[ADC_BUFFER_LENGTH]; //
unsigned short gl_ushort_adc3_buffer[ADC_BUFFER_LENGTH]; //
unsigned short gl_ushort_adc4_buffer[ADC_BUFFER_LENGTH]; //
unsigned short gl_ushort_adc5_buffer[ADC_BUFFER_LENGTH]; //
unsigned short gl_ushort_adc6_buffer[ADC_BUFFER_LENGTH]; //
unsigned short gl_ushort_adc7_buffer[ADC_BUFFER_LENGTH]; //
unsigned short gl_ushort_adc8_buffer[ADC_BUFFER_LENGTH]; //
unsigned short gl_ushort_adc9_buffer[ADC_BUFFER_LENGTH]; //
unsigned short gl_ushort_adc10_buffer[ADC_BUFFER_LENGTH]; //

//double gl_double_fft_spectrum[ADC_BUFFER_LENGTH/2];

unsigned int gl_uint_adc1_average[ADC_BUFFER_LENGTH]; //

unsigned int i_ch;
unsigned int max_nof_channels=10;

unsigned short* ushort_adcA_buffer_ptr; //
unsigned short* ushort_adcB_buffer_ptr; //


int gl_ymin;
int gl_ymax;


#define DefineCanvasBackgroundColor   10
#define DefineChannel_1_Color          2
#define DefineChannel_2_Color          3
#define DefineChannel_3_Color          4
#define DefineChannel_4_Color          6
#define DefineChannel_5_Color          7
#define DefineChannel_6_Color          8
#define DefineChannel_7_Color          9
#define DefineChannel_8_Color         28
#define DefineChannel_9_Color         30
#define DefineChannel_10_Color        50

#define DefineTGCanvasBackgroundColor   0xE0E0E0


#define newraus 1
#if newraus
    PSIS830X_DEVICE dev_pointer;
    SIS830X_STATUS stat;
#endif // newraus


int SIS8300_Read_Register(int device,  unsigned int reg_addr, unsigned int* read_data) ;

int SIS8300_Write_Register(int device,  unsigned int reg_addr, unsigned int  write_data);

int SIS8300_ADC_SPI_Setup(int device,  unsigned int adc_device_no);
int SIS8300_AD9510_SPI_Setup(int device, unsigned int* ch_divider_configuration_array, unsigned int ad9510_synch_cmd);
int SIS8300_ADC_SPI_Read(int device,  unsigned int adc_device_no, unsigned int spi_addr, unsigned int* read_data);

int si5326_set_external_clock_multiplier(int device, unsigned int bw_sel, unsigned int n1_hs, unsigned int n1_clk1, unsigned int n1_clk2, unsigned int n2, unsigned int n3, unsigned int clkin1_mhz );
int si5326_bypass_external_clock_multiplier(int device);
int si5326_bypass_internal_ref_osc_multiplier(int device);

#if newraus
SIS830X_STATUS setCommonModeDAC(uint8_t adc_no, double value);
SIS830X_STATUS setAttenuator(uint8_t addr, uint8_t data);

SIS830X_STATUS readADCTemp(float *value);
SIS830X_STATUS readLoVoltage(float *value);
SIS830X_STATUS readLoTemp(float *value);
SIS830X_STATUS readRefVoltage(float *value);
SIS830X_STATUS readRefTemp(float *value);

uint16_t double2hex16bit(double value);
#endif // newraus

#define MAX_SIS8300_DEVICES	6

struct atteElement
{
    uint8_t attenuation;
    const char discrip[10];
};

enum ETestCommandIdentifiers
{
    M_FILE_EXIT,
    M_SIS8300TEST1_DLG,
    M_HELP_ABOUT,
    ColorSel
};


class TileFrame;

class TestMainFrame : public TGMainFrame
{

private:
    TGDockableFrame    *fMenuDock;
    TGCompositeFrame   *fStatusFrame;
    TGCanvas           *fCanvasWindow;
    TileFrame          *fContainer;
    TGTextEntry        *fTestText;
    TGButton           *fTestButton;
    TGGroupFrame        *main_frameh1_fGrp[3];
    // TGColorSelect      *fColorSel;

    TGMenuBar          *fMenuBar;
    //TGPopupMenu        *fMenuFile, *fMenuTest, *fMenuView, *fMenuHelp;

    TGPopupMenu        *fMenuFile, *fMenuSIS8300Test,  *fMenuView, *fMenuHelp;
    TGLayoutHints      *fMenuBarLayout, *fMenuBarItemLayout, *fMenuBarHelpLayout;

    TGLabel *fLabel_main_frameh0[MAX_SIS8300_DEVICES] ;
    TGLabel *fLabel_main_frameh2[3] ;


    TGListBox	  	*fBox_SIS8300_modules;
//    TGListTreeItem	*item_SIS8300_module;


    int use_gl_class_sis8300_device[MAX_SIS8300_DEVICES];
    unsigned int use_class_sis8300_device_index;
    static const char *const sis8300_devices_txt[MAX_SIS8300_DEVICES];

public:
    TestMainFrame(const TGWindow *p, UInt_t w, UInt_t h);
    virtual ~TestMainFrame();

    virtual void CloseWindow();
    virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t);
};

/***********************************************************************************************/


class SIS8300TestDialog : public TGTransientFrame
{

private:
    TGCompositeFrame    *fFrame1, *fF1, *fF2, *fF3, *fF4, *fF5, *fF8, *fF9;
    TGCompositeFrame    *fF_tab1, *fF_tab2, *fF_tab3;//, *fF_tab4;
    TGGroupFrame			*fF_tab1_fGrp1, *fF_tab1_fGrp1A, *fF_tab1_fGrp2, *fF_tab1_fGrp3, *fF_tab1_fGrp4;
    TGGroupFrame			*fF_tab2_fGrp1, *fF_tab2_fGrp2, *fF_tab2_fGrp3, *fF_tab2_fGrp4;
    TGGroupFrame			*fF_tab3_fGrp1, *fF_tab3_fGrp2, *fF_tab3_fGrp3, *fF_tab3_fGrp4, *fF_tab3_fGrp5;

    TGLabel *fLabel_tab3_frameh3[4] ;

    TGHorizontalFrame    *fF[8];
    TGVerticalFrame		*fVF[8];

    TGGroupFrame        *fF6, *fF7;
    TGButton            *fQuitButton;
    TGButton            *fOkButton,  *fStartB, *fStopB, *fRDButton, *fMinMaxButton;
    TGButton            *fChkDisplayAutoZoom;
    TGButton            *fChkDisplayAdc[10];
    TGButton            *fChkHistoSum;
    TGButton            *fChkHistoZoomMean;
    TGButton            *fChkHistoDisplayMinMax;
    TGButton            *fChkHistoGaussFit;
    TGButton            *fChkHistoLogY;
    TGButton            *fChkFFT_Db, *fChkFFT_AutoScale, *fChkFFT_Sum, *fChkFFTLogY;

    TGPictureButton     *fPicBut1;
    TGCheckButton       *fCheck1;
    TGCheckButton       *fCheckMulti;
    TGComboBox          *fCombo;
    TGTab               *fTab;
    TGNumberEntry       *fNumericEntries[8];
    TGLabel              *fLabel[8];
    TGTextEntry         *fTxt1, *fTxt2;
    TGLayoutHints       *fL1, *fL2, *fL3, *fL4, *fL5, *fL7;
    Bool_t               fSis8300_Test1;
    TGraph               *fGraph_ch[11];
    TLatex              	*fGraph_Text_ch[10];
    TH1I                	*iHistoAdc[10];
    TGraph               *fGraph_fft[11];
    TGComboBox          *fCombo_Clock_source;
    TGComboBox          *fCombo_Display_Histos;
    TGComboBox          *fCombo_Display_FFT_Ch;
    TGComboBox          *fCombo_Display_FFT_Window;


    TCanvas              *fCanvas1;
    TCanvas              *fCanvas2;
    TCanvas              *fCanvas3;
    TPaveText		*histo_pave_text[10];

    TGNumberEntry       *fNumericEntriesGraph_Yaxis[2];

    TGComboBox          *fCombo_CHX_Attenuator[9];
    TGButton            *fChkAMCInterlock;
    TGComboBox          *fCombo_AMC_Modulator_Type;
    TGNumberEntry       *fNumericEntriesVectorVoltages[2];
    TGNumberEntry       *fNumericEntriesCommonVoltages[2];

    Bool_t              fReadSensors;
    Bool_t              fReadMinMax;

    int gl_class_sis8300_device;
    int gl_class_sis8300_250MHZ_FLAG;
    unsigned int gl_class_adc_tap_delay;



    static const char *const numlabel[8];
    static const char *const entryClock_source[15];

    static const Double_t numinit[8];
    static const char *const chkDisAdcLabel[10];
    static const char *const AdcHistogramLabel[10];
    static const char *const entryHistoLabel[12];

    static const char *const entryDisplayFFTLabel[11];
    static const char *const entryDisplayFFTWindowLabel[6];

    static const char *const sis8300_devices_txt[6];

    static struct atteElement attenuationValue[8];


    static const char *const amcModulatorType[3];

    void Sis8300_Test1();

protected:
    Bool_t fB_openfCanvas1WindowFlag; // shows if Canvas1 window is open
    Bool_t fB_openfCanvas2WindowFlag; // shows if Canvas2 window is open
    Bool_t fB_openfCanvas3WindowFlag; // shows if Canvas3 window is open

public:
    SIS8300TestDialog(const TGWindow *p, const TGWindow *main, Int_t sis8300_device_handle,
                      UInt_t w, UInt_t h, UInt_t options = kVerticalFrame);
    virtual ~SIS8300TestDialog();

    virtual void CloseWindow();
    virtual Bool_t ProcessMessage(Long_t msg, Long_t parm1, Long_t parm2);
};






class TileFrame : public TGCompositeFrame
{

private:
    TGCanvas *fCanvas;

public:
    TileFrame(const TGWindow *p);
    virtual ~TileFrame() { }

    void SetCanvas(TGCanvas *canvas)
    {
        fCanvas = canvas;
    }
    Bool_t HandleButton(Event_t *event);
};

TileFrame::TileFrame(const TGWindow *p) :
    TGCompositeFrame(p, 10, 10, kHorizontalFrame, GetWhitePixel())
{
    // Create tile view container. Used to show colormap.

    fCanvas = 0;
    SetLayoutManager(new TGTileLayout(this, 8));

    // Handle only buttons 4 and 5 used by the wheel mouse to scroll
    gVirtualX->GrabButton(fId, kButton4, kAnyModifier,
                          kButtonPressMask | kButtonReleaseMask,
                          kNone, kNone);
    gVirtualX->GrabButton(fId, kButton5, kAnyModifier,
                          kButtonPressMask | kButtonReleaseMask,
                          kNone, kNone);
}

Bool_t TileFrame::HandleButton(Event_t *event)
{
    // Handle wheel mouse to scroll.

    Int_t page = 0;
    if (event->fCode == kButton4 || event->fCode == kButton5)
    {
        if (!fCanvas) return kTRUE;
        if (fCanvas->GetContainer()->GetHeight())
            page = Int_t(Float_t(fCanvas->GetViewPort()->GetHeight() *
                                 fCanvas->GetViewPort()->GetHeight()) /
                         fCanvas->GetContainer()->GetHeight());
    }

    if (event->fCode == kButton4)
    {
        //scroll up
        Int_t newpos = fCanvas->GetVsbPosition() - page;
        if (newpos < 0) newpos = 0;
        fCanvas->SetVsbPosition(newpos);
        return kTRUE;
    }
    if (event->fCode == kButton5)
    {
        // scroll down
        Int_t newpos = fCanvas->GetVsbPosition() + page;
        fCanvas->SetVsbPosition(newpos);
        return kTRUE;
    }
    return kTRUE;
}


/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/
/*************************************************************************************************************************/





const char *const TestMainFrame::sis8300_devices_txt[MAX_SIS8300_DEVICES] =
{
    "/dev/sis8300-0",
    "/dev/sis8300-1",
    "/dev/sis8300-2",
    "/dev/sis8300-3",
    "/dev/sis8300-4",
    "/dev/sis8300-5",
};




TestMainFrame::TestMainFrame(const TGWindow *p, UInt_t w, UInt_t h)
    : TGMainFrame(p, w, h)
{
    unsigned int i_device;
    int sis8300_device ;
    unsigned int data ;
    unsigned int adc_device_no ;
    char s[64];

    // Create test main frame. A TGMainFrame is a top level window.

    // use hierarchical cleaning
    this->SetCleanup(kDeepCleanup);
    this->SetIconPixmap("sis1_sis8300.png");

    // Create menubar and popup menus. The hint objects are used to place
    // and group the different menu widgets with respect to eachother.
    fMenuDock = new TGDockableFrame(this);


    AddFrame(fMenuDock, new TGLayoutHints(kLHintsExpandX, 0, 0, 1, 0));

    fMenuDock->SetWindowName("Test Menu");

    fMenuBarLayout = new TGLayoutHints(kLHintsTop | kLHintsExpandX);
    fMenuBarItemLayout = new TGLayoutHints(kLHintsTop | kLHintsLeft, 0, 4, 0, 0);
    fMenuBarHelpLayout = new TGLayoutHints(kLHintsTop | kLHintsRight);

    fMenuFile = new TGPopupMenu(fClient->GetRoot());

    fMenuFile->AddEntry("E&xit", M_FILE_EXIT);

// add SIS8300
    fMenuSIS8300Test = new TGPopupMenu(fClient->GetRoot());
    fMenuSIS8300Test->AddLabel("&DWC8VM1 Tests");
    fMenuSIS8300Test->AddSeparator();
    fMenuSIS8300Test->AddEntry("&Test 1", M_SIS8300TEST1_DLG);

    fMenuHelp = new TGPopupMenu(fClient->GetRoot());
    fMenuHelp->AddEntry("&About", M_HELP_ABOUT);
    // Menu button messages are handled by the main frame (i.e. "this")
    // ProcessMessage() method.
    fMenuFile->Associate(this);

    fMenuSIS8300Test->Associate(this);
    fMenuHelp->Associate(this);

    fMenuBar = new TGMenuBar(fMenuDock, 1, 1, kHorizontalFrame);
    fMenuBar->AddPopup("&File", fMenuFile, fMenuBarItemLayout);
    fMenuBar->AddPopup("&DWC8VM1-Test", fMenuSIS8300Test, fMenuBarItemLayout);
    fMenuBar->AddPopup("&Help", fMenuHelp, fMenuBarHelpLayout);

    fMenuDock->AddFrame(fMenuBar, fMenuBarLayout);



    // Create TGCanvas and a canvas container which uses a tile layout manager
    fCanvasWindow = new TGCanvas(this, 400, 240);

    //  fCanvasWindow = new TGCanvas(this, 800, 240); // tino
    fContainer = new TileFrame(fCanvasWindow->GetViewPort());
    fContainer->SetCanvas(fCanvasWindow);
    fContainer->ChangeBackground(DefineTGCanvasBackgroundColor);


    fCanvasWindow->SetContainer(fContainer);



    // use hierarchical cleaning for container
    fContainer->SetCleanup(kDeepCleanup);

    TGIcon *fIcon1341 = new TGIcon(fCanvasWindow->GetContainer(), "sislogo.bmp");
    fIcon1341->SetName("fIcon1341");
    fCanvasWindow->AddFrame(fIcon1341, new TGLayoutHints(kLHintsLeft | kLHintsTop, 5, 5, 15, 15));


    main_frameh1_fGrp[0] = new TGGroupFrame(fCanvasWindow->GetContainer(), "Crate Information");
    fCanvasWindow->AddFrame(main_frameh1_fGrp[0], new TGLayoutHints(kLHintsExpandX, 5, 5, 25, 10)); // hints, left, right, top, bottom

    main_frameh1_fGrp[1] = new TGGroupFrame(fCanvasWindow->GetContainer(), "Select SIS8300");
    fCanvasWindow->AddFrame(main_frameh1_fGrp[1], new TGLayoutHints(kLHintsExpandX, 5, 5, 10, 10)); // hints, left, right, top, bottom

    main_frameh1_fGrp[2] = new TGGroupFrame(fCanvasWindow->GetContainer(), "Selected SIS8300 information");
    fCanvasWindow->AddFrame(main_frameh1_fGrp[2], new TGLayoutHints(kLHintsExpandX, 5, 5, 10, 25)); // hints, left, right, top, bottom

    fLabel_main_frameh2[0] = new TGLabel(main_frameh1_fGrp[2],"Firmware Version");
    fLabel_main_frameh2[0]->SetTextJustify(kTextLeft + kTextCenterX );
    fLabel_main_frameh2[0]->SetMargins(0,0,0,0);
    fLabel_main_frameh2[0]->SetWrapLength(-1);
    main_frameh1_fGrp[2]->AddFrame(fLabel_main_frameh2[0], new TGLayoutHints(kLHintsExpandX,2,2,5,2));

    fLabel_main_frameh2[1] = new TGLabel(main_frameh1_fGrp[2],"Serial Number   ");
    fLabel_main_frameh2[1]->SetTextJustify(kTextLeft + kTextCenterX );
    fLabel_main_frameh2[1]->SetMargins(0,0,0,0);
    fLabel_main_frameh2[1]->SetWrapLength(-1);
    main_frameh1_fGrp[2]->AddFrame(fLabel_main_frameh2[1], new TGLayoutHints(kLHintsExpandX,2,2,2,2));

    fLabel_main_frameh2[2] = new TGLabel(main_frameh1_fGrp[2],"ADC chip ID   ");
    fLabel_main_frameh2[2]->SetTextJustify(kTextLeft + kTextCenterX );
    fLabel_main_frameh2[2]->SetMargins(0,0,0,0);
    fLabel_main_frameh2[2]->SetWrapLength(-1);
    main_frameh1_fGrp[2]->AddFrame(fLabel_main_frameh2[2], new TGLayoutHints(kLHintsExpandX,2,2,2,2));


    fBox_SIS8300_modules = new TGListBox(main_frameh1_fGrp[1], 100);
    fBox_SIS8300_modules->Associate(this);

    fBox_SIS8300_modules->SetName("fBox_SIS8300_modules");
    fBox_SIS8300_modules->Resize(100,100);
    main_frameh1_fGrp[1]->AddFrame(fBox_SIS8300_modules, new TGLayoutHints(kLHintsExpandX,2,2,15,15));




// search for sis8300 devices
    i_device = 0;
    do
    {
        sis8300_device = open(this->sis8300_devices_txt[i_device], O_RDWR);
        if (sis8300_device >= 0)
        {
            use_gl_class_sis8300_device[i_device] = sis8300_device ;
            fLabel_main_frameh0[i_device] = new TGLabel(main_frameh1_fGrp[0]," ");
            fLabel_main_frameh0[i_device]->SetTextJustify(kTextLeft + kTextCenterX );
            fLabel_main_frameh0[i_device]->SetMargins(0,0,0,0);
            fLabel_main_frameh0[i_device]->SetWrapLength(-1);
            main_frameh1_fGrp[0]->AddFrame(fLabel_main_frameh0[i_device], new TGLayoutHints(kLHintsExpandX,2,2,5,2));
            //sprintf(s,"number of SIS8300 device(s) %d ",i_device-1);
            sprintf(s,"SIS8300 device %s",this->sis8300_devices_txt[i_device]);
            fLabel_main_frameh0[i_device]->SetText(s);
            fBox_SIS8300_modules->AddEntry(s,i_device);
        }
        i_device++;
        //printf("i_device %d \n",i_device);
    }
    while ((sis8300_device >= 0) && (i_device < MAX_SIS8300_DEVICES)) ;
    //} while ((sis8300_device >= 0) && (i_device < 2)) ;
    if(i_device > 1)
    {
        use_class_sis8300_device_index=0;
        fBox_SIS8300_modules->Select(use_class_sis8300_device_index,kTRUE);

        SIS8300_Read_Register(use_gl_class_sis8300_device[use_class_sis8300_device_index],  0, &data) ;

        #ifdef DWC8VM1
        if((data&0xF000) != 0x2000){
            printf("Wrong FW version: 0x%X (expected: 0x2xxx)\n",data&0xFFFF);
            fMenuSIS8300Test->DisableEntry(1);
        }
        else{
            fMenuSIS8300Test->EnableEntry(1);
        }
        #endif

        sprintf(s,"Firmware Version 0x%08x",data);
        fLabel_main_frameh2[0]->SetText(s);
        SIS8300_Read_Register(use_gl_class_sis8300_device[use_class_sis8300_device_index],  1, &data) ;
        sprintf(s,"Serial Number      %d  ",data);
        fLabel_main_frameh2[1]->SetText(s);

        adc_device_no = 0;
        SIS8300_ADC_SPI_Read(use_gl_class_sis8300_device[use_class_sis8300_device_index], adc_device_no, 0x1, &data);
        data = data & 0xff ;
        if (data == 0x32)
        {
            sprintf(s,"ADC chip ID      0x%02x      ->  125MHz 16 bit",data & 0xff);
        }
        else
        {
            sprintf(s,"ADC chip ID      0x%02x      ->  250MHz 14 bit",data & 0xff);
        }
        fLabel_main_frameh2[2]->SetText(s);

#if newraus
        dev_pointer = (PSIS830X_DEVICE)malloc(sizeof(SIS830X_DEVICE));
        dev_pointer->fp = use_gl_class_sis8300_device[use_class_sis8300_device_index];
        dev_pointer->open = true;
        dev_pointer->type = SIS8300L;
        uint8_t temp;
 /*
        uint32_t regTemp;
        sis830x_ReadRegister(dev_pointer, 0x0, &regTemp);
        printf("RegOut: %X\n", regTemp);
*/

#endif // raus


    }

    this->AddFrame(fCanvasWindow, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY,
                   0, 0, 2, 2));
    // Create status frame containing a button and a text entry widget
    fStatusFrame = new TGCompositeFrame(this, 60, 20, kHorizontalFrame |
                                        kSunkenFrame);
    fTestText = new TGTextEntry(fStatusFrame, new TGTextBuffer(100));
    fTestText->SetToolTipText("This is a text entry widget");
    fTestText->Resize(300, fTestText->GetDefaultHeight());
    fStatusFrame->AddFrame(fTestText, new TGLayoutHints(kLHintsTop | kLHintsLeft,
                           10, 2, 2, 2));
    AddFrame(fStatusFrame, new TGLayoutHints(kLHintsBottom | kLHintsExpandX, 0, 0, 1, 0));

    /******************************************************************************************/

    this->SetWindowName("SIS Test (01.06.2015)");
    SetWMPosition(MAIN_WINDOW_POSTION_X,MAIN_WINDOW_POSTION_Y);
    MapSubwindows();
    Resize(MAIN_WINDOW_WIDTH, MAIN_WINDOW_HIGH);   // resize to default size
    this->MapWindow();
}





TestMainFrame::~TestMainFrame()
{
    // Delete all created widgets.

    delete fMenuFile;
    delete fMenuSIS8300Test;
    delete fMenuHelp;
    delete fContainer;
}

void TestMainFrame::CloseWindow()
{
    // Got close message for this MainFrame. Terminate the application
    // or returns from the TApplication event loop (depending on the
    // argument specified in TApplication::Run()).

    gApplication->Terminate(0);
}


/*******************************************************************************************************************************/

Bool_t TestMainFrame::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
    // Handle messages send to the TestMainFrame object. E.g. all menu button
    // messages.
    unsigned int data ;
    unsigned int adc_device_no  ;
    char s[64];
    switch (GET_MSG(msg))
    {
    case kC_TEXTENTRY:
        //printf("TestMainFrame: kC_TEXTENTRY item SUBMSG= %d   parm1 = %d activated\n", GET_SUBMSG(msg) , parm1);
        switch (GET_SUBMSG(msg))
        {
        case kTE_TEXTCHANGED:
            switch(parm1)
            {
            case 20:
                //printf("\n pressed 20\n");
                break;
            case 21:
                //printf("\n pressed 21\n");
                break;
            }
            break;
        }
        break;

    case kC_COMMAND:
        switch (GET_SUBMSG(msg))
        {

        case kCM_LISTBOX:
            //printf("ListBox was pressed, id = %ld\n", parm1);
            use_class_sis8300_device_index = fBox_SIS8300_modules->GetSelected();

            SIS8300_Read_Register(use_gl_class_sis8300_device[use_class_sis8300_device_index],  0, &data) ;

            #ifdef DWC8VM1
            if((data&0xF000) != 0x2000){
                printf("Wrong FW version: 0x%X (expected: 0x2xxx)\n",data&0xFFFF);
                fMenuSIS8300Test->DisableEntry(1);
            }
            else{
                fMenuSIS8300Test->EnableEntry(1);
            }
            #endif

            sprintf(s,"Firmware Version 0x%08x",data);
            fLabel_main_frameh2[0]->SetText(s);
            SIS8300_Read_Register(use_gl_class_sis8300_device[use_class_sis8300_device_index],  1, &data) ;
            sprintf(s,"Serial Number      %d  ",data);
            fLabel_main_frameh2[1]->SetText(s);

            adc_device_no = 0;
            SIS8300_ADC_SPI_Read(use_gl_class_sis8300_device[use_class_sis8300_device_index], adc_device_no, 0x1, &data);
            data = data & 0xff ;
            if (data == 0x32)
            {
                sprintf(s,"ADC chip ID      0x%02x      ->  125MHz 16 bit",data & 0xff);
            }
            else
            {
                sprintf(s,"ADC chip ID      0x%02x      ->  250MHz 14 bit",data & 0xff);
            }
            fLabel_main_frameh2[2]->SetText(s);
            printf("SIS8300_ADC_SPI_Read  ADC chip ID = 0x%02x \n", data);



            break;

        case kCM_BUTTON:
            //printf("Button was pressed, id = %ld\n", parm1);
            break;

        case kCM_MENUSELECT:
            //printf("Pointer over menu entry, id=%ld\n", parm1);
            break;

        case kCM_MENU:
            switch (parm1)
            {


            case M_FILE_EXIT:
                CloseWindow();   // this also terminates theApp
                break;


            case M_SIS8300TEST1_DLG:
                printf("M_SIS8300TEST1_DLG\n");
                new SIS8300TestDialog(fClient->GetRoot(), this, use_gl_class_sis8300_device[use_class_sis8300_device_index], 800, 200);
                break;




            default:
                break;
            }
        default:
            break;
        }
    default:
        break;
    }


    return kTRUE;
}















/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/

// TGNumberEntry widget test dialog
const char *const SIS8300TestDialog::entryClock_source[15] =
{
    "internal Sample Clock 250 MHz",
    "internal Sample Clock 125 MHz",
    "internal Sample Clock 62.5 MHz",
    "internal Sample Clock 41.66 MHz",
    "internal Sample Clock 31.25 MHz",
    "internal Sample Clock 25 MHz",
    "internal Sample Clock 83.33 MHz",
    "external Front SMA",
    "external Front SMA via Multiplier CLKIN1 bypass",
    "internal ref Osc via Multiplier XA/XB bypass",
    "external Front SMA via Multiplier 10MHz to 125MHz",
    "external Front HARLINK",
    "external Rear RTM TCLKA (CLK1)",
    "external Rear RTM TCLKB (CLK2)",
    "external Rear RTM CLK2"
};



const char *const SIS8300TestDialog::numlabel[8] =
{
    "Sample Length",
    "Pre Trigger Delay",
    "free",
    "free",
    "free",
    "free",
    "Soft Average",
    "Tap Delay"
};

const Double_t SIS8300TestDialog::numinit[8] =
{
    1, 2, 3, 4, 5, 6,
    7,
    8
};


// graph
const char *const SIS8300TestDialog::chkDisAdcLabel[10] =
{
    "Ch 1",
    "Ch 2",
    "Ch 3",
    "Ch 4",
    "Ch 5",
    "Ch 6",
    "Ch 7",
    "Ch 8",
    "Ch 9",
    "Ch 10",
};

// graph
const char *const SIS8300TestDialog::AdcHistogramLabel[10] =
{
    "Histogram Ch 1",
    "Histogram Ch 2",
    "Histogram Ch 3",
    "Histogram Ch 4",
    "Histogram Ch 5",
    "Histogram Ch 6",
    "Histogram Ch 7",
    "Histogram Ch 8",
    "Histogram Ch 9",
    "Histogram Ch 10",
};


// histogram
const char *const SIS8300TestDialog::entryHistoLabel[12] =
{
    "No Histogramming",
    "Display Histogram Ch 1",
    "Display Histogram Ch 2",
    "Display Histogram Ch 3",
    "Display Histogram Ch 4",
    "Display Histogram Ch 5",
    "Display Histogram Ch 6",
    "Display Histogram Ch 7",
    "Display Histogram Ch 8",
    "Display Histogram Ch 9",
    "Display Histogram Ch 10",
    "Display Histogram Ch 1-10",
};

char errorstr[20];

// attenuator
struct atteElement SIS8300TestDialog::attenuationValue[8] =
{
    {0x3F, "    0 dB"},
    {0x3E, " -0,5 dB"},
    {0x3D, "   -1 dB"},
    {0x3B, "   -2 dB"},
    {0x37, "   -4 dB"},
    {0x2F, "   -8 dB"},
    {0x1F, "  -16 dB"},
    {0x00, "-31.5 dB"},
};

// amcModulatorType
const char *const SIS8300TestDialog::amcModulatorType[3] =
{
    "no AMC Signal",
    "free DC Signal",
    "ramp Signal",
};

const char *const SIS8300TestDialog::entryDisplayFFTWindowLabel[6] =
{
    "Rectangular window (no window)",
    "Hamming window",
    "Hann window",
    "Blackmann window",
    "Blackmann-Harris window",
    "No",
};

const char *const SIS8300TestDialog::entryDisplayFFTLabel[11] =
{
    "No FFT",
    "Display FFT Ch 1",
    "Display FFT Ch 2",
    "Display FFT Ch 3",
    "Display FFT Ch 4",
    "Display FFT Ch 5",
    "Display FFT Ch 6",
    "Display FFT Ch 7",
    "Display FFT Ch 8",
    "Display FFT Ch 9",
    "Display FFT Ch 10",
};


const char *const SIS8300TestDialog::sis8300_devices_txt[6] =
{
    "/dev/sis8300-0",
    "/dev/sis8300-1",
    "/dev/sis8300-2",
    "/dev/sis8300-3",
    "/dev/sis8300-4",
    "/dev/sis8300-5",
};


/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
// open Test dialog
SIS8300TestDialog::SIS8300TestDialog(const TGWindow *p, const TGWindow *main, Int_t sis8300_device_handle,
                                     UInt_t w, UInt_t h, UInt_t options)
    : TGTransientFrame(p, main, w, h, options)
{
    int i, j;
    unsigned int data ;
    char tempLabel[50];

    gl_class_sis8300_device = sis8300_device_handle ;
    SIS8300_ADC_SPI_Read(gl_class_sis8300_device, 0, 0x1, &data);
    if ((data & 0xff)  == 0x32)
    {
        gl_class_sis8300_250MHZ_FLAG = 0;
        gl_class_adc_tap_delay = 0;
    }
    else
    {
        gl_class_sis8300_250MHZ_FLAG = 1;
        gl_class_adc_tap_delay = 11;
    }

    // Create a dialog window. A dialog window pops up with respect to its
    // "main" window.
    Pixel_t yellow;
    fClient->GetColorByName("yellow", yellow);
    Pixel_t green;
    fClient->GetColorByName("green", green);
    Pixel_t red;
    fClient->GetColorByName("red", red);

    // use hierarchical cleani
    SetCleanup(kDeepCleanup);
    TGGC myGC = *fClient->GetResourcePool()->GetFrameGC();
    TGFont *myfont = fClient->GetFont("-adobe-helvetica-bold-r-*-*-12-*-*-*-*-*-iso8859-1");
    if (myfont) myGC.SetFont(myfont->GetFontHandle());

    fFrame1 = new TGHorizontalFrame(this, 160, 20, kFixedWidth);
    //fFrame1 = new TGHorizontalFrame(this, 500, 200 );

    fQuitButton = new TGTextButton(fFrame1, "&Quit",1);
    fQuitButton->Associate(this);


    fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY,  2, 2, 2, 2);
    fL2 = new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1);
    fL7 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  20, 2, 22, 2); // hints, left, right, top, bottom

    fFrame1->AddFrame(fQuitButton, fL1);

    //fFrame1->Resize(150, fOkButton->GetDefaultHeight());
    fFrame1->Resize(150, fQuitButton->GetDefaultHeight()); // quit button breite
    AddFrame(fFrame1, fL2);

    //--------- create Tab widget and some composite frames for Tab testing

#define scroll
#ifdef scroll
    TGCanvas *fCanvasScroll = new TGCanvas(this);
    TGViewPort *fViewPortScroll =  fCanvasScroll->GetViewPort();
    fTab = new TGTab(fViewPortScroll, 300, 300);
#endif

#ifndef scroll
    fTab = new TGTab(this, 300, 300);
#endif


    fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);
//   fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 6, 6, 6, 6);


    /**********************************************************************************************************/

// Tab1
#ifdef raus
    TGCompositeFrame *tf = fTab->AddTab("Tab 1");
    fF1 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);
    fF1->AddFrame(new TGTextButton(fF1, "&Test button", 0), fL3);
    fF1->AddFrame(fTxt1 = new TGTextEntry(fF1, new TGTextBuffer(100)), fL3);
    fF1->AddFrame(fTxt2 = new TGTextEntry(fF1, new TGTextBuffer(100)), fL3);
    tf->AddFrame(fF1, fL3);
    fTxt1->Resize(150, fTxt1->GetDefaultHeight());
    fTxt2->Resize(150, fTxt2->GetDefaultHeight());
#endif

    fSis8300_Test1 = kFALSE;
    fB_openfCanvas1WindowFlag = kFALSE; // Setup
    fB_openfCanvas2WindowFlag = kFALSE; // Setup
    fB_openfCanvas3WindowFlag = kFALSE; // Setup
    for (i=0; i<11; i++)
    {
        fGraph_ch[i]   = 0;
    }
    for (i=0; i<10; i++)
    {
        iHistoAdc[i]   = 0;
    }
    for (i=0; i<10; i++)
    {
        histo_pave_text[i]   = 0;
    }

    for (i=0; i<11; i++)
    {
        fGraph_fft[i]   = 0;
    }



    TGCompositeFrame *tf = fTab->AddTab("Sample Control");
    fF_tab1 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);

    fF_tab1_fGrp1 = new TGGroupFrame(fF_tab1, "Run Control");
    fF_tab1->AddFrame(fF_tab1_fGrp1, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    fF1 = new TGCompositeFrame(fF_tab1_fGrp1, 260, 220, kHorizontalFrame);
    fF_tab1_fGrp1->AddFrame(fF1, fL3);


    fStartB = new TGTextButton(fF1, "&Start Sampling", 40);
    fStopB  = new TGTextButton(fF1, "&Stop Sampling", 41);

    fStartB->Resize(200,50) ;
    fStopB->Resize(200,50) ;
    fStartB->ChangeBackground(green);
    fStopB->ChangeBackground(red);

    fStartB->Associate(this);
    fStopB->Associate(this);
    //fF1->AddFrame(fStartB, fL1);
    //fF1->AddFrame(fStopB, fL1);

    fF1->AddFrame(fStartB, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY,  2, 5, 5, 5));// hints, left, right, top, bottom
    fF1->AddFrame(fStopB, new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY,  5, 2, 5, 5));

    //tf->AddFrame(fF1, fL3);


    /**********************/

    fF_tab1_fGrp2 = new TGGroupFrame(fF_tab1, "Sample Clock");
    fF_tab1->AddFrame(fF_tab1_fGrp2, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    fF5 = new TGCompositeFrame(fF_tab1_fGrp2, 60, 20, kVerticalFrame);
    fF_tab1_fGrp2->AddFrame(fF5, fL3);


    fCombo_Clock_source = new TGComboBox(fF5, 45);
    //fCombo_Clock_source->Associate(this); // Event (IRQ) anmelden
    fL5 = new TGLayoutHints(kLHintsCenterY | kLHintsRight, 2, 2, 2, 2);
    fF5->AddFrame(fCombo_Clock_source, fL5);

    if (gl_class_sis8300_250MHZ_FLAG == 0)
    {
        for (i = 1; i < 15; i++)
        {
            fCombo_Clock_source->AddEntry(entryClock_source[i], i);
            fCombo_Clock_source->Select(1, kTRUE); //
        }
    }
    else
    {
        for (i = 0; i < 15; i++)
        {
            fCombo_Clock_source->AddEntry(entryClock_source[i], i);
            fCombo_Clock_source->Select(0, kTRUE); //
        }
    }
    fCombo_Clock_source->Resize(270, 20);

    #ifdef CLK2
        fCombo_Clock_source->Select(14, kTRUE);
        fCombo_Clock_source->SetEnabled(kFALSE);
    #endif // CLK2


    /**********************/
    fF_tab1_fGrp3 = new TGGroupFrame(fF_tab1, "Sample Parameter");
    fF_tab1->AddFrame(fF_tab1_fGrp3, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));
    fF5 = new TGCompositeFrame(fF_tab1_fGrp3, 60, 20, kVerticalFrame);
    fF_tab1_fGrp3->AddFrame(fF5, fL3);

    for ( i = 0; i < 8; i++)
    {
        fF[i] = new TGHorizontalFrame(fF5, 200, 30);
        fF5->AddFrame(fF[i], fL5);
        fNumericEntries[i] = new TGNumberEntry(fF[i], numinit[i], 12, i + 20, (TGNumberFormat::kNESInteger) ) ; //kNESHex
        //(TGNumberFormat::EStyle) i);
        //fNumericEntries[i]->Associate(this);
        fF[i]->AddFrame(fNumericEntries[i], fL5);
        fLabel[i] = new TGLabel(fF[i], numlabel[i], myGC(), myfont->GetFontStruct());
        fF[i]->AddFrame(fLabel[i], fL5);
    }

    if (gl_class_sis8300_250MHZ_FLAG == 0)
    {
        fNumericEntries[0]->SetIntNumber(32768); // sample_length
    }
    else
    {
        fNumericEntries[0]->SetIntNumber(4096); // sample_length
    }
    fNumericEntries[1]->SetIntNumber(511); // pretrigger delay

    fNumericEntries[6]->SetIntNumber(1); // soft_average

    //fNumericEntries[7]->SetIntNumber(0); // Tap delay
    fNumericEntries[7]->SetIntNumber(gl_class_adc_tap_delay); // Tap delay
    fNumericEntries[7]->Associate(this);

    tf->AddFrame(fF_tab1, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

    //make tab yellow
    TGTabElement *tabel = fTab->GetTabTab("Test 1");;
    tabel->ChangeBackground(yellow);




    /**********************************************************************************************************/
// Tab 2

    tf = fTab->AddTab("Display Control");
    fF_tab2 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);

    fF_tab2_fGrp1 = new TGGroupFrame(fF_tab2, "Raw data");
    fF_tab2->AddFrame(fF_tab2_fGrp1, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));


    fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2 ,2, 2, 12); //hints, left, right, top, bottom
    //fF2 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);
    fF[0] = new TGHorizontalFrame(fF_tab2_fGrp1, 250, 30);
    fF_tab2_fGrp1->AddFrame(fF[0], new TGLayoutHints(kLHintsExpandX, 2, 2, 12, 2));


    fChkDisplayAutoZoom = new TGCheckButton(fF[0], "Y-AutoZoom", 0);
    fChkDisplayAutoZoom->SetState(kButtonDown)   ; // is ON !
    fF[0]->AddFrame(fChkDisplayAutoZoom, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2)); //hints, left, right, top, bottom

    fF[1] = new TGHorizontalFrame(fF_tab2_fGrp1, 200, 30);
    fF_tab2_fGrp1->AddFrame(fF[1], new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 2, 2));
    fNumericEntriesGraph_Yaxis[0] = new TGNumberEntry(fF[1], 0 /* value */, 8 /* width */, 120 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
    //fNumericEntries[i]->Associate(this); // Event (IRQ) anmelden
    fF[1]->AddFrame(fNumericEntriesGraph_Yaxis[0],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[0] = new TGLabel(fF[1], "Y-max", myGC(), myfont->GetFontStruct());
    fF[1]->AddFrame(fLabel[0],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

    fF[2] = new TGHorizontalFrame(fF_tab2_fGrp1, 200, 30);
    fF_tab2_fGrp1->AddFrame(fF[2], new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 2, 10));
    fNumericEntriesGraph_Yaxis[1] = new TGNumberEntry(fF[2], 0 /* value */, 8 /* width */, 120 /* irq */ , (TGNumberFormat::kNESInteger) ) ; //kNESHex
    //fNumericEntries[i]->Associate(this); // Event (IRQ) anmelden
    fF[2]->AddFrame(fNumericEntriesGraph_Yaxis[1],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[1] = new TGLabel(fF[2], "Y-min", myGC(), myfont->GetFontStruct());
    fF[2]->AddFrame(fLabel[1],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

    if (gl_class_sis8300_250MHZ_FLAG == 0)
    {
        fNumericEntriesGraph_Yaxis[0]->SetIntNumber(65536); // Y-max
    }
    else
    {
        fNumericEntriesGraph_Yaxis[0]->SetIntNumber(16384); // Y-max
    }


    fNumericEntriesGraph_Yaxis[1]->SetIntNumber(0); // Y-min

    fVF[0] = new TGVerticalFrame(fF_tab2_fGrp1, 200, 30);
    fF_tab2_fGrp1->AddFrame(fVF[0], new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 2, 10));


    //fL2 = new TGLayoutHints(kLHintsTop | kLHintsRight | kLHintsExpandX,  150, 2, 2, 2); // hints, left, right, top, bottom
    fL2 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 2, 2); // hints, left, right, top, bottom
    for (i = 0; i < 10; i++)
    {
        fChkDisplayAdc[i] = new TGCheckButton(fVF[0], chkDisAdcLabel[i], 1+i);
        fChkDisplayAdc[i]->SetState(kButtonDown)   ; // is ON !
        fVF[0]->AddFrame(fChkDisplayAdc[i], new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandY , 2, 2, 1, 1));
    }



    // ************************************************************************************************************************************************************
    fL2 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 2, 2); // hints, left, right, top, bottom
// histograms
    fF_tab2_fGrp2 = new TGGroupFrame(fF_tab2, "Display ADC histograms");
    fF_tab2->AddFrame(fF_tab2_fGrp2, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    fCombo_Display_Histos = new TGComboBox(fF_tab2_fGrp2, 88);
    //fCombo_Display_Histos->Associate(this); // Event (IRQ) anmelden
    fF_tab2_fGrp2->AddFrame(fCombo_Display_Histos,  new TGLayoutHints(kLHintsLeft ,  5, 2, 15, 2));
    // tf->AddFrame(fF2, fL3);

    for (i = 0; i < 12; i++)
    {
        fCombo_Display_Histos->AddEntry(entryHistoLabel[i], i);
    }
    fCombo_Display_Histos->Select(0, kTRUE); // no display
    fCombo_Display_Histos->Resize(200, 20);


    fChkHistoSum = new TGCheckButton(fF_tab2_fGrp2, "Histogram Sum", 0);
    fChkHistoSum->SetState(kButtonUp)   ; // is Off !
    fF_tab2_fGrp2->AddFrame(fChkHistoSum, fL2);

    fChkHistoZoomMean = new TGCheckButton(fF_tab2_fGrp2, "Histogram Zoom to Mean", 0);
    fChkHistoZoomMean->SetState(kButtonUp)   ; // is Off !
    fF_tab2_fGrp2->AddFrame(fChkHistoZoomMean, fL2);

    fChkHistoDisplayMinMax = new TGCheckButton(fF_tab2_fGrp2, "Display Min/Max Info", 0);
    fChkHistoDisplayMinMax->SetState(kButtonDown)   ; // is On !
    fF_tab2_fGrp2->AddFrame(fChkHistoDisplayMinMax, fL2);

    fChkHistoLogY = new TGCheckButton(fF_tab2_fGrp2, "Display LogY", 0);
    fChkHistoLogY->SetState(kButtonUp)   ; // is Off !
    fF_tab2_fGrp2->AddFrame(fChkHistoLogY, fL2);

    fChkHistoGaussFit = new TGCheckButton(fF_tab2_fGrp2, "Histogram Gauss Fit", 0);
    fChkHistoGaussFit->SetState(kButtonUp)   ; // is Off !
    fF_tab2_fGrp2->AddFrame(fChkHistoGaussFit, fL1);

    /**********/


// FFT
    fF_tab2_fGrp3 = new TGGroupFrame(fF_tab2, "Display FFT ");
    fF_tab2->AddFrame(fF_tab2_fGrp3, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));


    fCombo_Display_FFT_Ch = new TGComboBox(fF_tab2_fGrp3, 89);
    //fCombo_Display_FFT_Ch->Associate(this); // Event (IRQ) anmelden
    //fL7 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  20, 2, 22, 2); // hints, left, right, top, bottom
    fF_tab2_fGrp3->AddFrame(fCombo_Display_FFT_Ch, fL7);

    for (i = 0; i < 11; i++)
    {
        fCombo_Display_FFT_Ch->AddEntry(entryDisplayFFTLabel[i], i);
    }
    fCombo_Display_FFT_Ch->Select(0, kTRUE); // display ch1
    fCombo_Display_FFT_Ch->Resize(200, 20);

    fCombo_Display_FFT_Window = new TGComboBox(fF_tab2_fGrp3, 90);
    fF_tab2_fGrp3->AddFrame(fCombo_Display_FFT_Window, fL2);
    for (i = 0; i < 6; i++)
    {
        fCombo_Display_FFT_Window->AddEntry(entryDisplayFFTWindowLabel[i], i);
    }
    fCombo_Display_FFT_Window->Select(0, kTRUE); // d
    fCombo_Display_FFT_Window->Resize(200, 20);




    fChkFFT_Sum = new TGCheckButton(fF_tab2_fGrp3, "FFT Spectrum Sum", 100);
    fChkFFT_Sum->Associate(this); // Event (IRQ) anmelden
    fChkFFT_Sum->SetState(kButtonUp)   ; // is Off !
    fChkFFT_Sum->SetEnabled(kFALSE)   ; //
    fF_tab2_fGrp3->AddFrame(fChkFFT_Sum, fL2);

    fChkFFTLogY = new TGCheckButton(fF_tab2_fGrp3, "FFT Display LogY", 101);
    fChkFFTLogY->Associate(this); // Event (IRQ) anmelden
    fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
    fChkFFTLogY->SetEnabled(kFALSE)   ; //
    fF_tab2_fGrp3->AddFrame(fChkFFTLogY, fL2);

    fChkFFT_Db = new TGCheckButton(fF_tab2_fGrp3, "FFT in dB", 102);
    fChkFFT_Db->Associate(this); // Event (IRQ) anmelden
    fChkFFT_Db->SetEnabled(kTRUE)   ; //
    fChkFFT_Db->SetState(kButtonDown)   ; // is On !
    fF_tab2_fGrp3->AddFrame(fChkFFT_Db, fL2);

    fChkFFT_AutoScale = new TGCheckButton(fF_tab2_fGrp3, "FFT Autoscale", 0);
    fChkFFT_AutoScale->SetState(kButtonUp)   ; // is Off !
    fF_tab2_fGrp3->AddFrame(fChkFFT_AutoScale, fL1);

    /**********/

    tf->AddFrame(fF_tab2, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));



    /**********************************************************************************************************/
// Tab 3

    tf = fTab->AddTab("DS8VM1 Control");
    fF_tab3 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);


    /*** Attenuator ***/

    fF_tab3_fGrp1 = new TGGroupFrame(fF_tab3, "Attenuator");
    fF_tab3->AddFrame(fF_tab3_fGrp1, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    //fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2 ,2, 2, 12); //hints, left, right, top, bottom
    //fF2 = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);
    fF[0] = new TGHorizontalFrame(fF_tab3_fGrp1, 250, 30);
    fF_tab3_fGrp1->AddFrame(fF[0], new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    for(j=0; j < 8; j++)
    {
        fCombo_CHX_Attenuator[j] = new TGComboBox(fF_tab3_fGrp1, 88);
        fF_tab3_fGrp1->AddFrame(fCombo_CHX_Attenuator[j],  new TGLayoutHints(kLHintsLeft ,  5, 2, 10, 2));

        for (i = 0; i < 8; i++)
        {
            sprintf(tempLabel, "CH %02d:  %s", j+1, attenuationValue[i].discrip);
            fCombo_CHX_Attenuator[j]->AddEntry(tempLabel, i);
        }
        fCombo_CHX_Attenuator[j]->Select(0, kTRUE); // set to 0 dBm
        fCombo_CHX_Attenuator[j]->Resize(200, 20);
    }

    fCombo_CHX_Attenuator[j] = new TGComboBox(fF_tab3_fGrp1, 88);
    fF_tab3_fGrp1->AddFrame(fCombo_CHX_Attenuator[j],  new TGLayoutHints(kLHintsLeft ,  5, 2, 10, 2));

    for (i = 0; i < 8; i++)
    {
        sprintf(tempLabel, "Vector Modulator:  %s", attenuationValue[i].discrip);
        fCombo_CHX_Attenuator[j]->AddEntry(tempLabel, i);
    }
    fCombo_CHX_Attenuator[j]->Select(0, kTRUE); // set to 0 dBm
    fCombo_CHX_Attenuator[j]->Resize(200, 20);


    fVF[1] = new TGVerticalFrame(fF_tab3_fGrp1, 200, 30);
    fF_tab3_fGrp1->AddFrame(fVF[1], new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 2, 10));


    /*** Vector Modulator ***/

    fL2 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 5, 2); // hints, left, right, top, bottom

    fF_tab3_fGrp2 = new TGGroupFrame(fF_tab3, "Vector Modulator");
    fF_tab3->AddFrame(fF_tab3_fGrp2, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

//  fChkDisplayAdc[i] = new TGCheckButton(fVF[0], chkDisAdcLabel[i], 1+i);
    fChkAMCInterlock = new TGCheckButton(fF_tab3_fGrp2, "AMC Interlock", 0);
    fChkAMCInterlock->SetState(kButtonDown);  // is On!
    fF_tab3_fGrp2->AddFrame(fChkAMCInterlock, fL2);

    fCombo_AMC_Modulator_Type = new TGComboBox(fF_tab3_fGrp2, 88);
    fF_tab3_fGrp2->AddFrame(fCombo_AMC_Modulator_Type,  new TGLayoutHints(kLHintsLeft ,  5, 2, 10, 2));

    for (i = 0; i < 3; i++)
    {
        fCombo_AMC_Modulator_Type->AddEntry(amcModulatorType[i], i);
    }
    fCombo_AMC_Modulator_Type->Select(1, kTRUE); // no AMC Signal
    fCombo_AMC_Modulator_Type->Resize(200, 20);

    fF[3] = new TGHorizontalFrame(fF_tab3_fGrp2, 200, 30);
    fF_tab3_fGrp2->AddFrame(fF[3], new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 10, 2));
    fNumericEntriesVectorVoltages[0] = new TGNumberEntry(fF[3], 0 /* value */, 8 /* width */, 120 /* irq */ , (TGNumberFormat::kNESRealTwo) ) ; //kNESHex
    fF[3]->AddFrame(fNumericEntriesVectorVoltages[0],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[2] = new TGLabel(fF[3], "I DAC", myGC(), myfont->GetFontStruct());
    fF[3]->AddFrame(fLabel[2],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

    fF[4] = new TGHorizontalFrame(fF_tab3_fGrp2, 200, 30);
    fF_tab3_fGrp2->AddFrame(fF[4], new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 2, 2));
    fNumericEntriesVectorVoltages[1] = new TGNumberEntry(fF[4], 0 /* value */, 8 /* width */, 120 /* irq */ , (TGNumberFormat::kNESRealTwo) ) ; //kNESHex
    fF[4]->AddFrame(fNumericEntriesVectorVoltages[1],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[3] = new TGLabel(fF[4], "Q DAC", myGC(), myfont->GetFontStruct());
    fF[4]->AddFrame(fLabel[3],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

    fNumericEntriesVectorVoltages[0]->SetLimitValues(-1.0, 1.0);
    fNumericEntriesVectorVoltages[0]->SetNumber(0.0);
    fNumericEntriesVectorVoltages[1]->SetLimitValues(-1.0, 1.0);
    fNumericEntriesVectorVoltages[1]->SetNumber(0.0);

    fVF[2] = new TGVerticalFrame(fF_tab3_fGrp2, 200, 30);
    fF_tab3_fGrp2->AddFrame(fVF[2], new TGLayoutHints(kLHintsTop | kLHintsLeft ,  2, 2, 2, 10));


    /*** Common Mode ***/

    fF_tab3_fGrp3 = new TGGroupFrame(fF_tab3, "Common Mode");
    fF_tab3->AddFrame(fF_tab3_fGrp3, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    fF[8] = new TGHorizontalFrame(fF_tab3_fGrp3, 200, 30);
    fF_tab3_fGrp3->AddFrame(fF[8], new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 10, 2));
    fNumericEntriesCommonVoltages[0] = new TGNumberEntry(fF[8], 0 /* value */, 8 /* width */, 120 /* irq */ , (TGNumberFormat::kNESRealTwo) ) ; //kNESHex
    fF[8]->AddFrame(fNumericEntriesCommonVoltages[0],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[4] = new TGLabel(fF[8], "Out Voltage I", myGC(), myfont->GetFontStruct());
    fF[8]->AddFrame(fLabel[4],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

    fF[9] = new TGHorizontalFrame(fF_tab3_fGrp3, 200, 30);
    fF_tab3_fGrp3->AddFrame(fF[9], new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX,  2, 2, 2, 2));
    fNumericEntriesCommonVoltages[1] = new TGNumberEntry(fF[9], 0 /* value */, 8 /* width */, 120 /* irq */ , (TGNumberFormat::kNESRealTwo) ) ; //kNESHex
    fF[9]->AddFrame(fNumericEntriesCommonVoltages[1],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));
    fLabel[5] = new TGLabel(fF[9], "Out Voltage II", myGC(), myfont->GetFontStruct());
    fF[9]->AddFrame(fLabel[5],  new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2));

    fNumericEntriesCommonVoltages[0]->SetLimitValues(0.0, 5.0);
    fNumericEntriesCommonVoltages[0]->SetNumber(0.85);
    fNumericEntriesCommonVoltages[1]->SetLimitValues(0.0, 5.0);
    fNumericEntriesCommonVoltages[1]->SetNumber(0.85);


    /*** LO/REF Distribution ***/

    fReadSensors = kFALSE;

    fF_tab3_fGrp4 = new TGGroupFrame(fF_tab3, "LO/REF Distribution");
    fF_tab3->AddFrame(fF_tab3_fGrp4, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    fLabel_tab3_frameh3[0] = new TGLabel(fF_tab3_fGrp4,"LO_Power:");
    fLabel_tab3_frameh3[0]->SetTextJustify(kTextLeft + kTextCenterX );
    fLabel_tab3_frameh3[0]->SetMargins(0,0,0,0);
    fLabel_tab3_frameh3[0]->SetWrapLength(-1);
    fF_tab3_fGrp4->AddFrame(fLabel_tab3_frameh3[0], new TGLayoutHints(kLHintsExpandX,2,2,5,2));

    fLabel_tab3_frameh3[1] = new TGLabel(fF_tab3_fGrp4,"LO_Temp:");
    fLabel_tab3_frameh3[1]->SetTextJustify(kTextLeft + kTextCenterX );
    fLabel_tab3_frameh3[1]->SetMargins(0,0,0,0);
    fLabel_tab3_frameh3[1]->SetWrapLength(-1);
    fF_tab3_fGrp4->AddFrame(fLabel_tab3_frameh3[1], new TGLayoutHints(kLHintsExpandX,2,2,5,2));

    fLabel_tab3_frameh3[2] = new TGLabel(fF_tab3_fGrp4,"REF_Power:");
    fLabel_tab3_frameh3[2]->SetTextJustify(kTextLeft + kTextCenterX );
    fLabel_tab3_frameh3[2]->SetMargins(0,0,0,0);
    fLabel_tab3_frameh3[2]->SetWrapLength(-1);
    fF_tab3_fGrp4->AddFrame(fLabel_tab3_frameh3[2], new TGLayoutHints(kLHintsExpandX,2,2,10,2));

    fLabel_tab3_frameh3[3] = new TGLabel(fF_tab3_fGrp4,"REF_Temp:");
    fLabel_tab3_frameh3[3]->SetTextJustify(kTextLeft + kTextCenterX );
    fLabel_tab3_frameh3[3]->SetMargins(0,0,0,0);
    fLabel_tab3_frameh3[3]->SetWrapLength(-1);
    fF_tab3_fGrp4->AddFrame(fLabel_tab3_frameh3[3], new TGLayoutHints(kLHintsExpandX,2,2,5,2));

    fRDButton = new TGTextButton(fF_tab3_fGrp4, "&Read", 60);
    fRDButton->Resize(40,50);
    fRDButton->Associate(this);
    fF_tab3_fGrp4->AddFrame(fRDButton, new TGLayoutHints(kLHintsExpandX,2,2,10,2));


    /*** Min/Max ***/

    fF_tab3_fGrp5 = new TGGroupFrame(fF_tab3, "Min/Max Values");
    fF_tab3->AddFrame(fF_tab3_fGrp5, new TGLayoutHints(kLHintsExpandX, 5, 5, 5, 5));

    fMinMaxButton = new TGTextButton(fF_tab3_fGrp5, "Read &Min/Max", 61);
    fMinMaxButton->Resize(40,50);
    fMinMaxButton->Associate(this);
    fF_tab3_fGrp5->AddFrame(fMinMaxButton, new TGLayoutHints(kLHintsExpandX,2,2,10,2));

    /**********/

    tf->AddFrame(fF_tab3, new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5));

    /**********************************************************************************************************/

    //TGLayoutHints *fL5 = new TGLayoutHints(kLHintsBottom | kLHintsExpandX |
    //                                       kLHintsExpandY, 2, 2, 5, 1);
#ifdef scroll
    fViewPortScroll->AddFrame(fTab, new TGLayoutHints(kLHintsNormal | kLHintsExpandX | kLHintsExpandY));

    //fVert1->SetLayoutManager(new TGTileLayout(fVert1,8));
    fTab->MapSubwindows();
    fCanvasScroll->SetContainer(fTab);
    fCanvasScroll->MapSubwindows();
    this->AddFrame(fCanvasScroll, new TGLayoutHints(kLHintsLeft | kLHintsTop | kLHintsExpandX | kLHintsExpandY ,2,2,2,2));

#endif

#ifndef scroll
    AddFrame(fTab, fL5);
#endif


    MapSubwindows();
    SetWMPosition(SIS8300_TEST_WINDOW_POSTION_X, SIS8300_TEST_WINDOW_POSTION_Y);
    Resize(SIS8300_TEST_WINDOW_WIDTH, SIS8300_TEST_WINDOW_HIGH);   // resize to default size



    // position relative to the parent's window
    //CenterOnParent();
    SetWindowName("SIS DWC8VM1 Test Dialog");
    MapWindow();
    //fClient->WaitFor(this);    // otherwise canvas contextmenu does not work
}


/********************************************************************************************************************************/
#if newraus
// Set the attenuator
// @param addr attenuator number 1 - 8
// @param data
SIS830X_STATUS setAttenuator(uint8_t addr, uint8_t data)
{
    SIS830X_STATUS stat;

    static uint8_t initPCA9535 = false;
    uint8_t i, mask = 0x20, data_tmp=0;

    if(!initPCA9535)
    {

        // Set PORT0 to output
        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 6, 0x00);
        if(stat != Stat830xSuccess) return stat;

        // Set PORT1 to output
        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 7, 0x00);
        if(stat != Stat830xSuccess) return stat;

        initPCA9535 = true;
    }

    stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 2, data_tmp);
    if(stat != Stat830xSuccess) return stat;

    stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 3, 0x8);       // VCM_DAC_CS to high!
    if(stat != Stat830xSuccess) return stat;

    for(i=0; i<6; i++)
    {
        if(mask & data)
        {
            data_tmp |= 0x02;
        }
        else
        {
            data_tmp &= ~0x02;
        }

        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 2, data_tmp);
        if(stat != Stat830xSuccess) return stat;
        mask>>=1;

        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 2, data_tmp | 1);
        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 2, data_tmp);
        if(stat != Stat830xSuccess) return stat;
    }

    stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 2, 0);

    if(addr < 6)
    {
        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 2, 1<<(addr+2));
        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 2, 0);
    }
    else
    {
        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 3, 0x8 | 1<<(addr-6));  // VCM_DAC_CS const to high!
        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 3, 0x8);
    }

    return stat;
}
#endif // newraus
/********************************************************************************************************************************/
#if newraus
// Init common mode DAC
// @param data init data
SIS830X_STATUS setCommonModeDAC(uint8_t adc_no, double value)
{
    SIS830X_STATUS stat;

    static uint8_t initPCA9535 = false;
    uint32_t mask = 0x800000, send_data=0;
    uint8_t i, data_tmp=0;
    uint16_t initValue;


    initValue = (uint16_t)(value/1e-3);


    if(!initPCA9535)
    {

        // Set PORT0 to output
        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 6, 0x00);
        if(stat != Stat830xSuccess) return stat;

        // Set PORT1 to output
        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 7, 0x00);
        if(stat != Stat830xSuccess) return stat;

        initPCA9535 = true;
    }

    send_data = (initValue << 4);

    // Command Definition :
    // C2 - C0:     3h     (Write to and update DAC)
    // Address Command:
    // A2 - A0:     7h     (All DACs)

    // CmdAddr [x x C2 C1 C0 A2 A1 A0]:     0x1F
    send_data |= (((0x3 << 3) | (adc_no & 0x7)) << 16);

    stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 2, data_tmp);
    if(stat != Stat830xSuccess) return stat;

    stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 3, 0x8);       // VCM_DAC_CS to high!
    stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 3, 0x0);       // VCM_DAC_CS to low!

    for(i=0; i<24; i++)
    {
        if(mask & send_data)
        {
            data_tmp |= 0x02;
        }
        else
        {
            data_tmp &= ~0x02;
        }

        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 2, data_tmp);
        if(stat != Stat830xSuccess) return stat;
        mask>>=1;

        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 2, data_tmp | 1);
        stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 2, data_tmp);
        if(stat != Stat830xSuccess) return stat;
    }

    stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 2, 0);

    stat = sis830x_rtm_pca9535_Write(dev_pointer, PCA9535ADDR, 3, 0x8);       // VCM_DAC_CS to high!

    return stat;
}

/********************************************************************************************************************************/
// Read the powerlevel of the LO section
// @param value
SIS830X_STATUS readADCTemp(float *value){

    uint8_t LTC2493_addr = 0x34;
    uint16_t twrite = 0xC0B0;
    uint32_t tread;

    stat = sis830x_rtm_ltc2493_Write(dev_pointer, LTC2493_addr, &twrite);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Write I²C failed (%d): %s\n",stat, errorstr);
        return stat;
    }

    usleep(500000);

    stat = sis830x_rtm_ltc2493_Read(dev_pointer, LTC2493_addr, &tread);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Read I²C failed (%d): %s\n",stat, errorstr);
        return stat;
    }
    else{

        tread = ((tread & 0xFF)<<24) | ((tread & 0xFF00)<<8) | ((tread & 0xFF0000)>>8) | ((tread & 0xFF000000)>>24);
        tread &= ~0x80000000;
        tread >>= 7;

        *value = (tread*(3.3/1570))- 273.15;


    }


    return Stat830xSuccess;
}

/********************************************************************************************************************************/
// Read the powerlevel of the LO section
// @param value
SIS830X_STATUS readLoVoltage(float *value){

    uint8_t LTC2493_addr = 0x34;
    uint16_t twrite = 0x80B0;
    uint32_t tread;

    stat = sis830x_rtm_ltc2493_Write(dev_pointer, LTC2493_addr, &twrite);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Write I²C failed (%d): %s\n",stat, errorstr);
        return stat;
    }

   usleep(200000);

    stat = sis830x_rtm_ltc2493_Read(dev_pointer, LTC2493_addr, &tread);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Read I²C failed (%d): %s\n",stat, errorstr);
        return stat;
    }
    else{

        tread = ((tread & 0xFF)<<24) | ((tread & 0xFF00)<<8) | ((tread & 0xFF0000)>>8) | ((tread & 0xFF000000)>>24);
        tread &= ~0x80000000;
        tread >>= 6;

        *value = (tread*(3.3/0x1000000));

    }


    return Stat830xSuccess;
}

/********************************************************************************************************************************/
// Read the LO temp sensor
// @param value
SIS830X_STATUS readLoTemp(float *value){

    uint8_t LTC2493_addr = 0x34;
    uint16_t twrite = 0x80B8;
    uint32_t tread;


    stat = sis830x_rtm_ltc2493_Write(dev_pointer, LTC2493_addr, &twrite);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Write I²C failed (%d): %s\n",stat, errorstr);
    }

    usleep(200000);

    stat = sis830x_rtm_ltc2493_Read(dev_pointer, LTC2493_addr, &tread);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Read I²C failed (%d): %s\n",stat, errorstr);
    }
    else{

        tread = ((tread & 0xFF)<<24) | ((tread & 0xFF00)<<8) | ((tread & 0xFF0000)>>8) | ((tread & 0xFF000000)>>24);
        tread &= ~0x80000000;
        tread>>=6;

        //*value = (tread*(3.3/0x1000000))*40; // (tRead*Ulsb) * 2/0.05
        *value = ((tread*(3.3/0x1000000))-1.27)/0.005; // (tRead*Ulsb) * 2/0.05

    }



   return Stat830xSuccess;
}

/********************************************************************************************************************************/
// Read the powerlevel of the REF section
// @param value
SIS830X_STATUS readRefVoltage(float *value){

    uint8_t LTC2493_addr = 0x34;
    uint16_t twrite = 0x80B1;
    uint32_t tread;

    stat = sis830x_rtm_ltc2493_Write(dev_pointer, LTC2493_addr, &twrite);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Write I²C failed (%d): %s\n",stat, errorstr);
        return stat;
    }

    usleep(200000);

    stat = sis830x_rtm_ltc2493_Read(dev_pointer, LTC2493_addr, &tread);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Read I²C failed (%d): %s\n",stat, errorstr);
        return stat;
    }
    else{

        tread = ((tread & 0xFF)<<24) | ((tread & 0xFF00)<<8) | ((tread & 0xFF0000)>>8) | ((tread & 0xFF000000)>>24);
        tread &= ~0x80000000;
        tread >>= 6;

        *value = (tread*(3.3/0x1000000));

    }

    return Stat830xSuccess;
}
/********************************************************************************************************************************/
// Read the REF temp sensor
// @param value
SIS830X_STATUS readRefTemp(float *value){

    uint8_t LTC2493_addr = 0x34;
    uint16_t twrite = 0x80B9;
    uint32_t tread;


    stat = sis830x_rtm_ltc2493_Write(dev_pointer, LTC2493_addr, &twrite);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Write I²C failed (%d): %s\n",stat, errorstr);
    }

    usleep(200000);

    stat = sis830x_rtm_ltc2493_Read(dev_pointer, LTC2493_addr, &tread);
    if(stat != Stat830xSuccess){
        sis830x_status2str(stat, errorstr);
        printf("Read I²C failed (%d): %s\n",stat, errorstr);
    }
    else{

        tread = ((tread & 0xFF)<<24) | ((tread & 0xFF00)<<8) | ((tread & 0xFF0000)>>8) | ((tread & 0xFF000000)>>24);
        tread &= ~0x80000000;
        tread>>=6;

//      *value = (tread*(3.3/0x1000000))*40; // (tRead*Ulsb) * 2/0.05
        *value = ((tread*(3.3/0x1000000))-1.27)/0.005; // (tRead*Ulsb) * 2/0.05

    }

    return Stat830xSuccess;
}

/********************************************************************************************************************************/

uint16_t double2hex16bit(double value){

    uint16_t temp;

    if(value < 0 ){
        value*=-1;
        temp = 0x7FFF + (uint16_t)(value/30.5176e-6);
    }
    else{
        temp = 0x7FFF - (uint16_t)(value/30.5176e-6);
    }

    return temp;
}

#endif // newraus

/********************************************************************************************************************************/



SIS8300TestDialog::~SIS8300TestDialog()
{
    // Delete test dialog widgets.
    if (fB_openfCanvas1WindowFlag == kTRUE)
    {
        delete fCanvas1 ; //
    }
    if (fB_openfCanvas2WindowFlag == kTRUE)
    {
        delete fCanvas2 ; //
    }
    if (fB_openfCanvas3WindowFlag == kTRUE)
    {
        delete fCanvas3 ; //
    }
    fB_openfCanvas1WindowFlag = kFALSE; // Setup
    fB_openfCanvas2WindowFlag = kFALSE; // Setup
    fB_openfCanvas3WindowFlag = kFALSE; // Setup

}


/**********************************************************************************************************************************/

#define HISTOGRAM
#define FFT_GRAPH

void SIS8300TestDialog::Sis8300_Test1()
{
    // Fill histograms till user clicks "Stop Filling" button.

    unsigned int ui, plot_length;
    int ymin, ymax ;
    int ywidth, y_delta ;
    unsigned int i, k;
    unsigned int sample_length, sample_block_length ;
    unsigned int  pre_trigger_delay = 1 ;


    unsigned int clock_source_choice;
    unsigned int ad9510_divider_data;

    unsigned int histo_bin_value ;
    unsigned int histo_data_find_first_X_flag;
    unsigned int histo_data_find_first_X_value;
    unsigned int histo_data_find_last_X_value;
    unsigned int histo_data_first_X[10];
    unsigned int histo_data_last_X[10];

    unsigned int i_adc;

    unsigned int display_histo_counter, display_histo_ch_no ;
    unsigned int data ;
    unsigned short* ushort_adc_buffer_array_ptr[10]; //
    unsigned short* ushort_adc_buffer_ptr; //

    unsigned int display_histogram_choice, new_display_histogram_choice, changed_display_histogram_choice_flag;
    unsigned int histogram_gausfit_enable_flag, histogram_gausfit_clear_flag;

    unsigned int fChkHistoLogY_changed_flag, fChkHistoLogY_old_flag, fChkHistoLogY_new_flag;

    unsigned int display_FFT_choice;
    unsigned int fft_plot_length  ;
    double double_fft_frequency  ;

    double double_histo_min_x ;
    double double_histo_max_x;
    double double_histo_mean ;
    double double_histo_min_mean[10] ;
    double double_histo_max_mean[10] ;
    double double_histo_rms_error ;
    double double_histo_rms ;
    double double_histo_min_rms[10] ;
    double double_histo_max_rms[10] ;
    char char_temp[80] ;


    int sis8300_device;
    sis8300_reg myReg;

    unsigned int ad9510_divider_configuration_array[8]  ;

    unsigned int header_offset_ushort_ptr;

// average test
    unsigned int uint_soft_gain;
    unsigned int uint_soft_offset;
    unsigned int uint_soft_average;
    unsigned int uint_temp_average;

#ifdef DWC8VM1
    bool AMCInterlockOldStatus = kFALSE;
    double  VMOldNum[2];
    double  CMOldNum[2];
    uint8_t comboOldStatus[10];

    memset(CMOldNum, 100.0, 2 * sizeof(double));
    memset(VMOldNum, 100.0, 2 * sizeof(double));
    memset(comboOldStatus, 0xFF, 10);

    uint8_t temp;
    float adc_readout;
#endif // DWC8VM1


    ushort_adc_buffer_array_ptr[0] = gl_ushort_adc1_buffer;
    ushort_adc_buffer_array_ptr[1] = gl_ushort_adc2_buffer;
    ushort_adc_buffer_array_ptr[2] = gl_ushort_adc3_buffer;
    ushort_adc_buffer_array_ptr[3] = gl_ushort_adc4_buffer;
    ushort_adc_buffer_array_ptr[4] = gl_ushort_adc5_buffer;
    ushort_adc_buffer_array_ptr[5] = gl_ushort_adc6_buffer;
    ushort_adc_buffer_array_ptr[6] = gl_ushort_adc7_buffer;
    ushort_adc_buffer_array_ptr[7] = gl_ushort_adc8_buffer;
    ushort_adc_buffer_array_ptr[8] = gl_ushort_adc9_buffer;
    ushort_adc_buffer_array_ptr[9] = gl_ushort_adc10_buffer;

    sis8300_device = gl_class_sis8300_device ;
    if(sis8300_device < 0)
    {
        printf("can't open device\n");
        //return -1;
    }


// Parameter Setup
    sample_length = fNumericEntries[0]->GetIntNumber();
    if (sample_length > MAX_ROOT_PLOT_LENGTH)
    {
        sample_length = MAX_ROOT_PLOT_LENGTH - 15;
    }
    sample_block_length = (sample_length + 15) / 16 ;
    sample_length = (sample_block_length * 16) ;
    fNumericEntries[0]->SetIntNumber(sample_length );

    pre_trigger_delay = fNumericEntries[1]->GetIntNumber();
    if (pre_trigger_delay > MAX_PRETRIGGER_DELAY)
    {
        pre_trigger_delay = MAX_PRETRIGGER_DELAY ;
    }
    fNumericEntries[1]->SetIntNumber(pre_trigger_delay );

// printf("sample_length = %d \n", sample_length);
// printf("sample_block_length = %d \n", sample_block_length);

    plot_length = sample_length ;

    display_histo_counter = 0 ;
    display_histo_ch_no   = 0 ;



    /**************************************************************************************************************/
// Raw Data Graph Setup
    if (fB_openfCanvas1WindowFlag == kFALSE)
    {
        fCanvas1 = new TCanvas("fCanvas1","ADC ",SIS8300_RAW_DATA_WINDOW_POSTION_X, SIS8300_RAW_DATA_WINDOW_POSTION_Y, SIS8300_RAW_DATA_WINDOW_WIDTH, SIS8300_RAW_DATA_WINDOW_HIGH); // X, y, WITDH, High
        fB_openfCanvas1WindowFlag = kTRUE; //
    }
    fCanvas1->Clear();
    fCanvas1->cd(1);
    fCanvas1->SetGrid();
    fCanvas1->SetFillColor(DefineCanvasBackgroundColor);
    //TCanvas *fCanvas1 = new TCanvas("fCanvas1","A Simple Graph Example",200,10,700,500);

    for (ui=0; ui<plot_length; ui++)
    {
        gl_x[ui] = ui ;
    }

    if (!fGraph_ch[0])
    {
        for (i=0; i<11; i++)
        {
            fGraph_ch[i]   = new TGraph(MAX_ROOT_PLOT_LENGTH,gl_x,gl_y);
        }
        for (i=0; i<10; i++)
        {
            fGraph_Text_ch[i]   = new TLatex(plot_length,10,"Ch ");;
        }
    }

    fGraph_ch[10]->SetLineColor(DefineCanvasBackgroundColor);
    fGraph_ch[0]->SetLineColor(DefineChannel_1_Color);
    fGraph_ch[1]->SetLineColor(DefineChannel_2_Color);
    fGraph_ch[2]->SetLineColor(DefineChannel_3_Color);
    fGraph_ch[3]->SetLineColor(DefineChannel_4_Color);
    fGraph_ch[4]->SetLineColor(DefineChannel_5_Color);
    fGraph_ch[5]->SetLineColor(DefineChannel_6_Color);
    fGraph_ch[6]->SetLineColor(DefineChannel_7_Color);
    fGraph_ch[7]->SetLineColor(DefineChannel_8_Color);
    fGraph_ch[8]->SetLineColor(DefineChannel_9_Color);
    fGraph_ch[9]->SetLineColor(DefineChannel_10_Color);

    fGraph_Text_ch[0]->SetTextColor(DefineChannel_1_Color);
    fGraph_Text_ch[1]->SetTextColor(DefineChannel_2_Color);
    fGraph_Text_ch[2]->SetTextColor(DefineChannel_3_Color);
    fGraph_Text_ch[3]->SetTextColor(DefineChannel_4_Color);
    fGraph_Text_ch[4]->SetTextColor(DefineChannel_5_Color);
    fGraph_Text_ch[5]->SetTextColor(DefineChannel_6_Color);
    fGraph_Text_ch[6]->SetTextColor(DefineChannel_7_Color);
    fGraph_Text_ch[7]->SetTextColor(DefineChannel_8_Color);
    fGraph_Text_ch[8]->SetTextColor(DefineChannel_9_Color);
    fGraph_Text_ch[9]->SetTextColor(DefineChannel_10_Color);




#ifdef HISTOGRAM
    new_display_histogram_choice = fCombo_Display_Histos->GetSelected();
    display_histogram_choice = new_display_histogram_choice ;
    changed_display_histogram_choice_flag = 0 ;

    if (display_histogram_choice > 0)
    {

        if (fB_openfCanvas2WindowFlag == kFALSE)
        {
            fCanvas2 = new TCanvas("fCanvas2","Histo ", SIS8300_HISTOGRAM_WINDOW_POSTION_X, SIS8300_HISTOGRAM_WINDOW_POSTION_Y, SIS8300_HISTOGRAM_WINDOW_WIDTH, SIS8300_HISTOGRAM_WINDOW_HIGH);
            fB_openfCanvas2WindowFlag = kTRUE; // Setup
        }

        if (display_histogram_choice != 11)
        {
            fCanvas2->Clear();
            fCanvas2->Divide(1);
        }
        else
        {
            fCanvas2->Clear();
            fCanvas2->Divide(2,5);
        }

        fCanvas2->SetGrid();
        fCanvas2->SetFillColor(DefineCanvasBackgroundColor);
        for (i=0; i<10; i++)
        {
            fCanvas2->cd(1+i);
            gPad->SetGrid();
            gPad->SetFillColor(DefineCanvasBackgroundColor);
        }

    }
    else
    {
        if (fB_openfCanvas2WindowFlag == kTRUE)
        {
            delete fCanvas2 ;
            fB_openfCanvas2WindowFlag = kFALSE; //
        }
    }



    fChkHistoLogY_changed_flag = 1 ;
    fChkHistoLogY_old_flag = 0;
    if (fChkHistoLogY->IsOn() == kTRUE)
    {
        fChkHistoLogY_old_flag = 1;
    }


    histogram_gausfit_clear_flag  = 0 ;
    histogram_gausfit_enable_flag = 0 ;
    if (fChkHistoGaussFit->IsOn() == kTRUE)
    {
        histogram_gausfit_enable_flag = 1 ;
        histogram_gausfit_clear_flag  = 0 ;
    }
    else
    {
        if (histogram_gausfit_enable_flag == 1)
        {
            histogram_gausfit_clear_flag  = 1 ;
        }
        histogram_gausfit_enable_flag = 0 ;
    }


    unsigned i_histo_ch ;
    unsigned i_histo_length ;
    if (gl_class_sis8300_250MHZ_FLAG == 0)
    {
        i_histo_ch = 10 ;
        i_histo_length = 0x10000 ;
    }
    else
    {
        i_histo_ch = 8 ;
        i_histo_length = 0x4000 ;
    }

    if (!iHistoAdc[0])
    {
        for (i=0; i<10; i++)
        {
            iHistoAdc[i]   = new TH1I(AdcHistogramLabel[i],AdcHistogramLabel[i],i_histo_length,0,i_histo_length-1);
            //iHistoAdc[i]->SetName("b");
            gStyle->SetStatFormat("6.6g");
            //gStyle->SetStatFontSize(0.11);
            //gStyle->SetStatH(0.15);
            //gStyle->SetStatW(0.2);
            gStyle->SetStatH(0.36);
            gStyle->SetStatW(0.3);
            gStyle->SetStatX(0.99);
            gStyle->SetStatY(1.00);
        }
    }
    for (i=0; i<10; i++)
    {
        // fCanvas2->cd(1+i);
        //gPad->SetGrid();
        gPad->SetFillColor(DefineCanvasBackgroundColor);
        iHistoAdc[i]->Reset(); //
        iHistoAdc[i]->BufferEmpty(1); // action =  1 histogram is filled and buffer is deleted
        iHistoAdc[i]->Draw();
        iHistoAdc[i]->SetFillColor(kRed);
    }

    for (i=0; i<10; i++)
    {
        double_histo_min_mean[i] = (double) i_histo_length ; //65536.0 ;
        double_histo_max_mean[i] = 0.0 ;
        double_histo_min_rms[i] = 1000.0 ;
        double_histo_max_rms[i] = 0.0 ;
        histo_data_first_X[i] = i_histo_length - 1 ;
        histo_data_last_X[i] = 0 ;
    }




//TPaveText *pt;
    //pt = histo_pave_text[]
    if (!histo_pave_text[0])
    {
        for (i=0; i<10; i++)
        {
            histo_pave_text[i] = new TPaveText(0.1, 0.75, 0.40, 0.92,"brNDC"); //x1,y1,x2,y2
            histo_pave_text[i]->SetFillColor(DefineCanvasBackgroundColor);
            histo_pave_text[i]->SetTextAlign(12);
            histo_pave_text[i]->SetBorderSize(1);
            //gStyle->SetOptStat(1111);
        }
    }


    if (fB_openfCanvas2WindowFlag == kTRUE)
    {
        fCanvas2->Modified();
        fCanvas2->Update();
    }

#endif




// FFT Graph Setup


    if (!fGraph_fft[0])
    {
        //for (i=0;i<11;i++) {
        //  fGraph_fft[i]   = new TGraph(MAX_ROOT_PLOT_LENGTH/2, gl_float_x,gl_float_y);
        //}
        fGraph_fft[0]   = new TGraph(MAX_ROOT_PLOT_LENGTH/2, gl_float_x,gl_float_y);
        fGraph_fft[1]   = new TGraph(MAX_ROOT_PLOT_LENGTH/2, gl_float_x,gl_float_y1);
    }
    fGraph_fft[0]->SetLineColor(DefineChannel_1_Color);
    fGraph_fft[1]->SetLineColor(DefineCanvasBackgroundColor);


    for (ui=0; ui<plot_length/2; ui++)
    {
        gl_float_y[ui] = 0.0 ;
    }
    for (ui=0; ui<plot_length/2; ui++)
    {
        gl_float_y1[ui] = 10.0 ;
    }
    gl_float_y1[40] = -140.0 ;

    fftw_plan p;
    fftw_complex *fftw_complex_in, *fftw_complex_out ;

    fftw_complex_in  =  (fftw_complex*) fftw_malloc ( sizeof (fftw_complex) * plot_length );
    fftw_complex_out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex)*plot_length);

    int *int_save_adc_buffer;
    int_save_adc_buffer = (int*) malloc(sizeof(int)*plot_length);

    double *double_fft_spectrum ;
    double_fft_spectrum = (double*) malloc(sizeof(double)*plot_length / 2);

    double *double_window_weight ;
    double_window_weight = (double*) malloc(sizeof(double)*plot_length);

    double_fft_frequency = 125000000.0 ;

    /**************************************************************************************************************/






    /**************************************************************************************************************/
// SIS8300 Test Setup (configuration)



// AD9510 Clock Distributuon IC

//ch_divider_configuration_array[i] :
//	bits <3:0>:   Divider High
//	bits <7:4>:   Divider Low
//	bits <11:8>:  Phase Offset
//	bit  <12>:    Select Start High
//	bit  <13>:    Force
//	bit  <14>:    Nosyn (individual)
//	bit  <15>:    Bypass Divider

//#define CLK_25MHZ //with Phase
    unsigned int uint_clock_mux_control_val ;
    clock_source_choice = fCombo_Clock_source->GetSelected();
    uint_clock_mux_control_val = 0x0 ;
    ad9510_divider_data = 0x0 ;
    switch (clock_source_choice)
    {
    case 0: // 250 MHz
        uint_clock_mux_control_val = 0x0000F ;
        ad9510_divider_data = 0x8000 ; // bypass
        double_fft_frequency = 250000000.0 ;
        break;
    case 1: // 125 MHz
        uint_clock_mux_control_val = 0x0000F ;
        ad9510_divider_data = 0x0 ;
        double_fft_frequency = 125000000.0 ;
        break;
    case 2: // 62.5 MHz
        uint_clock_mux_control_val = 0x0000F ;
        ad9510_divider_data = 0x11 ;
        double_fft_frequency = 62500000.0 ;
        break;
    case 3: // 41.66 MHz
        uint_clock_mux_control_val = 0x0000F ;
        ad9510_divider_data = 0x22 ;
        double_fft_frequency = 41660000.0 ;
        break;
    case 4: // 31.25 MHz
        uint_clock_mux_control_val = 0x0000F ;
        ad9510_divider_data = 0x33 ;
        double_fft_frequency = 31250000.0 ;
        break;
    case 5: // 25 MHz
        uint_clock_mux_control_val = 0x0000F ;
        ad9510_divider_data = 0x44 ;
        double_fft_frequency = 25000000.0 ;
        break;
    case 6: // 83.33 MHz
        uint_clock_mux_control_val = 0x0000F ;
        ad9510_divider_data = 0x10 ; // internal Sample Clock 83.33 MHz ; only test if possible
        double_fft_frequency = 83330000.0 ;
        break;
    case 7: // SMA
        uint_clock_mux_control_val = 0x00F0F ; // MUXD = 3, MUXE = 3
        ad9510_divider_data = 0x8000 ; // bypass
        double_fft_frequency = 125000000.0 ;
        break;
    case 8: // SMA via Multiplier CLKIN1 bypass
        uint_clock_mux_control_val = 0x0051F ; // MUXC = 1, MUXD = 1, MUXE = 1
        ad9510_divider_data = 0x8000 ; // bypass
        double_fft_frequency = 125000000.0 ;
        break;
    case 9: // int ref Osc via Multiplier XA/XB bypass
        uint_clock_mux_control_val = 0x0051F ; // MUXC = 1, MUXD = 1, MUXE = 1
        ad9510_divider_data = 0x8000 ; // bypass
        double_fft_frequency = 114285000.0 ;
        break;
    case 10: // SMA via Multiplier 10MHz to 125MHz
        uint_clock_mux_control_val = 0x0051F ; // MUXC = 1, MUXD = 1, MUXE = 1
        ad9510_divider_data = 0x8000 ; // bypass
        double_fft_frequency = 125000000.0 ;
        break;
    case 11: // Front HARLINK
        uint_clock_mux_control_val = 0x00A0F ;  // MUXD = 2, MUXE = 2
        ad9510_divider_data = 0x8000 ; // bypass
        double_fft_frequency = 125000000.0 ;
        break;
    case 12: // RTM TCLKA (CLK1)
        uint_clock_mux_control_val = 0x0000A ; // MUXA = 2, MUXE = 2
        ad9510_divider_data = 0x8000 ; // bypass
        double_fft_frequency = 125000000.0 ;
        break;
    case 13: // RTM TCLKB (CLK2)
        uint_clock_mux_control_val = 0x00005 ; // MUXA = 1, MUXE = 1
        ad9510_divider_data = 0x8000 ; // bypass
        double_fft_frequency = 125000000.0 ;
        break;
    case 14: // RTM CLK2
        uint_clock_mux_control_val = 0x00000 ; // MUXA = 0, MUXE = 0
        ad9510_divider_data = 0x8000 ; // bypass
        double_fft_frequency = 125000000.0 ;
        break;
    }

// Clock Distribution Multiplexer
    SIS8300_Write_Register(sis8300_device, SIS8300_CLOCK_DISTRIBUTION_MUX_REG, uint_clock_mux_control_val);

// ADC
    for (i_adc=0; i_adc<5; i_adc++)
    {
        SIS8300_ADC_SPI_Setup(sis8300_device, i_adc);
    }

    if (clock_source_choice == 8)   // SMA via Multiplier CLKIN1 bypass
    {
        printf("SMA via Multiplier CLKIN1 bypass\n");
        si5326_bypass_external_clock_multiplier(sis8300_device);
    }

    if (clock_source_choice == 9)   // int ref Osc via Multiplier XA/XB bypass
    {
        printf("int ref Osc via Multiplier XA/XB bypass\n");
        si5326_bypass_internal_ref_osc_multiplier(sis8300_device);
    }

    if (clock_source_choice == 10)   // SMA via Multiplier 10MHz to 125MHz
    {
        si5326_set_external_clock_multiplier(sis8300_device, 0, 4, 10, 10, 500, 1, 10 ) ; // bw=0    n1_hs=4   n1_clk1 = 10  n1_clk2 = 10  n2=500  n3=1   range 10,00 to 11,34 -> 125 to 141
    }








    ad9510_divider_configuration_array[0] = 0x0000 + ad9510_divider_data ;  // (ADC1-CLK, ch1/2)
    //ad9510_divider_configuration_array[1] = 0x0200 + ad9510_divider_data ;  // (ADC2-CLK, ch3/4) phase: 2 clks
    ad9510_divider_configuration_array[1] = 0x0000 + ad9510_divider_data ;  // (ADC2-CLK, ch3/4)
    ad9510_divider_configuration_array[2] = 0x0000 + ad9510_divider_data ;  // (ADC3-CLK, ch4/5)
    ad9510_divider_configuration_array[3] = 0x0000 + ad9510_divider_data ;  // (ADC4-CLK, ch6/7)
    ad9510_divider_configuration_array[4] = 0x0000 + ad9510_divider_data ;  // (ADC5-CLK, ch8/9)
    ad9510_divider_configuration_array[5] = 0x0000 + ad9510_divider_data ;  // (Frontpanel Clk, Harlink)
    ad9510_divider_configuration_array[6] = 0x0000 + ad9510_divider_data ;  // (FPGA DIV-CLK05) used for synch. of external Triggers

    ad9510_divider_configuration_array[7] = 0xC000 + 0x00 ;  // (FPGA DIV-CLK69) used for sychn. of AD910 ISc and Bypass
    SIS8300_AD9510_SPI_Setup(sis8300_device, ad9510_divider_configuration_array, 1 /*ad9510_synch_cmd*/ );

    /***************/

    // disable ddr2 test write interface
    SIS8300_Write_Register(sis8300_device, DDR2_ACCESS_CONTROL, 0);// (1<<DDR2_PCIE_TEST_ENABLE);
    SIS8300_Write_Register(sis8300_device, SIS8300_PRETRIGGER_DELAY_REG, pre_trigger_delay);
    usleep(1) ;

    SIS8300_Write_Register(sis8300_device, SIS8300_ADC_INPUT_TAP_DELAY_REG, 0x1f00 + (gl_class_adc_tap_delay & 0x3f));//
    SIS8300_Write_Register(sis8300_device, SIS8300_SAMPLE_LENGTH_REG, sample_block_length-1);
    usleep(1) ;

    //myReg.data = 0x000; // enable ch1-10
    //myReg.offset = SIS8300_SAMPLE_CONTROL_REG;

#ifdef raus
    int  test_sample_mode = -1 ;
// if external trigger
    if (test_sample_mode == 2)
    {
        myReg.data |= 0x800; // enable external trigger
    }
    // if interal trigger
    if (test_sample_mode == 1)
    {
        myReg.data |= 0x400; // enable internal trigger
    }
    ioctl(sis8300_device, SIS8300_REG_WRITE, &myReg);
    usleep(1) ;

    // external trigger setup
    if (test_sample_mode == 2)
    {
        myReg.offset = SIS8300_HARLINK_IN_OUT_CONTROL_REG ;
        myReg.data = 0x100  ; // Enable Harlink 1 Input
        ioctl(sis8300_device, SIS8300_REG_WRITE, &myReg);
    }

    // internal trigger setup
    if (test_sample_mode == 1)
    {
        myReg.offset = SIS8300_TRIGGER_SETUP_CH1_REG ;
        myReg.data = 0x0  ; //
        myReg.data = myReg.data + 0x7000000  ; // Enable / GT / FIR
        myReg.data = myReg.data + 0x100000  ; //Pulse Length = 0x10
        myReg.data = myReg.data + 0x0f0a  ; // G = 15, P = 10
        ioctl(sis8300_device, SIS8300_REG_WRITE, &myReg);

        myReg.offset = SIS8300_TRIGGER_THRESHOLD_CH1_REG ;
        //myReg.data = 0x1000110  ; // Threshold Off / On
        myReg.data = 0x1f00  ; // FIR Threshold  On
        ioctl(sis8300_device, SIS8300_REG_WRITE, &myReg);
    }
#endif
    // enable ch1-10
    if (gl_class_sis8300_250MHZ_FLAG == 0)
    {
        data = 0x0 ;
    }
    else
    {
        data = 0x300 ; // disable channe 9/10
    }
    SIS8300_Write_Register(sis8300_device, SIS8300_SAMPLE_CONTROL_REG, data);


#ifdef DWC8VM1

    //Initial Setup

    // attenuator change
    for(i=0; i < 9; i++)
    {
        if(comboOldStatus[i] != fCombo_CHX_Attenuator[i]->GetSelected())
        {
            comboOldStatus[i] = fCombo_CHX_Attenuator[i]->GetSelected();

            // Set Attenuator
            stat = setAttenuator(i, attenuationValue[comboOldStatus[i]].attenuation);
            if(stat != Stat830xSuccess){
                sis830x_status2str(stat, errorstr);
                printf("'setAttenuator' #[%d] failed (%d): %s\n", i, stat, errorstr);
            }
            #ifdef DEBUGPRINT
            else{
                printf("Init Attenuator %d to %s\n", i, attenuationValue[comboOldStatus[i]].discrip);
            }
            #endif // DEBUGPRINT
        }
    }

    // vector modulator
    if(comboOldStatus[9] != fCombo_AMC_Modulator_Type->GetSelected())
    {
        comboOldStatus[9] = fCombo_AMC_Modulator_Type->GetSelected();
        // Set register
        #ifdef DEBUGPRINT
        printf("Init Combo VM  -> %d\n", comboOldStatus[9]);
        #endif // DEBUGPRINT
        switch(comboOldStatus[9]){
            case 0:
                sis830x_WriteRegister(dev_pointer, 0x45, 0x0);
            break;
            case 1:
                sis830x_WriteRegister(dev_pointer, 0x45, 0x130);
            break;
            case 2:
                sis830x_WriteRegister(dev_pointer, 0x45, 0x131);
            break;
        }
    }

    // interlock
    if(AMCInterlockOldStatus != fChkAMCInterlock->IsOn())
    {
        AMCInterlockOldStatus = fChkAMCInterlock->IsOn();

        // Ebanle RTM LVDS
        sis830x_WriteRegister(dev_pointer, 0x12F, 0x700);

        // Set Interlock
        if(AMCInterlockOldStatus)

            sis830x_WriteRegister(dev_pointer, 0x12, 0x60600000);
        else
            sis830x_WriteRegister(dev_pointer, 0x12, 0x60000000);
        #ifdef DEBUGPRINT
        printf("Init InterLock -> %d\n", AMCInterlockOldStatus);
        #endif // DEBUGPRINT
    }



    if(VMOldNum[0] != fNumericEntriesVectorVoltages[0]->GetNumber())
    {
        VMOldNum[0] = fNumericEntriesVectorVoltages[0]->GetNumber();
        // if(comboOldStatus[9] = 1) ...
        #ifdef DEBUGPRINT
        printf("Init VMOldNum[0] -> %f [0x%X]\n", VMOldNum[0], double2hex16bit(VMOldNum[0]));
        #endif // DEBUGPRINT
    }

    if(VMOldNum[1] != fNumericEntriesVectorVoltages[1]->GetNumber())
    {
        VMOldNum[1] = fNumericEntriesVectorVoltages[1]->GetNumber();
        // if(comboOldStatus[9] = 1) ...
        #ifdef DEBUGPRINT
        printf("Init VMOldNum[1] -> %f [0x%X]\n", VMOldNum[1] , double2hex16bit(VMOldNum[1]));
        #endif // DEBUGPRINT

    }
    sis830x_WriteRegister(dev_pointer, 0x46, double2hex16bit(VMOldNum[1])<<16 | double2hex16bit(VMOldNum[0]));


    if(CMOldNum[0] != fNumericEntriesCommonVoltages[0]->GetNumber())
    {
        CMOldNum[0] = fNumericEntriesCommonVoltages[0]->GetNumber();
        stat= setCommonModeDAC(0, CMOldNum[0]);
        if(stat != Stat830xSuccess){
            sis830x_status2str(stat, errorstr);
            printf("'setCommonModeDAC' 1 failed (%d): %s\n", stat, errorstr);
        }
        #ifdef DEBUGPRINT
        else{
            printf("Init CMOldNum[0] -> %f\n", CMOldNum[0]);
        }
        #endif // DEBUGPRINT

    }

    if(CMOldNum[1] != fNumericEntriesCommonVoltages[1]->GetNumber())
    {
        CMOldNum[1] = fNumericEntriesCommonVoltages[1]->GetNumber();
        stat = setCommonModeDAC(1, CMOldNum[1]);
        if(stat != Stat830xSuccess){
            sis830x_status2str(stat, errorstr);
            printf("'setCommonModeDAC' 2 failed (%d): %s\n", stat, errorstr);
        }
        #ifdef DEBUGPRINT
        else{
             printf("Init CMOldNum[1] -> %f\n", CMOldNum[1]);
        }
        #endif // DEBUGPRINT
    }

#endif // DWC8VM1


    // reset sample logic
    SIS8300_Write_Register(sis8300_device, SIS8300_ACQUISITION_CONTROL_STATUS_REG, 0x00004); //



    /**************************************************************************************************************/


    while (fSis8300_Test1)
    {
        // setup Sample Start addresses
        SIS8300_Write_Register(sis8300_device, SIS8300_SAMPLE_START_ADDRESS_CH1_REG, 0x0); // 1. 1M-Block 16 Msamples
        SIS8300_Write_Register(sis8300_device, SIS8300_SAMPLE_START_ADDRESS_CH2_REG, 0x100000); // 2. 1M-Block 16 Msamples
        SIS8300_Write_Register(sis8300_device, SIS8300_SAMPLE_START_ADDRESS_CH3_REG, 0x200000); // 3. 1M-Block 16 Msamples
        SIS8300_Write_Register(sis8300_device, SIS8300_SAMPLE_START_ADDRESS_CH4_REG, 0x300000); // 4. 1M-Block 16 Msamples
        SIS8300_Write_Register(sis8300_device, SIS8300_SAMPLE_START_ADDRESS_CH5_REG, 0x400000); // 5. 1M-Block 16 Msamples

        SIS8300_Write_Register(sis8300_device, SIS8300_SAMPLE_START_ADDRESS_CH6_REG, 0x500000); // 6. 1M-Block 16 Msamples
        SIS8300_Write_Register(sis8300_device, SIS8300_SAMPLE_START_ADDRESS_CH7_REG, 0x600000); // 7. 1M-Block 16 Msamples
        SIS8300_Write_Register(sis8300_device, SIS8300_SAMPLE_START_ADDRESS_CH8_REG, 0x700000); // 8. 1M-Block 16 Msamples
        SIS8300_Write_Register(sis8300_device, SIS8300_SAMPLE_START_ADDRESS_CH9_REG, 0x800000); // 9. 1M-Block 16 Msamples
        SIS8300_Write_Register(sis8300_device, SIS8300_SAMPLE_START_ADDRESS_CH10_REG, 0x900000); // 10. 1M-Block 16 Msamples


        // start sampling
#ifdef raus
        myReg.offset = SIS8300_ACQUISITION_CONTROL_STATUS_REG;
        // TBD: mode 1 or 2
        if(test_sample_mode == 0)
        {
            myReg.data = 0x00001  ; // start immediately
        }
        else
        {
            myReg.data = 0x00002  ; // armed, wait for trigger
            printf("armed:   %d  \n",test_sample_mode);
            // usleep(1000000);
        }
        ioctl(fp, SIS8300_REG_WRITE, &myReg);
#endif
        SIS8300_Write_Register(sis8300_device, SIS8300_ACQUISITION_CONTROL_STATUS_REG, 0x00001)  ; // start immediately



        // wait if BUSY or Arm
        do
        {
            gSystem->ProcessEvents();  // handle GUI events
            SIS8300_Read_Register(sis8300_device, SIS8300_ACQUISITION_CONTROL_STATUS_REG, &data) ;
            //  printf("ctrl_reg = 0x%08X\n", data);
            usleep(1);
        }
        while (((data & 0x3) != 0 ) && fSis8300_Test1);
        if (!fSis8300_Test1)
        {
            break ;
        }

// read all ADC channels
        i_ch = 0;
        k = 10;
        if (gl_class_sis8300_250MHZ_FLAG == 0)
        {
            max_nof_channels = 10 ;
        }
        else
        {
            max_nof_channels = 8 ;
        }

        for (i_ch=0; i_ch<max_nof_channels; i_ch++)
        {
            //for(i_ch=0; i_ch < k;i_ch++){
            SIS8300_Write_Register(sis8300_device, DMA_READ_SRC_ADR_LO32, (i_ch) * (0x100000 * 16 * 2))  ; // // Blocklength * 16 Samples/Block * 2Byte/Sample
            read(sis8300_device, (uint32_t *) ushort_adc_buffer_array_ptr[i_ch], sample_length*2);
        }

        /****/
// avergage channel 1
        uint_soft_gain = 4;	//fNumericEntries[4]->GetIntNumber();
        uint_soft_offset = 3 * 32880;	//fNumericEntries[5]->GetIntNumber();
        uint_soft_average = fNumericEntries[6]->GetIntNumber();

        for (ui=0; ui<plot_length-uint_soft_average; ui++)
        {
            gl_uint_adc1_average[ui] = 0 ;
            ushort_adc_buffer_ptr =  ushort_adc_buffer_array_ptr[0];// + header_offset_ushort_ptr;
            for (i=0; i<uint_soft_average; i++)
            {
                gl_uint_adc1_average[ui] = gl_uint_adc1_average[ui] + ((unsigned int) ushort_adc_buffer_ptr[ui+i]);// - uint_soft_offset );
            }
            gl_uint_adc1_average[ui] = (uint_soft_gain * gl_uint_adc1_average[ui]  ) /(uint_soft_average) -((uint_soft_gain -1) * (unsigned int) ushort_adc_buffer_ptr[0])  ;

        }

        for (ui=plot_length - uint_soft_average; ui<plot_length; ui++)
        {
            gl_uint_adc1_average[ui] = gl_uint_adc1_average[plot_length-uint_soft_average-1]  ;
        }


        /****/


        gSystem->ProcessEvents();  // handle GUI events

        /*  Display Graph */

        // X-axis
        for (ui=0; ui<plot_length; ui++)
        {
            gl_x[ui] = ui ;
            //	ushort_adc_buffer_ptr[ui] = ushort_adc_buffer_ptr[ui] + 0x8000 ;

        }

// check min and max
        // check min and max for y-cordiante
        ymin = 0 ;
        ymax = 0xffff ;

        #if 1
        if ((fChkDisplayAutoZoom->IsOn() == kTRUE) || (fReadMinMax == kTRUE))
        {
            ymax = 0 ;
            ymin = 0xffff ;

            for(i = 0; i < max_nof_channels; i++)
            {
                ushort_adc_buffer_ptr =  ushort_adc_buffer_array_ptr[i];

                gl_ymax = 0;
                gl_ymin = 0xFFFF;

                if (fChkDisplayAdc[i]->IsOn() == kTRUE)
                {
                    for(ui = 0; ui < plot_length; ui++)
                    {
                        if(ushort_adc_buffer_ptr[ui] < ymin) ymin = ushort_adc_buffer_ptr[ui];
                        if(ushort_adc_buffer_ptr[ui] > ymax) ymax = ushort_adc_buffer_ptr[ui];
                        if(ushort_adc_buffer_ptr[ui] < gl_ymin) gl_ymin = ushort_adc_buffer_ptr[ui];
                        if(ushort_adc_buffer_ptr[ui] > gl_ymax) gl_ymax = ushort_adc_buffer_ptr[ui];
                    }

                    if(fReadMinMax == kTRUE){
                        printf("Ch%02d: Max: %5d - Min: %5d [Mean: %5d]\n", i+1, gl_ymax, gl_ymin, gl_ymax-gl_ymin);
                    }
                }
            } // i
        }

        if (fChkDisplayAutoZoom->IsOn() == kFALSE)
        {
            ymax = fNumericEntriesGraph_Yaxis[0]->GetIntNumber();
            ymin = fNumericEntriesGraph_Yaxis[1]->GetIntNumber();
            if (ymin > ymax)
            {
                fNumericEntriesGraph_Yaxis[0]->SetIntNumber(ymin); // Y-max
                fNumericEntriesGraph_Yaxis[1]->SetIntNumber(ymax); // Y-min      }
                ymax = fNumericEntriesGraph_Yaxis[0]->GetIntNumber();
                ymin = fNumericEntriesGraph_Yaxis[1]->GetIntNumber();
                ymax = ymax+1;
            }
        }
        #else
        if (fChkDisplayAutoZoom->IsOn() == kTRUE)
        {
            ymax = 0 ;
            ymin = 0xffff ;
            for(i = 0; i < max_nof_channels; i++)
            {
                ushort_adc_buffer_ptr =  ushort_adc_buffer_array_ptr[i];

                if (fChkDisplayAdc[i]->IsOn() == kTRUE)
                {
                    for(ui = 0; ui < plot_length; ui++)
                    {
                        if(ushort_adc_buffer_ptr[ui] < ymin) ymin = ushort_adc_buffer_ptr[ui];
                        if(ushort_adc_buffer_ptr[ui] > ymax) ymax = ushort_adc_buffer_ptr[ui];
                    }
                }
            } // i
        }
        else
        {
            ymax = fNumericEntriesGraph_Yaxis[0]->GetIntNumber();
            ymin = fNumericEntriesGraph_Yaxis[1]->GetIntNumber();
            if (ymin > ymax)
            {
                fNumericEntriesGraph_Yaxis[0]->SetIntNumber(ymin); // Y-max
                fNumericEntriesGraph_Yaxis[1]->SetIntNumber(ymax); // Y-min      }
                ymax = fNumericEntriesGraph_Yaxis[0]->GetIntNumber();
                ymin = fNumericEntriesGraph_Yaxis[1]->GetIntNumber();
                ymax = ymax+1;
            }
        }
        #endif // 1

        //printf("ymin = %d  ymax = %d    \n", ymin, ymax);
        ywidth = ymax - ymin ;
        y_delta = ywidth/11 ;

        fCanvas1->Clear();
        fCanvas1->cd(1);

        // Display axis
        for(i = 0; i < plot_length; i++)
        {
            gl_y[i]  = ymin+((ymax-ymin)/2)  ;
        }
        gl_y[(plot_length/2)] = ymin ;
        gl_y[((plot_length/2))+1] = ymax ;
        fGraph_ch[10]->DrawGraph(plot_length,gl_x,gl_y,"AL");

// Display channels
        //for(i = 0; i < 1; i++){
        for(i = 0; i < max_nof_channels; i++)
        {
 #if 0  // Tino fragen
            if (i == 1)
            {
                if (fChkDisplayAdc[0]->IsOn() == kTRUE)
                {
                    for (ui=0; ui<plot_length; ui++)
                    {
                        gl_y[ui] = gl_uint_adc1_average[ui]   ;
                    }
                    fGraph_ch[i]-> DrawGraph(plot_length, gl_x,gl_y,"L");
                    //fGraph_Text_ch[i_ch]->DrawLatex(plot_length+10,ymax-((16-i_ch)*y_delta),chkDisAdcLabel[i_ch]);
                }

            }
            else
            {
#endif
                ushort_adc_buffer_ptr =  ushort_adc_buffer_array_ptr[i];
                if (fChkDisplayAdc[i]->IsOn() == kTRUE)
                {

                    short tmp;

                    for (ui=0; ui<plot_length; ui++)
                    {

                        gl_y[ui] = ushort_adc_buffer_ptr[ui];

                        //	tmp = (short) ushort_adc_buffer_ptr[ui];
                        //	gl_y[ui] = (int) tmp;//(int) ushort_adc_buffer_ptr[ui];
                        //	printf("0x%08X\n", gl_y[ui]);

                        //if(ushort_adc_buffer_ptr[ui] < 0x8000){
                        //	gl_y[ui] = (ushort_adc_buffer_ptr[ui] + 0x8000);
                        //}else{
                        //	gl_y[ui] = ushort_adc_buffer_ptr[ui] & 0x7FFF;
                        //}
                        //gl_y[ui] = (int)((ushort_adc_buffer_ptr[ui] + 0x8000) & 0xFFFF);
                        //printf("%d   0x%0x4     %d\n", ui, ushort_adc_buffer_ptr[ui],  ushort_adc_buffer_ptr[ui]);

                    }
                    fGraph_ch[i]-> DrawGraph(plot_length, gl_x,gl_y,"L");
                    fGraph_Text_ch[i]->DrawLatex(plot_length+10,ymax-((i+1)*y_delta),chkDisAdcLabel[i]);
                }
   //         }
        }
        fCanvas1->Update();
        //fCanvas1->Modified();


        /***************************************************************************************************/

#ifdef FFT_GRAPH


        display_FFT_choice = fCombo_Display_FFT_Ch->GetSelected();
        if (display_FFT_choice > 0)
        {
            if (fB_openfCanvas3WindowFlag == kFALSE)
            {
                fCanvas3 = new TCanvas("fCanvas3","Fast Fourier Transform ", SIS8300_FFT_WINDOW_POSTION_X, SIS8300_FFT_WINDOW_POSTION_Y, SIS8300_FFT_WINDOW_WIDTH, SIS8300_FFT_WINDOW_HIGH);
                fB_openfCanvas3WindowFlag = kTRUE; //
                fChkFFT_Sum->SetState(kButtonUp)   ; // is Off !
                fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
                fChkFFT_Sum->SetEnabled(kFALSE)   ; //
                fChkFFTLogY->SetEnabled(kFALSE)   ; //
                fChkFFT_Db->SetEnabled(kTRUE)   ; //
                fChkFFT_Db->SetState(kButtonDown)   ; // is On !
            }

            fCanvas3->Clear();
            fCanvas3->cd(1);
            fCanvas3->SetGrid();
            fCanvas3->SetFillColor(DefineCanvasBackgroundColor);
            fCanvas3->SetFrameBorderMode(1);
            fCanvas3->SetFrameBorderSize(6);
            //fCanvas3->SetFrameFillColor(21);
            //fCanvas3->SetFrameFillColor(TColor::GetColor(240,240,240));
            fCanvas3->SetFrameFillColor(DefineCanvasBackgroundColor);
            //  gPad->SetGrid();
            //  gPad->SetFrameFillColor(19);

            //TGVerticalFrame *controls = new TGVerticalFrame(this);
            fCanvas3->Update();
        }
        else
        {
            if (fB_openfCanvas3WindowFlag == kTRUE)
            {
                delete fCanvas3 ;
                fB_openfCanvas3WindowFlag = kFALSE; //
                fChkFFT_Db->SetEnabled(kTRUE)   ; //
                if (fChkFFT_Db->IsOn() == kTRUE)
                {
                    fChkFFT_Sum->SetEnabled(kFALSE)   ; //
                    fChkFFTLogY->SetEnabled(kFALSE)   ; //
                }
                else
                {
                    fChkFFT_Sum->SetEnabled(kTRUE)   ; //
                    fChkFFTLogY->SetEnabled(kTRUE)   ; //
                }
            }
        }

        if (display_FFT_choice > 0)
        {
            ushort_adc_buffer_ptr =  ushort_adc_buffer_array_ptr[display_FFT_choice-1];
            fft_plot_length = sample_length / 2 ;


            int int_sum_value ;
            int_sum_value = 0 ;
            for(ui = 0; ui < sample_length; ui++)
            {
                int_sum_value =  int_sum_value + (unsigned int)(ushort_adc_buffer_ptr[ui] & 0xFFFF) ;
            }
            int_sum_value =  ((int_sum_value   /  sample_length) );

            int int_value ;
            for(ui = 0; ui < sample_length; ui++)
            {
                int_value =  (unsigned int)(ushort_adc_buffer_ptr[ui] & 0xFFFF) ;
                int_save_adc_buffer[ui] =   int_value - int_sum_value  ;
            }

            //#ifdef Window_FUNCTION
#define M_PI       3.14159265358979323846
            double double_a0, double_a1, double_a2, double_a3 ;
            double double_window_correctur_factor ;
            double_window_correctur_factor = 1.0 ;

            for(ui = 0; ui < sample_length; ui++)
            {
                double_window_weight[ui] =  1.015 ; // no window
                //	double_window_weight[ui] =  1.000 ; // no window TH: Versuch
            }
            display_FFT_choice = fCombo_Display_FFT_Window->GetSelected();
            switch(display_FFT_choice)
            {
            case 1: //Hamming window
                double_window_correctur_factor = 1.368 * 1.368 ;
                for(ui = 0; ui < sample_length; ui++)
                {
                    double_window_weight[ui] =  0.54 - (0.46 * (cos((2 * M_PI * ui) / (sample_length-1) )))  ; // Hamming Window
                    //printf ( "1  %3d  %12f \n", ui, double_window_weight[ui] );
                }
                break;
            case 2: //Hann window
                double_window_correctur_factor = 1.419  * 1.419 ;
                for(ui = 0; ui < sample_length; ui++)
                {
                    double_window_weight[ui] =  0.5 * (1 - cos( (2 * M_PI * ui) / (sample_length-1) ))  ; // HAnn Window
                    //printf ( "2  %3d  %12f \n", ui, double_window_weight[ui] );
                }
                break;
            case 3: //Blackmann window
                double_window_correctur_factor = 1.5365 * 1.5365 ;
                double_a0 = 7938.0/18608.0 ;
                double_a1 = 9240.0/18608.0 ;
                double_a2 = 1430.0/18608.0 ;
                for(ui = 0; ui < sample_length; ui++)
                {
                    double_window_weight[ui] =  double_a0 - (double_a1 * cos( (2 * M_PI * ui) / (sample_length-1) ))  + (double_a2 * cos( (4 * M_PI * ui) / (sample_length-1) )); // Blackmann Window
                    //printf ( "2  %3d  %12f \n", ui, double_window_weight[ui] );
                }
                break;
            case 4: //Blackmann-Harris window
                double_window_correctur_factor = 1.6697 * 1.6697 ;
                double_a0 = 0.35875 ;
                double_a1 = 0.48829 ;
                double_a2 = 0.14128 ;
                double_a3 = 0.01168 ;
                for(ui = 0; ui < sample_length; ui++)
                {
                    double_window_weight[ui] =  double_a0 - (double_a1 * cos( (2 * M_PI * ui) / (sample_length-1) ))  + (double_a2 * cos( (4 * M_PI * ui) / (sample_length-1) )) - (double_a3 * cos( (6 * M_PI * ui) / (sample_length-1) )) ; // Blackmann-Harris Window
                    //printf ( "2  %3d  %12f \n", ui, double_window_weight[ui] );
                }
                break;
            }
            //#endif


            // FFT
            for(ui = 0; ui < sample_length; ui++)
            {
                fftw_complex_in[ui][0] =  (double) int_save_adc_buffer[ui]  * double_window_weight[ui] ; // real.
                fftw_complex_in[ui][1] =  (double) 0.0 ; // imag..
            }

            p = fftw_plan_dft_1d (sample_length, fftw_complex_in, fftw_complex_out, FFTW_FORWARD, FFTW_ESTIMATE);
            fftw_execute(p);

            for(ui = 1; ui < fft_plot_length; ui++)
            {
                double_fft_spectrum[ui] = double_window_correctur_factor * sqrtf ( (fftw_complex_out[ui][0] * fftw_complex_out[ui][0]) + (fftw_complex_out[ui][1]  * fftw_complex_out[ui][1] ))  / (fft_plot_length/2) ;
                //printf ( "  %3d  %12f  %12f  %12f\n", ui, fftw_complex_out[ui][0], fftw_complex_out[ui][1], spectrum[ui] );
            }
            fftw_destroy_plan(p) ;
            fCanvas3->Clear();
            fCanvas3->cd(1);

            /********************/

            double log10_value ;
            double noise_floor;
            log10_value = log10 ((float)(sample_length/2));

            if (gl_class_sis8300_250MHZ_FLAG == 0)
            {
                noise_floor = ((6.02*16) + 1.76 + (10 * log10_value)) ;
            }
            else
            {
                noise_floor = ((6.02*14) + 1.76 + (10 * log10_value)) ; // 14-bit ADC
            }

            // printf("noise_floor = %f\n", noise_floor);
            /********************/

            for (ui=0; ui<fft_plot_length; ui++)
            {
                gl_float_x[ui] = (ui * double_fft_frequency) / fft_plot_length / 2  ;
                if (fChkFFT_Db->IsOn() == kTRUE)
                {
                    if (gl_class_sis8300_250MHZ_FLAG == 0)
                    {
                        gl_float_y[ui] = 20.0 * ( log10 (double_fft_spectrum[ui] / 65535.0) )  ; // Amplitude Spectrum 16 bit - CT: (2^16)/2  -->  2^16 - 1
                        //	gl_float_y[ui] = 10.0 * ( log10 ((double_fft_spectrum[ui] * double_fft_spectrum[ui]  ) / (32768.0 * 32768.0)) ) ; // Power Spectrum
                    }
                    else
                    {
                        gl_float_y[ui] = 20.0 * ( log10 (double_fft_spectrum[ui] / 16383.0) ) ; // Amplitude Spectrum 14 bit CT: (2^14)/2  -->  2^14 -1
                    }
                }
                else
                {
                    if (fChkFFT_Sum->IsOn() == kTRUE)
                    {
                        gl_float_y[ui] = gl_float_y[ui] + (Float_t)double_fft_spectrum[ui] ;
                    }
                    else
                    {
                        gl_float_y[ui] = (Float_t)double_fft_spectrum[ui]  ;
                    }
                }
            }
            if (fChkFFT_AutoScale->IsOn() == kFALSE)
            {
                fGraph_fft[1]-> DrawGraph(fft_plot_length-1, &gl_float_x[1], &gl_float_y1[1],"AL");
                fGraph_fft[0]-> DrawGraph(fft_plot_length-1, &gl_float_x[1], &gl_float_y[1],"L");
            }
            else
            {
                fGraph_fft[0]-> DrawGraph(fft_plot_length-1, &gl_float_x[1], &gl_float_y[1],"AL");
            }
            fCanvas3->Update();
        }

#endif
        gSystem->ProcessEvents();  // handle GUI events








#ifdef HISTOGRAM
        new_display_histogram_choice = fCombo_Display_Histos->GetSelected();

        if (new_display_histogram_choice > 0)
        {
            if (fB_openfCanvas2WindowFlag == kFALSE)
            {
                fCanvas2 = new TCanvas("fCanvas2","Histo ", SIS8300_HISTOGRAM_WINDOW_POSTION_X, SIS8300_HISTOGRAM_WINDOW_POSTION_Y, SIS8300_HISTOGRAM_WINDOW_WIDTH, SIS8300_HISTOGRAM_WINDOW_HIGH);
                fB_openfCanvas2WindowFlag = kTRUE; // Setup
            }

            if (display_histogram_choice != 11)
            {
                fCanvas2->Clear();
                fCanvas2->Divide(1);
            }
            else
            {
                fCanvas2->Clear();
                fCanvas2->Divide(2,5);
            }

            fCanvas2->SetGrid();
            fCanvas2->SetFillColor(DefineCanvasBackgroundColor);
            for (i=0; i<10; i++)
            {
                fCanvas2->cd(1+i);
                gPad->SetGrid();
                gPad->SetFillColor(DefineCanvasBackgroundColor);
            }


            // Histogramming
            for(i = 0; i < i_histo_ch; i++)
            {
                if(fChkHistoSum->IsOn() == kFALSE)
                {
                    iHistoAdc[i]->Reset(); //
                    iHistoAdc[i]->BufferEmpty(1); // action =  1 histogram is filled and buffer is deleted
                }
                ushort_adc_buffer_ptr =  ushort_adc_buffer_array_ptr[i];
                for (ui=0; ui<plot_length; ui++)
                {
                    iHistoAdc[i]->Fill((int)ushort_adc_buffer_ptr[ui]);
                }

                double_histo_mean = iHistoAdc[i]->GetMean(1); //
                if (double_histo_mean < double_histo_min_mean[i])
                {
                    double_histo_min_mean[i] = double_histo_mean ;
                }
                if (double_histo_mean > double_histo_max_mean[i])
                {
                    double_histo_max_mean[i] = double_histo_mean ;
                }

                double_histo_rms = iHistoAdc[i]->GetRMS(1); //
                if (double_histo_rms < double_histo_min_rms[i])
                {
                    double_histo_min_rms[i] = double_histo_rms ;
                }
                if (double_histo_rms > double_histo_max_rms[i])
                {
                    double_histo_max_rms[i] = double_histo_rms ;
                }

                //double_histo_mean_error = iHistoAdc[i]->GetMeanError(1); //
                //double_histo_rms_error  = iHistoAdc[i]->GetRMSError(1); //
                //printf("GetMinimumBin %d\n",iHistoAdc[i]->GetMinimumBin());
                //printf("GetMaximumBin %d\n",iHistoAdc[i]->GetMaximumBin());
                //printf("s1\n");

                if(fChkHistoDisplayMinMax->IsOn() == kTRUE)
                {
                    histo_data_find_first_X_flag = 0 ;
                    histo_data_find_first_X_value = 0 ;
                    histo_data_find_last_X_value = 0 ;
                    ushort_adc_buffer_ptr =  ushort_adc_buffer_array_ptr[i];
                    for(ui = 0; ui < i_histo_length; ui++) // length of histogram
                    {

                        histo_bin_value = iHistoAdc[i]->GetBinContent(ui) ;

                        if (histo_data_find_first_X_flag == 0)
                        {
                            if(histo_bin_value > 0)
                            {
                                histo_data_find_first_X_flag = 1 ;
                                histo_data_find_first_X_value = ui ;
                                histo_data_find_last_X_value = ui ;
                            }
                        }
                        else
                        {
                            if(histo_bin_value > 0)
                            {
                                histo_data_find_last_X_value =ui ;
                            }
                        }

                    }
                    if (histo_data_find_first_X_flag != 0)
                    {
                        if(histo_data_find_first_X_value < histo_data_first_X[i]) histo_data_first_X[i] = histo_data_find_first_X_value;
                        if(histo_data_find_last_X_value > histo_data_last_X[i])   histo_data_last_X[i] = histo_data_find_last_X_value;
                    }

                    //printf("i %d   histo_data_first_X %d    histo_data_last_X %d\n",i , histo_data_first_X[i], histo_data_last_X[i]);
                    //printf("i %d   histo_data_first_X %d    histo_data_last_X %d\n",i , histo_data_first_X[i], histo_data_last_X[i]);


                    if (display_histogram_choice == 11)   // all Histograms
                    {
                        histo_pave_text[i]->SetTextSize(0.066);
                        histo_pave_text[i]->SetX1NDC(0.10);
                        histo_pave_text[i]->SetY1NDC(0.50);
                        histo_pave_text[i]->SetX2NDC(0.40);
                        histo_pave_text[i]->SetY2NDC(0.93);
                    }
                    else
                    {
                        histo_pave_text[i]->SetTextSize(0.025);
                        histo_pave_text[i]->SetX1NDC(0.10);
                        histo_pave_text[i]->SetY1NDC(0.76);
                        histo_pave_text[i]->SetX2NDC(0.39);
                        histo_pave_text[i]->SetY2NDC(0.92);
                    }

                    histo_pave_text[i]->Clear();
                    //sprintf(char_temp,"current mean %5.1f",double_histo_mean);
                    //histo_pave_text[i]->AddText(char_temp);
                    //sprintf(char_temp,"Mean Error %3.5f",double_histo_mean_error);
                    //histo_pave_text[i]->AddText(char_temp);
                    sprintf(char_temp,"min Mean %5.1f",double_histo_min_mean[i]);
                    histo_pave_text[i]->AddText(char_temp);
                    sprintf(char_temp,"max Mean %5.1f",double_histo_max_mean[i]);
                    histo_pave_text[i]->AddText(char_temp);
                    //sprintf(char_temp,"RMS Error %3.5f",double_histo_rms_error);
                    //histo_pave_text[i]->AddText(char_temp);
                    sprintf(char_temp,"min RMS %5.3f",double_histo_min_rms[i]);
                    histo_pave_text[i]->AddText(char_temp);
                    sprintf(char_temp,"max RMS %5.3f",double_histo_max_rms[i]);
                    histo_pave_text[i]->AddText(char_temp);

                    sprintf(char_temp,"min EntryX %d",histo_data_first_X[i]);
                    histo_pave_text[i]->AddText(char_temp);
                    sprintf(char_temp,"max EntryX %d",histo_data_last_X[i]);
                    histo_pave_text[i]->AddText(char_temp);
                    sprintf(char_temp,"max diff EntryX %d",histo_data_last_X[i]-histo_data_first_X[i]);
                    histo_pave_text[i]->AddText(char_temp);
                    //sprintf(char_temp,"min RMS %d",raw_data_Ymin[i]);
                    //histo_pave_text[i]->AddText(char_temp);
                }




                if(fChkHistoZoomMean->IsOn() == kTRUE)
                {
                    double_histo_mean = iHistoAdc[i]->GetMean(1); //
                    //printf("double_histo_mean = %f\n", double_histo_mean);
                    double_histo_min_x = 0.0;
                    double_histo_max_x = (double) (i_histo_length-1) ; //  65535.0;
                    if (double_histo_mean > 20.0)
                    {
                        double_histo_min_x = double_histo_mean - 20.0;
                    }
                    //if (double_histo_mean < 65515.0) { double_histo_max_x = double_histo_mean + 20.0;}
                    if (double_histo_mean < (double) (i_histo_length-20))
                    {
                        double_histo_max_x = double_histo_mean + 20.0;
                    }
                    iHistoAdc[i]->SetAxisRange(double_histo_min_x,double_histo_max_x,"X"); //
                }
                else
                {
                    double_histo_min_x = 0.0;
                    //double_histo_max_x = 65535.0;
                    double_histo_max_x = (double) (i_histo_length-1);
                    iHistoAdc[i]->SetAxisRange(double_histo_min_x,double_histo_max_x,"X"); //
                }

            }

            // display histograms
            display_histo_counter++;
            //if (display_histo_counter > 20) {
            if (display_histo_counter >= 0)
            {
                //printf("\ndisplay_histo_counter  %d \n",display_histo_counter);
                display_histo_counter=0;

                new_display_histogram_choice = fCombo_Display_Histos->GetSelected();
                if (new_display_histogram_choice != display_histogram_choice)
                {
                    display_histogram_choice = new_display_histogram_choice ;
                    changed_display_histogram_choice_flag = 1 ;
                }
                else
                {
                    changed_display_histogram_choice_flag = 0 ;
                }

                if (changed_display_histogram_choice_flag == 1)
                {
                    if (display_histogram_choice != 11)
                    {
                        fCanvas2->Clear();
                        fCanvas2->Divide(1);
                    }
                    else
                    {
                        fCanvas2->Clear();
                        fCanvas2->Divide(2,5);
                    }
                }


                if (fChkHistoGaussFit->IsOn() == kTRUE)
                {
                    histogram_gausfit_enable_flag = 1 ;
                    histogram_gausfit_clear_flag  = 0 ;
                }
                else
                {
                    if (histogram_gausfit_enable_flag == 1)
                    {
                        histogram_gausfit_clear_flag  = 1 ;
                    }
                    histogram_gausfit_enable_flag = 0 ;
                }


                fChkHistoLogY_new_flag = 0;
                if (fChkHistoLogY->IsOn() == kTRUE)
                {
                    fChkHistoLogY_new_flag = 1;
                }
                if (fChkHistoLogY_new_flag != fChkHistoLogY_old_flag)
                {
                    fChkHistoLogY_changed_flag = 1 ;
                    fChkHistoLogY_old_flag =  fChkHistoLogY_new_flag;
                }



                if (display_histogram_choice == 11)
                {
                    // fCanvas2->Clear();
                    //fCanvas2->Divide(2,5);
                    for (i=0; i<10; i++)
                    {
                        fCanvas2->cd(1+display_histo_ch_no);
                        if (histogram_gausfit_enable_flag == 1)
                        {
                            iHistoAdc[display_histo_ch_no]->Fit("gaus","Q");
                        }
                        if (histogram_gausfit_clear_flag == 1)
                        {
                            iHistoAdc[display_histo_ch_no]->Fit("gaus","0","Q");
                            histogram_gausfit_clear_flag = 0 ;
                        }

                        if (fChkHistoLogY_changed_flag == 1)
                        {
                            if (fChkHistoLogY->IsOn() == kTRUE)
                            {
                                gPad->SetLogy(1);
                            }
                            else
                            {
                                gPad->SetLogy(0);
                            }
                        }

                        iHistoAdc[display_histo_ch_no]->Draw();
                        if(fChkHistoDisplayMinMax->IsOn() == kTRUE)
                        {
                            histo_pave_text[display_histo_ch_no]->Draw();
                        }

                        //printf("display_histo_ch_no = %d\n",display_histo_ch_no);
                        display_histo_ch_no++ ;
                        if (display_histo_ch_no > 9)
                        {
                            display_histo_ch_no = 0 ;
                        }
                    }
                    if (fChkHistoLogY_changed_flag == 1)
                    {
                        //fChkHistoLogY_changed_flag = 0 ;
                    }
                }
                else
                {
                    display_histo_ch_no = display_histogram_choice-1;
                    //fCanvas2->Clear();
                    //fCanvas2->Divide(1);
                    fCanvas2->cd(1);
                    if (histogram_gausfit_enable_flag == 1)
                    {
                        iHistoAdc[display_histo_ch_no]->Fit("gaus","Q");
                    }
                    if (histogram_gausfit_clear_flag == 1)
                    {
                        iHistoAdc[display_histo_ch_no]->Fit("gaus","0","Q");
                        histogram_gausfit_clear_flag = 0 ;
                    }

                    if (fChkHistoLogY_changed_flag == 1)
                    {
                        //fChkHistoLogY_changed_flag = 0 ;
                        if (fChkHistoLogY->IsOn() == kTRUE)
                        {
                            gPad->SetLogy(1);
                        }
                        else
                        {
                            gPad->SetLogy(0);
                        }
                    }

                    iHistoAdc[display_histo_ch_no]->Draw();
                    if(fChkHistoDisplayMinMax->IsOn() == kTRUE)
                    {
                        histo_pave_text[display_histo_ch_no]->Draw();
                    }
                }

#ifdef raus
                if (display_histogram_choice == 11)
                {
                    gStyle->SetOptStat(1111);
                    gStyle->SetStatH(0.36);
                    gStyle->SetStatW(0.3);
                }
                else
                {
                    gStyle->SetOptStat(1);
                    gStyle->SetStatH(0.15);
                    gStyle->SetStatW(0.2);
                }
#endif
                fCanvas2->Update();
                //fCanvas2->Modified();
            }
        }

        else
        {
            if (fB_openfCanvas2WindowFlag == kTRUE)
            {
                delete fCanvas2 ;
                fB_openfCanvas2WindowFlag = kFALSE; //
            }
        }

#endif


#ifdef DWC8VM1

        // Loop Set

        for(i=0; i < 9; i++)
        {
            if(comboOldStatus[i] != fCombo_CHX_Attenuator[i]->GetSelected())
            {
                comboOldStatus[i] = fCombo_CHX_Attenuator[i]->GetSelected();

                // Set Attenuator
                stat = setAttenuator(i, attenuationValue[comboOldStatus[i]].attenuation);
                if(stat != Stat830xSuccess){
                    sis830x_status2str(stat, errorstr);
                    printf("'setAttenuator' #[%d] failed (%d): %s\n", i, stat, errorstr);
                }
                #ifdef DEBUGPRINT
                else{
                    printf("Set Attenuator %d to %s\n", i, attenuationValue[comboOldStatus[i]].discrip);
                }
                #endif // DEBUGPRINT
            }
        }


        // vector modulator
        if(comboOldStatus[9] != fCombo_AMC_Modulator_Type->GetSelected())
        {
            comboOldStatus[9] = fCombo_AMC_Modulator_Type->GetSelected();
            // Set register
            #ifdef DEBUGPRINT
            printf("Set Combo VM  -> %d\n", comboOldStatus[9]);
            #endif // DEBUGPRINT
            switch(comboOldStatus[9]){
                case 0:
                    sis830x_WriteRegister(dev_pointer, 0x45, 0x0);
                break;
                case 1:
                    sis830x_WriteRegister(dev_pointer, 0x45, 0x130);
                break;
                case 2:
                    sis830x_WriteRegister(dev_pointer, 0x45, 0x131);
                break;
            }
        }

        // interlock
        if(AMCInterlockOldStatus != fChkAMCInterlock->IsOn())
        {
            AMCInterlockOldStatus = fChkAMCInterlock->IsOn();

            // Ebanle RTM LVDS
            sis830x_WriteRegister(dev_pointer, 0x12F, 0x700);

            // Set Interlock
            if(AMCInterlockOldStatus)
                sis830x_WriteRegister(dev_pointer, 0x12, 0x60600000);
            else
                sis830x_WriteRegister(dev_pointer, 0x12, 0x60000000);
            #ifdef DEBUGPRINT
            printf("Set InterLock -> %d\n", AMCInterlockOldStatus);
            #endif // DEBUGPRINT
        }


        if(VMOldNum[0] != fNumericEntriesVectorVoltages[0]->GetNumber())
        {
            VMOldNum[0] = fNumericEntriesVectorVoltages[0]->GetNumber();
            // if(comboOldStatus[9] = 1) ...
            #ifdef DEBUGPRINT
            printf("Set VMOldNum[0] -> %f [%X]\n", VMOldNum[0], double2hex16bit(VMOldNum[0]));
            #endif // DEBUGPRINT
            sis830x_WriteRegister(dev_pointer, 0x46, double2hex16bit(VMOldNum[1])<<16 | double2hex16bit(VMOldNum[0]));
        }

        if(VMOldNum[1] != fNumericEntriesVectorVoltages[1]->GetNumber())
        {
            VMOldNum[1] = fNumericEntriesVectorVoltages[1]->GetNumber();
            // if(comboOldStatus[9] = 1) ...
            #ifdef DEBUGPRINT
            printf("Set VMOldNum[1] -> %f [%X]\n", VMOldNum[1] , double2hex16bit(VMOldNum[1]));
            #endif // DEBUGPRINT
             sis830x_WriteRegister(dev_pointer, 0x46, double2hex16bit(VMOldNum[1])<<16 | double2hex16bit(VMOldNum[0]));
        }



        if(CMOldNum[0] != fNumericEntriesCommonVoltages[0]->GetNumber())
        {
            CMOldNum[0] = fNumericEntriesCommonVoltages[0]->GetNumber();
            stat= setCommonModeDAC(0, CMOldNum[0]);
            if(stat != Stat830xSuccess){
                sis830x_status2str(stat, errorstr);
                printf("'setCommonModeDAC' 1 failed (%d): %s\n", stat, errorstr);
            }
            #ifdef DEBUGPRINT
            else{
                printf("Set CMOldNum[0] -> %f\n", CMOldNum[0]);
            }
            #endif // DEBUGPRINT

        }

        if(CMOldNum[1] != fNumericEntriesCommonVoltages[1]->GetNumber())
        {
            CMOldNum[1] = fNumericEntriesCommonVoltages[1]->GetNumber();
            stat = setCommonModeDAC(1, CMOldNum[1]);
            if(stat != Stat830xSuccess){
                sis830x_status2str(stat, errorstr);
                printf("'setCommonModeDAC' 2 failed (%d): %s\n", stat, errorstr);
            }
            #ifdef DEBUGPRINT
            else{
                printf("Set CMOldNum[1] -> %f\n", CMOldNum[1]);
                T
            }
            #endif // DEBUGPRIN
        }


    if(fReadSensors == kTRUE){
        fReadSensors = kFALSE;
/*
        readADCTemp(&adc_readout);
        printf("ADC_Temp:\t %f C\n", adc_readout);
        usleep(500000);
*/
        readLoVoltage(&adc_readout);
        sprintf(char_temp,"LO_Power:\t %3.3f V",adc_readout);
        fLabel_tab3_frameh3[0]->SetText(char_temp);
        #ifdef DEBUGPRINT
        printf("%s\n", char_temp);
        #endif // DEBUGPRINT
        usleep(200000);

        readLoTemp(&adc_readout);
        sprintf(char_temp,"LO_Temp:\t %3.2f C", adc_readout);
        fLabel_tab3_frameh3[1]->SetText(char_temp);
        #ifdef DEBUGPRINT
        printf("%s\n", char_temp);
        #endif // DEBUGPRINT
        usleep(200000);

        readRefVoltage(&adc_readout);
        sprintf(char_temp,"REF_Power:\t %3.3f V", adc_readout);
        fLabel_tab3_frameh3[2]->SetText(char_temp);
        #ifdef DEBUGPRINT
        printf("%s\n", char_temp);
        #endif // DEBUGPRINT
        usleep(200000);

        readRefTemp(&adc_readout);
        sprintf(char_temp,"REF_Temp:\t %3.2f C", adc_readout);
        fLabel_tab3_frameh3[3]->SetText(char_temp);
        #ifdef DEBUGPRINT
        printf("%s\n", char_temp);
        printf("\n");
        #endif // DEBUGPRINT
        usleep(200000);
    }

    if(fReadMinMax == kTRUE){
        fReadMinMax = kFALSE;
        printf("\n");
    }


#endif // DWC8VM1


        //display_histo_counter++;
        //  printf("display_histo_counter = %d\n\n",display_histo_counter);
        gSystem->ProcessEvents();  // handle GUI events

    } //while (fSis8300_Test1)

    /**************************************************************************************************************/


    fftw_free(fftw_complex_in);
    fftw_free(fftw_complex_out);
    free(double_fft_spectrum);
    //close(sis8300_device);


    printf("\nLeave SIS8300TestDialog::Sis8300_Test1 \n");
}

/**********************************************************************************************************************************/





void SIS8300TestDialog::CloseWindow()
{
    // Called when window is closed (via the window manager or not).
    // Let's stop histogram filling...
    fSis8300_Test1 = kFALSE;
    // Add protection against double-clicks
    fQuitButton->SetState(kButtonDisabled);
    // ... and close the Ged editor if it was activated.
    if (TVirtualPadEditor::GetPadEditor(kFALSE) != 0)
        TVirtualPadEditor::Terminate();
    DeleteWindow();
}



Bool_t SIS8300TestDialog::ProcessMessage(Long_t msg, Long_t parm1, Long_t)
{
    // Process messages coming from widgets associated with the dialog.

    Pixel_t green;
    fClient->GetColorByName("green", green);
    Pixel_t red;
    fClient->GetColorByName("red", red);

    //printf("\nSIS8300TestDialog::ProcessMessage:case kC_COMMAND;kCM_BUTTON \n");


    switch (GET_MSG(msg))
    {
    case kC_COMMAND:
        switch (GET_SUBMSG(msg))
        {
        case kCM_BUTTON:
            //printf("\nSIS8300TestDialog::ProcessMessage:case kC_COMMAND;kCM_BUTTON \n");
            switch(parm1)
            {
            case 1:
                //case 2:
                printf("\nTerminating dialog: %s pressed\n",
                       (parm1 == 1) ? "Quit" : "Cancel");
                fSis8300_Test1 = kFALSE;
                CloseWindow();
                break;
            case 40:  // start histogram filling
                if(fSis8300_Test1 != kTRUE)   // start only if not running
                {
                    fStartB->ChangeBackground(red);
                    fStopB->ChangeBackground(green);
                    fSis8300_Test1 = kTRUE;
                    Sis8300_Test1();
                }
                break;
            case 41:  //
                fSis8300_Test1 = kFALSE;
                fStartB->ChangeBackground(green);
                fStopB->ChangeBackground(red);
                break;

            case 60: //
                fReadSensors = kTRUE;
                break;

            case 61: //
                fReadMinMax = kTRUE;
                break;

            default:
                break;
            }
            break;

#ifdef not_used
        case kCM_RADIOBUTTON:
            switch (parm1)
            {
            case 81:
                break;
            case 82:
                break;
            }
            break;
#endif

        case kCM_CHECKBUTTON:
            //printf("\nSIS8300TestDialog::ProcessMessage:case kC_COMMAND;kCM_CHECKBUTTON parm1 = %d \n",parm1);
            switch (parm1)
            {
            case 100: //fChkFFT_Sum
                if (fChkFFT_Sum->IsOn() == kTRUE)
                {
                    fChkFFT_Db->SetEnabled(kFALSE)   ; //
                    //fChkFFT_Db->SetState(kButtonUp)   ; // is Off !
                    fChkFFTLogY->SetEnabled(kTRUE)   ; //
                    fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
                }
                else
                {
                    //fChkFFT_Db->SetState(kButtonDown)   ; // is On !
                    fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
                    fChkFFTLogY->SetEnabled(kFALSE)   ; //
                    fChkFFT_Db->SetEnabled(kTRUE)   ; //
                    if (fChkFFT_Db->IsOn() == kTRUE)
                    {
                        fChkFFT_Sum->SetEnabled(kFALSE)   ; //
                    }
                    if (fB_openfCanvas3WindowFlag == kTRUE)
                    {
                        fCanvas3->SetLogy(fChkFFTLogY->IsOn() == kTRUE);
                    }
                }
                break;

            case 101: // fChkFFTLogY
                if (fB_openfCanvas3WindowFlag == kTRUE)
                {
                    fCanvas3->SetLogy(fChkFFTLogY->IsOn() == kTRUE);
                }
                else
                {
                    fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
                }
                break;

            case 102: // fChkFFT_Db
                if (fChkFFT_Db->IsOn() == kTRUE)
                {
                    fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
                    fChkFFTLogY->SetEnabled(kFALSE)   ; //
                    fChkFFT_Sum->SetEnabled(kFALSE)   ; //
                }
                else
                {
                    fChkFFT_Sum->SetEnabled(kTRUE)   ; //
                    fChkFFTLogY->SetEnabled(kTRUE)   ; //
                }

                if (fB_openfCanvas3WindowFlag == kTRUE)
                {
                    fCanvas3->SetLogy(fChkFFTLogY->IsOn() == kTRUE);
                }
                else
                {
                    //fChkFFTLogY->SetState(kButtonUp)   ; // is Off !
                }
                break;
            default:
                break;
            }
            break;



#ifdef not_used

        case kCM_TAB:
            //printf("Tab item %ld activated\n", parm1);
            break;

        case kCM_COMBOBOX:
            break;
#endif


        default:
            //printf("default item %ld activated\n", parm1);
            break;
        }
        break;



    case kC_TEXTENTRY:
        //printf("kC_TEXTENTRY item %ld activated\n", parm1);
        switch (GET_SUBMSG(msg))
        {
        case kTE_TEXTCHANGED:
            switch(parm1)
            {
            case 20:
                //printf("\n pressed 20\n");
                break;
            case 21:
                //printf("\n pressed 21\n");
                break;
            case 27:
                gl_class_adc_tap_delay = fNumericEntries[7]->GetIntNumber();
                gl_class_adc_tap_delay = (gl_class_adc_tap_delay & 0x3f);
                fNumericEntries[7]->SetIntNumber(gl_class_adc_tap_delay); //
                SIS8300_Write_Register(gl_class_sis8300_device, SIS8300_ADC_INPUT_TAP_DELAY_REG, 0x1f00 + (gl_class_adc_tap_delay & 0x3f));//
                //printf("\n pressed 27\n");
                break;
            }
            break;
        }
        break;






    default:
        //printf("\nSIS8300TestDialog::ProcessMessage:case default parm1 = %d\n",parm1);
        break;
    }
    return kTRUE;
}

/*******************************************************/




/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
/*******************************************************************************************************************************/
//---- Main program ------------------------------------------------------------

int main(int argc, char **argv)
{
    TApplication theApp("App", &argc, argv);

    if (gROOT->IsBatch())
    {
        fprintf(stderr, "%s: cannot run in batch mode\n", argv[0]);
        return 1;
    }

    //TestMainFrame mainWindow(gClient->GetRoot(), 400, 220);
    new TestMainFrame (gClient->GetRoot(), 400, 220);

    theApp.Run();

    return 0;
}

