#ifndef LIB_ADAFRUIT_MP3_H
#define LIB_ADAFRUIT_MP3_H

#include "mp3dec.h"
#include <cstdint>
#include <cstddef>
#include <cstdlib>

using namespace std;

//TODO: decide on a reasonable buffer size
#define MP3_OUTBUF_SIZE (4 * 1024)
#define MP3_INBUF_SIZE (2 * 1024)

#define BUFFER_LOWER_THRESH (1 * 1024)

#define MP3_SAMPLE_RATE_DEFAULT 44100

struct Adafruit_MP3_outbuf {
	volatile int count;
	int16_t buffer[MP3_OUTBUF_SIZE];
};

class Adafruit_MP3 {
public:
	Adafruit_MP3() : hMP3Decoder() { inbufend = (inBuf + MP3_INBUF_SIZE); }
	~Adafruit_MP3() { MP3FreeDecoder(hMP3Decoder); };
	bool begin();
	void setBufferCallback(int (*fun_ptr)(uint8_t *, int));
	void setSampleReadyCallback(void (*fun_ptr)(int16_t, int16_t));
		
	void play();
	void pause();
	void resume();
	
	int tick();

	static uint8_t numChannels;
	
protected:
	HMP3Decoder hMP3Decoder;
	
	volatile int bytesLeft;
	uint8_t *readPtr;
	uint8_t *writePtr;
	uint8_t inBuf[MP3_INBUF_SIZE];
	uint8_t *inbufend;
	bool playing = false;
	
	int (*bufferCallback)(uint8_t *, int);
	int findID3Offset(uint8_t *readPtr);

};

class Adafruit_MP3_DMA : public Adafruit_MP3 {
public:
	Adafruit_MP3_DMA() : Adafruit_MP3() {
		framebuf = NULL;
		decodeCallback = NULL;
	}
	~Adafruit_MP3_DMA() {
		if(framebuf != NULL) free(framebuf);
	}

	void getBuffers(int16_t **ping, int16_t **pong);
	void setDecodeCallback(void (*fun_ptr)(int16_t *, int)) { decodeCallback = fun_ptr; }

	void play();
	int fill();
private:
	int16_t *framebuf, *leftover;
	int leftoverSamples;
	MP3FrameInfo frameInfo;
	void (*decodeCallback)(int16_t *, int);
};

void MP3_Handler();

#endif
