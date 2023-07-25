#include "cameralink.h"

//#define _NIWIN

    static SESSION_ID   Sid = 0;
    static INTERFACE_ID Iid = 1;
    static BUFLIST_ID   Bid = 0;
    static Int8*       ImaqBufferGrab[NUM_GRAB_BUFFERS];


CameraLink::CameraLink() {
     this->IsClRunning = false;
}

CameraLink::~CameraLink() {
     CameraLink_close();
}

bool CameraLink::send_TC(quint32 addr, quint8 data) {

    return send_TC(1, addr, &data);
}

bool CameraLink::send_TC(quint8 nb_data, quint32 addr, quint32 data) {
    quint8 data_ar [TMTC_SIZE_MAX];

    for (quint8 i = 0; i < nb_data; i++) {
        data_ar[i] = (data >> (8*i)) & 0xff;
    }

    return send_TC(nb_data, addr, data_ar);
}


bool CameraLink::send_TC(quint32 addr, QByteArray data) {
    quint8 data_ar [TMTC_SIZE_MAX];

    if (data.size() > TMTC_SIZE_MAX) {
        qDebug() << "ERROR in send_TC function: Too many data requested";
        return false;
    }

    for (quint8 i = 0; i < data.size(); i++) {
        data_ar[i] = static_cast<quint8>(data.at(i));
    }

    return send_TC(static_cast<quint8>(data.size()), addr, data_ar);
}

bool CameraLink::send_TC(quint8 nb_data, quint32 addr, const quint8* data){
     uInt32  datasize = uInt32(nb_data+5);
     char sentData[TMTC_SIZE_MAX];
     QString cmd_str = "WRITE " + QString::number(addr, 16);

     sentData[0] = char (nb_data);
     sentData[1] = char((addr & 0xFF000000) >>24);
     sentData[2] = char((addr & 0x00FF0000) >>16);
     sentData[3] = char((addr & 0x0000FF00) >>8);
     sentData[4] = char(addr & 0x000000FF);
     for ( quint8 i=0; i<nb_data; i++){
         sentData[i+5] =  char(data[i]);
         cmd_str.append(" " + QString::number(data[i], 16));
      }
      this->GUI_commands.append(cmd_str); // save command for export script

     // Flush then Write register through nimaq s
     int status =  imgSessionSerialFlush(Iid);
     if (status != 0)
         return false;
     status = imgSessionSerialWrite(Iid, sentData, &datasize, CAMERA_LINK_SERIAL_TIMEOUT);
//     qDebug().noquote() << "Write Status (Data : 0x"+ QString::number(sentData[5] & 0xFF,16)+" at 0x" + QString::number(addr,16) + "): " + QString::number(status);

     if (status != 0)
         return false;

     return true;
}

void CameraLink::write_data(quint32 addr, QByteArray data) {
    quint32 nb_burst;
    quint32 nb_data = static_cast<quint32>(data.size());

    if (nb_data % TMTC_SIZE_MAX == 0) {
        nb_burst = (nb_data / TMTC_SIZE_MAX);
    }
    else {
        nb_burst = (nb_data / TMTC_SIZE_MAX) + 1;
    }

    // qDebug () << "TC nbBurst = " + QString::number(nb_burst, 10);

    for (quint32 num_burst = 0; num_burst < nb_burst; num_burst++) {
        quint8 nb_sub_data = TMTC_SIZE_MAX;
        if (num_burst == nb_burst-1) {
            nb_sub_data = static_cast<quint8>(nb_data - TMTC_SIZE_MAX*(nb_burst-1));
        }

        QByteArray dataToSend = data.mid(static_cast<qint32>(num_burst * TMTC_SIZE_MAX), nb_sub_data);

        send_TC(addr + num_burst * TMTC_SIZE_MAX, dataToSend);
    }
}


bool CameraLink::CameraLink_open(){

    QString cmd_str = "CONFIG_CL";
    this->GUI_commands.append(cmd_str);

     qDebug().noquote() << "CameraLink Configuration...";

      char	        intfName[64] = "img0.iid";
      // Open an interface and a session
      Int32 status1 = imgInterfaceOpen (intfName, &Iid);
      Int32 status2 = imgSessionOpen (Iid, &Sid);

      DisplayIMAQError(status1);
     // if (status1 == 0)
     //      qDebug().noquote() << "Interface open: OK";
    Sleep(100);

      // get the pixel depth of the camera.
      quint8 bitsPerPixel;
      imgGetAttribute (Sid, IMG_ATTR_BITSPERPIXEL, &bitsPerPixel);
//     qDebug().noquote() << "Bits per pixel = " + QString::number(bitsPerPixel,10);
       qDebug().noquote() << "";
      Sleep(100);

      if ((status1 | status2 ) !=0)
          return false;
      else{
          this->IsClRunning = true;
          return true;
        }
}

void CameraLink::CameraLink_close() {
    QString cmd_str = "CLOSE_CL";
    this->GUI_commands.append(cmd_str);

    // qDebug().noquote() << "CLOSE_CL";


    Int32 status2 = imgInterfaceReset (Iid); // close ID and clean buffer
     DisplayIMAQError(status2);
    // if (status2 == 0)
    //     qDebug().noquote() << "Interface reset: OK";

    // Open an interface and a session
        Int32 status1 = imgClose (Iid,1); // close ID and clean buffer
      DisplayIMAQError(status1);
      // if (status1 == 0)
      //     qDebug().noquote() << "Interface close: OK";

      this->IsClRunning = false;
}


quint8 CameraLink::send_TM(quint32 addr) {
    QByteArray ret = send_TM(1, addr);
    return static_cast<quint8>(ret.at(0));
}

QByteArray CameraLink::send_TM(quint8 nb_data, quint32 addr) {

    if (nb_data > 128) {
        qDebug().noquote() << "ERROR: Too many data in Cameralink TM";
    }

   char addr_byte_3 = char((addr & 0xFF000000) >>24);
   char addr_byte_2 = char((addr & 0x00FF0000) >>16);
   char addr_byte_1 = char((addr & 0x0000FF00) >>8);
   char addr_byte_0 = char(addr & 0x000000FF);
   char RW_nb_data  = char( 0x80 + nb_data);

    if (nb_data == 128) {
        RW_nb_data     =  char(0x80);
    }

    char data[5];
    data[0]= (RW_nb_data);
    data[1]= (addr_byte_3);
    data[2]= (addr_byte_2);
    data[3]= (addr_byte_1);
    data[4]= (addr_byte_0);

    uInt32 datasize = 5;
//    this->spacewire.SPW_send_pkt(transmitBuffer, this->tmtc_channel, this->spw_port_num);
    int status = imgSessionSerialWrite(Iid, data, &datasize, CAMERA_LINK_SERIAL_TIMEOUT);
    DisplayIMAQError(status);
    char* buffer = new char[8192];
    bufferSize = nb_data;

    int result = imgSessionSerialReadBytes(Iid,buffer,&bufferSize,CAMERA_LINK_SERIAL_TIMEOUT);
    DisplayIMAQError(result);

//    qDebug().noquote() << "Read Status: " + QString::number(static_cast<quint32>(result));

    QByteArray final_buff;
    final_buff.clear();

    for (quint8 i = 0 ; i < nb_data ; i++) {
//        qDebug().noquote() << "Data read (at 0x" + QString::number(addr + i ,16) + ") " + QString::number(i) + ": 0x" + QString::number(buffer[i] & 0xFF,16);
        final_buff.append(buffer[i] & 0xFF);
    }
    return final_buff;
}


QByteArray CameraLink::read_data(quint32 nb_data, quint32 addr) {
    quint32 nb_burst;
    QByteArray dataOut;
    dataOut.clear();

    if (nb_data % TMTC_SIZE_MAX == 0) {
        nb_burst = (nb_data / TMTC_SIZE_MAX);
    }
    else {
        nb_burst = (nb_data / TMTC_SIZE_MAX) + 1;
    }
    //qDebug () << "TM nbBurst = " + QString::number(nb_burst, 10);

    for (quint32 num_burst = 0; num_burst < nb_burst; num_burst++) {
        quint8 nb_sub_data = TMTC_SIZE_MAX;
        if (num_burst == nb_burst-1) {
            nb_sub_data = static_cast<quint8>(nb_data - TMTC_SIZE_MAX*(nb_burst-1));
        }
        dataOut.append(send_TM(nb_sub_data, addr + num_burst * TMTC_SIZE_MAX));
        //qDebug () << "TM address = 0x" + QString::number(addr + num_burst * TMTC_SIZE_MAX, 16);
    }

    return dataOut;
}




void CameraLink::Set_Cl_buffer_params(quint32 width, quint32 height, quint8 BinX, quint8 BinY, quint8 TapNumber)
{

    this->CL_width  = width/BinX;
    this->CL_height = height/BinY;
    //rajouter bytes per pixel

    quint32 addr = (0x184);
    quint16 headerSize = NB_PXL_PER_CC*HEADER_SIZE;
    quint16 nb_extra_pxl = ((quint16(width)/BinX - headerSize)/TapNumber) -1;
    quint8 data_ar2[2];
    // qDebug().noquote() << "Set number of header extra bits register to  " + QString::number(nb_extra_pxl,10) + " (0x" + QString::number(nb_extra_pxl,16) + ")";
    // qDebug().noquote() << "--------------------------------------";
    data_ar2[0] = quint8(nb_extra_pxl & 0x00FF);
    data_ar2[1] = quint8((nb_extra_pxl & 0xFF00)>>8);
    this->send_TC(2, addr, data_ar2);
}

void CameraLink::Grab() {

 quint32 USER_WIDTH  =  this->CL_width;
 quint32 USER_HEIGHT =  this->CL_height + (HEADER_EN);

//  qDebug().noquote() << "Sid GRAB : " + QString::number(Sid, 10);

  // Set
  imgSetAttribute2 (Sid, IMG_ATTR_ROWPIXELS, USER_WIDTH);
//  qDebug().noquote() << "Set row pixel attribute to " + QString::number(USER_WIDTH, 10);

  // Set attributes
  imgSetAttribute2 (Sid, IMG_ATTR_ACQWINDOW_WIDTH, USER_WIDTH);
//  qDebug().noquote() << "Set WIDTH to " + QString::number(USER_WIDTH, 10);
//  qDebug().noquote() << "-------------------------------";

  imgSetAttribute2 (Sid, IMG_ATTR_ACQWINDOW_HEIGHT, USER_HEIGHT);
//  qDebug().noquote() << "Set HEIGHT to " + QString::number(USER_HEIGHT, 10);
//  qDebug().noquote() << "-------------------------------";

  // compute the size of the required buffer
  uInt32 bytesPerPixel;
  uInt32 bitsPerPixel;
  uInt32 clockFreq;
  uInt32 LastValidFrame;
  uInt32 LostFrames;
  uInt32 timout_ms;

  imgGetAttribute (Sid, IMG_ATTR_BYTESPERPIXEL, &bytesPerPixel);
//  qDebug().noquote() << "bytesPerPixel : " + QString::number(bytesPerPixel, 10);

  imgGetAttribute (Sid, IMG_ATTR_BITSPERPIXEL, &bitsPerPixel);
//  qDebug().noquote() << "bitsPerPixel : " + QString::number(bitsPerPixel, 10);

  imgGetAttribute (Sid, IMG_ATTR_FRAMEWAIT_MSEC, &timout_ms);
//  qDebug().noquote() << "timout_ms : " + QString::number(timout_ms, 10);

  this->bufSize  = USER_WIDTH * (USER_HEIGHT) * bytesPerPixel;
//  qDebug().noquote() << "Buffer Size : " + QString::number(bufSize, 10);


  // create a buffer and configure the buffer list
  imgCreateBufList (NUM_GRAB_BUFFERS, &Bid);
  int bufferCommand;
  for(quint8 i = 0; i<NUM_GRAB_BUFFERS; i++){
    imgCreateBuffer(Sid, FALSE, bufSize, (void **) &ImaqBufferGrab[i]);
    imgSetBufferElement2(Bid, i, IMG_BUFF_ADDRESS, ImaqBufferGrab[i]);
    imgSetBufferElement2(Bid, i, IMG_BUFF_SIZE, bufSize);
    bufferCommand = (i == (NUM_GRAB_BUFFERS - 1)) ? IMG_CMD_LOOP : IMG_CMD_NEXT;
    imgSetBufferElement2(Bid, i, IMG_BUFF_COMMAND, bufferCommand);
  }
  /* the following configuration assigns the following to buffer list
     element 0:
          1) buffer pointer that will contain image
          2) size of the buffer for buffer element 0
          3) command to stop acquisition when this element is reached

   */

  // lock down the buffers contained in the buffer list
  imgMemLock(Bid);

  // configure the session to use this buffer list
  imgSessionConfigure(Sid, Bid);

  // start the acquisition, synchronous
  int result =  imgSessionAcquire(Sid, TRUE, NULL);

//  ImaqBufferSnap = NULL;
//  int result = imgSnap (Sid,(void **) &ImaqBufferSnap);
  DisplayIMAQError(result);
  if (result != 0){
    qDebug().noquote() << "GRAB Acquisition   : NOT OK";
  }

}

void CameraLink::ReleaseGrabBuffer(){

    // unlock the buffers in the buffer list
    if (Bid != 0)
        imgMemUnlock(Bid);

    // dispose of the buffer
    for(quint8 i =0; i<NUM_GRAB_BUFFERS; i++) {
    if (ImaqBufferGrab[i] != NULL)
        imgDisposeBuffer(ImaqBufferGrab[i]);
     }
    // close this buffer list
    if (Bid != 0)
        imgDisposeBufList(Bid, FALSE);
}

//
void CameraLink::save_script_file(QString file_name) {
    QFile outputFile(file_name);
    QFileInfo fileinfo(file_name);

    qDebug().noquote()  << "Save script file to " << fileinfo.absoluteFilePath();

    // Create new file
    if (outputFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QTextStream out(&outputFile);

        for(int i = 0; i < this->GUI_commands.size(); ++i) {
            out << this->GUI_commands.at(i) << "\n";
        }

        outputFile.close();
    }
    else {
        qDebug () << "ERROR: File not opened";
    }
}

bool CameraLink::get_Cl_Running() {
    return this->IsClRunning;
}


// in case of error this function will display a dialog box
// with the error message
void CameraLink::DisplayIMAQError(Int32 error)
{

    if (error != 0) {
        static Int8 ErrorMessage[256];
        QString ErrorChar = "Erreur 0x" + QString::number((quint32(error)),16) + "\n\n";
        memset(ErrorMessage, 0x00, sizeof(ErrorMessage));
        // converts error code to a message
        imgShowError(error, ErrorMessage);

        for (quint16 i =0; i < 256; i++) {
        //  qDebug().noquote() << QString::number(ErrorMessage[i]);
            ErrorChar = ErrorChar.append(QChar(ErrorMessage[i]));
        }

        ErrorChar.replace("&apos;","'");

        qDebug().noquote() << "Displayed Error: " + ErrorChar + "\n";

        QMessageBox msgBox;

        msgBox.setIcon(QMessageBox::Warning);
        QString msg = ErrorChar;
        msgBox.setText(msg);
        msgBox.exec();
    }
}


void CameraLink::setCl_vid_TimeoutVal(int timeout_ms){

    ModifyIcd("FrameTimeout", QString::number(timeout_ms,10));
    Sleep(2000);
}



bool CameraLink::ModifyIcd(QString ParameterName, QString Data,bool reload){
    QFile file("C:/Users/Public/Documents/National Instruments/NI-IMAQ/Data/3DCM800_temp.icd");
       QString line;
       QString Buffer;
        QTextStream out(&file);

            if (file.open(QIODevice::ReadOnly| QIODevice::Text))
          {
                 while(!file.atEnd())
                {
                    line =file.readLine();
//                    qDebug().noquote() << "Line: " + line;
                    QTextStream in(&line);
                    QString line2 = line.trimmed();
//                    qDebug().noquote() << "Trimmed Line: " + line2;
                    int k= 1;
                     if ( line2.split(" (").first() == ParameterName)
                         k = 0;
//                     qDebug().noquote() << "k " + QString::number(k,10);
//                     qDebug().noquote() << "start Line: " + line2.split("(").first();
//                     qDebug().noquote() << "End Line: " + line2.split("(").last();
                    if(k==0)
                       line.replace(line.split("(").last(), Data +")\n");

                    Buffer += line;

                  }

                  file.close();

            }
            else {
                qDebug() << file.errorString();
                return 1;
            }

    // replace file by the buffer
    if (file.open(QIODevice::WriteOnly| QIODevice::Text))
    {
            out<< Buffer  ;
            file.close();
    }
    else {
        qDebug() << file.errorString();
        return 1;
    }

    // recharge icd
    if (reload == true){
        CameraLink_close();
        Sleep(100);
        CameraLink_open();
        Sleep(100);
    }

    return 0;
}

bool CameraLink::ModifyIcd(QString ParameterName, QString Data){

    return ModifyIcd( ParameterName,  Data, true);

}

bool CameraLink::InitIcd(){

    QFile fileIn("3DCM800.icd");
    QFile fileOut("C:/Users/Public/Documents/National Instruments/NI-IMAQ/Data/3DCM800_temp.icd");

    QString line;
    QString Buffer;
    QTextStream out(&fileOut);

         if (fileIn.open(QIODevice::ReadOnly| QIODevice::Text))
        {
             while(!fileIn.atEnd())
            {
                line = fileIn.readLine();
                Buffer += line;

              }

              fileIn.close();
         }
         else {
             qDebug() << fileIn.errorString();
             return 1;
         }

         // replace file by the buffer
         if (fileOut.open(QIODevice::WriteOnly| QIODevice::Text))
         {
                 out<< Buffer  ;
                 fileOut.close();
         }
         else {
             qDebug() << fileOut.errorString();
             return 1;
         }
return 0;
}

void CameraLink::UpdateTapNumber(quint8 TapNumber, bool reaload_icd){

    QString ParameterName = "NumCameraTaps";
    this->ModifyIcd(ParameterName,QString::number(TapNumber), false);

    // set CL controller
    quint8 nb_data = 1;
    quint8 data_ar[1];
    qDebug().noquote() << "Set number of taps to " + QString::number(TapNumber,10);
    data_ar[0] = TapNumber;
    this->send_TC(nb_data, 0x182, data_ar);

    ParameterName = "NumCameraTapsPerXZone";
    this->ModifyIcd(ParameterName,QString::number(TapNumber), false);

    ParameterName = "NeedYChip";
    if (TapNumber > 1){
        this->ModifyIcd(ParameterName,"Yes", false);
    }
    else {
         this->ModifyIcd(ParameterName,"No", false);
    }

    ParameterName = "NeedZChip";
    if (TapNumber > 2){
        this->ModifyIcd(ParameterName,"Yes", reaload_icd);
    }
    else {
         this->ModifyIcd(ParameterName,"No", reaload_icd);
    }

}


void CameraLink::Update_ICD(quint8 TapNumber){



    QString ParameterName = "FrameTimeout";
//    this->ModifyIcd(ParameterName,QString::number(FRAME_TIMEOUT_MS));

    ParameterName = "BitDepth";
    this->ModifyIcd(ParameterName,QString::number(NB_BITS),false);

    ParameterName = "BitsPerComponent";
    this->ModifyIcd(ParameterName,QString::number(NB_BITS),false);

    quint8 DVAL_MODE = 1;
    ParameterName = "DVALMode";
    this->ModifyIcd(ParameterName,QString::number(DVAL_MODE),false);
	
	/*******************************************/
	// Temporary (test 24 bit RGB)
    if (NB_BITS > 12) {
		quint8 BitsPerComponent = 8;
		ParameterName = "BitsPerComponent";
        this->ModifyIcd(ParameterName, QString::number(BitsPerComponent),false);
	
		quint8 NumComponents = 3;
		ParameterName = "NumComponents";
        this->ModifyIcd(ParameterName, QString::number(NumComponents),false);
		
		quint8 NumPhantomComponents = 1;
		ParameterName = "NumPhantomComponents";
        this->ModifyIcd(ParameterName, QString::number(NumPhantomComponents),false);
	}
	/******************************************/
	
    /////////////////////////////////////////
    this->UpdateTapNumber(TapNumber, true);
    //////////////////////////////////
    quint8 nb_data = 1;
    quint8 data_ar[1];
    quint32 addr = (0x183);
    quint8 header_en = HEADER_EN;
//    qDebug().noquote() << "Set Header to " + QString::number(header_en,10);
    data_ar[0] = header_en;
    this->send_TC(nb_data, addr, data_ar);


//    addr = (0x184);
//    quint16 headerSize = 8*28;
//    // if (NB_BITS > 8) {
//    //     headerSize = 8*28*2;
//    // }
//    quint16 nb_extra_pxl = ((quint16(this->CL_width) - headerSize)/TapNumber) -1;
//    quint8 data_ar2[2];
//    qDebug().noquote() << "Set number of extra bits to " + QString::number(nb_extra_pxl,10);
//    qDebug().noquote() << "--------------------------------------";
//    data_ar2[0] = quint8(nb_extra_pxl & 0x00FF);
//    data_ar2[1] = quint8((nb_extra_pxl & 0xFF00)>>8);
//    this->send_TC(2, addr, data_ar2);





}


uInt32 CameraLink::GetTest(){

    uInt32 num_frame;
   int result = imgGetAttribute (Sid, IMG_ATTR_LAST_VALID_FRAME, &num_frame);

   if (!result) {
       qDebug().noquote() <<  "ERROR Get Attribute! TEST";
   }

   return num_frame;
}



Acquisition_params CameraLink::Get_CL_Acq_params(){
     Acquisition_params CL_Acq_params;
    CL_Acq_params.Sid = Sid;
    CL_Acq_params.Buffsize = this->bufSize;
    return CL_Acq_params;
}
