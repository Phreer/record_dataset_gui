#include "realsense_capture.h"

/*
 * utility function to transfer realsense format image to opencv format Mat
 * params:
 *  pxcImage: pointer to PXCImage instance
 *  format: can be NULL or predefined PIXEL_FORMAT_... format
 * return:
 *  a desired Mat
 */
cv::Mat PXCImage2CVMat(PXCImage *pxcImage, PXCImage::PixelFormat format){
    PXCImage::ImageData data;
    pxcImage->AcquireAccess(PXCImage::ACCESS_READ, format, &data);
    int width = pxcImage->QueryInfo().width;
    int height = pxcImage->QueryInfo().height;

    if(!format)
        format = pxcImage->QueryInfo().format;
    int type;
    switch(format){
    case PXCImage::PIXEL_FORMAT_Y8:
        type = CV_8UC1;
        break;
    case PXCImage::PIXEL_FORMAT_RGB24:
        type = CV_8UC3;
        break;
    case PXCImage::PIXEL_FORMAT_DEPTH_F32:
        type = CV_32FC1;
        break;
    }
    cv::Mat ocvImage = cv::Mat(cv::Size(width, height), type, data.planes[0]);
    pxcImage->ReleaseAccess(&data);
    return ocvImage;
}

int write3DFloat32(PXCPoint3DF32 point, FILE *file){
    return fprintf_s(file, "(%f, %f, %f)", double(point.x), double(point.y), double(point.z));
}


realsense_capture::realsense_capture(const char filename_color[], const char filename_depth[]){
    pxcStatus status;
    end = false;
    pCnt = cCnt = 0;
    time(&pTime);
    file = fopen("joints_data.txt", "w");

    this->writerColorRe.open(filename_color, CV_FOURCC('D','I','V','X'), frameRate, size, true);
    this->writerDepthRe.open(filename_depth, CV_FOURCC('D','I','V','X'), frameRate, size, false);
    this->pxcSenseManager = PXCSenseManager::CreateInstance();

    // enable hand tracking
    status = this->pxcSenseManager->EnableHand();
    if(status==pxcStatus::PXC_STATUS_NO_ERROR) {
        printf("Enable realsense hand.\n");
    }else{
        printf("Unable realsense hand!\n");
    }

    status = pxcSenseManager->EnableStream(PXCCapture::STREAM_TYPE_COLOR, \
                                  size.width, \
                                  size.height, \
                                  frameRate);
                                  //PXCCapture::Device::STREAM_OPTION_STRONG_STREAM_SYNC);
    if(status==pxcStatus::PXC_STATUS_NO_ERROR) {
        printf("Enable realsense color stream successfully.\n");
    }else{
        printf("Unable realsense color stream!\n");
    }

    status = pxcSenseManager->EnableStream(PXCCapture::STREAM_TYPE_DEPTH, \
                                  size.width, \
                                  size.height, \
                                  frameRate);
                                  //PXCCapture::Device::STREAM_OPTION_STRONG_STREAM_SYNC);

    if(status==pxcStatus::PXC_STATUS_NO_ERROR) {
        printf("Enable realsense depth stream successfully.\n");
    }else{
        printf("Unable realsense depth stream!\n");
    }

    PXCHandModule *pxcHandModule = pxcSenseManager->QueryHand();
    PXCHandConfiguration* pxcHandConfig = pxcHandModule->CreateActiveConfiguration();
    pxcHandConfig->EnableTrackedJoints(true);
    pxcHandConfig->SetTrackingMode(PXCHandData::TRACKING_MODE_FULL_HAND);
    pxcHandConfig->ApplyChanges();

    this->pxcHandData = pxcHandModule->CreateOutput();

    if(pxcSenseManager->Init() == pxcStatus::PXC_STATUS_NO_ERROR){
        printf("startRealsense: Init successfully.\n");
    }else{
        printf("startRealsense: Init failed!\n");
        end = true;
    }

    //initialize Mat to store image from realsense
    frameColor = cv::Mat::zeros(size, CV_8UC3);
    frameDepth = cv::Mat::zeros(size, CV_8UC1);
}

void realsense_capture::stop(){
    end = true;
}

void realsense_capture::run(){
    // TODO: we should wait for realsense initialized completely
    // so that we can run this
    for(;;){
        if(!end){
            //QuerySample function will NULL untill all frames are available
            //unless you set its param ifall false
            if(pxcSenseManager->AcquireFrame(true)<pxcStatus::PXC_STATUS_NO_ERROR) break;
            PXCCapture::Sample *sample = pxcSenseManager->QuerySample();
            pxcHandData->Update();
            PXCHandData::IHand *ihand = 0;
            PXCHandData::JointData jointData[JOINT_TYPE_NUM];

            // Now only one hand is supoorted
            pxcHandData->QueryHandData(PXCHandData::ACCESS_ORDER_NEAR_TO_FAR, 0, ihand);
//            if(ihand->HasTrackedJoints()) {
//                for(int i=0;i<JOINT_TYPE_NUM;i++){
//                    ihand->QueryTrackedJoint(i, jointData[i]);
//                    write3DFloat32(jointData[i].positionWorld, file);
//                    fprintf(file, "\n");
//                }

//            }

            if(sample && !sample->IsEmpty()){
                frameColor = PXCImage2CVMat(sample->color, PXCImage::PIXEL_FORMAT_RGB24);
                PXCImage2CVMat(sample->depth, PXCImage::PIXEL_FORMAT_DEPTH_F32).convertTo(frameDepth, CV_8UC1);

                writerColorRe << frameColor;
                writerDepthRe << frameDepth;

                QImage image((const uchar*)(frameDepth.data), \
                             frameDepth.cols, \
                             frameDepth.rows, \
                             QImage::Format_Grayscale8);
                emit imageReady(image);
                pxcSenseManager->ReleaseFrame();

                cCnt ++;
                if((cCnt-pCnt)==200){
                    time(&cTime);
                    double fps = (cCnt-pCnt)/(cTime-pTime);
                    printf("Realsense capture fps: %f\n", fps);
                    pCnt = cCnt;
                    pTime = cTime;
                }

            }
        }else{
            if(pxcSenseManager->IsConnected()) pxcSenseManager->Release();
            if(writerColorRe.isOpened())writerColorRe.release();
            if(writerDepthRe.isOpened())writerDepthRe.release();
            fclose(file);
            printf("realsense_thread terminated.\n");
            return;
        }
    }
}
