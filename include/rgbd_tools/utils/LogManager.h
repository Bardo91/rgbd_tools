//---------------------------------------------------------------------------------------------------------------------
//  RGBD_TOOLS
//---------------------------------------------------------------------------------------------------------------------
//  Copyright 2018 Pablo Ramon Soria (a.k.a. Bardo91) pabramsor@gmail.com
//---------------------------------------------------------------------------------------------------------------------
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software
//  and associated documentation files (the "Software"), to deal in the Software without restriction,
//  including without limitation the rights to use, copy, modify, merge, publish, distribute,
//  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in all copies or substantial
//  portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES
//  OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
//  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//---------------------------------------------------------------------------------------------------------------------


#ifndef LOGMANAGER_H_
#define LOGMANAGER_H_

#include <string>
#include <fstream>
#include <mutex>
#include <chrono>

#include <unordered_map>

/// Thread safe class used as logging system. 
class LogManager {
public:	//	Static interface.
	/// Initialize the logging system. 
	/// \param _appName: Base name used for the log file.
	/// \param _useCout: Write to cout too.
    static void init(const std::string _appName);

	/// Close the logging system. It makes sure that the log is closed properly.
	static void close();

	/// Get current instance of the logging system.
	static LogManager* get();

public:	// Public interface.
	/// Write message to the log system with a custom tag
    void message(const std::string &_msg, const std::string &_tag, bool _useCout = false);
	
	/// Write to the log system with status tag.
    void status(const std::string &_msg, bool _useCout = false);

	/// Write to the log system with warning tag.
    void warning(const std::string &_msg, bool _useCout = false);

	/// Write to the log system with error tag.
    void error(const std::string &_msg, bool _useCout = false);

    void saveTimeMark(std::string _tag);

    double measureTimeBetweenMarks(std::string _tag1, std::string _tag2);

private:	// Private interface.
    LogManager(const std::string _appName);
	~LogManager();

	static LogManager *mSingleton;

	bool mUseCout = false;

    std::unordered_map<std::string, std::chrono::high_resolution_clock::time_point> mTimeMap;
	std::chrono::high_resolution_clock::time_point  mInitTime;
	std::ofstream mLogFile;
	std::mutex mSecureGuard;
};

#endif
