% Building and Deploying a Markdown Site
% Michal
% 16/5/2020

# Audio DSP on Arduino

#### Making each cycle of the 16Mhz clock count

# Introduction

An Arduino Uno can be picked up for under $10. It's a hobbyist board with an _ATMEGA16U2_ microcontroller, 32Kb of flash memory and a clock speed of 16MHz.

The 16MHz controller is nothing to rave about, but enough to do some basic realtime processing with audio. Audio can be sampled at as little as 8KHz which gives us 2000 cycles per sample to work with. For some simple tasks this is sufficient.

I deciced to start messing around in order to create a pretty light display using some audio input. Here's what I've learned.

# Audio Input

## The AUX Dream

The audio input would be best coming from a headphone jack. The arduino does not have one. That said, you can buy one for dirt cheap on most component retailers.

There is one issue. Headphone jacks provide 2.5V AC input (more if coming from an amplifier). The Arduino analog input pins use a 5V DC input. This makes the problem slightly more involved than just wiring up a headphone jack. We would only be able to detect the upper half of sound waves and would risk damaging the arduino with negative voltages.

To successfully take AUX input, we would need to add a DC component of 2.5V to the input. This can be done with a simple circuit using an op-amp, but that means you need to buy more stuff.

## The Cheaper (and Easier) Alternative

You can pick up an Arduino friendly KY-038, big sound, sensor. This a small microphone with adjustable potentiometer. Its not perfect and picks up interferance (like people talking or droping things). However, when placed very close to the speaker, it works very well. Since I already had one from an earlier project, and I could not be bothered to mess with the op-amp DC component, I decided to stick with the KY-038.

Its super easy to wire up. Takes a 3.3V input, ground and has either a digital or an analog output. We have to use the analog output obviously.

The potentiometer can be adjusted to zero the analog ouput at 500 such that input values range between 0 and 1000.

These can then be trivially converted to a floating point value: `float sound_value = (float)(analogRead(A0)-500)/1000;`

# Output

The output is less constrained than input, pretty much anything could work. I settled for a SMD LED module (KY-009) since it can be used to blend various colours together.

The next step is to get addressable LED strips which would make for more interesting output.

# Method

## Development Environment

I know people like using the IDE that ships with Arduino. Personally, I hate it. The language is limited to just C and the IDE is super annoying. I recommend using [PlatformIO](https://platformio.org/). It's a free toolkit that lets to compile and deploy code onto various microcontrollers without being stuck using a crap editor. It additionally lets you write C++.

## Sampling

First, we decide on a sampling frequency. Initially I used 8KHz but soon found out that 16KHz was doable too.

The analog input signal must be converted into a digital one. This is achieved by sampling which is a regular storing of signal amplitude. Higher sampling frequency allows for detecting higher frequency sound waves with higher accuracy. For music this needs to be at least 8KHz (allowing us to detect up to 4KHz sound waves).

### Realtime

Since we want realtime, we have to start the output as soon as we start the input. The time taken to start outputting since the first input is called latency. Latency of 0 is impossible, but we should aim for as low as possible. 10ms is the holy grail.

To achieve good latency we have to process the input in chunks (frames). The size of the frame plays a direct role on our latency.

Sampling at 16KHz for 10ms results in 160 samples. Of course there is additionall latency in processing of the input so to achieve a latency of 10ms we stictly have to store less thant 160 samples before we start processing.

This is a tricky science full of tweaking and adjusting various constants around to achieve the best results. Fortunately, the FFT library requires the frame size to be a power of 2. This means the closest we can get to 160 is 128 which gives us around 2ms (32000 ticks at 16MHz) to process our samples and output them.

After every 8ms we will have a full buffer of 128 readings. These are just readings of the sound wave amplitude which can be used to guage volume. Not very useful since recorded volume depends on how close the speaker is to the microphone.

Sampling code is essentially something like this.

```
float buffer[FRAME_SIZE];
int buf_idx = 0;
int total_ticks = 0;
int last_ticks = milis();

void loop() {
  int ticks = milis(); // miliseconds since arduino start
  total_ticks += ticks-last_ticks;
  last_ticks = ticks;

  if (total_ticks > 1000000/SAMPLE_RATE && buf_idx < FRAME_SIZE) {
    buffer[buf_idx++] = (float)(analogRead(A0)-500)/1000;
  } else if (buf_idx == FRAME_SIZE) {
    process_buffer();
    buf_idx = 0;
  }
}

```

## Spectral Analysis

### Discrete Frequency Reponse

The buffer of samples can be called a discrete time response. For each time reading we have a value. This is useful if we wished to convert the signal back into analog. However, we want pretty lights generated from the sound. Unless we want to go on just volume, we need to disect this wave into a series of waves of different frequencies (sound wave frequencies determine their pitch).

The natural way to achieve this is to perform a _"Fourier Transform"_ on our discrete time series. The _"Fourier Transform"_ can be used to convert any periodic function into a sum of sines and cosines with various amplitudes and phases. In real life this maps to disecting a wave into multiple other waves.

The _"Fourier Transform"_ can turn any continous function into multiple functions. Our frame is discrete so we have to use a _"Discrete Fourier Transform"_. There is a very famous algorithm which does this for us called the _"Fast Fourier Transform"_ (FFT for short). It's already implemented for us and open-sourced as [arduinoFFT](https://github.com/kosme/arduinoFFT).

The output of the algorithm is a series of amplitudes of various frequency waves - _also known as a frequency response_. This tells us how much bass, vocals, high pitched sounds we have in our frame. If we were developing a digital filter, we could then supress some frequencies and boost others before converting the result back into a discrete time response for playback.

Human hearing ranges from 20Hz to around 20KHz, but the most important parts are between 250Hz and 4KHz. We can sample waves up to 8KHz with our 16KHz sampling rate.

* Lower frequencies are bass, generally 250-500Hz. Under 250 is known as sub-bass (which we feel more than hear).
* Frequencies under 2000 are known as middle range - we are more sensitive to these thant the bass ones.
* Frequencies between 2000 and 4000 are upper-middle range which we are most sensitive to.

You can read more about this [here](https://www.teachmeaudio.com/mixing/techniques/audio-spectrum).

Any frame will contain a mixture of all of these in varying quantities...

This is how you can use arduinoFFT to get a frequency response:

```
float buffer_img[FRAME_SIZE]; // needed as FFT input and output are complex numbers.

arduinoFFT fft();
fft->Windowing(buffer, FRAME_SIZE, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
fft->Compute(buffer, buffer_img, FRAME_SIZE, FFT_FORWARD);
fft->ComplexToMagnitude(buffer, buffer_img, FRAME_SIZE);

// buffer[0] to buffer[FRAME_SIZE/2] is now the frequency amplitudes

```

### Calculating Colours

Since I have already divided the frequencies into 3 main types, I can use accumulate the frequency readings in those ranges into 3 bins: blue, green and red.

I will additionally amplify and de-amplify these to account for our sensitivity to them: 

```
float bins[3] = {0};
for (int i = 2; i < FRAME_SIZE/2; i++) {
  if (buffer[i] < 0.01) continue; // too low to count

  //  calculate frequency basing on index
  //  (0.5*sample_rate)/(0.5*frame_size) = sample_rate/frame_size
  const int frequency = i*(float)SAMPLE_RATE/FRAME_SIZE;

  if (frequency < 450) bins[0] += buffer[i] * 0.5; // blue bin
  else if (frequency < 2000) bins[1] += buffer[i] * 2; // green bin
  else if (frequency < 4000) bins[2] += buffer[i] * 3; // red bin
}

float bluef = bins[0];
float greenf = bins[1];
float redf = bins[2];

// normalize values
normalize(redf, greenf, bluef);

// output in range of 0-255
uint8_t red = (uint8_t)(redf*255);
uint8_t green = (uint8_t)(greenf*255);
uint8_t blue = (uint8_t)(bluef*255);
```

The normalize function is very basic:

```
#define max(a,b) ((a)>(b)?(a):(b))

void normalize(float& r, float& g, float& b) {
  float maximum = max(r, max(g, b));
  r = r/maximum;
  g = g/maximum;
  b = b/maximum;
}
```

And there you have it!

The next steps would be to get in the headphone jack working and outputting into something better than just a single LED. For example an addressable LED strip would allow for displaying an entire frequency band with each LED being responsible for a different frequency range.

As always, project is open sourced [here](https://github.com/Dachckol/music_led).
