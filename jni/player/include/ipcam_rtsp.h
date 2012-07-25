#ifndef _IPCAM_RTSP_H_
#define _IPCAM_RTSP_H_

#include "BasicUsageEnvironment.hh"
#include "GroupsockHelper.hh"
#include "liveMedia.hh"

#include "ipcam_ringsink.h"

//#include <utils/threads.h>
class StreamClientState;
class ourRTSPClient;
class playRTSPClient;

class ipcam_rtsp
{
	public:
		unsigned short fVideoHeight;
		unsigned short fVideoWidth;
		unsigned fVideoFPS;
		char* fCodecName;
		char watchVariable;    ///< a flag to stop doEventLoop() set to nonzero will return from doEventLoop()

	public:
		virtual int StartRecv ()=0;
		virtual int Close ()=0;

		unsigned short videoWidth() const { return fVideoWidth; }
		unsigned short videoHeight() const { return fVideoHeight; }
		unsigned videoFPS() const { return fVideoFPS; }
		char const* codecName() const { return fCodecName; }
		
		virtual ~ipcam_rtsp() { };

	public:
		pthread_t rtsp_thread; ///< the thread hanlder of created RTSP thread
		UsageEnvironment* env; ///< Specify the environment parameters
};

class ipcam_rtsp_rec :  public ipcam_rtsp
{
	public:
		char* filename;
		int fps;
		
	public:
		int StartRecv ();
		int Close ();
		
		int Init(char *url, char* filename, int fps);
		ourRTSPClient* rtspClient;
		ipcam_rtsp_rec ();
		~ipcam_rtsp_rec ();
};


class ipcam_rtsp_play :  public ipcam_rtsp
{
    private:
		ringbufferwriter *pVideoBuffer; ///< video buffer to save the received depacketized video stream
		ringbufferwriter *pAudioBuffer; ///< audio buffer to save the received depacketized audio stream

    public:
		int StartRecv ();
		int Close ();

		playRTSPClient* rtspClient;
		int Init (char *url, ringbufferwriter *pCodecHRtspVideoBuffer,
					ringbufferwriter *pCodecHRtspAudioBuffer);
		ipcam_rtsp_play ();
		~ipcam_rtsp_play ();
};

// Define a class to hold per-stream state that we maintain throughout each stream's lifetime:
class StreamClientState {
public:
	StreamClientState();
	virtual ~StreamClientState();

public:
	MediaSubsessionIterator* iter;
	MediaSession* session;
	MediaSubsession* subsession;
	TaskToken streamTimerTask;
	double duration;
};

class playRTSPClient: public RTSPClient {
public:
	static playRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
			int verbosityLevel = 0,
			char const* applicationName = NULL,
			portNumBits tunnelOverHTTPPortNum = 0,
			ringbufferwriter* vbuffer = NULL,
			ringbufferwriter* abuffer = NULL);

protected:
	playRTSPClient(UsageEnvironment& env, char const* rtspURL,
			int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum ,ringbufferwriter *vbuffer, ringbufferwriter * abuffer);
	// called only by createNew();
	virtual ~playRTSPClient();

public:
	ringbufferwriter *vbuffer;
	ringbufferwriter *abuffer;
	StreamClientState scs;
};

class ourRTSPClient: public RTSPClient {
public:
	static ourRTSPClient* createNew(UsageEnvironment& env, char const* rtspURL,
			int verbosityLevel = 0,
			char const* applicationName = NULL,
			portNumBits tunnelOverHTTPPortNum = 0,
			char const* filename = NULL);

protected:
	ourRTSPClient(UsageEnvironment& env, char const* rtspURL,
			int verbosityLevel, char const* applicationName, portNumBits tunnelOverHTTPPortNum, char const* filename);
	// called only by createNew();
	virtual ~ourRTSPClient();

public:
	char fname[200];
	StreamClientState scs;
	QuickTimeFileSink* qtOut;
};

//temp class to try direct decoding, instead of using ringbuffer

class DummySink: public MediaSink {
public:
  static DummySink* createNew(UsageEnvironment& env,
			      MediaSubsession& subsession, // identifies the kind of data that's being received
			      char const* streamId = NULL); // identifies the stream itself (optional)

private:
  DummySink(UsageEnvironment& env, MediaSubsession& subsession, char const* streamId);
    // called only by "createNew()"
  virtual ~DummySink();

  static void afterGettingFrame(void* clientData, unsigned frameSize,
                                unsigned numTruncatedBytes,
				struct timeval presentationTime,
                                unsigned durationInMicroseconds);
  void afterGettingFrame(unsigned frameSize, unsigned numTruncatedBytes,
			 struct timeval presentationTime, unsigned durationInMicroseconds);
  void addData(unsigned char const* data, unsigned dataSize,
	       struct timeval presentationTime);
  int DecVideo (unsigned char* pBuffer, unsigned int bufferSize);

private:
  // redefined virtual functions:
  virtual Boolean continuePlaying();

private:
  u_int8_t* fReceiveBuffer;
  MediaSubsession& fSubsession;
  char* fStreamId;
};

#endif // _IPCAM_RTSP_H_
