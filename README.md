[![Test Build](https://github.com/edgeimpulse/example-standalone-inferencing-zephyr/actions/workflows/test-build.yml/badge.svg?branch=master)](https://github.com/edgeimpulse/example-standalone-inferencing-zephyr/actions/workflows/test-build.yml)

# nrf-blueberry
Edge impulse trained and deployed model running on Nordic device (zephyr environment, nRF Connect SDK v2.4.0)

| Compatible devices|
|---|
| nRF52840DK*|
| nRF5340DK|
> *should work, haven't tried. Overlay provided either way.


## description
! This sample is trained for me saying blueberry at my desk, and the only test sets for noise and unknown used for training this model were the stock ones provided by edge impulse !

![image](https://github.com/droidecahedron/nrf-blueberry/assets/63935881/c50cf519-e989-4f1c-b5d6-3b333c99abda)

![image](https://github.com/droidecahedron/nrf-blueberry/assets/63935881/41386a84-ff62-40e3-94d7-342d439fda71)

_(Example running static classification with raw features from my test set after training with my training set. You can grab this from the static_classification branch, and it should build out of the box.)_

### Example running with mic data, and I say blueberry
![image](https://github.com/droidecahedron/nrf-blueberry/assets/63935881/0bb2cd25-cc62-4bae-b91d-77560a41f891)


You can deploy your model as a C++ library, and work it into the boilerplate project listed in resources.

> (It may be tempting to just get the binary deployment for nrf5340DK + X-NUCLEO-IKS02A1.
> As of Aug 16, 2023 it puts my board into a boot loop.)

## resources used
Initial guide: https://docs.edgeimpulse.com/docs/tutorials/responding-to-your-voice

Overlays taken from: https://github.com/NordicPlayground/nRF-Beehavior-Firmware/tree/master/Legacy_Modules/ei_sample

Boilerplate project taken from: https://github.com/edgeimpulse/example-standalone-inferencing-zephyr

Audio classification guide from EI: https://docs.edgeimpulse.com/docs/tutorials/audio-classification

Continuous audio classification: https://docs.edgeimpulse.com/docs/tutorials/continuous-audio-sampling

## hardware / documentation
- nRF5340DK / [nRF5340 doc](https://infocenter.nordicsemi.com/topic/struct_nrf53/struct/nrf5340.html), [nRF5340DK doc](https://infocenter.nordicsemi.com/index.jsp?topic=%2Fstruct_nrf53%2Fstruct%2Fnrf5340.html)
- X-NUCLEO-IKS02A1 / [X-NUCLEO-IKS02A1 doc](https://www.st.com/en/ecosystems/x-nucleo-iks02a1.html#documentation)

<img src="https://github.com/droidecahedron/nrf-blueberry/assets/63935881/12612a0e-9f81-4431-8b22-f69704248f89" width=25% height=25%><img src="https://github.com/droidecahedron/nrf-blueberry/assets/63935881/0ff7470b-d5f0-46d7-bbbb-c867447e65c0" width=15% height=15%>

> (nice little sensor pack, slots right onto the ARD headers of the DK)


## overlay/hw pin notes from [a great blog](https://devzone.nordicsemi.com/nordic/nordic-blog/b/blog/posts/sound-and-edge-computing-using-the-nrf-52-53)
![image](https://github.com/droidecahedron/nrf-blueberry/assets/63935881/6ee4e732-e0ef-4d7e-8567-207b63f5ee98)

> The "C9" header on the shield corresponds to pins P1.01-P1.08 on the nRF52840 DK and P1.00-P1.09 (omitting P1.02 and P1.03) on the nRF5340 DK.

```
52840dk.overlay
&pinctrl {
    pdm0_default_alt: pdm0_default_alt {
        group1 {
            psels = <NRF_PSEL(PDM_CLK, 1, 4)>,
                <NRF_PSEL(PDM_DIN, 1, 5)>;
        };
    };
};
```

```
5340dk.overlay
&pinctrl {
	pdm0_default_alt: pdm0_default_alt {
		group1 {
			psels = <NRF_PSEL(PDM_CLK, 1, 5)>,
				<NRF_PSEL(PDM_DIN, 1, 6)>;
```


# Impulse design blocks
![image](https://github.com/droidecahedron/nrf-blueberry/assets/63935881/0a1a8ba8-1993-4ab2-9116-dedae96d9582)
