#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

typedef float float32_t;
typedef struct {
	float Re;
	float Im;
} complex;

#define PI 			3.14159265358979323846264338f
#define AMP_THR 	1.2589f 	//amplitude equivalent to 2 dB
#define FREQ_BINS 	256
#define N_INDICES	4			//number of indices to write to sd card

/* buffer variables */
float floatBuffer[2 * FREQ_BINS]; //convert the int16 in DMA to float32
float fftBuffer[2 * FREQ_BINS]; //used to store fft values
float * dataBuffer;

static const float hamming[2 * FREQ_BINS] =  //hamming filter
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

float aci_previous[FREQ_BINS], aci_sumDiff[FREQ_BINS], sum[FREQ_BINS]; //numbers for aci
float h_t_h[FREQ_BINS], sumSquared[FREQ_BINS]; //numbers for h_t
float cvr_noise[FREQ_BINS], cvr_count[FREQ_BINS]; //numbers for cvr

int sampleRate = 46511;
int time_span = 1; //time span to update the numbers (in seconds)
int cycles_to_write_to_sd = 30; //numbers of time spans before writing to sd card
int segments_in_time_span; // number of segments in a time_span
int initial_segments_in_time_span; // number of segments in the very first time_span = 1/2 *segments_in_time_span
float segments_in_time_span_inverse, segments_in_time_span_log2_inverse,
		initial_segments_in_time_span_log2_inverse;

/*
 fft(v,N):
 [0] If N==1 then return.
 [1] For k = 0 to N/2-1, let ve[k] = v[2*k]
 [2] Compute fft(ve, N/2);
 [3] For k = 0 to N/2-1, let vo[k] = v[2*k+1]
 [4] Compute fft(vo, N/2);
 [5] For m = 0 to N/2-1, do [6] through [9]
 [6]   Let w.re = cos(2*PI*m/N)
 [7]   Let w.im = -sin(2*PI*m/N)
 [8]   Let v[m] = ve[m] + w*vo[m]
 [9]   Let v[m+N/2] = ve[m] - w*vo[m]
 */
void fft(complex *v, int n, complex *tmp) {
	if (n > 1) { /* otherwise, do nothing and return */
		int k, m;
		complex z, w, *vo, *ve;
		ve = tmp;
		vo = tmp + n / 2;
		for (k = 0; k < n / 2; k++) {
			ve[k] = v[2 * k];
			vo[k] = v[2 * k + 1];
		}
		fft(ve, n / 2, v); /* FFT on even-indexed elements of v[] */
		fft(vo, n / 2, v); /* FFT on odd-indexed elements of v[] */
		for (m = 0; m < n / 2; m++) {
			w.Re = cosf(2 * PI * m / n);
			w.Im = -sinf(2 * PI * m / n);
			z.Re = w.Re * vo[m].Re - w.Im * vo[m].Im; /* Re(w*vo[m]) */
			z.Im = w.Re * vo[m].Im + w.Im * vo[m].Re; /* Im(w*vo[m]) */
			v[m].Re = ve[m].Re + z.Re;
			v[m].Im = ve[m].Im + z.Im;
			v[m + n / 2].Re = ve[m].Re - z.Re;
			v[m + n / 2].Im = ve[m].Im - z.Im;
		}
	}
	return;
}

void rfft(float* in, float* out) { //size = 2*FREQ_BINS
	int size = 2 * FREQ_BINS;
	complex v[size], scratch[size];
	for (int k = 0; k < size; k++) {
		v[k].Re = in[k];
		v[k].Im = 0;
	}
	fft(v, size, scratch);
	for (int k = 0; k < size / 2; k++)
		out[k] = sqrtf(v[k].Re * v[k].Re + v[k].Im * v[k].Im);
}

/* reset the numbers for acoustic indices every minute */
void reset(float* array, int size) {
	for (int i = 0; i < size; i++)
		array[i] = 0.0f;
}

float fastlog2(float x) {
	union {
		float f;
		unsigned int i;
	} vx = { x };
	union {
		unsigned int i;
		float f;
	} mx = { (vx.i & 0x007FFFFF) | 0x3f000000 };
	float y = vx.i;
	y *= 1.1920928955078125e-7f;

	return y - 124.22551499f - 1.498030302f * mx.f
			- 1.72587999f / (0.3520887068f + mx.f);
}

int main(void) {

	dataBuffer = malloc(240 * 1024);

	FILE * file = fopen(
			"C:\\Users\\kwea123\\Desktop\\Acoustic Indices\\NewForestBee", "rb");
	if (!file) {
		printf("Unable to open file!");
		return 1;
	}

	// obtain file size:
	fseek(file, 0, SEEK_END);
	long f_size = ftell(file);
	rewind(file);

	// allocate memory to contain the whole file:
	float * bufferReady = (float*) malloc(sizeof(float) * f_size);
	if (bufferReady == NULL) {
		fputs("Memory error", stderr);
		exit(2);
	}

	// copy the file into the buffer:
	size_t result = fread(bufferReady, sizeof(float), f_size / sizeof(float),
			file);
	if (result != f_size / sizeof(float)) {
		fputs("Reading error", stderr);
		exit(3);
	}

	/* Compute constants */
	segments_in_time_span = time_span * sampleRate / (2 * FREQ_BINS); //total number of segments in one whole time span
	segments_in_time_span_inverse = 1.0f / segments_in_time_span;
	segments_in_time_span_log2_inverse = 1.0f / fastlog2(segments_in_time_span);
	initial_segments_in_time_span = segments_in_time_span / 2; //initial time span is half size of a normal time span
	initial_segments_in_time_span_log2_inverse = 1.0f
			/ fastlog2(initial_segments_in_time_span);

	int cycleCount = 0;
	int dataCount = 0;
	int cycles_to_write_to_sd = 30;

	while (cycleCount < cycles_to_write_to_sd) {
		for (int i = 0; i < 2 * FREQ_BINS; i++)
			floatBuffer[i] = bufferReady[i + cycleCount * sampleRate
					+ dataCount * 2 * FREQ_BINS];
		dataCount++;
		for (int i = 0; i < 2 * FREQ_BINS; i++)
			floatBuffer[i] *= hamming[i];
		rfft(floatBuffer, fftBuffer);

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
					float32_t mean = sum[i] * segments_in_time_span_inverse
							* 2.0f;
					cvr_noise[i] =
							mean
									+ 0.1f
											* sqrtf(
													sumSquared[i]
															* segments_in_time_span_inverse
															* 2.0f
															- mean * mean);
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
					float32_t factor2_i = (factor_i - 0.5f) * (factor_i - 0.5f);
					float32_t factor4_i = factor2_i * factor2_i;
					h_t_h[i] = h_t_h[i] * factor_i
							- 96.0f * factor4_i * factor4_i - 2.5f * factor2_i
							+ 1.0005f; // approx by a degree 8 polynomial err<=0.004 between 0.998 and 1
					// h_t_true ~= h_t_approx * 1.13043478f (1.3/1.15)
					if (a_i - cvr_noise[i] > AMP_THR)
						cvr_count[i]++;

					aci_previous[i] = a_i;
					sum[i] += a_i;
					sumSquared[i] += a_i_2;
				}
			}
			if (dataCount == segments_in_time_span) { //when the first time span in total is finished (dataCount==segments_in_time_span), compute the indices
				for (int i = 0; i < FREQ_BINS; i++) {
					*dataBuffer++ = aci_sumDiff[i] / sum[i]; //aci
					*dataBuffer++ = h_t_h[i]
							* initial_segments_in_time_span_log2_inverse; //h_t
					*dataBuffer++ = cvr_count[i] * segments_in_time_span_inverse
							* 2.0f; //cvr
					*dataBuffer++ = sum[i]; //sum of ffts
				}
				/* Update cvr_noise that will be used for the next time_span */
				for (int i = 0; i < FREQ_BINS; i++) {
					float32_t mean = sum[i] * segments_in_time_span_inverse
							* 2.0f;
					cvr_noise[i] =
							mean
									+ 0.1f
											* sqrtf(
													sumSquared[i]
															* segments_in_time_span_inverse
															* 2.0f
															- mean * mean);
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

				float32_t factor_i = sumSquared[i] / (sumSquared[i] + a_i_2);
				float32_t factor2_i = (factor_i - 0.5f) * (factor_i - 0.5f);
				float32_t factor4_i = factor2_i * factor2_i;
				h_t_h[i] = h_t_h[i] * factor_i - 96.0f * factor4_i * factor4_i
						- 2.5f * factor2_i + 1.0005f; // approx by a degree 8 polynomial err<=0.004 between 0.998 and 1
				// h_t_true ~= h_t_approx * 1.13043478f (1.3/1.15)
				if (a_i - cvr_noise[i] > AMP_THR)
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
				}
				/* Update cvr_noise that will be used for the next time_span */
				for (int i = 0; i < FREQ_BINS; i++) {
					float32_t mean = sum[i] * segments_in_time_span_inverse;
					cvr_noise[i] =
							mean
									+ 0.1f
											* sqrtf(
													sumSquared[i]
															* segments_in_time_span_inverse
															- mean * mean);
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
	}

	// terminate
	fclose(file);
	free(bufferReady);

	file = fopen("C:\\Users\\kwea123\\Desktop\\Acoustic Indices\\NewForestBee_C",
			"wb");
	int n_elements = N_INDICES * FREQ_BINS * cycles_to_write_to_sd;
	dataBuffer -= n_elements;
	fwrite(dataBuffer, sizeof(float), n_elements, file);

	// terminate
	fclose(file);

	puts("SUCCESS");

	return EXIT_SUCCESS;
}
