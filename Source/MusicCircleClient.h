/*
  ==============================================================================

    MusicCircleClient.h
    Created: 4 May 2020 7:57:46pm
    Author:  matth

  ==============================================================================
*/

#pragma once

#include <string>

class MusicCircleClient 
{
  public:
    MusicCircleClient(std::string username, std::string password, std::string apiUrl = "https://demo.museifi.com/api2/");
    /** login to mc with the sent user and pass. returns true if it works */
    bool login();
    /** retrieve the user id from the API*/
    unsigned int getUserId();
    /** test function that lists the comments on the sent audio item*/
    void listAnnotations(unsigned int mediaId);
    /** post a comment to the sent track, returns true if it works */ 
    bool postAnnotation(unsigned int mediaId, 
                      double start, 
                      double end, 
                      std::string content);
    /** upload the sent audio file, returns the id on MC or 0 if it fails*/
    unsigned int postMedia(std::string filePath);
    /** tramsfer comments from one audiocontent id to another. used after a file upload
     * if you want to transfer temporary comments from the default
     * audio file over to a newly uploaded audio file
    */
    bool transferAnnotations(unsigned int fromMediaId, unsigned int toMediaId);
    /** post a media file to the sent track, returns true if it works */
    bool postMediaFile(unsigned int id, std::string filePath);
    
  private:
    std::string username;
    std::string password; 
    std::string apiUrl; 
    std::string sessionId;
    unsigned int userId;

};
