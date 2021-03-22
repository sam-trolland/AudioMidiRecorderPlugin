/*
  ==============================================================================

    LogginManager.cpp
    Created: 20 Apr 2020 1:58:21pm
    Author:  Teresa

  ==============================================================================
*/

#include "LogginManager.h"
#include <iostream>     // std::cout, std::ios
#include <functional>

LogginManager::LogginManager()
{
    
}

// add a listener
void LogginManager::addEventListener(LogginListener* listener)
{
    listeners.push_back(listener);
}

// call this private func when events occur
void LogginManager::sendEventToListeners(MusicCircleEvent event)
{
    for (LogginListener* listener : listeners)
    {
        listener->musicCircleEvent(event);
    }
}

void LogginManager::setUsername(std::string usr) {
    usrName = usr;
}

void LogginManager::setPassword(std::string pass) {
    usrPassword = pass;
}


// // Component interface
// void LogginManager::paint (Graphics& g)
// {

// }
// void LogginManager::resized()
// {
//     std::cout << "Loggin: resized" << std::endl;
//     //usernameField.setBounds(0, 0, getWidth()/2, getHeight());
//     //passwordField.setBounds(0, getWidth()/2, getWidth()/2, getHeight());
    
// }

std::string LogginManager::getUserId() const  
{
    return this->userId;
}

void LogginManager::resetClockAndAnnotationQueue()
{
    DBG("LogginManager::resetClock() ");
    startTime = std::chrono::system_clock::now();
    annotationQueue.clear();
}




void LogginManager::postMedia(std::string filePath, std::function<void(int)> resultCallback)
{
   threadPool.addJob(
          [filePath, resultCallback, this](){
              std::cout << "thread pool doing post Media on " << filePath << std::endl; 
              //this->postMediaReal(filePath);
              int result = this->postMediaForThreadPool(filePath);
              sendEventToListeners(MusicCircleEvent::media_upload_succeeded);

              resultCallback(result);
        }
    );    
}

void LogginManager::postMediaFile(std::string filePath, std::function<void(int)> resultCallback)
{
    threadPool.addJob(
                      [filePath, resultCallback, this](){
                          std::cout << "thread pool doing post Media on " << filePath << std::endl;
                          //this->postMediaReal(filePath);
                          
                          int result = this->postMediaFileForThreadPool(audioId,filePath);
                          sendEventToListeners(MusicCircleEvent::media_upload_succeeded);
                          
                          resultCallback(result);
                      }
                      );
}

std::string LogginManager::loginToMC(std::string username, std::string password)
{
    std::queue<std::pair<std::string,std::string>> postFields;
    std::pair<std::string,std::string> username_pair;
    username_pair.first = "username";
    username_pair.second = username;
    std::pair<std::string,std::string> password_pair;
    password_pair.first = "password";
    password_pair.second = password;
    postFields.push(username_pair);
    postFields.push(password_pair);
    httpPostRequest("login", postFields, "",
        [this](std::string responseStr)
        {
            std::cout << "LogginManager::loginToMC loginToMC request callback." << std::endl;
        juce::String response{responseStr};
        // extract and store the session id
        //std::cout << response << std::endl;
        var data = JSON::parse(response);
        
        if (data.hasProperty("session_id"))
        {
            juce::String sId = data["session_id"];
            sessionId = sId.toStdString();
            std::cout << "LogginManager::loginToMC login successful " << sessionId << std::endl;
            // now get the user id      
            // juce::String userResponse = 
            httpGetRequest("user", [this](std::string userResponse){
                std::cout << "LogginManager::loginToMC getUser data callback " << std::endl;
                var data = JSON::parse(userResponse);
                if (data.hasProperty("id"))
                {
                    juce::String uId = data["id"];
                    // userId is the userId inside the logginManager class...
                    userId = uId.toStdString();
                    // note  - I don't think the DBG macro works properly inside 
                    std::cout << "LogginManager::loginToMC getUser " << userId << std::endl;
                    sendEventToListeners(MusicCircleEvent::login_succeeded);
                }
                else {
                    sendEventToListeners(MusicCircleEvent::login_failed);          
                }// no user id  
            });            
        }// end of have session id (aka successful login)
        else {
            sendEventToListeners(MusicCircleEvent::login_failed);        

        }
        } // end of lambda callback
    ); // end of httpPost function call
return "Success";
}



void LogginManager::logData(std::string improviser, std::string data)
{
   // std::cout << "LogginManager::logData Data to loggin from improviser: " << improviser << "\n";
   // std::cout << data << "\n\n";

    /** Calculating the time to associate the comment to the audio. */
    std::chrono::system_clock::time_point currentTime = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = currentTime - startTime;
    //std::cout << "\n\n ELAPSED TIME: " << elapsed_seconds.count() << "\n\n";

    /** Time is converted to milliseconds. Additionally the start time of the comment is taken to be 1 second earlier to count on the time that the system takes to log in the information. */
    std::string startTime = std::to_string((elapsed_seconds.count()*1000)-1000);
    std::string endTime = std::to_string(elapsed_seconds.count()*1000);
    Annotation a;
    a.startTime = startTime;
    a.endTime = endTime;
    a.improviserName = improviser;
    a.comment = data;
    annotationQueue.push_back(
        a
    );
}


std::string LogginManager::logoutFromMC()
{
    httpGetRequest("logout", [this](std::string userResponse){
        std::cout << "LogginManager::logoutFromMC logout callback " << std::endl;
        var data = JSON::parse(userResponse);
        if (data.hasProperty("result"))
        {
            juce::String result = data["result"];
            std::cout << "LogginManager::logoutFromMC result: " << result.toStdString();
            sendEventToListeners(MusicCircleEvent::logout_succeeded);
        }
        else
            sendEventToListeners(MusicCircleEvent::logout_failed);
    });
    return "Success";
}

// TODO: Send annotations with system's user.
void LogginManager::sendQueuedAnnotations()
{
    std::string logoutUser = logoutFromMC();
    
    if(logoutUser == "Success") {
        loginToMC("csys2", "test123");
        //while (threadPool.getNumJobs() > 0)
          //  std::cout << "Waiting for thread jobs to finish: system login to MC";
        
        std::cout << "Session id: " << sessionId;
        std::cout << "User id: " << userId;
    
        for (Annotation& annotation : annotationQueue){
            //if(loginToMC() == "Success") // not sure we need to login every time we post a comment/ probablt only need t odo that once
            if(sessionId != "") // we have a session id to use
            {
                std::string startTime = annotation.startTime;
                std::string endTime = annotation.endTime;
                std::string message = annotation.improviserName + ": " + annotation.comment;
                std::string annotation = "{ \"linked_media_id\": \"\", \"media_id\": \"" + audioId + "\", \"region_end\": \"" + endTime + "\", \"region_start\": \"" + startTime + "\", \"replies\": [], \"title\": \"(MODIFIED) Annotation from " +message + "\", \"user_id\": \"" + userId + "\" }";
                std::queue<std::pair<std::string,std::string>> postFields;

                httpPostRequest("media/"+audioId+"/annotations", postFields, annotation, [](std::string result)
                {
                    std::cout << "LogginManager::logData post callback " << result << std::endl;
                    if(result == "error")
                        std::cout << "LogginManager::logData There was an error posting the annotation to MC.";

                });// end of call the httpPostRequest
            }// end of if have session id
        }// end iterate over annotations
    }
    
    std::string logoutSystem = logoutFromMC();
   // while (threadPool.getNumJobs() > 0) {
     //   std::cout << "Waiting for thread jobs to finish: system logout from MC";
    //}
    std::cout << "About to login with username and password: " << usrName << ", " << usrPassword;
    if(logoutSystem == "Success")
        loginToMC(usrName, usrPassword);
}

//////////////// private stuff

void LogginManager::httpGetRequest(const std::string &apiMenu, std::function<void(std::string)> resultCallback)
//juce::String LogginManager::httpGetRequest(const std::string &apiMenu, std::function<void(std::string)> resultCallback)
{   

    threadPool.addJob(
          [apiMenu, resultCallback,  this](){
              std::cout << "thread pool doing GET on " << apiMenu << std::endl; 
              juce::String result = this->httpGetRequestForThreadPool(apiMenu);
              resultCallback(result.toStdString());
        }
    );    
}

void LogginManager::httpPostRequest(const std::string &apiMenu, std::queue<std::pair<std::string,std::string>> postFields, std::string postdata, std::function<void(std::string)> resultCallback)
{

    threadPool.addJob(
            [apiMenu, postFields, postdata, resultCallback, this](){
                std::cout << "thread pool doing POST on " << apiMenu << std::endl; 
                juce::String result = this->httpPostRequestForThreadPool(apiMenu, postFields, postdata);
                resultCallback(result.toStdString());
            }
        );   
}



juce::String LogginManager::httpGetRequestForThreadPool(const std::string &apiMenu)
{
    juce::URL url{ urlBase + apiMenu };
    if(sessionId != "") {
        juce::String extraHeaders{ "Cookie:PHPSESSID=" + sessionId };
        std::unique_ptr<InputStream> inStream = (url.createInputStream(
        //WebInputStream* inStream = dynamic_cast<WebInputStream*>(url.createInputStream(
                false, // false means GET
                nullptr, //    OpenStreamProgressCallback *     progressCallback = nullptr,
                nullptr, //void *     progressCallbackContext = nullptr,
                extraHeaders
            ));
    
        juce::String response = inStream->readEntireStreamAsString();
        //delete inStream;
        return response;
    }
    else
        return "error";
}
 
 juce::String LogginManager::httpPostRequestForThreadPool(const std::string &apiMenu, std::queue<std::pair<std::string,std::string>> postFields, std::string postdata)
{
    juce::URL url{ urlBase + apiMenu };
    //WebInputStream* inStream;
    juce::String result = "error";
    
    if(sessionId != "") {
        juce::String extraHeaders{ "Cookie:PHPSESSID=" + sessionId };
        extraHeaders += "\nContent-Type: application/json";
        juce::String json{ postdata };
        url = url.withPOSTData(json);
        std::unique_ptr<InputStream> inStream = (url.createInputStream(
        //inStream = dynamic_cast<WebInputStream*>(url.createInputStream(
            true, // false means GET
            nullptr, //    OpenStreamProgressCallback * progressCallback = nullptr,
            nullptr, //void *    progressCallbackContext = nullptr,
            extraHeaders
        ));
        
        // Read Result
        if (inStream != NULL) {
            result = inStream->readEntireStreamAsString();
        }
    }
    
    if(!postFields.empty()) {
        while (!postFields.empty()) {
            std::pair<std::string,std::string> field = postFields.front();
            const std::string &name = field.first;
            const std::string &value = field.second;
            url = url.withParameter(name, value);
            postFields.pop();
        }
        // POST the details
        std::unique_ptr<InputStream> inStream = (url.createInputStream(true));
        // inStream = dynamic_cast<WebInputStream*>(url.createInputStream(true));
        
        // Read Result
        if (inStream != NULL) {
            result = inStream->readEntireStreamAsString();
        }
    }
    
    //juce::String result = inStream->readEntireStreamAsString();
    // delete inStream;

    /* TODO: Check if the request was successful */
    //if (http_code == 200 && res != CURLE_ABORTED_BY_CALLBACK)
    return result;
    
    /* TODO: Return error if query not successful. */
    //return "error";
}



/** upload the sent audio file, returns the id on MC or 0 if it fails*/
unsigned int LogginManager::postMediaForThreadPool(std::string filePath)
{
    if (sessionId == "")
    {
        DBG("MusicCircleClient::postAnnotation Not logged in. call login first");
        return false;
    }
    else
    {
        juce::URL url{ urlBase + "/media" };
        url = url.withParameter("filename", "test.wav");

      //  url = url.withFileToUpload("files[]", File{ "C:\\Users\\matth\\Documents\\JUCE Demo Audio Recording.wav" }, "audio/wav");
        url = url.withFileToUpload("files[]", File{ juce::String{filePath} }, "audio/wav");
        juce::String extraHeaders{ "Cookie:PHPSESSID=" + sessionId };
        //extraHeaders += "\nContent-Type: application/json";

        // "Content-Type: application/json"
        std::unique_ptr<InputStream> inStream = (url.createInputStream(
        //WebInputStream* inStream = dynamic_cast<WebInputStream*>(url.createInputStream(
            true, // false means GET
            nullptr, //    OpenStreamProgressCallback *     progressCallback = nullptr,
            nullptr, //void *     progressCallbackContext = nullptr,
            extraHeaders
            //int     connectionTimeOutMs = 0,
            //StringPairArray *     responseHeaders = nullptr,
            //int *     statusCode = nullptr,
            //int     numRedirectsToFollow = 5,
            //String     httpRequestCmd = {}
        ));
        juce::String result = inStream->readEntireStreamAsString();
        //delete inStream;
        DBG(result);

        // update the audio id
        var data = JSON::parse(result);

        if (data.hasProperty("id"))
        {
            juce::String aId = data["id"];
            audioId = aId.toStdString();
            DBG("Saving posted audio id " << audioId);
        }
         

        //var data = JSON::parse(result);
        return true;
    }

    return false;

    return 0;
}

/** upload media attachment for given mediaID, returns the id on MC or 0 if it fails*/
unsigned int LogginManager::postMediaFileForThreadPool(std::string mediaId, std::string filePath)
{
    if (sessionId == "")
    {
        DBG("MusicCircleClient::postMediaFileForThreadPool Not logged in. call login first");
        return false;
    }
    else
    {
        juce::URL url{ urlBase + "/media" + "/" + mediaId + "/files" };
        url = url.withParameter("filename", juce::String(filePath));
        url = url.withFileToUpload("files[]", File{ juce::String{filePath} }, "audio/midi");
        juce::String extraHeaders{ "Cookie:PHPSESSID=" + sessionId };
        
        // "Content-Type: application/json"
        std::unique_ptr<InputStream> inStream = (url.createInputStream(
                                                                       //WebInputStream* inStream = dynamic_cast<WebInputStream*>(url.createInputStream(
                                                                       true, // false means GET
                                                                       nullptr, //    OpenStreamProgressCallback *     progressCallback = nullptr,
                                                                       nullptr, //void *     progressCallbackContext = nullptr,
                                                                       extraHeaders
                                                                       //int     connectionTimeOutMs = 0,
                                                                       //StringPairArray *     responseHeaders = nullptr,
                                                                       //int *     statusCode = nullptr,
                                                                       //int     numRedirectsToFollow = 5,
                                                                       //String     httpRequestCmd = {}
                                                                       ));
        juce::String result = inStream->readEntireStreamAsString();
        //delete inStream;
        DBG(result);
        
        // update the audio id
        var data = JSON::parse(result);
        
        if (data.hasProperty("id"))
        {
            juce::String mId = data["id"];
            std::string midiId = mId.toStdString();
            DBG("Saving posted midi file id " << midiId);
        }
        
        
        //var data = JSON::parse(result);
        return true;
    }
    
    return false;
    
    return 0;
}
