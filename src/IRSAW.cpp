/*
Copyright (c) <YEAR>, Intel Corporation

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
* Neither the name of Intel Corporation nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/





// Important information **** If you use the camera in the vertical config, the cable comes out of the top of the camera,
// if you use it in horizontal config, the cable comes out from the right of the camera.

/*
 INSTRUCTIONS ON HOW TO SETUP THE PROJECT

 A. Things you need

 1. x86 based computer with Visual Studio 2012, Cinder 0.8.6 VC2012, OpenCV 2.4.9, RSSDK 4.0 or higher and Depth Camera Manager (DCM) for Intel RealSense R200 depth camera
 2. Portable Wireless router (We used Dlink DIR-510L. It has a battery built in)
 3. 8 x Vibration Actuators (Battery powered Spark Cores with Vibration Motors)
 4. Intel RealSense Camera (R200)
 5. Making sure that the Computer, The router battery and the Batteries of Vibration Actuators are charged

 B. Getting started....

 1. Make sure that you've installed Visual Studio 2012, Cinder 0.8.6 VC2012, OpenCV 2.4.9.
 2. Install the DCM (Depth Camera Manager) for R200 first, and then install the RealSense SDK 4.X. While Installing the DCM, your camera (R200) must be plugged in
	The DCM installation updates the firmware of the camera. 
 3. Once the SDK is installed, try running the samples in "C:\Program Files (x86)\Intel\RSSDK\bin\x64" (This is the location where you install the RSSDK) All the samples starting with DF and RF should work fine.
 4. (Very Important) In the project properties, make sure you are letting visual studio know where the include file and library files are for the Debug and Release configurations.
 5. Once that is set, you should be able to run the program. 

 C. Setting up the router

 Connect to your router using your computer and open the router settings and then assign fixed IP addresses to 
 the laptop. Once these IPs are assigned, In the Spark Core code, you change the IP of the server
 to the currently assigned static IP of the laptop. 

 Connect every single vibration actuator to the router and assign fixed IP's to them as well.
 Once you have these IP address, you have to assign those IP addresses below in this code. 

 Once that's done you plug in the RealSense Camera and run the application.. 

 The application is a TCP server implementation which waits for connection from the Vibration Actuators which function as TCP Clients

 Because you have already specified in the code, The server know which client connecting is which Vibration Actuator. 

 Once the all the Actuators are connected to the server, the System is ready to go.
*/

///Cinder
#include "cinder/app/AppNative.h"
#include "cinder/gl/gl.h"
#include "cinder/app/AppNative.h"
#include "cinder/ip/Resize.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/ImageIo.h"
#include "cinder/Surface.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"
#include "cinder\CinderMath.h"
#include <iostream>
#include "cinder/app/AppBasic.h"
#include "cinder/params/Params.h"

#include <functional>


//OpenCV
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"


//RealSense SDK
#include <memory>
#include "pxcsensemanager.h"
#include "pxcprojection.h"

// for Winsock
#include<stdio.h>
#include<winsock2.h>
 
#pragma comment(lib, "ws2_32.lib") //Winsock Library

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace cv;

#define SIZEOF_ARRAY( a ) (sizeof( a ) / sizeof( a[ 0 ] )) 


class IRSAWApp : public AppNative {
  public:
	  


	//Variables for RSSDK for holding camera data.
	PXCSenseManager		*mSenseMgr;
	PXCProjection		*mCoordinateMapper;
	PXCCapture::Sample	*mCurrentSample;

	bool mIsInit,
		mIsRunning,
		mHasRgb,
		mHasDepth,
		mShouldAlign,
		mShouldGetDepthAsColor;
	 
	ci::Vec2i		mDepthSize;
	pxcStatus cStatus;


	// Booleans for params for togging specific motor, all the motors, vertical horizontal camera configuration
	bool verticalConfig;
	bool areAllMotorsOn;
	bool isTROn;
	bool isTCOn;
	bool isTLOn;
	bool isMROn;
	bool isMCOn;
	bool isMLOn;
	bool isBROn;
	bool isBLOn;
	bool isTopOn;
	bool isMiddleOn;
	bool isBottomOn;
	bool isCenterOn;

	//variable for toggling rendering of visualizer.
	bool isVisualizerVisible;

	//these are parameters to set the distance of detection. The distance can be specified millimeters
	int topDetectionThreshold;
	int middleDetectionThreshold;
	int bottomDetectionThreshold;

	//parameters for streaming
	int numberOfFramesSkipped;
	int frameCounter;

	//Variables for receiving data from the Vibration Actuator with the button to turn vibration On/Off.
	char isVibrationOn[1];
	int recvResult;
	int isVibrationOnInt;

	// variables for initial Server message to clients on connection
	char lightsON;

	//Fixed IP's of the clients are stored in these variables
	char* TLIP;
	char* TCIP;
	char* TRIP;
	char* MLIP;
	char* MCIP;
	char* MRIP;
	char* BLIP;
	char* BRIP;
	char* ClientIP;

	//chars to store data to be sent to the cores
	char cdv1;
	char cdv2;
	char cdv3;
	char cdv4;
	char cdv5;
	char cdv6;
	char cdv7;
	char cdv8;

	char zero;

	//Depth buffer which is streamed
	char* depthdata;

	// variables to store the position of the client  in the client_socket[] array

	int TLPos;
	int TCPos;
	int TRPos;
	int MLPos;
	int MCPos;
	int MRPos;
	int BLPos;
	int BRPos;
	int ClientPos;

	//storing depth pixels as unsigned int 8 bit.
	uint8_t clampedPixel0;
	uint8_t clampedPixel1;
	uint8_t clampedPixel2;
	uint8_t clampedPixel3;
	uint8_t clampedPixel4;
	uint8_t clampedPixel5;
	uint8_t clampedPixel6;
	uint8_t clampedPixel7;
	uint8_t clampedPixel8;

	//stores the depth image
	uint16_t * depth;
	//stores the depth image for streaming
	uint8_t  sendDepthBuffer[480*360];


	// matrix pointer to do openCV operations
	Mat src;

	//element is used to specify the kernel size for the erode function. changing the kernel size  will alter the result of erosion
	Mat element;

	int closestDepthValue1;   //for top left
	int closestDepthValue2;   //for top right
	int closestDepthValue3;   //for middle left
	int closestDepthValue4;   // for middle right
	int closestDepthValue5;   // for bottom left
	int closestDepthValue6;   // for bottom right
	int closestDepthValue7;    // for top center
	int closestDepthValue8;    // for middle center

	// ints to store the current depth values of the pixel being processed.
	int currentDepthValue1;
	int currentDepthValue2;
	int currentDepthValue3;
	int currentDepthValue4;
	int currentDepthValue5;
	int currentDepthValue6;
	int currentDepthValue7;
	int currentDepthValue8;

	//Parameters to set the bounds of the sections
	int HoT;    //Height of Top
	int HoM;	//Height of Middle
	int HoB;	//Height of Bottom
	int WoR;	//Width of Right
	int WoC;	//Width of Center
	int WoL;	//Width of Left
	int WoBR;	//Width of Bottom Right
	int WoBL;	//Width of Bottom Left

	// Rectangles drawn by on Screen representing every single section.
	Rectf TLzone;
	Rectf TCzone;
	Rectf TRzone;
	Rectf MLzone;
	Rectf MCzone;
	Rectf MRzone;
	Rectf BLzone;
	Rectf BRzone;

	// Variables for Winsock
	WSADATA wsa;
	SOCKET master , new_socket , client_socket[30] , s;
	struct sockaddr_in server, address;
	int max_clients , activity, addrlen, i;

	//set of socket descriptors
	fd_set readfds;
	timeval waitTime; //  wait time for the server to wait for client connections

	//For Cinder Params
	params::InterfaceGlRef	mParams;

	void setup();
	void update();
	void draw();
	void CloseDS();
	void shutdown();
	void toggleTopMotors();
	void toggleMiddleMotors();
	void toggleBottomMotors();
	void toggleCenterMotors();
	void toggleAllMotors();
	void toggleVerticalHorizontal();
	void keyDown( KeyEvent event );
	
	//variables used to convert the depth buffer visualize it.
	Channel8u		mDepthChannel;
	gl::Texture		mDepthTex;
	int			mDepthW, mDepthH; // These variables store the height and depth of the raw depth buffer

	//map and clampToByte function map the depth values from 0 to 256 and convert that data into byte
	float map(float value, float inputMin, float inputMax, float outputMin, float outputMax);
	uint8_t clampToByte(float value, float min, float max);

	

};


void IRSAWApp::keyDown( KeyEvent event )
{
	// Pressing q will quit the program
	if( (event.getChar() == 'q') || (event.getChar() == 'Q') )
	{
		closesocket(client_socket[TRPos]);
		closesocket(client_socket[TCPos]);
		closesocket(client_socket[TLPos]);
		closesocket(client_socket[MRPos]);
		closesocket(client_socket[MCPos]);
		closesocket(client_socket[MLPos]);
		closesocket(client_socket[BRPos]);
		closesocket(client_socket[BLPos]);
		WSACleanup();
		quit();
	}

	//pressing h will switch toggle verticle/horizontal camera config
	if ((event.getChar() == 'H') || (event.getChar() == 'h'))
	{
		toggleVerticalHorizontal();
	}

	//pressing v will switch toggle visiblility of visualizer
	if ((event.getChar() == 'V') || (event.getChar() == 'v'))
	{
		isVisualizerVisible = !isVisualizerVisible;
	}

	//pressing t will toggle the top row of motors
	if( (event.getChar() == 'T') || (event.getChar() == 't') )
	{
		toggleTopMotors();
	}

	//pressing b will toggle the bottom row of motors
	if( (event.getChar() == 'B') || (event.getChar() == 'b') )
	{
		toggleBottomMotors();
	}	

	//pressing m will toggle the middle row of motors
	if( (event.getChar() == 'M') || (event.getChar() == 'm') )
	{
		toggleMiddleMotors();
	}

	//pressing c will toggle the center column of motors
	if( (event.getChar() == 'C') || (event.getChar() == 'c') )
	{
		toggleCenterMotors();
	}

	//pressing o will toggle all the motors
	if( (event.getChar() == 'o') || (event.getChar() == 'O') )
	{
		toggleAllMotors();
	}
}

//Close camera stream while shutting down
void IRSAWApp::shutdown()
{
	if (mSenseMgr)
	{
		mCoordinateMapper->Release();
		mSenseMgr->Close();
	}
	
}

 //toggles Vertical camera orienation vs horizontal camera orientation
 void IRSAWApp::toggleVerticalHorizontal()
 {
	 if(verticalConfig == true)
	 {
		verticalConfig = false;
		HoT = mDepthH/3;
		HoM = mDepthH*2/3;
		HoB = mDepthH;
		WoR = mDepthW/3;
		WoC = mDepthW*2/3;
		WoL = mDepthW;
		WoBR = mDepthW/2;
		WoBL = mDepthW;
	 }
	 else
  	 {
		verticalConfig = true;
		HoT = mDepthW/3;
		HoM = mDepthW*2/3;
		HoB = mDepthW;
		WoR = mDepthH/3;
		WoC = mDepthH*2/3;
		WoL = mDepthH;
		WoBR = mDepthH/2;
		WoBL = mDepthH;
	 }
 }

 // Toggles all the motors on/off
 void IRSAWApp::toggleAllMotors()
 {
	 if(areAllMotorsOn == true)
	 {
		 areAllMotorsOn = false;
		 isTopOn = false;
		 isMiddleOn = false;
		 isBottomOn = false;
		 isTROn = false;
		 isTCOn = false;
		 isTLOn = false;
		 isMROn = false;
		 isMCOn = false;
		 isMLOn = false;
		 isBROn = false;
		 isBLOn = false;
	 }
	 else
	 {
		 areAllMotorsOn = true;
		 isTopOn = true;
		 isMiddleOn = true;
		 isBottomOn = true;
		 isTROn = true;
		 isTCOn = true;
		 isTLOn = true;
		 isMROn = true;
		 isMCOn = true;
		 isMLOn = true;
		 isBROn = true;
		 isBLOn = true;
	 } 
 }


 // toggles the center column of motors on/off
 void IRSAWApp::toggleCenterMotors()
 {
	 if(isCenterOn == true)
	 {
		 isCenterOn = false;
		 isTCOn = false;
		 isMCOn = false;
	 }
	 else
	 {
		 isCenterOn = true;
		 isTCOn = true;
		 isMCOn = true;
	 }
 }

 //toggles the top row of the motors on/off
 void IRSAWApp::toggleTopMotors()
 {
	 if(isTopOn == true)
	 {
		 isTopOn = false;
		 isTROn = false;
		 isTCOn = false;
		 isTLOn = false;
	 }
	 else
	 {
		 isTopOn = true;
		 isTROn = true;
		 isTCOn = true;
		 isTLOn = true;	 
	 } 
 }

 // toggles the middle row of motors on/off
 void IRSAWApp::toggleMiddleMotors()
 {
	 if(isMiddleOn == true)
	 {
		 isMiddleOn = false;
		 isMROn = false;
		 isMCOn = false;
		 isMLOn = false;
	 }
	 else
	 {
		 isMiddleOn = true;
		 isMROn = true;
		 isMCOn = true;
		 isMLOn = true;
	 }
 }

 //toggles the bottom row of the motors on/off
 void IRSAWApp::toggleBottomMotors()
 {
	 if(isBottomOn == true)
	 {
		 isBottomOn = false;
		 isBROn = false;
		 isBLOn = false;
	 }
	 else
	 {
		 isBottomOn = true;
		 isBROn = true;
		 isBLOn = true;
	 }
 }


//This is the mapping function to map the values for 0 to 255
float IRSAWApp::map(float value, float inputMin, float inputMax, float outputMin, float outputMax) {
      return ((value - inputMin) / (inputMax - inputMin) * (outputMax - outputMin) + outputMin);
}

//This function converts the value to byte
uint8_t IRSAWApp::clampToByte(float value, float min, float max) {
      return uint8_t(value < min ? min : value > max ? max : value);
}

void IRSAWApp::setup()
{
	// Initialize all the variables

	verticalConfig = true;
	areAllMotorsOn = true;
	isTROn = true;
	isTCOn = true;
	isTLOn = true;
	isMROn = true;
	isMCOn = true;
	isMLOn = true;
	isBROn = true;
	isBLOn = true;
	isTopOn = true;
	isMiddleOn = true;
	isBottomOn = true;

	isVisualizerVisible = false;
	
	frameCounter = 0;
	numberOfFramesSkipped = 5; // skip 5 frames before sending one. Important only if you use the streamer.

	waitTime.tv_sec = 0;      
	waitTime.tv_usec = 0;

	isVibrationOn[0] = 1;
	isVibrationOnInt = 1;
	recvResult = 0;

	lightsON = 1;

	//add the params controls to be displayed in the window
	mParams = params::InterfaceGl::create( getWindow(), "App parameters", toPixels( cinder::Vec2i( 250, 500 ) ) );
	
	mParams->addParam( "Top Threshold in mm", &topDetectionThreshold );
	mParams->addParam( "Middle Threshold in mm", &middleDetectionThreshold );
	mParams->addParam( "Bottom Threshold in mm", &bottomDetectionThreshold );
	mParams->addSeparator();
	mParams->addParam( "Height of Top", &HoT );
	mParams->addParam( "Height of Middle", &HoM );
	mParams->addParam( "Width of Center", &WoC );
	mParams->addSeparator();
	mParams->addParam("Toggle Visualizer Rendering", &isVisualizerVisible);
	mParams->addSeparator();
	mParams->addButton("Toggle Vertical / Horizontal",std::bind( &IRSAWApp::toggleVerticalHorizontal, this ));
	mParams->addSeparator();
	mParams->addButton("Toggle All Motors",std::bind( &IRSAWApp::toggleAllMotors, this ));
	mParams->addSeparator();
	mParams->addButton("Toggle Top Motors",std::bind( &IRSAWApp::toggleTopMotors, this ));
	mParams->addButton("Toggle Middle Motors",std::bind( &IRSAWApp::toggleMiddleMotors, this ));
	mParams->addButton("Toggle Bottom Motors",std::bind( &IRSAWApp::toggleBottomMotors, this ));
	mParams->addButton("Toggle Center Motors",std::bind( &IRSAWApp::toggleCenterMotors, this ));
	mParams->addSeparator();
	mParams->addParam( "Toggle Top Right Motor", &isTROn );
	mParams->addParam( "Toggle Top Center Motor", &isTCOn );
	mParams->addParam( "Toggle Top Left Motor", &isTLOn );
	mParams->addSeparator();
	mParams->addParam( "Toggle Middle Right Motor", &isMROn );
	mParams->addParam( "Toggle Middle Center Motor", &isMCOn );
	mParams->addParam( "Toggle Middle Left Motor", &isMLOn );
	mParams->addSeparator();
	mParams->addParam( "Toggle Bottom Right Motor", &isBROn );
	mParams->addParam( "Toggle Bottom Left Motor", &isBLOn );
	
    //Fixed IP's of the clients
	//Here you specify the assigned fixed IP's of the Vibration Actuators which you stored in the router 

	TLIP = "192.168.0.101";
	TCIP = "192.168.0.103";     
	TRIP = "192.168.0.111";
	MLIP = "192.168.0.105";
	MCIP = "192.168.0.106";
	MRIP = "192.168.0.107";
	BLIP = "192.168.0.108";
	BRIP = "192.168.0.109";
	//ClientIP = "192.168.0.110"; // for streaming client

    // Initialize variables to store the position of the client  in the client_socket[] array
    // Do not initialize this variable to 0

	TLPos = 100;
	TCPos = 100;
	TRPos = 100;
	MLPos = 100;
	MCPos = 100;
	MRPos = 100;
	BLPos = 100;
	BRPos = 100;
	//ClientPos = 100;

	max_clients = 30; // maximum number of clients that can connect

	//Initial detection Threshold is 2000mm
    topDetectionThreshold = 2000;
	middleDetectionThreshold = 2000;
	bottomDetectionThreshold = 2000;

	//Winsock Server Initialization begins here. 
	for(i = 0 ; i < 30;i++)
    {
        client_socket[i] = 0;
    }
	console() << "\nInitialising Winsock..." << std::endl;
	    
    if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
    {
		console() << "Failed. Error Code : " << WSAGetLastError() << std::endl;
        exit(EXIT_FAILURE);
    }
	console() << "Initialised. \n" << std::endl;

    //Create a socket
    if((master = socket(AF_INET , SOCK_STREAM , 0 )) == INVALID_SOCKET)
    {
		console() << "Could not create socket : " << WSAGetLastError() << std::endl;
        exit(EXIT_FAILURE);
    } 
	console() << "Socket Created.\n" << std::endl;
  
    //Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons( 5555 );
     
    //Bind
    if( ::bind(master ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
    {
        console() << "Bind failed : " << WSAGetLastError() << std::endl;
        exit(EXIT_FAILURE);
    }
    console() << "Bind Done\n" << std::endl;
 
    //Listen to incoming connections
    listen(master , 3);
     
    //Accept and incoming connection
    console() << "Waiting for incoming connections\n" << std::endl; 
    addrlen = sizeof(struct sockaddr_in);

	//Initialize RSSDK to receive the depth data
	mSenseMgr = PXCSenseManager::CreateInstance();
	if (mSenseMgr)
		mIsInit = true;


	if (mSenseMgr)
	{
		//Resolution of the depth image we need is 480 X 360
		mDepthSize = ci::Vec2i(480, 360);
		cStatus = mSenseMgr->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, mDepthSize.x, mDepthSize.y, 60); // This call enables the depth stream and sets the size of the depth frame and the frame rate, 60 fps 
		if (cStatus >= PXC_STATUS_NO_ERROR)
		{
			mHasDepth = true;
			console() << "Depth Camera Parameters successfully set" << std::endl;
		}
	}

    // Start Camera to capture images
	pxcStatus cStatus = mSenseMgr->Init();
	if (cStatus >= PXC_STATUS_NO_ERROR)
	{
		mCoordinateMapper = mSenseMgr->QueryCaptureManager()->QueryDevice()->CreateProjection();
		console() << "Sense Manager Initialization successful" << std::endl;
	}
	else
		console() << "Sense Manager Initialization failed" << std::endl;

	mDepthW = mDepthSize.x; // get the width of the depth buffer
	mDepthH = mDepthSize.y; // get the height of the depth buffer

	// assign initialization values to the parameters setting the bounds of the sections.
	HoT = mDepthW/3;
	HoM = mDepthW*2/3;
	HoB = mDepthW;
	WoR = mDepthH/3;
	WoC = mDepthH*2/3;
	WoL = mDepthH;
	WoBR = mDepthH/2;
	WoBL = mDepthH;

	mDepthChannel = Channel8u(mDepthW, mDepthH);

	setWindowSize(mDepthH*2, mDepthW);
}



void IRSAWApp::update()
{
	pxcStatus cStatus;
	if (mSenseMgr)
	{
		cStatus = mSenseMgr->AcquireFrame(); //Acquire the frame based on given stream parameters
		if (cStatus < PXC_STATUS_NO_ERROR)
			console() << "Failed to acquire frame" << std::endl;

		PXCCapture::Sample *mCurrentSample = mSenseMgr->QuerySample();
		if (!mCurrentSample)
			console() << "failed to query sample" << std::endl;
		
		if (mHasDepth)
		{
			if (!mCurrentSample->depth)
				console() << "Failed to sample depth" << std::endl;
			PXCImage *cDepthImage = mCurrentSample->depth;
			PXCImage::ImageData cDepthData;
			cStatus = cDepthImage->AcquireAccess(PXCImage::ACCESS_READ, PXCImage::PIXEL_FORMAT_DEPTH, &cDepthData); // read the depth frame in cDepthData
			if (cStatus < PXC_STATUS_NO_ERROR)
			{
				cDepthImage->ReleaseAccess(&cDepthData);
				console() << "Failed to access depth data" << std::endl;
			}
			depth = reinterpret_cast<uint16_t *>(cDepthData.planes[0]); // we store the depth map in the "depth" variable which is a "uint16_t*"

			// copy the latest data and resample to fit 500mm - 2500mm into 0-255 grayscale for sending from the streamer
			/*for(int i = 0;i<480*360;i++)
			{
			int temp = depth[i];
			if(temp > 2500 || temp < 500)
			{
			sendDepthBuffer[i] = 0;
			}
			else
			sendDepthBuffer[i] = clampToByte(map(temp, 500, 2500, 255, 0), 0, 255);
			}*/

			//// Resample depth data to fit 500mm - 4000mm into 0-255 grayscale for sending from the visualizer
			if (isVisualizerVisible)
			{
				int q = 0;
				auto it = mDepthChannel.getIter();
				while (it.line())
				{
					while (it.pixel())
					{
						uint16_t val = *(depth + q);
						if (val < 500)
							it.v() = 0;
						else
							it.v() = (int)math<float>::clamp(lmap<float>((float)val, 500, 4000, 255, 0), 0, 255);
						q++;
					}
				}
				mDepthTex = gl::Texture(mDepthChannel);
			}

			//determining the widths of the right and the left section.
			if (verticalConfig == true)
			{
				WoR = (mDepthH - (WoC - WoR)) / 2;
			}
			else
			{
				WoR = (mDepthW - (WoC - WoR)) / 2;
			}

			// Setting locks on all variables so their value cannot go beyond a certain point when you tweak them in the window
			//Locks on Low detection thresholds. 
			//You cannnot detect below 501 mm because of the Minimum z distance of the camera
			if (topDetectionThreshold < 502)
				topDetectionThreshold = 502;
			if (middleDetectionThreshold < 502)
				middleDetectionThreshold = 502;
			if (bottomDetectionThreshold < 502)
				bottomDetectionThreshold = 502;

			//Locks on High detection thresholds
			//You cannnot detect above 4000 mm because iR emitter cannot illuminate beyond that.
			if (topDetectionThreshold > 4000)
				topDetectionThreshold = 4000;
			if (middleDetectionThreshold > 4000)
				middleDetectionThreshold = 4000;
			if (bottomDetectionThreshold > 4000)
				bottomDetectionThreshold = 4000;

			//Setting Locks on Height and Width Parameters
			if (verticalConfig == true)
			{
				if (WoC < 180)
					WoC = 180;
				if (WoC > 360)
					WoC = 360;
				if (HoM < HoT)
					HoM = HoT;
				if (HoM > mDepthW)
					HoM = mDepthW;
			}
			else
			{
				if (WoC < 240)
					WoC = 240;
				if (WoC > 480)
					WoC = 480;
				if (HoM < HoT)
					HoM = HoT;
				if (HoM > mDepthH)
					HoM = mDepthH;
			}

			if (HoT < 0)
				HoT = 0;
			if (HoT > HoM)
				HoT = HoM;

			// OPENCV MAGIC BEGINS HERE 
			// Assign a Mat header to depth data array
			src = Mat(cvSize(480, 360), CV_16UC1, depth);

			//element is used to specify the kernel size for the erode function. changing the kernel size  will alter the result of erosion
			element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(-1, -1));

			//We run the erode function on the depth data to get rid of any noise from the data
			//***** Important, src Matrix is just a header which points to the depth data stored in depth[]
			//any changes made to src affects depth[]
			erode(src, src, element);
			// the result of the erode is stored in src to save some memory space

			// Now follows the code for finding the closest pixel in every section
			closestDepthValue1 = 10000;   //for top left
			closestDepthValue2 = 10000;   //for top right
			closestDepthValue3 = 10000;   //for middle left
			closestDepthValue4 = 10000;   // for middle right
			closestDepthValue5 = 10000;   // for bottom left
			closestDepthValue6 = 10000;   // for bottom right
			closestDepthValue7 = 10000;   // for top center
			closestDepthValue8 = 10000;   // for middle center

			currentDepthValue1 = 0;
			currentDepthValue2 = 0;
			currentDepthValue3 = 0;
			currentDepthValue4 = 0;
			currentDepthValue5 = 0;
			currentDepthValue6 = 0;
			currentDepthValue7 = 0;
			currentDepthValue8 = 0;

			//for loops below to find out the closest pixels in the 8 sections...
			if (verticalConfig == true) // findclosest pixels in 8 sections for vertical configuration
			{
				// bottom right
				for (int j = 0; j < WoBR; j++)
				{
					for (int i = HoM; i < HoB; i++)
					{
						currentDepthValue6 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue6 > 500 && currentDepthValue6 < closestDepthValue6)
						{
							closestDepthValue6 = currentDepthValue6;
						}
					}
				}

				// top center
				for (int j = WoR; j < WoC; j++)
				{
					for (int i = 0; i < HoT; i++)
					{
						currentDepthValue7 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue7 > 500 && currentDepthValue7 < closestDepthValue7)
						{
							closestDepthValue7 = currentDepthValue7;
						}
					}
				}

				//bottom left 
				for (int j = WoBR; j < WoBL; j++)
				{
					for (int i = HoM; i < HoB; i++)
					{
						currentDepthValue5 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue5 > 500 && currentDepthValue5 < closestDepthValue5)
						{
							closestDepthValue5 = currentDepthValue5;
						}
					}
				}

				//middle right 
				for (int j = 0; j < WoR; j++)
				{
					for (int i = HoT; i < HoM; i++)
					{
						currentDepthValue4 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue4 > 500 && currentDepthValue4 < closestDepthValue4)
						{
							closestDepthValue4 = currentDepthValue4;
						}
					}
				}

				// middle center
				for (int j = WoR; j < WoC; j++)
				{
					for (int i = HoT; i < HoM; i++)
					{
						currentDepthValue8 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue8 > 500 && currentDepthValue8 < closestDepthValue8)
						{
							closestDepthValue8 = currentDepthValue8;
						}
					}
				}

				//middle left 
				for (int j = WoC; j < WoL; j++)
				{
					for (int i = HoT; i < HoM; i++)
					{
						currentDepthValue3 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue3 > 500 && currentDepthValue3 < closestDepthValue3)
						{
							closestDepthValue3 = currentDepthValue3;
						}
					}
				}

				//top right
				for (int j = 0; j < WoR; j++)
				{
					for (int i = 0; i < HoT; i++)
					{
						currentDepthValue2 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue2 > 500 && currentDepthValue2 < closestDepthValue2)
						{
							closestDepthValue2 = currentDepthValue2;
						}
					}
				}

				//top left
				for (int j = WoC; j < WoL; j++)
				{
					for (int i = 0; i < HoT; i++)
					{
						currentDepthValue1 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue1 > 500 && currentDepthValue1 < closestDepthValue1)
						{
							closestDepthValue1 = currentDepthValue1;
						}
					}
				}
			}
			else // find closest pixels in 8 sections for horizontal configuration
			{
				// bottom right
				for (int j = HoM; j < HoB; j++)
				{
					for (int i = WoBR; i < WoBL; i++)
					{
						currentDepthValue6 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue6 > 500 && currentDepthValue6 < closestDepthValue6)
						{
							closestDepthValue6 = currentDepthValue6;
						}
					}
				}

				// top center
				for (int j = 0; j < HoT; j++)
				{
					for (int i = WoR; i < WoC; i++)
					{
						currentDepthValue7 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue7 > 500 && currentDepthValue7 < closestDepthValue7)
						{
							closestDepthValue7 = currentDepthValue7;
						}
					}
				}

				//bottom left 
				for (int j = HoM; j < HoB; j++)
				{
					for (int i = 0; i < WoBR; i++)
					{
						currentDepthValue5 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue5 > 500 && currentDepthValue5 < closestDepthValue5)
						{
							closestDepthValue5 = currentDepthValue5;
						}
					}
				}

				//middle right 
				for (int j = HoT; j < HoM; j++)
				{
					for (int i = WoC; i < WoL; i++)
					{
						currentDepthValue4 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue4 > 500 && currentDepthValue4 < closestDepthValue4)
						{
							closestDepthValue4 = currentDepthValue4;
						}
					}
				}

				// middle center
				for (int j = HoT; j < HoM; j++)
				{
					for (int i = WoR; i < WoC; i++)
					{
						currentDepthValue8 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue8 > 500 && currentDepthValue8 < closestDepthValue8)
						{
							closestDepthValue8 = currentDepthValue8;
						}
					}
				}

				//middle left 
				for (int j = HoT; j < HoM; j++)
				{
					for (int i = 0; i < WoR; i++)
					{
						currentDepthValue3 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue3 > 500 && currentDepthValue3 < closestDepthValue3)
						{
							closestDepthValue3 = currentDepthValue3;
						}
					}
				}

				//top right
				for (int j = 0; j < HoT; j++)
				{
					for (int i = WoC; i < WoL; i++)
					{
						currentDepthValue2 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue2 > 500 && currentDepthValue2 < closestDepthValue2)
						{
							closestDepthValue2 = currentDepthValue2;
						}
					}
				}

				//top left
				for (int j = 0; j < HoT; j++)
				{
					for (int i = 0; i < WoR; i++)
					{
						currentDepthValue1 = depth[i + (j*mDepthSize.x)];
						// this if finds the closest pixel
						if (currentDepthValue1 > 500 && currentDepthValue1 < closestDepthValue1)
						{
							closestDepthValue1 = currentDepthValue1;
						}
					}
				}
			}

			//Print the distance from the closest pixel in every section
			//console() //<< "cp:" << closestDepthValue << " "
			//	      << "cp1:" << closestDepthValue1 << " " 
			//		  << "cp2:" << closestDepthValue2 << " " 
			//		  << "cp3:" << closestDepthValue3 << " " 
			//		  << "cp4:" << closestDepthValue4 << " " 
			//		  << "cp5:" << closestDepthValue5 << " " 
			//		  << "cp6:" << closestDepthValue6 << " " 
			//		  << std::endl;


			// Remapping depth distance of the closest depth pixel in each section to 0 to 255
			//and then converting it to char to send to the specific spark cores

			clampedPixel0 = 0;
			//std::cout << "clampedPixel1: " << static_cast<unsigned>(clampedPixel1) << std::endl; // pretty print 
			zero = (char)clampedPixel0;

			clampedPixel1 = clampToByte(map(closestDepthValue1, topDetectionThreshold, 501, 0, 255), 0, 255);
			//std::cout << "clampedPixel1: " << static_cast<unsigned>(clampedPixel1) << std::endl; // pretty print 
			cdv1 = (char)clampedPixel1;

			clampedPixel2 = clampToByte(map(closestDepthValue2, topDetectionThreshold, 501, 0, 255), 0, 255);
			//std::cout << "clampedPixel2: " << static_cast<unsigned>(clampedPixel2) << std::endl; // pretty print 
			cdv2 = (char)clampedPixel2;

			clampedPixel3 = clampToByte(map(closestDepthValue3, middleDetectionThreshold, 501, 0, 255), 0, 255);
			//std::cout << "clampedPixel3: " << static_cast<unsigned>(clampedPixel3) << std::endl; // pretty print 
			cdv3 = (char)clampedPixel3;

			clampedPixel4 = clampToByte(map(closestDepthValue4, middleDetectionThreshold, 501, 0, 255), 0, 255);
			//std::cout << "clampedPixel4: " << static_cast<unsigned>(clampedPixel4) << std::endl; // pretty print 
			cdv4 = (char)clampedPixel4;

			clampedPixel5 = clampToByte(map(closestDepthValue5, bottomDetectionThreshold, 501, 0, 255), 0, 255);
			//std::cout << "clampedPixel5: " << static_cast<unsigned>(clampedPixel5) << std::endl; // pretty print 
			cdv5 = (char)clampedPixel5;

			clampedPixel6 = clampToByte(map(closestDepthValue6, bottomDetectionThreshold, 501, 0, 255), 0, 255);
			//std::cout << "clampedPixel6: " << static_cast<unsigned>(clampedPixel6) << std::endl; // pretty print 
			cdv6 = (char)clampedPixel6;

			clampedPixel7 = clampToByte(map(closestDepthValue7, topDetectionThreshold, 501, 0, 255), 0, 255);
			//std::cout << "clampedPixel7: " << static_cast<unsigned>(clampedPixel7) << std::endl; // pretty print 
			cdv7 = (char)clampedPixel7;

			clampedPixel8 = clampToByte(map(closestDepthValue8, middleDetectionThreshold, 501, 0, 255), 0, 255);
			//std::cout << "clampedPixel8: " << static_cast<unsigned>(clampedPixel8) << std::endl; // pretty print 
			cdv8 = (char)clampedPixel8;


			//ACCEPTING SPARK CORE CONNECTIONS

			//clear the socket fd set
			FD_ZERO(&readfds);

			//add master socket to fd set
			FD_SET(master, &readfds);

			//add child sockets to fd set
			for (i = 0; i < max_clients; i++)
			{
				s = client_socket[i];
				if (s > 0)
				{
					FD_SET(s, &readfds);
				}
			}
			// console() << "before select\n" << std::endl;
			//wait for an activity on any of the sockets, timeout is NULL , so wait indefinitely
			activity = select(0, &readfds, NULL, NULL, &waitTime);
			//console() << "after select\n" << std::endl;

			if (activity == SOCKET_ERROR)
			{
				console() << "select call failed with error code : " << WSAGetLastError() << std::endl;
				exit(EXIT_FAILURE);
			}

			//If something happened on the master socket , then its an incoming connection
			if (FD_ISSET(master, &readfds))
			{
				console() << "Incoming connection\n" << std::endl;
				if ((new_socket = accept(master, (struct sockaddr *)&address, (int *)&addrlen))<0)
				{
					perror("accept");
					exit(EXIT_FAILURE);
				}

				//inform user of socket number - used in send and receive commands
				console() << "New Connection , socket fd is " << new_socket << " , ip is : " << inet_ntoa(address.sin_addr) << " , Port : " << ntohs(address.sin_port) << std::endl;

				//send new connection greeting message
				if (send(new_socket, &lightsON, 1, 0))
				{
					perror("send failed");
				}
				console() << "Welcome message sent successfully\n" << std::endl;

				//add new socket to array of sockets
				for (i = 0; i < max_clients; i++)
				{
					if (client_socket[i] == 0)
					{
						client_socket[i] = new_socket;
						console() << "Adding to list of sockets at index\n" << i << std::endl;

						//store the location of the new client in the list to the position variables
						if (strcmp(inet_ntoa(address.sin_addr), TLIP) == 0)
						{
							TLPos = i;
							console() << "This is Top Left Client\n" << std::endl;
						}
						else if (strcmp(inet_ntoa(address.sin_addr), TCIP) == 0)
						{
							TCPos = i;
							console() << "This is Top Center Client\n" << std::endl;
						}
						else if (strcmp(inet_ntoa(address.sin_addr), TRIP) == 0)
						{
							TRPos = i;
							console() << "This is Top Right Client\n" << std::endl;
						}
						else if (strcmp(inet_ntoa(address.sin_addr), MLIP) == 0)
						{
							MLPos = i;
							console() << "This is Middle Left Client\n" << std::endl;
						}
						else if (strcmp(inet_ntoa(address.sin_addr), MCIP) == 0)
						{
							MCPos = i;
							console() << "This is Middle Center Client\n" << std::endl;
						}
						else if (strcmp(inet_ntoa(address.sin_addr), MRIP) == 0)
						{
							MRPos = i;
							console() << "This is Middle Right Client\n" << std::endl;
						}
						else if (strcmp(inet_ntoa(address.sin_addr), BLIP) == 0)
						{
							BLPos = i;
							console() << "This is Bottom Left Client\n" << std::endl;
						}
						else if (strcmp(inet_ntoa(address.sin_addr), BRIP) == 0)
						{
							BRPos = i;
							console() << "This is Bottom Right Client\n" << std::endl;
						}

						else if (strcmp(inet_ntoa(address.sin_addr), ClientIP) == 0)
						{
							ClientPos = i;
							console() << "This is Streaming Client\n" << std::endl;
						}
						break;
					}
				}
			}

			//SENDING SIGNALS TO THE SPECIFIC SPARK CORES

			//receiving on and off from top right vibration actuator
			s = client_socket[TRPos];
			getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
			recvResult = recv(s, isVibrationOn, 1, 0);
			isVibrationOnInt = (int)isVibrationOn[0];
			//console() << " Received Number: " << isVibrationOnInt << std::endl;


			//STREAMING DEPTH BUFFER TO THE STREAMER CLIENT
			//Send 1 frame after every 5 frames to the streaming client. Can be changed in the setup function above

			//if(frameCounter < numberOfFramesSkipped)
			//{
			//	frameCounter++;
			//}
			//else
			//{
			//	frameCounter = 0;
			//	depthdata = (char*)sendDepthBuffer;
			//	s = client_socket[ClientPos];
			//	getpeername(s , (struct sockaddr*)&address , (int*)&addrlen);
			//	send( s , depthdata , mDepthH * mDepthW , 0 ); 
			//	//console() << "Bytes being sent: " << mDepthH * mDepthW << std::endl;
			//}

			//for top left
			if (closestDepthValue1 > 500 && closestDepthValue1 <= topDetectionThreshold  && isVibrationOnInt == 1 && isTLOn)
			{
				s = client_socket[TLPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &cdv1, 1, 0);
			}
			else
			{
				s = client_socket[TLPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &zero, 1, 0);
			}

			//for top center
			if (closestDepthValue7 > 500 && closestDepthValue7 <= topDetectionThreshold && isVibrationOnInt == 1 && isTCOn)
			{
				s = client_socket[TCPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &cdv7, 1, 0);
			}
			else
			{
				s = client_socket[TCPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &zero, 1, 0);
			}

			//for top right
			if (closestDepthValue2 > 500 && closestDepthValue2 <= topDetectionThreshold && isVibrationOnInt == 1 && isTROn)
			{
				s = client_socket[TRPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &cdv2, 1, 0);
			}
			else
			{
				s = client_socket[TRPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &zero, 1, 0);
			}

			//for middle left
			if (closestDepthValue3 > 500 && closestDepthValue3 <= middleDetectionThreshold && isVibrationOnInt == 1 && isMLOn)
			{
				s = client_socket[MLPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &cdv3, 1, 0);
			}
			else
			{
				s = client_socket[MLPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &zero, 1, 0);
			}

			//for middle right
			if (closestDepthValue4 > 500 && closestDepthValue4 <= middleDetectionThreshold && isVibrationOnInt == 1 && isMROn)
			{
				s = client_socket[MRPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &cdv4, 1, 0);
			}
			else
			{
				s = client_socket[MRPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &zero, 1, 0);
			}

			//for middle center
			if (closestDepthValue8 > 500 && closestDepthValue8 <= middleDetectionThreshold && isVibrationOnInt == 1 && isMCOn)
			{
				s = client_socket[MCPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &cdv8, 1, 0);
			}
			else
			{
				s = client_socket[MCPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &zero, 1, 0);
			}

			//for bottom left
			if (closestDepthValue5 > 500 && closestDepthValue5 <= bottomDetectionThreshold && isVibrationOnInt == 1 && isBLOn)
			{
				s = client_socket[BLPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &cdv5, 1, 0);
			}
			else
			{
				s = client_socket[BLPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &zero, 1, 0);
			}

			//for bottom right
			if (closestDepthValue6 > 500 && closestDepthValue6 <= bottomDetectionThreshold && isVibrationOnInt == 1 && isBROn)
			{
				s = client_socket[BRPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &cdv6, 1, 0);
			}
			else
			{
				s = client_socket[BRPos];
				getpeername(s, (struct sockaddr*)&address, (int*)&addrlen);
				send(s, &zero, 1, 0);
			}

			cDepthImage->ReleaseAccess(&cDepthData); //Don't forget to release the depth frame after update() completes as this will cause a memory leak and crash after a while
		}
		mSenseMgr->ReleaseFrame();
	}
	

	
//there should be an if statement before this to check for a successful grab.
	else
	{
		//If the grab fails close all the sockets and quit the application
		console() << "No depth frame to grab" << std::endl;
		closesocket(client_socket[TRPos]);
		closesocket(client_socket[TCPos]);
		closesocket(client_socket[TLPos]);
		closesocket(client_socket[MRPos]);
		closesocket(client_socket[MCPos]);
		closesocket(client_socket[MLPos]);
		closesocket(client_socket[BRPos]);
		closesocket(client_socket[BLPos]);
		WSACleanup();
		quit();
	}
}

void IRSAWApp::draw()
{
	gl::enableAlphaBlending();

	// clear out the window with black
	gl::clear( Color( 0, 0, 0 ) ); 

	if (isVisualizerVisible)
	{

		//Set the rectangles for drawing the overlay
		TLzone = Rectf(0, 0, WoR, HoT);
		TCzone = Rectf(WoR, 0, WoC, HoT);
		TRzone = Rectf(WoC, 0, WoL, HoT);
		MLzone = Rectf(0, HoT, WoR, HoM);
		MCzone = Rectf(WoR, HoT, WoC, HoM);
		MRzone = Rectf(WoC, HoT, WoL, HoM);
		BLzone = Rectf(0, HoM, WoBR, HoB);
		BRzone = Rectf(WoBR, HoM, WoBL, HoB);


		if (verticalConfig == true) // displaying depth buffer for vertical camera configuration
		{
			//Rotate the image by 90 degress. // It is a stack operation sequence
			gl::pushMatrices();
			gl::translate(360 / 2, 480 / 2);
			gl::rotate(90);
			gl::translate(-mDepthW / 2, -mDepthH / 2);
			gl::color(ColorA(1, 1, 1, 1));
			gl::draw(mDepthTex);
			gl::popMatrices();
		}
		else // displaying depth buffer for horizontal camera configuration
		{
			gl::color(ColorA(1, 1, 1, 1));
			gl::draw(mDepthTex);
		}


		// draw rectangles for sections

		// color change gradient from blue to red
		gl::color(ColorA(map(closestDepthValue1, topDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue1, topDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(TLzone);

		gl::color(ColorA(map(closestDepthValue7, topDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue7, topDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(TCzone);

		gl::color(ColorA(map(closestDepthValue2, topDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue2, topDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(TRzone);

		gl::color(ColorA(map(closestDepthValue3, middleDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue3, middleDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(MLzone);

		gl::color(ColorA(map(closestDepthValue8, middleDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue8, middleDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(MCzone);

		gl::color(ColorA(map(closestDepthValue4, middleDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue4, middleDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(MRzone);

		gl::color(ColorA(map(closestDepthValue5, bottomDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue5, bottomDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(BLzone);

		gl::color(ColorA(map(closestDepthValue6, bottomDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue6, bottomDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(BRzone);

		// draw stroked rectangles representing borders
		gl::color(ColorA(1.0f, 1.0f, 1.0f, 1.0f));
		gl::drawStrokedRect(TLzone);
		gl::drawStrokedRect(TCzone);
		gl::drawStrokedRect(TRzone);
		gl::drawStrokedRect(MLzone);
		gl::drawStrokedRect(MCzone);
		gl::drawStrokedRect(MRzone);
		gl::drawStrokedRect(BLzone);
		gl::drawStrokedRect(BRzone);
	}

	//Draw the params on the screen
	mParams->draw();

	gl::disableAlphaBlending();
}

CINDER_APP_NATIVE( IRSAWApp, RendererGl )
