
#include "inc/mic.h"

//Old stuff from static classification
/*
#include <zephyr/kernel.h>
#include "edge-impulse-sdk/classifier/ei_run_classifier.h"
#include "edge-impulse-sdk/dsp/numpy.hpp"
#include <nrfx_clock.h>
*/

#define FEATURE_BUFSEL_PING 1
#define FEATURE_BUFSEL_PONG 2

LOG_MODULE_REGISTER(MAINCPP);

static int64_t sampling_freq = EI_CLASSIFIER_FREQUENCY; // in Hz.
static int64_t time_between_samples_us = (1000000 / (sampling_freq - 1));

static const float features[] = {
    // copy raw features here (for example from the 'Live classification' page)
    // see https://docs.edgeimpulse.com/docs/running-your-impulse-locally-zephyr
    //blueberry
};

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

/* Edge Impulse results ready. */
static void result_ready_cb(int err)
{
	if(err){
		LOG_INF("Result ready callback error: %d", err);
		return;
	}

	const char *label;
	float value;
	size_t inx;

	while(true){
		err = ei_wrapper_get_next_classification_result(&label, &value, &inx);

		if(err){
			LOG_INF("Unable to get next classification result: %d", err);
			break;
		}
		if(inx == 2){

			LOG_INF("blueberry probabilaty: %f", value);
			break;
            
		}
	}
	if(!err){
		if(ei_wrapper_classifier_has_anomaly()){
			float anomaly;
			err = ei_wrapper_get_anomaly(&anomaly);
		}
	}

	bool cancelled;
	err = ei_wrapper_clear_data(&cancelled);
	if(err){
		LOG_INF("Unable to clear data: %i", err);
	}
}

static int do_pdm_transfer(const struct device *dmic_dev,
			   struct dmic_cfg *cfg,
			   size_t block_count)
{
	int ret;

	LOG_INF("Starting sampling:");

	/* Start the microphone. */
	ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_START);
	if (ret < 0) {
		LOG_INF("START trigger failed: %d", ret);
		return ret;
	}
	
	for (int i = 0; i < 11; ++i) {
		void *buffer;
		uint32_t size;

		ret = dmic_read(dmic_dev, 0, &buffer, &size, READ_TIMEOUT);
		if (ret < 0) {
			LOG_INF("Read failed: %d", ret);
			break;
		}

		/* Discard first readout due to microphone needing to 
		 * stabilize before valid data can be read. */
		if(i!=0){
			int16_t tempInt;
			float tempFloat;
			for(int j=0; j<1600; j++){
				memcpy(&tempInt, buffer + 2*j, 2);
				tempFloat = (float)tempInt;
				audio_buf_ptr[(i-1)*1600+j] = tempFloat;
			}
		}
		k_mem_slab_free(&mem_slab, &buffer);
	}

	/* Stop the microphone. */
	ret = dmic_trigger(dmic_dev, DMIC_TRIGGER_STOP);
	if (ret < 0) {
		return ret;
	}

	/* Give the microphone data to Edge Impulse. */
	ret = ei_wrapper_add_data(audio_buf_ptr, ei_wrapper_get_window_size());
	if (ret) {
		LOG_INF("Cannot provide input data (err: %d)\n", ret);
		LOG_INF("Increase CONFIG_EI_WRAPPER_DATA_BUF_SIZE\n");
	}
	ei_wrapper_start_prediction(0,0);

	return 0;
}


int main()
{
	int ret;
	ret = ei_wrapper_init(result_ready_cb);

	if(ret){
		LOG_ERR("EI wrapper failed to init: %i", ret);
	}
    k_sleep(K_SECONDS(15));
	
	const struct device *dmic_dev = DEVICE_DT_GET(DT_NODELABEL(dmic_dev));
	LOG_INF("DMIC module");
	if (!device_is_ready(dmic_dev)) {
		LOG_ERR("%s is not ready", dmic_dev->name);
		return -1;
	}

	/* Configuration of the microphone. */
	struct pcm_stream_cfg stream = {
		.pcm_width = SAMPLE_BIT_WIDTH,
		.mem_slab  = &mem_slab,
	};
	struct dmic_cfg cfg = {
		.io = {
			/* These fields can be used to limit the PDM clock
			 * configurations that the driver is allowed to use
			 * to those supported by the microphone.
			 */
			.min_pdm_clk_freq = 1100000,
			.max_pdm_clk_freq = 3500000,
			.min_pdm_clk_dc   = 40,
			.max_pdm_clk_dc   = 60,
		},
		.streams = &stream,
		.channel = {
			.req_num_streams = 1,
		},
	};

	cfg.channel.req_num_chan = 1;
	cfg.channel.req_chan_map_lo =
		dmic_build_channel_map(0, 0, PDM_CHAN_LEFT);
	cfg.streams[0].pcm_rate = MAX_SAMPLE_RATE;
	cfg.streams[0].block_size =
		BLOCK_SIZE(cfg.streams[0].pcm_rate, cfg.channel.req_num_chan);


	cfg.channel.req_num_chan = 2;
	cfg.channel.req_chan_map_lo =
		dmic_build_channel_map(0, 0, PDM_CHAN_LEFT) |
		dmic_build_channel_map(1, 0, PDM_CHAN_RIGHT);
	cfg.streams[0].pcm_rate = MAX_SAMPLE_RATE;
	cfg.streams[0].block_size =
		BLOCK_SIZE(cfg.streams[0].pcm_rate, cfg.channel.req_num_chan);

	ret = dmic_configure(dmic_dev, &cfg);
	if (ret < 0) {
		return -2;
	}


	while(true){
 

			ret = do_pdm_transfer(dmic_dev, &cfg, 2 * BLOCK_COUNT); // ei classification happens in here
			if (ret < 0) 
            {
				return -3;
		    }
			k_sleep(K_SECONDS(1));
	}
}









/* Old static classification main*/
/*
int main() {
    // This is needed so that output of printf is output immediately without buffering
    setvbuf(stdout, NULL, _IONBF, 0);

#ifdef CONFIG_SOC_NRF5340_CPUAPP
    // Switch CPU core clock to 128 MHz
    nrfx_clock_divider_set(NRF_CLOCK_DOMAIN_HFCLK, NRF_CLOCK_HFCLK_DIV_1);
#endif

    printk("Edge Impulse standalone inferencing (Zephyr)\n");

    if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
        printk("The size of your 'features' array is not correct. Expected %d items, but had %u\n",
            EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
        return 1;
    }

    ei_impulse_result_t result = { 0 };

    while (1) {
        // the features are stored into flash, and we don't want to load everything into RAM
        signal_t features_signal;
        features_signal.total_length = sizeof(features) / sizeof(features[0]);
        features_signal.get_data = &raw_feature_get_data;

        // invoke the impulse
        EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, true);
        printk("run_classifier returned: %d\n", res);

        if (res != 0) return 1;

        printk("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
                result.timing.dsp, result.timing.classification, result.timing.anomaly);
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
        bool bb_found = result.bounding_boxes[0].value > 0;
        for (size_t ix = 0; ix < result.bounding_boxes_count; ix++) {
            auto bb = result.bounding_boxes[ix];
            if (bb.value == 0) {
                continue;
            }
            printk("    %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\n", bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
        }
        if (!bb_found) {
            printk("    No objects found\n");
        }
#else
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            printk("    %s: %.5f\n", result.classification[ix].label,
                                    result.classification[ix].value);
        }
#if EI_CLASSIFIER_HAS_ANOMALY == 1
        printk("    anomaly score: %.3f\n", result.anomaly);
#endif
#endif
        k_msleep(12000);
    }
}
*/