#include "Adafruit_MP3.h"
#include "mp3common.h"
#include <cstring>
#include <iostream>

volatile bool activeOutbuf;
Adafruit_MP3_outbuf outbufs[2];
volatile int16_t *outptr;
static void (*sampleReadyCallback)(int16_t, int16_t);

uint8_t Adafruit_MP3::numChannels = 0;

/**
*****************************************************************************************
*  @brief      Begin the mp3 player. This initializes the playback timer and necessary interrupts.
*
*  @return     none
****************************************************************************************/
bool Adafruit_MP3::begin()
{	
  sampleReadyCallback = NULL;
  bufferCallback = NULL;

  if ((hMP3Decoder = MP3InitDecoder()) == 0)
    {
      return false;
    }
  else return true;
}

/**
*****************************************************************************************
*  @brief      Set the function the player will call when it's buffers need to be filled. 
*				Care must be taken to ensure that the callback function is efficient.
*				If the callback takes too long to fill the buffer, playback will be choppy
*
*	@param		fun_ptr the pointer to the callback function. This function must take a pointer
*				to the location the bytes will be written, as well as an integer that represents
*				the maximum possible bytes that can be written. The function should return the 
*				actual number of bytes that were written.
*
*  @return     none
****************************************************************************************/
void Adafruit_MP3::setBufferCallback(int (*fun_ptr)(uint8_t *, int)){ bufferCallback = fun_ptr; }

/**
*****************************************************************************************
*  @brief      Set the function that the player will call when the playback timer fires.
*				The callback is called inside of an ISR, so it should be short and efficient.
*				This will usually just be writing samples to the DAC.
*
*	@param		fun_ptr the pointer to the callback function. The function must take two 
*				unsigned 16 bit integers. The first argument to the callback will be the
*				left channel sample, and the second channel will be the right channel sample.
*				If the played file is mono, only the left channel data is used.
*
*  @return     none
****************************************************************************************/
void Adafruit_MP3::setSampleReadyCallback(void (*fun_ptr)(int16_t, int16_t)) { sampleReadyCallback = fun_ptr; }

/**
*****************************************************************************************
*  @brief      Play an mp3 file. This function resets the buffers and should only be used
*				when beginning playback of a new mp3 file. If playback has been stopped
*				and you would like to resume playback at the current location, use Adafruit_MP3::resume instead.
*
*  @return     none
****************************************************************************************/
void Adafruit_MP3::play()
{
  bytesLeft = 0;
  activeOutbuf = 0;
  readPtr = inBuf;
  writePtr = inBuf;
	
  outbufs[0].count = 0;
  outbufs[1].count = 0;
  playing = false;

}

void Adafruit_MP3_DMA::play()
{
	Adafruit_MP3::play();
	leftoverSamples = 0;

  //fill both buffers
  fill();
}

/**
*****************************************************************************************
*  @brief      pause playback. This function stops the playback timer.
*
*  @return     none
****************************************************************************************/
void Adafruit_MP3::pause()
{
  
}

/**
*****************************************************************************************
*  @brief      Resume playback. This function re-enables the playback timer. If you are
*				starting playback of a new file, use Adafruit_MP3::play instead
*
*  @return     none
****************************************************************************************/
void Adafruit_MP3::resume()
{
  
}

/**
*****************************************************************************************
*  @brief      Get the number of bytes until the end of the ID3 tag.
*
*	@param		readPtr current read pointer
*
*  @return     none
****************************************************************************************/
int Adafruit_MP3::findID3Offset(uint8_t *readPtr)
{
  char header[10];
  memcpy(header, readPtr, 10);
  //http://id3.org/id3v2.3.0#ID3v2_header
  if(header[0] == 0x49 && header[1] == 0x44 && header[2] == 0x33 && header[3] < 0xFF){
    //this is a tag
    uint32_t sz = ((uint32_t)header[6] << 23) | ((uint32_t)header[7] << 15) | ((uint32_t)header[8] << 7) | header[9];
    return sz;
  }
  else{
    //this is not a tag
    return 0;
  }
}

void Adafruit_MP3_DMA::getBuffers(int16_t **ping, int16_t **pong){
  *pong = outbufs[0].buffer;
  *ping = outbufs[1].buffer;
}

/**
*****************************************************************************************
*  @brief      The main loop of the mp3 player. This function should be called as fast as
*				possible in the loop() function of your sketch. This checks to see if the
*				buffers need to be filled, and calls the buffer callback function if necessary.
*				It also calls the functions to decode another frame of mp3 data.
*
*  @return     none
****************************************************************************************/
int Adafruit_MP3::tick(){
  if (outbufs[activeOutbuf].count == 0 && outbufs[!activeOutbuf].count > 0) {
    //time to swap the buffers
    activeOutbuf = !activeOutbuf;
    outptr = outbufs[activeOutbuf].buffer;
  }
	
  //if we are running out of samples, and don't yet have another buffer ready, get busy.
  if ((outbufs[activeOutbuf].count < BUFFER_LOWER_THRESH) && 
      (outbufs[!activeOutbuf].count < (MP3_OUTBUF_SIZE/2))) {
    //Serial.println("running low");
    //dumb, but we need to move any bytes to the beginning of the buffer
    if ((readPtr != inBuf) && (bytesLeft > 0) && (bytesLeft < BUFFER_LOWER_THRESH)) {
      /*
	Serial.print("move bytes: ");
	Serial.print((uint32_t)&inBuf, HEX); Serial.print(", ");
	Serial.print((uint32_t)&readPtr, HEX); Serial.print(", ");
	Serial.println(bytesLeft);
      */
      memmove(inBuf, readPtr, bytesLeft);
      std::cout << "moved" << std::endl;
      //Serial.println("moved");
      readPtr = inBuf;
      writePtr = inBuf + bytesLeft;
    }
		
    //get more data from the user application
    if (bufferCallback != NULL){
      std::cout << "ask" << std::endl;
      //Serial.println("ask!");
      if ( (inbufend - writePtr) > 0) {
	int bytesRead = bufferCallback(writePtr, inbufend - writePtr);
	writePtr += bytesRead;
	bytesLeft += bytesRead;
      }
    }
		
    MP3FrameInfo frameInfo;
    int err, offset;
    
    if (!playing) {
      // printf("Starting playback");
      std::cout << "Starting playback" << std::endl;
      
      /* Find start of next MP3 frame. Assume EOF if no sync found. */
      offset = MP3FindSyncWord(readPtr, bytesLeft);
      // Serial.print("Offset: "); Serial.println(offset);
      std::cout << "Offset: " << offset << std::endl;
      if (offset >= 0) {
	readPtr += offset;
	bytesLeft -= offset;
      }
		  
      err = MP3GetNextFrameInfo(hMP3Decoder, &frameInfo, readPtr);
      if (err == ERR_MP3_INVALID_FRAMEHEADER) {
        std::cout << "err == ERR_MP3_INVALID_FRAMEHEADER" << std::endl;
	readPtr += 1;
	bytesLeft -= 1;
      } else {
	// Serial.print("Setting timer sample rate to: "); Serial.println(frameInfo.samprate);
  std::cout << "Setting timer sample rate to: " << frameInfo.samprate << std::endl;
	if (frameInfo.samprate != MP3_SAMPLE_RATE_DEFAULT) {
	  // updateTimerFreq(frameInfo.samprate);
	}
	playing = true;
	numChannels = frameInfo.nChans;
      }
      return 1;
    }
		
    offset = MP3FindSyncWord(readPtr, bytesLeft);
    if (offset >= 0) {
      //Serial.print("found sync @ "); Serial.println(offset);
      std::cout << "found sync " << offset << std::endl;
      readPtr += offset;
      bytesLeft -= offset;

      //fill the inactive outbuffer
      err = MP3Decode(hMP3Decoder, &readPtr, (int*) &bytesLeft, outbufs[!activeOutbuf].buffer + outbufs[!activeOutbuf].count, 0);
      if (err) {
	// sometimes we have a bad frame, lets just nudge forward one byte
	if (err == ERR_MP3_INVALID_FRAMEHEADER) {
    std::cout << "err == ERR_MP3_INVALID_FRAMEHEADER " << std::endl;
	  readPtr += 1;
	  bytesLeft -= 1;
	}
  std::cout << "err " << err << std::endl;
	return err;
      }
      MP3DecInfo *mp3DecInfo = (MP3DecInfo *)hMP3Decoder;
      outbufs[!activeOutbuf].count += mp3DecInfo->nGrans * mp3DecInfo->nGranSamps * mp3DecInfo->nChans;
    }
  }

  std::cout << "return 0 " << std::endl;
  return 0;
}

//fill a buffer with data
int Adafruit_MP3_DMA::fill(){

  int ret = 0;
  int16_t *curBuf = outbufs[activeOutbuf].buffer;

  //put any leftover samples in the new buffer
  if(leftoverSamples > 0){
    memcpy(outbufs[activeOutbuf].buffer, leftover, leftoverSamples*sizeof(int16_t));
    outbufs[activeOutbuf].count = leftoverSamples;
    leftoverSamples = 0;
  }

  while(outbufs[activeOutbuf].count < MP3_OUTBUF_SIZE){
  loopstart:
    //dumb, but we need to move any bytes to the beginning of the buffer
    if(readPtr != inBuf && bytesLeft < BUFFER_LOWER_THRESH){
      memmove(inBuf, readPtr, bytesLeft);
      readPtr = inBuf;
      writePtr = inBuf + bytesLeft;
    }

    //get more data from the user application
    if(bufferCallback != NULL){
      if(inbufend - writePtr > 0){
	int bytesRead = bufferCallback(writePtr, inbufend - writePtr);
	if(bytesRead == 0){
	  ret = 1;
	  break;
	}
	writePtr += bytesRead;
	bytesLeft += bytesRead;
      }
    }

    int err, offset;

    if(!playing){
      /* Find start of next MP3 frame. Assume EOF if no sync found. */
      offset = MP3FindSyncWord(readPtr, bytesLeft);
      if(offset >= 0){
	readPtr += offset;
	bytesLeft -= offset;
      }

      err = MP3GetNextFrameInfo(hMP3Decoder, &frameInfo, readPtr);
      if(err != ERR_MP3_INVALID_FRAMEHEADER){
	if(frameInfo.samprate != MP3_SAMPLE_RATE_DEFAULT)
	  {
	    // updateTimerFreq(frameInfo.samprate);
	  }
	playing = true;
	Adafruit_MP3::numChannels = frameInfo.nChans;
      }
      if(framebuf != NULL) free(framebuf);
      framebuf = (int16_t *)malloc(frameInfo.outputSamps*sizeof(int16_t));
      goto loopstart;
    }

    offset = MP3FindSyncWord(readPtr, bytesLeft);
    if(offset >= 0){
      readPtr += offset;
      bytesLeft -= offset;

			MP3DecInfo *mp3DecInfo = (MP3DecInfo *)hMP3Decoder;
			int toRead = mp3DecInfo->nGrans * mp3DecInfo->nGranSamps * mp3DecInfo->nChans;
			if(outbufs[activeOutbuf].count + toRead < MP3_OUTBUF_SIZE){
				//we can read directly into the output buffer so lets do that
				err = MP3Decode(hMP3Decoder, &readPtr, (int*) &bytesLeft, outbufs[activeOutbuf].buffer + outbufs[activeOutbuf].count, 0);
				outbufs[activeOutbuf].count += toRead;
			}
			else{
				//the frame would cross byte boundaries, we need to split manually
				err = MP3Decode(hMP3Decoder, &readPtr, (int*) &bytesLeft, framebuf, 0);
				int remainder = MP3_OUTBUF_SIZE - outbufs[activeOutbuf].count;
				memcpy(outbufs[activeOutbuf].buffer + outbufs[activeOutbuf].count, framebuf, remainder*sizeof(int16_t));
				leftover = framebuf + remainder;
				leftoverSamples = (toRead-remainder);

				//swap buffers
				activeOutbuf = !activeOutbuf;
				outbufs[activeOutbuf].count = 0;
				ret = 0;
				break;
			}

			if (err) {
				return err;
			}
		}
	}

	if(decodeCallback != NULL) decodeCallback(curBuf, MP3_OUTBUF_SIZE);


	return ret;
}



/**
 *****************************************************************************************
 *  @brief      The IRQ function that gets called whenever the playback timer fires.
 *
 *  @return     none
 ****************************************************************************************/
void MP3_Handler()
{
  //disableTimer();
  
  if(outbufs[activeOutbuf].count >= Adafruit_MP3::numChannels){
    //it's sample time!
    if(sampleReadyCallback != NULL){
      if(Adafruit_MP3::numChannels == 1)
	sampleReadyCallback(*outptr, 0);
      else
	sampleReadyCallback(*outptr, *(outptr + 1));
      
      //increment the read position and decrement the remaining sample count
      outptr += Adafruit_MP3::numChannels;
      outbufs[activeOutbuf].count -= Adafruit_MP3::numChannels;
    }
  }
  
  //enableTimer();
  
  // acknowledgeInterrupt();
}
#if defined(NRF52)
}
#endif
