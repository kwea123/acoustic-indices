/****************************************************************************
 * main.c
 * openacousticdevices.info
 * June 2017
 *****************************************************************************/

#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#include "audioMoth.h"

/* ARM libraries for FFT */
#include "arm_math.h"
#include "arm_const_structs.h"

/* Sleep and LED constants */

#define DEFAULT_WAIT_INTERVAL               1

#define WAITING_LED_FLASH_INTERVAL          2
#define WAITING_LED_FLASH_DURATION          1

#define LOW_BATTERY_LED_FLASHES             10

#define SHORT_LED_FLASH_DURATION            100
#define LONG_LED_FLASH_DURATION             500

/* Useful time constants */

#define SECONDS_IN_MINUTE                   60
#define SECONDS_IN_HOUR                     (60 * SECONDS_IN_MINUTE)
#define SECONDS_IN_DAY                      (24 * SECONDS_IN_HOUR)

/* SRAM buffer constants */

#define NUMBER_OF_BUFFERS                   8
#define NUMBER_OF_SAMPLES_IN_BUFFER         512
#define NUMBER_OF_SAMPLES_IN_DMA_TRANSFER   512
#define NUMBER_OF_BUFFERS_TO_SKIP           32
#define MAX_FLOATS_TO_WRITE				8*1024			//the maximum floats to write in one call
#define MAX_BYTES_TO_WRITE					32*1024			//the maximum bytes to write in one call
static volatile int dataCount = -NUMBER_OF_BUFFERS_TO_SKIP;

/* WAVE header constant */

#define PCM_FORMAT                          1
#define RIFF_ID_LENGTH                      4
#define LENGTH_OF_COMMENT                   128

/* Acoustic indices constants */
#define FREQ_BINS			    256 		//number of frequency bins
#define N_INDICES			    4			//number of indices to write to sd card
#define TIME_SPAN			    60			//time span for one index in seconds
#define NOISE_FACTOR			    0.3f		//noise = mean+NOISE_FACTOR*std
static const float32_t hamming[NUMBER_OF_SAMPLES_IN_DMA_TRANSFER] = //hamming filter
		{ 0.08, 0.08003477, 0.08013909, 0.08031292, 0.08055626, 0.08086906,
				0.08125127, 0.08170284, 0.0822237, 0.08281376, 0.08347295,
				0.08420116, 0.08499828, 0.08586418, 0.08679875, 0.08780184,
				0.08887329, 0.09001294, 0.09122063, 0.09249617, 0.09383936,
				0.09525001, 0.09672789, 0.0982728, 0.09988448, 0.10156271,
				0.10330722, 0.10511775, 0.10699403, 0.10893578, 0.1109427,
				0.11301448, 0.11515082, 0.11735139, 0.11961586, 0.12194389,
				0.12433512, 0.12678919, 0.12930573, 0.13188437, 0.13452471,
				0.13722635, 0.13998888, 0.14281189, 0.14569495, 0.14863762,
				0.15163946, 0.15470002, 0.15781883, 0.16099542, 0.16422931,
				0.16752001, 0.17086702, 0.17426984, 0.17772796, 0.18124085,
				0.18480797, 0.1884288, 0.19210278, 0.19582935, 0.19960796,
				0.20343803, 0.20731899, 0.21125024, 0.2152312, 0.21926125,
				0.2233398, 0.22746622, 0.2316399, 0.23586019, 0.24012646,
				0.24443807, 0.24879437, 0.25319469, 0.25763837, 0.26212475,
				0.26665313, 0.27122284, 0.27583319, 0.28048347, 0.28517299,
				0.28990103, 0.29466689, 0.29946984, 0.30430915, 0.3091841,
				0.31409394, 0.31903793, 0.32401534, 0.32902539, 0.33406735,
				0.33914043, 0.34424389, 0.34937694, 0.35453881, 0.35972872,
				0.36494588, 0.37018951, 0.37545881, 0.38075299, 0.38607124,
				0.39141277, 0.39677676, 0.4021624, 0.40756889, 0.41299539,
				0.4184411, 0.42390518, 0.42938682, 0.43488518, 0.44039943,
				0.44592874, 0.45147227, 0.45702919, 0.46259865, 0.46817981,
				0.47377183, 0.47937386, 0.48498506, 0.49060458, 0.49623157,
				0.50186517, 0.50750453, 0.51314881, 0.51879715, 0.5244487,
				0.5301026, 0.53575799, 0.54141402, 0.54706984, 0.55272459,
				0.55837742, 0.56402747, 0.56967389, 0.57531582, 0.58095241,
				0.58658281, 0.59220616, 0.59782163, 0.60342835, 0.60902548,
				0.61461218, 0.6201876, 0.62575089, 0.63130122, 0.63683774,
				0.64235963, 0.64786604, 0.65335614, 0.6588291, 0.6642841,
				0.66972031, 0.67513691, 0.68053307, 0.68590799, 0.69126085,
				0.69659084, 0.70189716, 0.707179, 0.71243557, 0.71766606,
				0.72286969, 0.72804568, 0.73319324, 0.73831159, 0.74339995,
				0.74845757, 0.75348367, 0.75847749, 0.76343829, 0.7683653,
				0.77325778, 0.77811501, 0.78293623, 0.78772072, 0.79246776,
				0.79717663, 0.80184662, 0.80647702, 0.81106714, 0.81561627,
				0.82012373, 0.82458885, 0.82901093, 0.83338932, 0.83772336,
				0.84201238, 0.84625575, 0.85045281, 0.85460293, 0.8587055,
				0.86275987, 0.86676546, 0.87072163, 0.87462781, 0.8784834,
				0.88228781, 0.88604048, 0.88974082, 0.89338829, 0.89698234,
				0.90052241, 0.90400798, 0.90743851, 0.91081349, 0.91413241,
				0.91739477, 0.92060007, 0.92374783, 0.92683757, 0.92986882,
				0.93284114, 0.93575406, 0.93860715, 0.94139997, 0.94413211,
				0.94680315, 0.94941269, 0.95196032, 0.95444568, 0.95686838,
				0.95922805, 0.96152434, 0.9637569, 0.9659254, 0.9680295,
				0.97006889, 0.97204326, 0.97395231, 0.97579575, 0.97757331,
				0.97928471, 0.98092969, 0.98250802, 0.98401944, 0.98546374,
				0.98684068, 0.98815007, 0.98939171, 0.9905654, 0.99167097,
				0.99270826, 0.99367711, 0.99457736, 0.99540889, 0.99617157,
				0.99686528, 0.99748992, 0.99804539, 0.99853161, 0.99894851,
				0.99929602, 0.99957409, 0.99978268, 0.99992176, 0.99999131,
				0.99999131, 0.99992176, 0.99978268, 0.99957409, 0.99929602,
				0.99894851, 0.99853161, 0.99804539, 0.99748992, 0.99686528,
				0.99617157, 0.99540889, 0.99457736, 0.99367711, 0.99270826,
				0.99167097, 0.9905654, 0.98939171, 0.98815007, 0.98684068,
				0.98546374, 0.98401944, 0.98250802, 0.98092969, 0.97928471,
				0.97757331, 0.97579575, 0.97395231, 0.97204326, 0.97006889,
				0.9680295, 0.9659254, 0.9637569, 0.96152434, 0.95922805,
				0.95686838, 0.95444568, 0.95196032, 0.94941269, 0.94680315,
				0.94413211, 0.94139997, 0.93860715, 0.93575406, 0.93284114,
				0.92986882, 0.92683757, 0.92374783, 0.92060007, 0.91739477,
				0.91413241, 0.91081349, 0.90743851, 0.90400798, 0.90052241,
				0.89698234, 0.89338829, 0.88974082, 0.88604048, 0.88228781,
				0.8784834, 0.87462781, 0.87072163, 0.86676546, 0.86275987,
				0.8587055, 0.85460293, 0.85045281, 0.84625575, 0.84201238,
				0.83772336, 0.83338932, 0.82901093, 0.82458885, 0.82012373,
				0.81561627, 0.81106714, 0.80647702, 0.80184662, 0.79717663,
				0.79246776, 0.78772072, 0.78293623, 0.77811501, 0.77325778,
				0.7683653, 0.76343829, 0.75847749, 0.75348367, 0.74845757,
				0.74339995, 0.73831159, 0.73319324, 0.72804568, 0.72286969,
				0.71766606, 0.71243557, 0.707179, 0.70189716, 0.69659084,
				0.69126085, 0.68590799, 0.68053307, 0.67513691, 0.66972031,
				0.6642841, 0.6588291, 0.65335614, 0.64786604, 0.64235963,
				0.63683774, 0.63130122, 0.62575089, 0.6201876, 0.61461218,
				0.60902548, 0.60342835, 0.59782163, 0.59220616, 0.58658281,
				0.58095241, 0.57531582, 0.56967389, 0.56402747, 0.55837742,
				0.55272459, 0.54706984, 0.54141402, 0.53575799, 0.5301026,
				0.5244487, 0.51879715, 0.51314881, 0.50750453, 0.50186517,
				0.49623157, 0.49060458, 0.48498506, 0.47937386, 0.47377183,
				0.46817981, 0.46259865, 0.45702919, 0.45147227, 0.44592874,
				0.44039943, 0.43488518, 0.42938682, 0.42390518, 0.4184411,
				0.41299539, 0.40756889, 0.4021624, 0.39677676, 0.39141277,
				0.38607124, 0.38075299, 0.37545881, 0.37018951, 0.36494588,
				0.35972872, 0.35453881, 0.34937694, 0.34424389, 0.33914043,
				0.33406735, 0.32902539, 0.32401534, 0.31903793, 0.31409394,
				0.3091841, 0.30430915, 0.29946984, 0.29466689, 0.28990103,
				0.28517299, 0.28048347, 0.27583319, 0.27122284, 0.26665313,
				0.26212475, 0.25763837, 0.25319469, 0.24879437, 0.24443807,
				0.24012646, 0.23586019, 0.2316399, 0.22746622, 0.2233398,
				0.21926125, 0.2152312, 0.21125024, 0.20731899, 0.20343803,
				0.19960796, 0.19582935, 0.19210278, 0.1884288, 0.18480797,
				0.18124085, 0.17772796, 0.17426984, 0.17086702, 0.16752001,
				0.16422931, 0.16099542, 0.15781883, 0.15470002, 0.15163946,
				0.14863762, 0.14569495, 0.14281189, 0.13998888, 0.13722635,
				0.13452471, 0.13188437, 0.12930573, 0.12678919, 0.12433512,
				0.12194389, 0.11961586, 0.11735139, 0.11515082, 0.11301448,
				0.1109427, 0.10893578, 0.10699403, 0.10511775, 0.10330722,
				0.10156271, 0.09988448, 0.0982728, 0.09672789, 0.09525001,
				0.09383936, 0.09249617, 0.09122063, 0.09001294, 0.08887329,
				0.08780184, 0.08679875, 0.08586418, 0.08499828, 0.08420116,
				0.08347295, 0.08281376, 0.0822237, 0.08170284, 0.08125127,
				0.08086906, 0.08055626, 0.08031292, 0.08013909, 0.08003477, 0.08 };
float32_t aci_previous[FREQ_BINS], aci_sumDiff[FREQ_BINS], sum[FREQ_BINS]; //numbers for aci
float32_t h_t_h[FREQ_BINS], sumSquared[FREQ_BINS]; //numbers for h_t
float32_t cvr_noise[FREQ_BINS], cvr_count[FREQ_BINS]; //numbers for cvr
arm_rfft_fast_instance_f32 S1; //instance for rfft

int32_t segments_in_time_span; // number of segments in a time_span
int32_t initial_segments_in_time_span; // number of segments in the very first time_span = 1/2 *segments_in_time_span
float32_t segments_in_time_span_inverse, segments_in_time_span_log2_inverse,
		initial_segments_in_time_span_log2_inverse;

float32_t floatBufferReady[NUMBER_OF_SAMPLES_IN_DMA_TRANSFER]; //convert the int16 in DMA to float32
float32_t fftBuffer[NUMBER_OF_SAMPLES_IN_DMA_TRANSFER]; //used to store fft values

/* Useful macros */

#define FLASH_LED(led, duration) { \
    AudioMoth_set ## led ## LED(true); \
    AudioMoth_delay(duration); \
    AudioMoth_set ## led ## LED(false); \
}

#define RETURN_ON_ERROR(fn) { \
    bool success = (fn); \
    if (success != true) { \
        FLASH_LED(Both, LONG_LED_FLASH_DURATION) \
        return; \
    } \
}

#define SAVE_SWITCH_POSITION_AND_POWER_DOWN(duration) { \
    *previousSwitchPosition = switchPosition; \
    AudioMoth_powerDownAndWake(duration, true); \
}

#define MAX(a,b) (((a) (b)) ? (a) : (b))

#define MIN(a,b) (((a) < (b)) ? (a) : (b))

/* WAV header */

#pragma pack(push, 1)

typedef struct {
	char id[RIFF_ID_LENGTH];
	uint32_t size;
} chunk_t;

typedef struct {
	chunk_t icmt;
	char comment[LENGTH_OF_COMMENT];
} icmt_t;

typedef struct {
	uint16_t format;
	uint16_t numberOfChannels;
	uint32_t samplesPerSecond;
	uint32_t bytesPerSecond;
	uint16_t bytesPerCapture;
	uint16_t bitsPerSample;
} wavFormat_t;

typedef struct {
	chunk_t riff;
	char format[RIFF_ID_LENGTH];
	chunk_t fmt;
	wavFormat_t wavFormat;
	chunk_t list;
	char info[RIFF_ID_LENGTH];
	icmt_t icmt;
	chunk_t data;
} wavHeader_t;

#pragma pack(pop)

/* USB configuration data structure */

#pragma pack(push, 1)

typedef struct {
	uint16_t startMinutes;
	uint16_t stopMinutes;
} startStopPeriod_t;

typedef struct {
	uint32_t time;
	uint8_t gain;
	uint8_t clockBand;
	uint8_t clockDivider;
	uint8_t acquisitionCycles;
	uint8_t oversampleRate;
	uint32_t sampleRate;
	uint16_t sleepDuration;
	uint16_t recordDuration;
	uint8_t enableLED;
	uint8_t activeStartStopPeriods;
	startStopPeriod_t startStopPeriods[5];
} configSettings_t;

#pragma pack(pop)

configSettings_t defaultConfigSettings = { .time = 0, .gain = 2,
		.clockBand = 4, .clockDivider = 2, .acquisitionCycles = 2,
		.oversampleRate = 16, .sampleRate = 48000, .sleepDuration = 10000,
		.recordDuration = 120, .enableLED = 1, .activeStartStopPeriods = 0,
		.startStopPeriods = { { .startMinutes = 60, .stopMinutes = 120 }, {
				.startMinutes = 300, .stopMinutes = 420 }, {
				.startMinutes = 540, .stopMinutes = 600 }, {
				.startMinutes = 720, .stopMinutes = 780 }, {
				.startMinutes = 900, .stopMinutes = 960 } } };

uint32_t *previousSwitchPosition = (uint32_t*) AM_BACKUP_DOMAIN_START_ADDRESS;

uint32_t *timeOfNextRecording = (uint32_t*) (AM_BACKUP_DOMAIN_START_ADDRESS + 4);

uint32_t *durationOfNextRecording = (uint32_t*) (AM_BACKUP_DOMAIN_START_ADDRESS
		+ 8);

configSettings_t *configSettings =
		(configSettings_t*) (AM_BACKUP_DOMAIN_START_ADDRESS + 12);

/* DC filter variables */

static int32_t previousSample;
static int32_t previousFilterOutput;

/* SRAM buffer variables */

static volatile uint32_t writeBuffer;
static volatile uint32_t writeBufferIndex;

static volatile bool recordingCancelled;

static int16_t* buffers[NUMBER_OF_BUFFERS];

/* Current recording file name */

static char fileName[13];

/* Function prototypes */

static void filter(int16_t* data, int size);
static void flashLedToIndicateBatteryLife(void);
static void makeRecording(uint32_t currentTime, uint32_t recordDuration,
bool enableLED);
static void scheduleRecording(uint32_t currentTime,
		uint32_t *timeOfNextRecording, uint32_t *durationOfNextRecording);
static void reset(float32_t* array, int size);
static float32_t fasterlog2(float32_t x);

/* Main function */

int main(void) {

	/* Initialise device */

	AudioMoth_initialise();

	AM_switchPosition_t switchPosition = AudioMoth_getSwitchPosition();

	if (AudioMoth_isInitialPowerUp()) {

		*timeOfNextRecording = 0;

		*durationOfNextRecording = 0;

		*previousSwitchPosition = AM_SWITCH_NONE;

		memcpy(configSettings, &defaultConfigSettings,
				sizeof(configSettings_t));

	} else {

		/* Indicate battery state is not initial power up and switch has been moved into USB */

		if (switchPosition != *previousSwitchPosition
				&& switchPosition == AM_SWITCH_USB) {

			flashLedToIndicateBatteryLife();

		}

	}

    /* Handle the case that the switch is in USB position  */

    if (switchPosition == AM_SWITCH_USB) {

        AudioMoth_handleUSB();

        SAVE_SWITCH_POSITION_AND_POWER_DOWN(DEFAULT_WAIT_INTERVAL);

    }

	/* Handle the case that the switch is in CUSTOM position but the time has not been set */

	if (switchPosition == AM_SWITCH_CUSTOM
			&& (AudioMoth_hasTimeBeenSet() == false
					|| configSettings->activeStartStopPeriods == 0)) {

		FLASH_LED(Both, SHORT_LED_FLASH_DURATION)

		SAVE_SWITCH_POSITION_AND_POWER_DOWN(DEFAULT_WAIT_INTERVAL);

	}

	/* Calculate time of next recording if switch has changed position */

	uint32_t currentTime = AudioMoth_getTime();

	if (switchPosition != *previousSwitchPosition) {

		if (switchPosition == AM_SWITCH_DEFAULT) {

			/* Set parameters to start recording now */

			*timeOfNextRecording = currentTime;

			*durationOfNextRecording = configSettings->recordDuration;

		} else {

			/* Determine starting time and duration of next recording */

			scheduleRecording(currentTime, timeOfNextRecording,
					durationOfNextRecording);

		}

	}

	/* Make recording if appropriate */

	bool enableLED = (switchPosition == AM_SWITCH_DEFAULT)
			|| configSettings->enableLED;

	if (currentTime >= *timeOfNextRecording) {

		makeRecording(currentTime, *durationOfNextRecording, enableLED);

		if (switchPosition == AM_SWITCH_DEFAULT) {

			/* Set parameters to start recording after sleep period */

			if (!recordingCancelled) {

				*timeOfNextRecording = currentTime
						+ configSettings->recordDuration
						+ configSettings->sleepDuration;

			}

		} else {

			/* Determine starting time and duration of next recording */

			scheduleRecording(currentTime, timeOfNextRecording,
					durationOfNextRecording);

		}

	} else if (enableLED) {

		/* Flash LED to indicate waiting */

		FLASH_LED(Green, WAITING_LED_FLASH_DURATION)

	}

	/* Determine how long to power down */

	uint32_t secondsToSleep = 0;

	if (*timeOfNextRecording > currentTime) {

		secondsToSleep = MIN(*timeOfNextRecording - currentTime,
				WAITING_LED_FLASH_INTERVAL);

	}

	/* Power down */

	SAVE_SWITCH_POSITION_AND_POWER_DOWN(secondsToSleep);

}

/* AudioMoth handlers */

void AudioMoth_handleSwitchInterrupt() {

	recordingCancelled = true;

}

void AudioMoth_handleMicrophoneInterrupt(int16_t sample) {
}

void AudioMoth_handleDirectMemoryAccessInterrupt(bool primaryChannel,
		int16_t **nextBuffer) {

	/* Update the current buffer index and write buffer */

	writeBufferIndex += NUMBER_OF_SAMPLES_IN_DMA_TRANSFER;

	if (writeBufferIndex == NUMBER_OF_SAMPLES_IN_BUFFER) {

		writeBufferIndex = 0;

		writeBuffer = (writeBuffer + 1) & (NUMBER_OF_BUFFERS - 1);

	}

	/* Update the next buffer index and write buffer */

	int nextWriteBuffer = writeBuffer;

	int nextWriteBufferIndex = writeBufferIndex
			+ NUMBER_OF_SAMPLES_IN_DMA_TRANSFER;

	if (nextWriteBufferIndex == NUMBER_OF_SAMPLES_IN_BUFFER) {

		nextWriteBufferIndex = 0;

		nextWriteBuffer = (nextWriteBuffer + 1) & (NUMBER_OF_BUFFERS - 1);

	}

	dataCount++;

	/* Re-activate the DMA */

	*nextBuffer = buffers[nextWriteBuffer] + nextWriteBufferIndex;

}

void AudioMoth_usbApplicationPacketRequested(uint32_t messageType,
		uint8_t *transmitBuffer, uint32_t size) {

	/* Copy the current time to the USB packet */

	uint32_t currentTime = AudioMoth_getTime();

	memcpy(transmitBuffer + 1, &currentTime, 4);

	/* Copy the unique ID to the USB packet */

	memcpy(transmitBuffer + 5, (uint8_t*) AM_UNIQUE_ID_START_ADDRESS,
	AM_UNIQUE_ID_SIZE_IN_BYTES);

	/* Copy the battery state to the USB packet */

	AM_batteryState_t batteryState = AudioMoth_getBatteryState();

	memcpy(transmitBuffer + 5 + AM_UNIQUE_ID_SIZE_IN_BYTES, &batteryState, 1);

}

void AudioMoth_usbApplicationPacketReceived(uint32_t messageType,
		uint8_t* receiveBuffer, uint8_t *transmitBuffer, uint32_t size) {

	/* Copy the USB packet contents to the back-up register data structure location */

	memcpy(configSettings, receiveBuffer + 1, sizeof(configSettings_t));

	/* Copy the back-up register data structure to the USB packet */

	memcpy(transmitBuffer + 1, configSettings, sizeof(configSettings_t));

	/* Set the time */

	AudioMoth_setTime(configSettings->time);

}

/* Remove DC offset from the microphone samples */

static void filter(int16_t *data, int size) {

	/* Calculate the multiplier to normalise the volume across different over sampling rates */

	uint32_t multiplier = 16 / configSettings->oversampleRate;

	if (multiplier == 0) {
		multiplier = 1;
	}

	/* DC filter */

	int32_t filteredOutput;
	int32_t scaledPreviousFilterOutput;

	for (int i = 0; i < size; i++) {

		int16_t sample = multiplier * data[i];

		scaledPreviousFilterOutput = (int32_t) (0.995f * previousFilterOutput);

		filteredOutput = sample - previousSample + scaledPreviousFilterOutput;

		data[i] = (int16_t) filteredOutput;

		previousFilterOutput = filteredOutput;

		previousSample = (int32_t) sample;

	}

}

/* reset the numbers for acoustic indices every minute */
static void reset(float32_t* array, int size) {
	for (int i = 0; i < size; i++)
		array[i] = 0.0f;
}

static float32_t fasterlog2(float32_t x) {
	union {
		float32_t f;
		unsigned int i;
	} vx = { x };
	float32_t y = vx.i;
	y *= 1.1920928955078125e-7f;
	return y - 126.94269504f;
}

/* Save recording to SD card */

static void makeRecording(uint32_t currentTime, uint32_t recordDuration,
bool enableLED) {

	if (enableLED) {
		AudioMoth_setGreenLED(true);
	}

	/* Initialise buffers */

	writeBuffer = 0;

	writeBufferIndex = 0;

	recordingCancelled = false;

	buffers[0] = (int16_t*) AM_EXTERNAL_SRAM_START_ADDRESS;

	for (int i = 1; i < NUMBER_OF_BUFFERS; i += 1) {
		buffers[i] = buffers[i - 1] + NUMBER_OF_SAMPLES_IN_BUFFER;
	}

	float32_t * dataBuffer = (float32_t*) (AM_EXTERNAL_SRAM_START_ADDRESS
			+ 2 * NUMBER_OF_BUFFERS * NUMBER_OF_SAMPLES_IN_BUFFER);

	/* Switch to HFRCO */

	if (configSettings->clockBand < AM_HFXO) {

		AudioMoth_enableHFRCO(configSettings->clockBand);

		uint32_t clockFrequency = AudioMoth_getClockFrequency(
				configSettings->clockBand);

		uint32_t actualSampleRate = AudioMoth_calculateSampleRate(
				clockFrequency, configSettings->clockDivider,
				configSettings->acquisitionCycles,
				configSettings->oversampleRate);

		uint32_t targetFrequency = (float) clockFrequency
				* (float) configSettings->sampleRate / (float) actualSampleRate;

		AudioMoth_calibrateHFRCO(targetFrequency);

		AudioMoth_selectHFRCO();

	}

	/* Initialise microphone for recording */

	AudioMoth_enableExternalSRAM();

	AudioMoth_enableMicrophone(configSettings->gain,
			configSettings->clockDivider, configSettings->acquisitionCycles,
			configSettings->oversampleRate);

	AudioMoth_initialiseDirectMemoryAccess(buffers[0],
			buffers[0] + NUMBER_OF_SAMPLES_IN_DMA_TRANSFER,
			NUMBER_OF_SAMPLES_IN_DMA_TRANSFER);

	AudioMoth_startMicrophoneSamples();

	/* Initialise file system and open a new file */

	RETURN_ON_ERROR(AudioMoth_enableFileSystem());

	/* Open a file with the name as a UNIX time stamp in HEX */

	sprintf(fileName, "%08X.TXT", (unsigned int) currentTime);

	RETURN_ON_ERROR(AudioMoth_openFile(fileName));

	/* Main record loop */

	uint32_t buffersProcessed = 0;

	uint32_t readBuffer = writeBuffer;

	/* Initialise the rfft instance */
	arm_rfft_fast_init_f32(&S1, NUMBER_OF_SAMPLES_IN_DMA_TRANSFER); // size = 2*256

	/* Compute constants */
	segments_in_time_span = TIME_SPAN
			* configSettings->sampleRate / (2 * FREQ_BINS); //total number of segments in one whole time span
	segments_in_time_span_inverse = 1.0f / segments_in_time_span;
	segments_in_time_span_log2_inverse = 1.0f
			/ fasterlog2(segments_in_time_span);
	initial_segments_in_time_span = segments_in_time_span / 2; //initial time span is half size of a normal time span
	initial_segments_in_time_span_log2_inverse = 1.0f
			/ fasterlog2(initial_segments_in_time_span);

	int cycleCount = 0;

	int cycles_to_write_to_sd = recordDuration / TIME_SPAN;

	int floatsAdded = 0;

	while (!recordingCancelled && cycleCount < cycles_to_write_to_sd) {

		while (!recordingCancelled && readBuffer != writeBuffer) {

			/* DC filter the microphone samples */
			filter(buffers[readBuffer], NUMBER_OF_SAMPLES_IN_BUFFER);

			if (buffersProcessed >= NUMBER_OF_BUFFERS_TO_SKIP) {
				/* Convert to float32 */
				arm_q15_to_float(buffers[readBuffer], floatBufferReady,
						NUMBER_OF_SAMPLES_IN_BUFFER);

				/* Apply hamming filter */
				for (int i = 0; i < NUMBER_OF_SAMPLES_IN_BUFFER; i++) {
					floatBufferReady[i] *= hamming[i];
				}

				/* Compute fft and amplitudes */
				arm_rfft_fast_f32(&S1, floatBufferReady, fftBuffer, 0);
				arm_cmplx_mag_f32(fftBuffer, fftBuffer, FREQ_BINS);

				if (cycleCount == 0) { //for the very first time span
					if (dataCount <= initial_segments_in_time_span) { //for the initial time span, compute sum and sumSquared that are needed for cvr_noise
						for (int i = 0; i < FREQ_BINS; i++) {
							sum[i] += fftBuffer[i];
							sumSquared[i] += fftBuffer[i] * fftBuffer[i];
						}
					}
					if (dataCount == initial_segments_in_time_span) { //update cvr_noise when the initial time span is finished
						for (int i = 0; i < FREQ_BINS; i++) {
							aci_previous[i] = fftBuffer[i]; //initialise aci_previous
							float32_t mean = sum[i]
									* segments_in_time_span_inverse * 2.0f;
							cvr_noise[i] =
									mean
											+ NOISE_FACTOR
													* sqrtf(
															sumSquared[i]
																	* segments_in_time_span_inverse
																	* 2.0f
																	- mean
																			* mean);
						}
						//reset sum and sumSquared
						reset(sum, FREQ_BINS);
						reset(sumSquared, FREQ_BINS);
					}
					if (initial_segments_in_time_span <= dataCount
							&& dataCount < segments_in_time_span) { // for the remaining time in the first time span (the other half)
						for (int i = 0; i < FREQ_BINS; i++) {
							float32_t a_i = fftBuffer[i]; //current amplitude
							float32_t a_i_2 = a_i * a_i;  //squared amp

							aci_sumDiff[i] += fabsf(a_i - aci_previous[i]);

							float32_t factor_i = sumSquared[i]
									/ (sumSquared[i] + a_i_2);
							h_t_h[i] = h_t_h[i] * factor_i
									- (1 - factor_i) * fasterlog2(1 - factor_i);
							if (a_i - cvr_noise[i] > 0)
								cvr_count[i]++;

							aci_previous[i] = a_i;
							sum[i] += a_i;
							sumSquared[i] += a_i_2;
						}
					}
					if (dataCount == segments_in_time_span) { //when the first time span in total is finished (dataCount==segments_in_time_span), compute the indices
						for (int i = 0; i < FREQ_BINS; i++) {
							*dataBuffer++ = aci_sumDiff[i] / sum[i]; //aci
							*dataBuffer++ =
									h_t_h[i]
											* initial_segments_in_time_span_log2_inverse; //h_t
							*dataBuffer++ = cvr_count[i]
									* segments_in_time_span_inverse * 2.0f; //cvr
							*dataBuffer++ = sum[i]; //sum of ffts
							floatsAdded += N_INDICES;
						}
						/* Update cvr_noise that will be used for the next time_span */
						for (int i = 0; i < FREQ_BINS; i++) {
							float32_t mean = sum[i]
									* segments_in_time_span_inverse * 2.0f;
							cvr_noise[i] =
									mean
											+ NOISE_FACTOR
													* sqrtf(
															sumSquared[i]
																	* segments_in_time_span_inverse
																	* 2.0f
																	- mean
																			* mean);
						}
						reset(aci_sumDiff, FREQ_BINS);
						reset(sum, FREQ_BINS);
						reset(sumSquared, FREQ_BINS);
						reset(h_t_h, FREQ_BINS);
						reset(cvr_count, FREQ_BINS);
						dataCount = 0;
						cycleCount++;
					}
				} else { // for 2nd and later time spans (cycleCount>=1)
					for (int i = 0; i < FREQ_BINS; i++) {
						float32_t a_i = fftBuffer[i]; //current amplitude
						float32_t a_i_2 = a_i * a_i;  //squared amp

						aci_sumDiff[i] += fabsf(a_i - aci_previous[i]);

						float32_t factor_i = sumSquared[i]
								/ (sumSquared[i] + a_i_2);
						h_t_h[i] = h_t_h[i] * factor_i
								- (1 - factor_i) * fasterlog2(1 - factor_i);
						if (a_i - cvr_noise[i] > 0)
							cvr_count[i]++;

						aci_previous[i] = a_i;
						sum[i] += a_i;
						sumSquared[i] += a_i_2;
					}
					if (dataCount == segments_in_time_span) { //if the time span is completed, compute indices and write to ram
						for (int i = 0; i < FREQ_BINS; i++) {
							*dataBuffer++ = aci_sumDiff[i] / sum[i]; //aci
							*dataBuffer++ = h_t_h[i]
									* segments_in_time_span_log2_inverse; //h_t
							*dataBuffer++ = cvr_count[i]
									* segments_in_time_span_inverse; //cvr
							*dataBuffer++ = sum[i]; //sum of ffts
							floatsAdded += N_INDICES;
						}
						/* Update cvr_noise that will be used for the next time_span */
						for (int i = 0; i < FREQ_BINS; i++) {
							float32_t mean = sum[i]
									* segments_in_time_span_inverse;
							cvr_noise[i] =
									mean
											+ NOISE_FACTOR
													* sqrtf(
															sumSquared[i]
																	* segments_in_time_span_inverse
																	- mean
																			* mean);
						}
						reset(aci_sumDiff, FREQ_BINS);
						reset(sum, FREQ_BINS);
						reset(sumSquared, FREQ_BINS);
						reset(h_t_h, FREQ_BINS);
						reset(cvr_count, FREQ_BINS);
						dataCount = 0;
						cycleCount++;
					}
				}
				/* Write to SD card if max number is added to the buffer */
				if (floatsAdded > MAX_FLOATS_TO_WRITE - N_INDICES) {
					dataBuffer -= floatsAdded;
					AudioMoth_writeToFile(dataBuffer, floatsAdded * 4);
					floatsAdded = 0;
				}
			}

			readBuffer = (readBuffer + 1) & (NUMBER_OF_BUFFERS - 1);

			buffersProcessed++;

		}

		/* Sleep until next DMA transfer is complete */
		AudioMoth_sleep();
	}

	/* Write to SD card (also works if the recoding is cancelled) */

	if (enableLED) {
		AudioMoth_setRedLED(true);
	}

	dataBuffer -= floatsAdded;
	AudioMoth_writeToFile(dataBuffer, floatsAdded * 4);

	/* Close the file */

	RETURN_ON_ERROR(AudioMoth_closeFile());

	/* Clear LED */

	AudioMoth_setGreenLED(false);
	AudioMoth_setRedLED(false);

}

static void scheduleRecording(uint32_t currentTime,
		uint32_t *timeOfNextRecording, uint32_t *durationOfNextRecording) {

	/* No active periods */

	if (configSettings->activeStartStopPeriods == 0) {

		*timeOfNextRecording = UINT32_MAX;

		*durationOfNextRecording = configSettings->recordDuration;

		return;

	}

	/* Calculate the number of seconds of this day */

	time_t rawtime = currentTime;

	struct tm *time = gmtime(&rawtime);

	uint32_t currentSeconds = SECONDS_IN_HOUR * time->tm_hour
			+ SECONDS_IN_MINUTE * time->tm_min + time->tm_sec;

	/* Check each active start stop period */

	uint32_t durationOfCycle = configSettings->recordDuration
			+ configSettings->sleepDuration;

	for (uint32_t i = 0; i < configSettings->activeStartStopPeriods; i += 1) {

		startStopPeriod_t *period = configSettings->startStopPeriods + i;

		/* Calculate the start and stop time of the current period */

		uint32_t startSeconds = SECONDS_IN_MINUTE * period->startMinutes;

		uint32_t stopSeconds = SECONDS_IN_MINUTE * period->stopMinutes;

		/* Calculate time to next period or time to next start in this period */

		if (currentSeconds < startSeconds) {

			*timeOfNextRecording = currentTime
					+ (startSeconds - currentSeconds);

			*durationOfNextRecording = MIN(configSettings->recordDuration,
					stopSeconds - startSeconds);

			return;

		} else if (currentSeconds < stopSeconds) {

			uint32_t cycles = (currentSeconds - startSeconds + durationOfCycle)
					/ durationOfCycle;

			uint32_t secondsFromStartOfPeriod = cycles * durationOfCycle;

			if (secondsFromStartOfPeriod < stopSeconds - startSeconds) {

				*timeOfNextRecording = currentTime
						+ (startSeconds - currentSeconds)
						+ secondsFromStartOfPeriod;

				*durationOfNextRecording = MIN(configSettings->recordDuration,
						stopSeconds - startSeconds - secondsFromStartOfPeriod);

				return;

			}

		}

	}

	/* Calculate time until first period tomorrow */

	startStopPeriod_t *firstPeriod = configSettings->startStopPeriods;

	uint32_t startSeconds = SECONDS_IN_MINUTE * firstPeriod->startMinutes;

	uint32_t stopSeconds = SECONDS_IN_MINUTE * firstPeriod->stopMinutes;

	*timeOfNextRecording = currentTime + (SECONDS_IN_DAY - currentSeconds)
			+ startSeconds;

	*durationOfNextRecording = MIN(configSettings->recordDuration,
			stopSeconds - startSeconds);

}

static void flashLedToIndicateBatteryLife(void) {

	uint32_t numberOfFlashes = LOW_BATTERY_LED_FLASHES;

	AM_batteryState_t batteryState = AudioMoth_getBatteryState();

	/* Set number of flashes according to battery state */

	if (batteryState > AM_BATTERY_LOW) {

		numberOfFlashes = (batteryState >= AM_BATTERY_4V6) ? 4 :
							(batteryState >= AM_BATTERY_4V4) ? 3 :
							(batteryState >= AM_BATTERY_4V0) ? 2 : 1;

	}

	/* Flash LED */

	for (uint32_t i = 0; i < numberOfFlashes; i += 1) {

		FLASH_LED(Red, SHORT_LED_FLASH_DURATION)

		if (numberOfFlashes == LOW_BATTERY_LED_FLASHES) {

			AudioMoth_delay(SHORT_LED_FLASH_DURATION);

		} else {

			AudioMoth_delay(LONG_LED_FLASH_DURATION);

		}

	}

}

