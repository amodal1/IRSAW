#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/params/Params.h"
#include "cinder/ip/Resize.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/TextureFont.h"
#include "cinder/ImageIo.h"
#include "cinder/Surface.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"
#include "cinder/CinderMath.h"

#include <functional>

//OpenCV
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <stdlib.h>
#include <sstream>
#include <iomanip>
#include <thread>
#include <algorithm>

// for Bluetooth
#include<stdio.h>
//Bluetooth include (bluez?)


//realsense 
#include <librealsense/rs.hpp>

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace cv;


class IRSAWApp : public App {
  public:

	//variables used to convert the depth buffer for visualizations
	Channel16u		  mDepthChannel;
	gl::Texture2dRef  mDepthTex;
	int			mDepthW, mDepthH; // These variables store the height and depth of the raw depth buffer

	//librealsense
	rs::context ctx;
	rs::device * dev;

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

	//Pulsing and gradual toggles
	bool isPulsingOn;
	bool isPulsingOnSent;
	//int pulsingState, gradualState;
	char pulsingOn, gradualOn;

	//Variables for receiving data
	char whichNumber[1];
	int recvResult;
	int clientNumber;

	//variable for toggling rendering of visualizer.
	bool isVisualizerVisible;

	//these are parameters to set the distance of detection. The distance can be specified millimeters
	int topDetectionThreshold;
	int middleDetectionThreshold;
	int bottomDetectionThreshold;


	// variables for initial Server message to clients on connection
	char lightsON;

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


	//For Cinder Params
	params::InterfaceGlRef	mParams;

	void setup() override;
	void update() override;
	void draw() override;
	void shutdown();
	void toggleTopMotors();
	void toggleMiddleMotors();
	void toggleBottomMotors();
	void toggleCenterMotors();
	void toggleAllMotors();
	void toggleVerticalHorizontal();
	void keyDown(KeyEvent event);

	//map and clampToByte function map the depth values from 0 to 256 and convert that data into byte
	float map(float value, float inputMin, float inputMax, float outputMin, float outputMax);
	uint8_t clampToByte(float value, float min, float max);
};

void IRSAWApp::keyDown(KeyEvent event)
{
	// Pressing q will quit the program
	if ((event.getChar() == 'q') || (event.getChar() == 'Q'))
	{
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
	if ((event.getChar() == 'T') || (event.getChar() == 't'))
	{
		toggleTopMotors();
	}

	//pressing b will toggle the bottom row of motors
	if ((event.getChar() == 'B') || (event.getChar() == 'b'))
	{
		toggleBottomMotors();
	}

	//pressing m will toggle the middle row of motors
	if ((event.getChar() == 'M') || (event.getChar() == 'm'))
	{
		toggleMiddleMotors();
	}

	//pressing c will toggle the center column of motors
	if ((event.getChar() == 'C') || (event.getChar() == 'c'))
	{
		toggleCenterMotors();
	}

	//pressing o will toggle all the motors
	if ((event.getChar() == 'o') || (event.getChar() == 'O'))
	{
		toggleAllMotors();
	}
}

void IRSAWApp::shutdown()
{
//destroy camera context here
	quit();
}

//toggles Vertical camera orienation vs horizontal camera orientation
void IRSAWApp::toggleVerticalHorizontal()
{
	if (verticalConfig == true)
	{
		verticalConfig = false;
		HoT = mDepthH / 3;
		HoM = mDepthH * 2 / 3;
		HoB = mDepthH;
		WoR = mDepthW / 3;
		WoC = mDepthW * 2 / 3;
		WoL = mDepthW;
		WoBR = mDepthW / 2;
		WoBL = mDepthW;
	}
	else
	{
		verticalConfig = true;
		HoT = mDepthW / 3;
		HoM = mDepthW * 2 / 3;
		HoB = mDepthW;
		WoR = mDepthH / 3;
		WoC = mDepthH * 2 / 3;
		WoL = mDepthH;
		WoBR = mDepthH / 2;
		WoBL = mDepthH;
	}
}

// Toggles all the motors on/off
void IRSAWApp::toggleAllMotors()
{
	if (areAllMotorsOn == true)
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
	if (isCenterOn == true)
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
	if (isTopOn == true)
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
	if (isMiddleOn == true)
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
	if (isBottomOn == true)
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

	//values for setting type of vibration
	isPulsingOn = false;
	isPulsingOnSent = false;
	pulsingOn = (char)255;
	gradualOn = (char)254;

	isVisualizerVisible = false;

	whichNumber[0] = 100;
	clientNumber = 100;
	recvResult = 0;

	lightsON = 1;

	//add the params controls to be displayed in the window
	mParams = params::InterfaceGl::create(getWindow(), "App parameters", toPixels(ci::vec2(270, 450)));

	mParams->addParam("Top Threshold in mm", &topDetectionThreshold);
	mParams->addParam("Middle Threshold in mm", &middleDetectionThreshold);
	mParams->addParam("Bottom Threshold in mm", &bottomDetectionThreshold);
	mParams->addSeparator();
	mParams->addParam("Height of Top", &HoT);
	mParams->addParam("Height of Middle", &HoM);
	mParams->addParam("Width of Center", &WoC);
	mParams->addSeparator();
	mParams->addParam("Toggle Visualizer Rendering", &isVisualizerVisible);
	mParams->addSeparator();
	mParams->addButton("Toggle Vertical / Horizontal", std::bind(&IRSAWApp::toggleVerticalHorizontal, this));
	mParams->addSeparator();
	mParams->addButton("Toggle All Motors", std::bind(&IRSAWApp::toggleAllMotors, this));
	mParams->addSeparator();
	mParams->addButton("Toggle Top Motors", std::bind(&IRSAWApp::toggleTopMotors, this));
	mParams->addButton("Toggle Middle Motors", std::bind(&IRSAWApp::toggleMiddleMotors, this));
	mParams->addButton("Toggle Bottom Motors", std::bind(&IRSAWApp::toggleBottomMotors, this));
	mParams->addButton("Toggle Center Motors", std::bind(&IRSAWApp::toggleCenterMotors, this));
	mParams->addSeparator();
	mParams->addParam("Top Right Curie Connected", &isTROn);
	mParams->addParam("Center Curie Connected", &isTCOn);
	mParams->addParam("Top Left Curie Connected", &isTLOn);
	mParams->addSeparator();
	mParams->addParam("Middle Right Curie Connected", &isMROn);
	mParams->addParam("Middle Center Curie Connected", &isMCOn);
	mParams->addParam("Middle Left Curie Connected", &isMLOn);
	mParams->addSeparator();
	mParams->addParam("Bottom Right Curie Connected", &isBROn);
	mParams->addParam("Bottom Left Curie Connected", &isBLOn);



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


	//Initial detection Threshold is 2000mm
	topDetectionThreshold = 2000;
	middleDetectionThreshold = 2000;
	bottomDetectionThreshold = 2000;

	//Initialize Bluetooth

	//Check for RealSense camera and initialize
	if (ctx.get_device_count() == 0) throw std::runtime_error("No device detected. Is it plugged in?");
		dev = ctx.get_device(0);

	 dev->enable_stream(rs::stream::depth, 480, 360, rs::format::z16, 60);
	 dev->start();
	 mDepthW = dev->get_stream_intrinsics(rs::stream::depth).width;
	 mDepthH = dev->get_stream_intrinsics(rs::stream::depth).height;
	
	 // assign initialization values to the parameters setting the bounds of the sections.
	 HoT = mDepthW / 3;
	 HoM = mDepthW * 2 / 3;
	 HoB = mDepthW;
	 WoR = mDepthH / 3;
	 WoC = mDepthH * 2 / 3;
	 WoL = mDepthH;
	 WoBR = mDepthH / 2;
	 WoBL = mDepthH;

	 mDepthChannel = Channel16u(mDepthW, mDepthH);

	 setWindowSize(mDepthH * 2, mDepthW);
		
}


void IRSAWApp::update()
{
	dev->wait_for_frames();
	depth = (uint16_t *)(dev->get_frame_data(rs::stream::depth)); // dev ka locha hai

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
					it.v() = (int)math<float>::clamp(lmap<float>((float)val, 500, 4000, 65535, 0), 0, 65535);
				q++;
			}
		}
//		gl::Texture2dRef mDepthTexFlip = gl::Texture2d::create(mDepthChannel); /// see what has changed in new cinder
		mDepthTex = gl::Texture2d::create(mDepthChannel);
//		cv::flip(mDepthTexFlip, mDepthTex, 1);

		
	}

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
	src = Mat(cvSize(mDepthW, mDepthH), CV_16UC1, depth);
	//cv::flip(src, src, 1);

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
				currentDepthValue6 = depth[i + (j*mDepthW)];
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
				currentDepthValue7 = depth[i + (j*mDepthW)];
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
				currentDepthValue5 = depth[i + (j*mDepthW)];
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
				currentDepthValue4 = depth[i + (j*mDepthW)];
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
				currentDepthValue8 = depth[i + (j*mDepthW)];
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
				currentDepthValue3 = depth[i + (j*mDepthW)];
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
				currentDepthValue2 = depth[i + (j*mDepthW)];
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
				currentDepthValue1 = depth[i + (j*mDepthW)];
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
				currentDepthValue6 = depth[i + (j*mDepthW)];
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
				currentDepthValue7 = depth[i + (j*mDepthW)];
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
				currentDepthValue5 = depth[i + (j*mDepthW)];
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
				currentDepthValue4 = depth[i + (j*mDepthW)];
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
				currentDepthValue8 = depth[i + (j*mDepthW)];
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
				currentDepthValue3 = depth[i + (j*mDepthW)];
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
				currentDepthValue2 = depth[i + (j*mDepthW)];
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
				currentDepthValue1 = depth[i + (j*mDepthW)];
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

	clampedPixel1 = clampToByte(map(closestDepthValue1, topDetectionThreshold, 501, 0, 253), 0, 253);
	//std::cout << "clampedPixel1: " << static_cast<unsigned>(clampedPixel1) << std::endl; // pretty print 
	cdv1 = (char)clampedPixel1;

	clampedPixel2 = clampToByte(map(closestDepthValue2, topDetectionThreshold, 501, 0, 253), 0, 253);
	//std::cout << "clampedPixel2: " << static_cast<unsigned>(clampedPixel2) << std::endl; // pretty print 
	cdv2 = (char)clampedPixel2;

	clampedPixel3 = clampToByte(map(closestDepthValue3, middleDetectionThreshold, 501, 0, 253), 0, 253);
	//std::cout << "clampedPixel3: " << static_cast<unsigned>(clampedPixel3) << std::endl; // pretty print 
	cdv3 = (char)clampedPixel3;

	clampedPixel4 = clampToByte(map(closestDepthValue4, middleDetectionThreshold, 501, 0, 253), 0, 253);
	//std::cout << "clampedPixel4: " << static_cast<unsigned>(clampedPixel4) << std::endl; // pretty print 
	cdv4 = (char)clampedPixel4;

	clampedPixel5 = clampToByte(map(closestDepthValue5, bottomDetectionThreshold, 501, 0, 253), 0, 253);
	//std::cout << "clampedPixel5: " << static_cast<unsigned>(clampedPixel5) << std::endl; // pretty print 
	cdv5 = (char)clampedPixel5;

	clampedPixel6 = clampToByte(map(closestDepthValue6, bottomDetectionThreshold, 501, 0, 253), 0, 253);
	//std::cout << "clampedPixel6: " << static_cast<unsigned>(clampedPixel6) << std::endl; // pretty print 
	cdv6 = (char)clampedPixel6;

	clampedPixel7 = clampToByte(map(closestDepthValue7, topDetectionThreshold, 501, 0, 253), 0, 253);
	//std::cout << "clampedPixel7: " << static_cast<unsigned>(clampedPixel7) << std::endl; // pretty print 
	cdv7 = (char)clampedPixel7;

	clampedPixel8 = clampToByte(map(closestDepthValue8, middleDetectionThreshold, 501, 0, 253), 0, 253);
	//std::cout << "clampedPixel8: " << static_cast<unsigned>(clampedPixel8) << std::endl; // pretty print 
	cdv8 = (char)clampedPixel8;


}

void IRSAWApp::draw()
{
	gl::enableAlphaBlending();

	// clear out the window with black
	gl::clear(Color(0, 0, 0));

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
			gl::translate(mDepthH / 2, mDepthW / 2);
			gl::rotate(M_PI*0.5); //in radians
			gl::translate(-mDepthW / 2, -mDepthH / 2);

			//Mirror the image from the camera for display
			gl::translate(0, mDepthH/2);
			gl::scale(1,-1,1);
			gl::color(ColorA(1, 1, 1, 1));
			gl::translate(0, -mDepthH/2);
			gl::draw(mDepthTex);
			gl::popMatrices();


		}
		else // displaying depth buffer for horizontal camera configuration
		{
			//Mirror the image from the camera for display
			gl::pushMatrices();
			gl::translate(0, mDepthH/2);
			gl::scale(1,-1, 1);
			gl::translate(0, -mDepthH/2);

			gl::color(ColorA(1, 1, 1, 1));
			gl::draw(mDepthTex);
			gl::popMatrices();
		}


		// draw rectangles for section

		// color change gradient from blue to red
		gl::color(ColorA(map(closestDepthValue2, topDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue2, topDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(TLzone);

		gl::color(ColorA(map(closestDepthValue7, topDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue7, topDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(TCzone);

		gl::color(ColorA(map(closestDepthValue1, topDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue1, topDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(TRzone);

		gl::color(ColorA(map(closestDepthValue4, middleDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue4, middleDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(MLzone);

		gl::color(ColorA(map(closestDepthValue8, middleDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue8, middleDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(MCzone);

		gl::color(ColorA(map(closestDepthValue3, middleDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue3, middleDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(MRzone);

		gl::color(ColorA(map(closestDepthValue6, bottomDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue6, bottomDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(BLzone);

		gl::color(ColorA(map(closestDepthValue5, bottomDetectionThreshold, 501, 0, 1), 0, 1.0f - map(closestDepthValue5, bottomDetectionThreshold, 501, 0, 1), 0.5f));
		gl::drawSolidRect(BRzone);

		// draw stroked rectangles representing borders
		gl::color(ColorA(1.0f, 1.0f, 1.0f, 1.0f));
		gl::drawStrokedRect(TRzone);
		gl::drawStrokedRect(TCzone);
		gl::drawStrokedRect(TLzone);
		gl::drawStrokedRect(MRzone);
		gl::drawStrokedRect(MCzone);
		gl::drawStrokedRect(MLzone);
		gl::drawStrokedRect(BRzone);
		gl::drawStrokedRect(BLzone);


	}
	
	//Draw the params on the screen
	mParams->draw();


	gl::disableAlphaBlending();
}

CINDER_APP( IRSAWApp, RendererGl )
