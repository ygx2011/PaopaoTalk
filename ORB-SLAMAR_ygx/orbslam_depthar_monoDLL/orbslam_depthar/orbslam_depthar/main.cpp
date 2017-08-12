#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <iostream>

#include<iostream>
#include<fstream>
#include<boost/thread.hpp>

#include "Tracking.h"
#include "Map.h"
#include "LocalMapping.h"
#include "LoopClosing.h"
#include "KeyFrameDatabase.h"
#include "ORBVocabulary.h"

#include "Converter.h"

#include "SaveLoadWorld.h"
#include <opencv2\opencv.hpp>

#include "error.h"
using namespace std;

//extern "C" __declspec(dllexport) ERROR_CODE Initialize(int);
//extern "C" __declspec(dllexport) ERROR_CODE process_Image(uchar [],float [],float []);
//extern "C" __declspec(dllexport) void Release();

static bool isStop = FALSE;

cv::VideoCapture capCam;

ORB_SLAM::Tracking *Tracker;
ORB_SLAM::LocalMapping *LocalMapper;
ORB_SLAM::LoopClosing *LoopCloser;

ORB_SLAM::ORBVocabulary* Vocabulary;

ORB_SLAM::KeyFrameDatabase* Database;
 //Create the map
ORB_SLAM::Map* World;

ERROR_CODE Initialize(int deviceID)
{
	capCam.open(deviceID);
	if (!capCam.isOpened())
	{
		return CAMERA_OPEN_FAILED;
	}
	capCam.set(CV_CAP_PROP_FRAME_WIDTH, 640);
	capCam.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
	capCam.set(CV_CAP_PROP_FRAME_COUNT, 30);
	
	string strSettingsFile = "./Settings.yaml";
    cv::FileStorage fsSettings(strSettingsFile.c_str(), cv::FileStorage::READ);
    if(!fsSettings.isOpened())
    {
        return WRONG_SETTING_FILE_PATH;
    }

    //Load ORB Vocabulary
	string strVocFile = "./ORBvoc.bin";
	// cout << "Loading Vocabulary!" << endl;
	
    Vocabulary = new ORB_SLAM::ORBVocabulary();
	Vocabulary->loadFromBinaryFile(strVocFile);

   // cout << "Vocabulary loaded!" << endl << endl;

    //Create KeyFrame Database
    Database = new ORB_SLAM::KeyFrameDatabase(*Vocabulary);

    //Create the map
    World = new ORB_SLAM::Map();

	// ------------------------------------
    KeyFrame *tpLastKF;
    bool bReadOK = LoadWroldFromFile(Database, World, Vocabulary, tpLastKF);
    if(bReadOK)
    {
        //cout<<"load world file successfully."<<endl;

        vector<KeyFrame*> vpAllKFs=World->GetAllKeyFrames();
        for(vector<KeyFrame*>::iterator vit=vpAllKFs.begin(),vend=vpAllKFs.end();vit!=vend;vit++)
        {
            KeyFrame* pKF = *vit;
            if(!pKF)
			{
                //cout<<"pKF==NULL"<<endl;
			}
            KeyFrame* pKFpar = pKF->GetParent();
            set<KeyFrame*> spKFch = pKF->GetChilds();
            //cout<<"pKF-"<<pKF->mnId;
            if(pKF->mnId!=0)
			{
                //cout<<", parent-"<<pKFpar->mnId;
			}
            if(spKFch.empty())
            {
                cout<<endl;
                continue;
            }
            //cout<<", children-";
            for(set<KeyFrame*>::iterator sit=spKFch.begin(),send=spKFch.end();sit!=send;sit++)
            {
                KeyFrame* pKFc = *sit;
                if(!pKFc)
				{
                    //cout<<"pKFc==NULL"<<endl;
				}
                //cout<<pKFc->mnId<<", ";
            }
            //cout<<endl;
        }
    }
    else
    {
        //cout<<"load world file failed."<<endl;
        // operations
        World->clear();
        Database->clear();
    }
    // ------------------------------------

    //Initialize the Tracking Thread and launch
    Tracker = new ORB_SLAM::Tracking(Vocabulary,/* &FramePub, &MapPub,*/ World, strSettingsFile/*, capCam*/);
    boost::thread trackingThread(&ORB_SLAM::Tracking::Run,Tracker);

    Tracker->SetKeyFrameDatabase(Database);

	// ------------------------------------
    if(bReadOK)
    {
        Tracker->mState = Tracking::LOST;
        Tracker->mLastProcessedState = Tracking::LOST;
        Tracker->SetLastKeyframe(tpLastKF);
        Tracker->SetLastFrameId(Frame::nNextId-1);
    }
    // ------------------------------------

    //Initialize the Local Mapping Thread and launch
    LocalMapper = new ORB_SLAM::LocalMapping(World);
    boost::thread localMappingThread(&ORB_SLAM::LocalMapping::Run,LocalMapper);

    //Initialize the Loop Closing Thread and launch
    LoopCloser = new ORB_SLAM::LoopClosing(World, Database, Vocabulary);
    boost::thread loopClosingThread(&ORB_SLAM::LoopClosing::Run, LoopCloser);

    //Set pointers between threads
    Tracker->SetLocalMapper(LocalMapper);
    Tracker->SetLoopClosing(LoopCloser);

    LocalMapper->SetTracker(Tracker);
    LocalMapper->SetLoopCloser(LoopCloser);

    LoopCloser->SetTracker(Tracker);
    LoopCloser->SetLocalMapper(LocalMapper);

	return WORKING;
}



ERROR_CODE process_Image(uchar imgData[],float R[],float T[])
{
		cv::Mat im;
		capCam >> im;

		cv::Mat im_ygx = im.clone();
		cv::cvtColor(im_ygx, im_ygx, CV_BGR2RGB);

		memcpy(imgData,im_ygx.data,640*480*3); 
		
		Tracker->GrabImage(im);
		cv::imshow("orbslam",im);
		cv::waitKey(5);

		switch (Tracker->mState)
		{
		case -1:
			{
				return SYSTEM_NOT_READY;	
			}
		case 0:
			{
				return SYSTEM_NOT_READY;	
			}
		case 1:
			{
				return NOT_INITIALIZED;
			}
		case 2:
			{
				return INITIALIZING;
			}
		case 3:
			{
				cv::Mat R_Temp = Tracker->GetPose_R();
				memcpy(R,R_Temp.data,R_Temp.cols*R_Temp.rows*sizeof(float));
				
				cv::Mat T_Temp = Tracker->GetPose_T();				
				memcpy(T,T_Temp.data,T_Temp.cols*T_Temp.rows*sizeof(float));
				cout<<"working"<<endl;
				
				return WORKING;
			}
		case 4:
			{
				cout<<"lost"<<endl;
				return TRACKING_LOST;
			}
		default:
			{
				return UNSPECIFIC_ERROR;
			}
		}
}

void Release()
{
	SaveWorldToFile(*World,*Database);

	delete LocalMapper;
	delete LoopCloser;

	delete Tracker;

	delete Vocabulary;
	delete Database;
	delete World;

	capCam.release();

	exit(0);
}


int main()
{
	uchar imgData[640*480*3] = {0};
	float R[9] = {0};
	float T[3] = {0};
	
	Initialize(0);
	cout<<"init success"<<endl;
	while(true)
	{
		process_Image(imgData,R,T);
	}
	Release();
	return 0;
}