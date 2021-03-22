/*
  ==============================================================================

    MusicCircleClient.cpp
    Created: 4 May 2020 7:57:46pm
    Author:  matth

  ==============================================================================
*/

#include "MusicCircleClient.h"
#include "../JuceLibraryCode/JuceHeader.h"

MusicCircleClient::MusicCircleClient(std::string username, std::string password, std::string apiUrl)
    : username{ username }, password{ password }, apiUrl{ apiUrl }, sessionId{ "0" }
{

}
/** login to mc with the sent user and pass. returns true if it works */
bool MusicCircleClient::login()
{
    juce::URL url{ apiUrl + "/login" };
    url = url.withParameter("username", username);
    url = url.withParameter("password", password);
    // POST the details
    WebInputStream* inStream = dynamic_cast<WebInputStream*>( url.createInputStream(true).get() );
    juce::String result = inStream->readEntireStreamAsString();
    delete inStream;

    // extract and store the session id
    DBG(result);
    var data = JSON::parse(result);
    if (data.hasProperty("session_id"))
    {
        juce::String session_id = data["session_id"];
        DBG("login successful " << session_id);
        this->sessionId = session_id.toStdString();
        this->userId = getUserId();
        return true;
    }
    else
    {
        DBG("login falied ");
        return false;
    }

    return false;
}

unsigned int MusicCircleClient::getUserId()
{
    if (sessionId == "0")
    {
        DBG("MusicCircleClient::getUserId Not logged in. call login first");
        return 0;
    }
    else
    {
        // build the JSON string
        juce::URL url{ apiUrl + "/user" };
        juce::String extraHeaders{ "Cookie:PHPSESSID=" + sessionId };
        WebInputStream* inStream = dynamic_cast<WebInputStream*>(url.createInputStream(
            false, // false means GET
            nullptr, //	OpenStreamProgressCallback * 	progressCallback = nullptr,
            nullptr, //void * 	progressCallbackContext = nullptr,
            extraHeaders
        ).get() );
        juce::String result = inStream->readEntireStreamAsString();
        delete inStream;
        DBG(result);
        var data = JSON::parse(result);
        juce::String userId = data["id"];
        DBG("MusicCircleClient::getUserId " << userId);
        return std::stoi(userId.toStdString());
        //var data = JSON::parse(result);
    }
}

void MusicCircleClient::listAnnotations(unsigned int mediaId)
{
    if (sessionId == "0")
    {
        DBG("MusicCircleClient::listAnnotation Not logged in. call login first");
        return;
    }
    else
    {
        // build the JSON string
        juce::String json{};
        json = "{}";
        juce::URL url{ apiUrl + "/media/" + std::to_string(mediaId) + "/annotations" };
        url = url.withPOSTData(json);
        juce::String extraHeaders{ "Cookie:PHPSESSID=" + sessionId };
        WebInputStream* inStream = dynamic_cast<WebInputStream*>(url.createInputStream(
            false, // false means GET
            nullptr, //	OpenStreamProgressCallback * 	progressCallback = nullptr,
            nullptr, //void * 	progressCallbackContext = nullptr,
            extraHeaders
            //int 	connectionTimeOutMs = 0,
            //StringPairArray * 	responseHeaders = nullptr,
            //int * 	statusCode = nullptr,
            //int 	numRedirectsToFollow = 5,
            //String 	httpRequestCmd = {} 
        ).get() );
        juce::String result = inStream->readEntireStreamAsString();
        delete inStream;
        DBG(result);
        //var data = JSON::parse(result);

    }

}

/** post a comment to the sent track, returns true if it works */
bool MusicCircleClient::postAnnotation(unsigned int mediaId,
    double start,
    double end,
    std::string content)
{
    if (sessionId == "0")
    {
        DBG("MusicCircleClient::postAnnotation Not logged in. call login first");
        return false;
    }
    else
    {
        juce::URL url{ apiUrl + "/media/" + std::to_string(mediaId) + "/annotations" };
        juce::String json{ "{\"user_id\":\"" + std::to_string(this->userId) + "\", \"region_start\":\"" + std::to_string(start) + "\",\"region_end\" : \"" + std::to_string(end) + "\",\"title\" : \"" + content + "\", \"media_id\" : \"" + std::to_string(mediaId) + "\"}" };
        url = url.withPOSTData(json);
        juce::String extraHeaders{ "Cookie:PHPSESSID=" + sessionId };
        extraHeaders += "\nContent-Type: application/json";

        // "Content-Type: application/json"
        WebInputStream* inStream = dynamic_cast<WebInputStream*>(url.createInputStream(
            true, // false means GET
            nullptr, //	OpenStreamProgressCallback * 	progressCallback = nullptr,
            nullptr, //void * 	progressCallbackContext = nullptr,
            extraHeaders
            //int 	connectionTimeOutMs = 0,
            //StringPairArray * 	responseHeaders = nullptr,
            //int * 	statusCode = nullptr,
            //int 	numRedirectsToFollow = 5,
            //String 	httpRequestCmd = {} 
        ).get() );
        juce::String result = inStream->readEntireStreamAsString();
        delete inStream;
        DBG(result);
        //var data = JSON::parse(result);
        return true;
    }

    return false;
}
/** upload the sent audio file, returns the id on MC or 0 if it fails*/
unsigned int MusicCircleClient::postMedia(std::string filePath)
{
    if (sessionId == "0")
    {
        DBG("MusicCircleClient::postAnnotation Not logged in. call login first");
        return false;
    }
    else
    {
        juce::URL url{ apiUrl + "/media" };
        url = url.withParameter("filename", "test.wav");

      //  url = url.withFileToUpload("files[]", File{ "C:\\Users\\matth\\Documents\\JUCE Demo Audio Recording.wav" }, "audio/wav");
        url = url.withFileToUpload("files[]", File{ juce::String{filePath} }, "audio/wav");
        juce::String extraHeaders{ "Cookie:PHPSESSID=" + sessionId };
        //extraHeaders += "\nContent-Type: application/json";

        // "Content-Type: application/json"
        WebInputStream* inStream = dynamic_cast<WebInputStream*>(url.createInputStream(
            true, // false means GET
            nullptr, //	OpenStreamProgressCallback * 	progressCallback = nullptr,
            nullptr, //void * 	progressCallbackContext = nullptr,
            extraHeaders
            //int 	connectionTimeOutMs = 0,
            //StringPairArray * 	responseHeaders = nullptr,
            //int * 	statusCode = nullptr,
            //int 	numRedirectsToFollow = 5,
            //String 	httpRequestCmd = {} 
        ).get() );
        juce::String result = inStream->readEntireStreamAsString();
        delete inStream;
        DBG(result);
        //var data = JSON::parse(result);
        return true;
    }

    return false;

    return 0;
}
/** tramsfer comments from one audiocontent id to another. used after a file upload
 * if you want to transfer temporary comments from the default
 * audio file over to a newly uploaded audio file
*/
bool MusicCircleClient::transferAnnotations(unsigned int fromMediaId,
    unsigned int toMediaId)
{
    return false;
}

/** post a media file to the sent track, returns true if it works */
bool MusicCircleClient::postMediaFile(unsigned int id, std::string filePath)
{
    if (sessionId == "0")
    {
        DBG("MusicCircleClient::postAnnotation Not logged in. call login first");
        return false;
    }
    else
    {
        // Post Media File
        juce::URL url{ apiUrl + "/media/" + std::to_string(id) + "/files" };
        url = url.withParameter("filename", juce::String(filePath)); // ST: Not sure what this line does??
        url = url.withFileToUpload("files[]", File{ juce::String{filePath} }, "audio/midi");
        juce::String extraHeaders{ "Cookie:PHPSESSID=" + sessionId };
        
        // "Content-Type: application/json"
        WebInputStream* inStream = dynamic_cast<WebInputStream*>(url.createInputStream(
                                                                                       true, // false means GET
                                                                                       nullptr, //    OpenStreamProgressCallback *     progressCallback = nullptr,
                                                                                       nullptr, //void *     progressCallbackContext = nullptr,
                                                                                       extraHeaders
                                                                                       //int     connectionTimeOutMs = 0,
                                                                                       //StringPairArray *     responseHeaders = nullptr,
                                                                                       //int *     statusCode = nullptr,
                                                                                       //int     numRedirectsToFollow = 5,
                                                                                       //String     httpRequestCmd = {}
                                                                                       ).get() );
        juce::String result = inStream->readEntireStreamAsString();
        delete inStream;
        DBG(result);
        //var data = JSON::parse(result);
        return true;
        
    }
    return false;
}
