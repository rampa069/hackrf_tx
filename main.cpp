//#include <Windows.h>
#include "WavSource.h"
#include "FMModulator.h"
#include "HackRFDevice.h"
#include "write_wav.h"
#include <unistd.h>
#include <signal.h>
#define HACKRF_SAMPLE 2000000
#define SAMPLE_COUNT 2048



int wav_end=0;

void sig_handler(int signo)
{
  if (signo == SIGINT)
    printf("received SIGINT\n");
    wav_end=1;
}


double GetTickCount(void) 
{
  struct timespec now;
  if (clock_gettime(CLOCK_MONOTONIC, &now))
    return 0;
  return now.tv_sec * 1000.0 + now.tv_nsec / 1000000.0;
}

int main(int argc, char *argv[])
{

        int gain=40;
        unsigned long center_freq=107700000;
        int mode=0;
        char c;
        char* wav_file="in.wav";

 	while(( c = getopt( argc, argv, "f:m:g:w:" )) != -1 )
	  switch ( c )
	  {
            case 'f':
                center_freq=atoi(optarg);
                break;
            case 'm':
                mode=atoi( optarg );
                break;
            case 'g':
                gain=atoi(optarg );
                break;
	    case 'w':
	        wav_file=optarg;
	        break;
            case '?':
                fprintf( stderr, "Unrecognized option!\n" );
                fprintf( stderr, "-f (freq) default: %ul\n",center_freq );
                fprintf( stderr, "-m (mode 0=WFM,1=NFM,2=AM) default: %i\n",mode );
                fprintf( stderr, "-g (gain) default: %i\n",gain );
                fprintf( stderr, "-w (wavfile) default %s\n",wav_file);
                return -1;
                break;
          }
        
	WavSource *wav = new WavSource(wav_file, SAMPLE_COUNT);

	uint32_t avSamplesPerSecByte = wav->getSampleByte() * wav->getChannels() * wav->getSampleRate();
	//176400
	int PerTimeStamp = 1000 / (avSamplesPerSecByte / (SAMPLE_COUNT * wav->getChannels()));
	int readTotalAudioByte = 0;
	int PerSamplesPerSecByte = SAMPLE_COUNT * wav->getSampleByte();
	int m_read_buf_size;
	int nTimeStamp = 0;
	int fTimeStamp = 0;
	double audioTimeTick = GetTickCount();

	int sample_rate = wav->getSampleRate()*1.0 / SAMPLE_COUNT*BUF_LEN;

        printf("Sample_rate: %i",sample_rate);
        sample_rate=5120000;

	FMModulator *mod = new FMModulator(90, mode, sample_rate);
	HackRFDevice *device = new HackRFDevice();

	if (!device->Open(mod)) {
		return 0;
	}

	device->SetSampleRate(sample_rate);
	device->SetFrequency(center_freq);
	device->SetGain(gain);
	device->SetAMP(0);
	device->StartTx();

	//WAV_Writer wavFile;
	//Audio_WAV_OpenWriter(&wavFile, "test11111111111.wav", wav->getSampleRate(), wav->getSampleByte());

        if (signal(SIGINT, sig_handler) == SIG_ERR)	
            printf("\ncan't catch SIGINT\n");
   

	while (wav_end != 1)
	{
		nTimeStamp = GetTickCount() - audioTimeTick;
		fTimeStamp = ((readTotalAudioByte + SAMPLE_COUNT)*2 / PerSamplesPerSecByte) * PerTimeStamp;

		if (nTimeStamp > fTimeStamp){
			m_read_buf_size = wav->readData();			

			//fwrite(wav->getData(), wav->getChannels(), SAMPLE_COUNT * sizeof(float), wavFile.fid);
			//wavFile.dataSize += wav->getChannels()* SAMPLE_COUNT * sizeof(float);
			
			mod->Start(wav);
			readTotalAudioByte += m_read_buf_size;
			if (!m_read_buf_size){
				audioTimeTick = GetTickCount();
				readTotalAudioByte = 0;
				wav->reset();
                                printf("Wav end.\n");
				wav_end=1;
			}
		}
		usleep(1000);
	}

	//Audio_WAV_CloseWriter(&wavFile);
	device->Close();
	delete(mod);
	return 0;
}
