# Beamformer project

This is a simple beamformer visualization project. First, a virtual target is created some 10 meters away from the sensor array.
The sound propagation from the target is simulated to the microphone array. Then the microphone signals are used for beamforming,
and the pattern is vialualized to localize the original source direction.

Some special maths were invented to make this run on a regular CPU.

## Building and running the beamformer visualization

This project uses JUCE for the beamformer visualization.
To build the project, make sure you have the necessary C++ build tools
and Cmake installed. To build the project from a fresh clone, run:

```console
git submodule init
git submodule update
cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo -S . -B build
cmake --build build --config RelWithDebInfo
```

On Windows, run the project with:
```console
.\wrun.bat
```

On other platforms, use:
```console
./run.sh
```

## Using the beamformer plugin

Clicking and dragging the beamformer visualization moves the target around.
The rest of the UI is hopefully self-explanatory.

## How it works

To make the computations feasible on a regular CPU, the following approach was developed:

1. Use one-pole resonators to separate the incoming signal into complex-valued frequency bands. Do this for all microphones. Fill the relevant parts of the spectrum by exponentially increasing center frequency of the next filter in the filter bank.
2. For each frequency band, compute the angluar velocity of the complex filter value. Average over all the microphones.
3. Use the angular velocity to average the phase of the frequency band over time. Use a leaky integrator for the averaging and calibrate it so that the average is representative of the one-frame time-frame when we visualize the beampatterns.
4. Also average the angular velocities over time.
5. Conver the average angular velocities to wavenumbers, and use them for beamforming with the averaged filter phases.

With this, the processing of the filter is O(fs * microphones * filters) with a very good constant coefficient,
while the beamforming is O(sampled_directions * microphones * filters).

## Running the jupyter notebooks

There are some jupyter notebooks that I used for prototyping and designing the system.
If you are interested, you can check them out too!

Make sure you have python and the anaconda/miniconda environment installed. You also
need a host for the notebook, like JupyterLab or the Jupyter plugin VS code.

Create the environment for the notebooks:

```console
conda env create -f dsp_env.yml
```

On windows, you need to use the special "anaconda prompt" console to create the environment.

Once the environment is ready, run the notebooks in your chosen host.

## Next steps

- The implementation does not currently account for the frequency dependent main lobe width. Use a pure sine in the beamformer to see this in action. Windowing for the high frequencies and some postprocessing for the low frequencies is needed.
