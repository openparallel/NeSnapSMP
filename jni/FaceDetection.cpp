//
//  FaceDetection.cpp
//  FaceIt
//
//  Created by Beau Johnston on 1/06/12.
//  Copyright (c) 2012 OpenParallel.com all rights reserved.
//

#include "FaceDetection.h"

/*
 * Private variable to track the number of features
 */
int numberOfFeatures = 0;
char* pwd = (char*)"";
#ifdef ANDROID
//this is only used as a flag for the android device to wait for the NDK
bool processingFinished = false;
#endif


/*
 * Private Utility functions
 */

char* stringCat(char*s, char*s1){
    char* target = new char[strlen(s) + strlen(s1) + 1];
    strcpy(target, s);
    strcat(target, s1);
    return target;
}

void Log(char* message, bool errorFlag){
    //append a newline to the end of the log string
    message = stringCat(message, (char*)"\n");
    
#ifdef ANDROID
    //android log
    if (errorFlag) {
        LOGE(message);
    }
    else{
        LOGV(message);
    }
#endif
#ifndef ANDROID
    //regular log
    if (errorFlag) {
        printf("%s", message);
        exit(EXIT_FAILURE);
    }
    else {
        printf("%s", message);
    }
#endif
    return;
}

void applySepiaTone(IplImage* target){
    for (int ix=0; ix<target->width; ix++) {
        for (int iy=0; iy<target->height; iy++) {
            
            //extract each pixel
            int r = cvGet2D(target, iy, ix).val[2];
            int g = cvGet2D(target, iy, ix).val[1];
            int b = cvGet2D(target, iy, ix).val[0];
            
            //generate a grayscale pixel
            int p = round(((r+g+b)/3));
            
            //to generate sepia tone colouration, use the colourspace
            //rgb (+40,+20,-20)
            //CvScalar expects bgr colour so:
            CvScalar bgr = cvScalar(p-20, p+20, p+40);
            
            cvSet2D(target, iy, ix, bgr);
        }
    }
}

void overlayImage(IplImage* target, IplImage* source, int x, int y) {
    
    for (int ix=0; ix<source->width; ix++) {
        for (int iy=0; iy<source->height; iy++) {
            int r = cvGet2D(source, iy, ix).val[2];
            int g = cvGet2D(source, iy, ix).val[1];
            int b = cvGet2D(source, iy, ix).val[0];
            CvScalar bgr = cvScalar(b, g, r);
            cvSet2D(target, iy+y, ix+x, bgr);
        }
    }
}


/*
 * End of Private utility functions
 */








/*
 * Public feature detection functions
 */


IplImage* drawRectangleOnImage(CvRect featureRect, IplImage*inputImage){
    
    cvRectangle(inputImage, cvPoint(featureRect.x, featureRect.y), cvPoint(featureRect.x + featureRect.width, featureRect.y + featureRect.height), cvScalar(0, 255, 255, 255), 3, 1, 0);
    
    return inputImage;
}

IplImage* drawRectangleOnImageWithColour(CvRect featureRect, IplImage*inputImage,CvScalar colour){
    
    cvRectangle(inputImage, cvPoint(featureRect.x, featureRect.y), cvPoint(featureRect.x + featureRect.width, featureRect.y + featureRect.height), colour, 3, 1, 0);
    
    return inputImage;
}

IplImage* drawRectangleOnImageWithColourAndFilled(CvRect featureRect, IplImage*inputImage,CvScalar colour){
    
    cvRectangle(inputImage, cvPoint(featureRect.x, featureRect.y), cvPoint(featureRect.x + featureRect.width, featureRect.y + featureRect.height), colour, CV_FILLED, 1, 0);
    
    return inputImage;
}


    /* initialize random seed: */
    //srand ( time(NULL) );
    
    /* generate number between 1 and 50 */
    //int moeNo = rand() % 50 + 1;
    
    //char moeFileNameBuffer[50];
    //sprintf(moeFileNameBuffer,"moeResources/moe%i.png", moeNo);
    
    //append the pwd (mostly for android's sake ;) )
    //char* moeFileName = stringCat(pwd, moeFileNameBuffer);
    
    //Log(moeFileName, false);
    

/*
 * End of public feature detection functions
 */

#ifndef ANDROID
void setWorkingDir(char* wd){
    pwd = wd;
}
#endif

/*
 * Now for android stuff
 */
#ifdef ANDROID
JNIEXPORT
jboolean
JNICALL
Java_org_openparallel_imagethresh_ImageThreshActivity_doChainOfImageProcessingOperations(JNIEnv* env,
                                                                                        jobject thiz){
    processingFinished = false;
    applySepiaTone(m_sourceImage);
    processingFinished = true;
    return true;
    
}

JNIEXPORT
void 
JNICALL
Java_org_openparallel_imagethresh_ImageThreshActivity_setWorkingDir(JNIEnv* env, jobject thiz, jstring javaString){
    
    const char *nativeString = env->GetStringUTFChars(javaString, 0);
    
    pwd = (char*)nativeString;
    
    //env->ReleaseStringUTFChars(javaString, nativeString);
    
    return;
}

JNIEXPORT
void
JNICALL
Java_org_openparallel_imagethresh_ImageThreshActivity_doGrayscaleTransform(JNIEnv* env,
                                                                           jobject thiz){
    
    IplImage* r = cvCreateImage( cvGetSize(m_sourceImage), IPL_DEPTH_8U, 1 ); 
    IplImage* g = cvCreateImage( cvGetSize(m_sourceImage), IPL_DEPTH_8U, 1 ); 
    IplImage* b = cvCreateImage( cvGetSize(m_sourceImage), IPL_DEPTH_8U, 1 );
    
    
    // Split image onto the color planes. 
    cvSplit( m_sourceImage, b, g, r, NULL );
    
    // Temporary storage
    IplImage* s = cvCreateImage(cvGetSize(m_sourceImage), IPL_DEPTH_8U, 1 );
    
    // Release the source image
    if (m_sourceImage) {
		cvReleaseImage(&m_sourceImage);
		m_sourceImage = 0;
	}
    
    // Add equally weighted rgb values. 
    cvAddWeighted( r, 1./3., g, 1./3., 0.0, s ); 
    cvAddWeighted( s, 2./3., b, 1./3., 0.0, s );
    
    // Merge the 4 channel to an BGRA image
    m_sourceImage = cvCreateImage(cvGetSize(s), 8, 3);
    
    cvMerge(s, s, s, NULL, m_sourceImage);
    
    cvReleaseImage(&r); 
    cvReleaseImage(&g); 
    cvReleaseImage(&b); 
    cvReleaseImage(&s);
    
    return;
    
}

// Given an integer array of image data, load an IplImage.
// It is the responsibility of the caller to release the IplImage.
IplImage* getIplImageFromIntArray(JNIEnv* env, jintArray array_data, 
								  jint width, jint height) {
	// Load Image
	int *pixels = env->GetIntArrayElements(array_data, 0);
	if (pixels == 0) {
		LOGE("Error getting int array of pixels.");
		return 0;
	}
	
	IplImage *image = loadPixels(pixels, width, height);
	env->ReleaseIntArrayElements(array_data, pixels, 0);
	if(image == 0) {
		LOGE("Error loading pixel array.");
		return 0;
	}
	
	return image;
}


// Generate and return a boolean array from the source image.
// Return 0 if a failure occurs or if the source image is undefined.
JNIEXPORT
jbyteArray
JNICALL
Java_org_openparallel_imagethresh_ImageThreshActivity_getSourceImage(JNIEnv* env,
                                                                     jobject thiz)
{
    
    
	if (m_sourceImage == 0) {
		LOGE("Error source image was not set.");
		return 0;
	}
	
	CvMat stub;
    CvMat *mat_image = cvGetMat(m_sourceImage, &stub);
    int channels = CV_MAT_CN( mat_image->type );
    int ipl_depth = cvCvToIplDepth(mat_image->type);
    
	WLNonFileByteStream *strm = new WLNonFileByteStream();
    loadImageBytes(mat_image->data.ptr, mat_image->step, mat_image->width,
                   mat_image->height, ipl_depth, channels, strm);
	
	int imageSize = strm->GetSize();
    
    //if you wanted to return an array of 1's and 0's ()
    /*
     jbooleanArray res_array = env->NewBooleanArray(imageSize);
     if (res_array == 0) {
     LOGE("Unable to allocate a new boolean array for the source image.");
     return 0;
     }
     env->SetBooleanArrayRegion(res_array, 0, imageSize, (jboolean*)strm->GetByte());
     */
    
    jbyteArray res_array = env->NewByteArray(imageSize);
    if (res_array == 0) {
        LOGE("Unable to allocate a new byte array for the source image.");
        return 0;
    }
    env->SetByteArrayRegion(res_array, 0, imageSize, (jbyte*)strm->GetByte());
    
	strm->Close();
	//SAFE_DELETE(strm);
	
	return res_array;
}

// Set the source image and return true if successful or false otherwise.
JNIEXPORT
jboolean
JNICALL
Java_org_openparallel_imagethresh_ImageThreshActivity_setSourceImage(JNIEnv* env,
                                                                     jobject thiz,
                                                                     jintArray photo_data,
                                                                     jint width,
                                                                     jint height)
{	
	// Release the image if it hasn't already been released.
	if (m_sourceImage) {
		cvReleaseImage(&m_sourceImage);
		m_sourceImage = 0;
	}
	m_facesFound = 0;
	
	m_sourceImage = getIplImageFromIntArray(env, photo_data, width, height);
	if (m_sourceImage == 0) {
		LOGE("Error source image could not be created.");
		return false;
	}
	
	return true;
}

JNIEXPORT
jboolean
JNICALL
Java_org_openparallel_imagethresh_ImageThreshActivity_imageProcessingHasFinished(JNIEnv* env,
                                                                                 jobject thiz){
    return processingFinished;
}

JNIEXPORT jstring JNICALL
Java_org_openparallel_imagethresh_ImageThreshActivity_stringFromJNI(JNIEnv* env, jobject thiz){
    
    //do a little bit of simple float arithmetric (vector by scalar)
    //if it runs, and computes the correct result... we know it works!
    
    float* src = new float[1];
    src[0] = 1.5f;
    float* dest = new float[1];
    addc_float_c(dest, src, 1.0f, 1);
    
    if (dest[0] == 2.5f) {
        free(src);
        free(dest);
        return env->NewStringUTF("Hello from JNI! (ps... neon can compute floats too!)");
    }
    delete(src);
    delete(dest);
    
    return env->NewStringUTF("Hello from JNI! (but... neon can't compute floats :( )");
}

#endif

/*
 * End of android specific stuff
 */


