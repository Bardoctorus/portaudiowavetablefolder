# Wavetable and Wavefolding Synth using Portaudio



Todo: 

- ~~Implement frequency~~
- ~~revert example to fixed phase increment, add frequency variable to paTestData struct~~
- ~~add other wave shape tables and hardcode blend sweep~~
- fix saw wave - the weighting doesn't make sense on edge cases. if you try to interpolate between a value near one and value near minus two you'll get an erroneous value

- implement basic triangle wavefolding
- Implement stereo detune
- Implement midi in
- Alternate control? Seperate python webserver for GUI?


Below is legacy scribblings
_________
# Then make another and do some wave table blending
If you make other wavetables for commons shapes, you can blend between them using exactly the same weighting idea as you need to use to increment using float values - except its slightly different. For 2 waves its as simple as multiplying one by a position and the other by 1 - it - e.g. if you want 3/4 of one wave you do thatWave\*0.75 and otherWave\*(1-0.75). For more waveshapes you'd need to introduce a weighting equation.


___
# Circular buffering

Adding to a buffer can't hurt as it'll give you the opportunity to add some simple delay effects for space. Potential for stereo delay amount as parameter


then look at midi maybe?

Waveshaping?
