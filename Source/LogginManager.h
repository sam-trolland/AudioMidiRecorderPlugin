/*
  ==============================================================================

    LogginManager.h
    Created: 20 Apr 2020 1:58:21pm
    Author:  Teresa

  ==============================================================================
*/

#include <iostream>
#include <queue>
#include <utility>
//#include <curl/curl.h>
#include <chrono>
#include <ctime>
#include <string>
#include <functional>

#include "../JuceLibraryCode/JuceHeader.h"

#pragma once

/** simple interface to receive events from the the logginManager */


enum class MusicCircleEvent{login_succeeded, login_failed, media_upload_succeeded, media_upload_failed, logout_succeeded, logout_failed};

class LogginListener {
  public:
    LogginListener(){}
    /** implement this pure virtual function to receive events from the loggin*/
    virtual void musicCircleEvent(MusicCircleEvent event) = 0;
    
};

struct Annotation{
    std::string startTime; 
    std::string endTime;
    std::string improviserName;
    std::string comment;   
};

class LogginManager  {
  public:
    LogginManager();

  /** call this to register for events*/
    void addEventListener(LogginListener* listener);

    std::string getUserId() const;

    /** Login to Music Circle to get session id and user id. */
    std::string loginToMC(std::string username, std::string password);
    /** Logout from Music Circle. */
    std::string logoutFromMC();

    /** Log data about the performance and working of the system -- used for explanations */
    void logData(std::string improviser, std::string data);
    /** upload the sent audio file, returns the id on MC or 0 if it fails*/
    void postMedia(std::string filePath, std::function<void(int)> resultCallback);
    /** resets the clock used for annotation timings*/
    void resetClockAndAnnotationQueue();
     /** iterates over the annotation queue and sends them over */
    void sendQueuedAnnotations();

    /** this is called by the threadpool when you call httpPost */
    juce::String httpPostRequestForThreadPool(const std::string &apiMenu, std::queue<std::pair<std::string,std::string>> postFields, std::string postdata);
    /** this is called by the threadpool when you call httpGet */
    juce::String httpGetRequestForThreadPool(const std::string &apiMenu);

    unsigned int postMediaForThreadPool(std::string filePath);
    void setUsername(std::string usr);
    void setPassword(std::string pass);

  
private:
    /** vector of things that want to know about events*/
    std::vector<LogginListener*> listeners;
    void sendEventToListeners(MusicCircleEvent event);

    /** launch an http get request in a background thread using the thread pool*/
    void httpPostRequest(const std::string &apiMenu, 
                         std::queue<std::pair<std::string,std::string>> postFields, 
                         std::string postdata,
                         std::function<void(std::string)> resultCallback);
    /** launch an http post request in a background thread using the thread pool*/
    void httpGetRequest(const std::string &apiMenu, std::function<void(std::string)> resultCallback);
    
    /** manages the threads that do the uploading etc.*/
    juce::ThreadPool threadPool{10};

    /** url to Music Circle's (MC) API. */
    std::string urlBase = "https://demo.museifi.com/api2/";
    /** one loggeg into MC, the API will retrieve a session id used for following requests */
    std::string sessionId = "";
    /** used id used for uploading the audio and annotations. */
    std::string userId = "";
    /** audio id used for uploading the annoations. */
    std::string audioId = "6817"; // id for the 'dummy' audio file
    /** */
    std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();

    std::vector<Annotation> annotationQueue;
    std::string usrName;
    std::string usrPassword;
};



// ******************************************************************
// ******************************************************************
// ******************************************************************
//#include <iostream>
//#include <queue>
//#include <utility>
//#include <curl/curl.h>
//#include <chrono>
//#include <ctime>
//
//#pragma once
//
//class LogginManager {
//  public:
//    LogginManager();
//
//    /** url to Music Circle's (MC) API. */
//    std::string urlBase = "https://demo.museifi.com/api2/";
//    /** one loggeg into MC, the API will retrieve a session id used for following requests */
//    std::string sessionId = "";
//    /** used id used for uploading the audio and annotations. */
//    std::string userId = "";
//    /** audio id used for uploading the annoations. */
//    std::string audioId = "6817"; // id for the 'dummy' audio file
//    /** */
//    std::chrono::system_clock::time_point startTime = std::chrono::system_clock::now();
//
//    /** Allows to make a post request to MC. Use postfields when post request format is a form-data,
//     use postdata when post request is handled with json. */
//    std::string httpPostRequest(const std::string &apiMenu, std::queue<std::pair<std::string,std::string>> postFields, std::string postdata);
//    /** Allows to make a get request to MC */
//    std::string httpGetRequest(const std::string &apiMenu);
//    /** Log data about the performance and working of the system -- used for explanations */
//    void logData(std::string improviser, std::string data);
//    /** Login to Music Circle to get session id and user id. */
//    std::string loginToMC();
//    /** upload the sent audio file, returns the id on MC or 0 if it fails*/
//    unsigned int postMedia(std::string filePath);
//
//  protected:
//    /** Use to encode the fields that are sent through a post request. */
//    std::string encodePostFields(CURL *curl, std::queue<std::pair<std::string,std::string>> postFields);
//
//};
//
//
