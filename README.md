# Wavetable and Wavefolding Synth using Portaudio



Todo: 

- ~~Implement frequency~~
- ~~revert example to fixed phase increment, add frequency variable to paTestData struct~~
- add other wave shape tables and hardcode blend sweep
- implement basic triangle wavefolding
- Implement stereo detune
- Implement midi in
- Alternate control? Seperate python webserver for GUI?


Below is legacy scribblings
_________
# Then make another and do some wave table blending
If you make other wavetables for commons shapes, you can blend between them using exactly the same weighting idea as you need to use to increment using float values


___
# Circular buffering

Adding to a buffer can't hurt as it'll give you the opportunity to add some simple delay effects for space. Potential for stereo delay amount as parameter


then look at midi maybe?

Waveshaping?