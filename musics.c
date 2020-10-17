#include <stdio.h>
#include <stdlib.h>
#include <math.h>

typedef short TYPE; //having TYPE as char or signed char doesn't seem to work. ? Dunno why.
TYPE TYPE_MAX = ~(TYPE)(1 << (sizeof(TYPE) * 8 - 1));

const int i = 1;
#define Endianness() ((*(char*)&i) == 0)
#define Big_Endian 1
#define Little_Endian 0

void write(FILE* fd, void* data, unsigned int size, int endianness) {
	char *buff = (char*)data;
	if(Endianness() == endianness) {
		for(int i = 0; i < size; i++) {
			putc(buff[i], fd);
		}
	} else {
		for(int i = size - 1; i >= 0; i--) {
			putc(buff[i], fd);
		}
	}
}

void wuil(FILE* fd, unsigned int data) {
	write(fd, &data, sizeof(unsigned int), Little_Endian);
}

void wusl(FILE* fd, unsigned short data) {
	write(fd, &data, sizeof(unsigned short), Little_Endian);
}

void generateSin2(TYPE* data, int size, unsigned int sampleRate, float freq) {
	float div = size / 4.f;
	float div2 = size / 44.1f;
	for(int i = 0; i < size; i++) {
		float d = pow(M_E, -(i/div));
		float a = log((i+div2)/div2);
		data[i] = (TYPE_MAX * sin(i * (M_PI / (float)sampleRate) * freq) * (d < a ? d : a)); 
	}
}

void generateSquare(TYPE* data, int size, unsigned int sampleRate, float freq) {
	for(int i = 0; i < size; i++) {
		data[i] = TYPE_MAX * (sin(i * (M_PI / (float)sampleRate) * freq) < 0.f ? -1.f : 1.f);
	}
}

void generateSin(TYPE* data, int size, unsigned int sampleRate, float freq) {
	for(int i = 0; i < size; i++) {
		data[i] = (TYPE_MAX * sin(i * (M_PI / (float)sampleRate) * freq));
	}
}

float noteFreq(char c, int offset) {
	char step[7] = {0, 2, 3, 5, 7, 8, 10};
	return 27.5f * pow(1.059463094359, step[c - 'a'] + offset);
}

float* combFilter(TYPE* data, int size, float msDelay, float decayFactor, unsigned int sampleRate) {
	float* sample = (float*)malloc(size * sizeof(float));
	int delay = (int)((float)msDelay * (sampleRate / 1000.f));
	for(int i = 0; i < size; i++) {
		sample[i] = (float)data[i];
	}
	for(int i = 0; i < size - delay; i++) {
		sample[i+delay] += (sample[i] * decayFactor);
	}
	return sample;
}

float* allPassFilter(float* data, int size, unsigned int sampleRate) {
	float* sample = (float*)malloc(size * sizeof(float));
	int delay = (int)((float)89.27f * (sampleRate / 1000.f));
	float decayFactor = 0.131f;
	for(int i = 0; i < size; i++) {
		sample[i] = data[i];
		if(i - delay >= 0) {
			sample[i] += decayFactor * sample[i-delay];
		}
		if(i - delay >= 1) {
			sample[i] += decayFactor * sample[i + 20 - delay];
		}
	}
	float value = sample[i];
	float max = 0.f;
	for(int i = 0; i < size; i++) {
		float tmp = fabs(sample[i]);
		if(tmp > max) {
			max = tmp;
		}
	}
	for(int i = 0; i < size; i++) {
		value = ((value + (sample[i] - value))/max);
		sample[i] = value;
	}
	return sample;
}

void ADSR(TYPE* data, int size, unsigned int at, unsigned int dt, float s, unsigned int rt) {
	float ls;
	for(int i = 0; i < size; i++) {
		float k;
		if(i > size - rt) {
			k = ls * ((size - i) / (float)rt);
		} else if(i < at) {
			k = (i / (float)at);
			ls = k;
		} else if(i < at + dt) {
			k = 1.f - ((i - at) / (float)dt) * (1.f - s);
			ls = k;
		} else {
			k = s;
			ls = k;
		}
		data[i] *= k;
	}
}

unsigned int getNumber() {
	unsigned int retVal = 0;
	char c;
	while((c = getchar()) >= '0' && c <= '9') {
		retVal = retVal * 10 + (c - '0');
	}
	ungetc(c, stdin);
	return retVal;
}

int main(int argc, char* argv[]) {

	printf("Generating file...\nBit Depth:\t%d\nTYPE_MAX:\t%d\n", sizeof(TYPE), TYPE_MAX);

	char notes[100];
	unsigned int length[100];
	char offset[100];
	unsigned int tempo = 60;
	int noteCount = 0;
	char c;
	char octave = 0;
	char sign = 0;
	unsigned int currLength = ((44100 * 240) / tempo) / 4;
	unsigned int dataSize = 0;
	unsigned int voice, at, dt, rt;
	float s;
	float msDelay, decayFactor, mix;
	scanf("%u %u %u %f %u %f %f %f", &voice, &at, &dt, &s, &rt, &msDelay, &decayFactor, &mix);
	printf("%u, %u, %u, %f, %u\n%f, %f, %f\n", voice, at, dt, s, rt, msDelay, decayFactor, mix);
	while((c = getchar()) != '.' && noteCount < 100) {
		if(c >= '`' && c <= 'g') {
			length[noteCount] = currLength;
			offset[noteCount] = sign + octave * 12;
			notes[noteCount++] = c;
			dataSize += currLength;
			sign = 0;
		} else if(c == 'l') {
			currLength = ((44100 * 240) / (tempo)) / getNumber();
		} else if(c == 'o') {
			octave = getNumber();
		} else if(c == '-') {
			sign = -1;
		} else if(c == '+') {
			sign = 1;
		} else if(c == 't') {
			tempo = getNumber();
		} else if(c == '<') {
			if(octave > 0) {
				octave--;
			}
		} else if(c == '>') {
			octave++;
		}
	}

	TYPE *data = (TYPE*)malloc(dataSize * sizeof(TYPE));

	void (*generateVoice[2])(TYPE*, int, unsigned int, float) = { &generateSin, &generateSquare };

	unsigned int noteStart = 0;
	for(int i = 0; i < noteCount; i++) {
		char note = notes[i];
		if(note - '`') {
			(*generateVoice[voice])(&data[noteStart], length[i], 44100, noteFreq(note, offset[i]));
			ADSR(&data[noteStart], length[i], at, dt, s, rt);
		}
		noteStart += length[i];
	}

	float* t1 = combFilter(data, dataSize, msDelay, decayFactor, 44100);
	float* t2 = combFilter(data, dataSize, msDelay - 11.73f, decayFactor - 0.1313f, 44100);
	float* t3 = combFilter(data, dataSize, msDelay + 19.31f, decayFactor - 0.2743f, 44100);
	float* t4 = combFilter(data, dataSize, msDelay - 7.97f, decayFactor - 0.31f, 44100);

	for(int i = 0; i < dataSize; i++) {
		t1[i] = (100.f - mix) * (t1[i] + t2[i] + t3[i] + t4[i]) + mix * (float)data[i];
	}

	free(t2);
	free(t3);
	free(t4);

	t2 = allPassFilter(t1, dataSize, 44100);
	t3 = allPassFilter(t2, dataSize, 44100);

	for(int i = 0; i < dataSize; i++) {
		data[i] = TYPE_MAX * t3[i];
	}

	free(t2);
	free(t3);

	char* filename = "wavefile.wav";
	if(argc == 2) {
		filename = argv[1];
	}

	FILE* fd = fopen(filename, "wb");
	
	//RIFF Chunk
	fprintf(fd, "RIFF");
	wuil(fd, 36 + dataSize * sizeof(TYPE));
	fprintf(fd, "WAVE");

	//fmt Chunk
	fprintf(fd, "fmt ");
	wuil(fd, 16);		//16 for PCM data (Size of the rest of the Subchunk which follows this number)
	wusl(fd, 1);		//PCM = 1 (other values indicate some form of compression)
	wusl(fd, 1);		//Number of Channels
	wuil(fd, 44100);		//Sample Rate
	wuil(fd, 44100 * 1 * sizeof(TYPE));	//ByteRate = Sample Rate * Number of Channels * Bits Per Sample / 8
	wusl(fd, 1 * sizeof(TYPE));		//BlockAlign = Number of Channels * Bits Per Sample / 8
	wusl(fd, sizeof(TYPE) * 8);		//Bits Per Sample

	//data Chunk
	fprintf(fd, "data");
	wuil(fd, dataSize * 1 * sizeof(TYPE));		//Subchunk2Size = numSamples * numChannels * bitsPerSample/8
	for(int i = 0; i < dataSize; i++) {
		write(fd, &data[i], sizeof(TYPE), Little_Endian);
	}

	free(data);

	fclose(fd);

	return 0;
}	
