
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <pocketsphinx/pocketsphinx.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/err.h>

const char * recognize_from_microphone();

ps_decoder_t* ps;                  // create pocketsphinx decoder structure
cmd_ln_t* config;                  // create configuration structure
ad_rec_t* ad;                      // create audio recording structure - for use with ALSA functions

int16 adbuf[4096];                 // buffer array to hold audio data
uint8 utt_started, in_speech;      // flags for tracking active speech - has speech started? - is speech currently happening?
int32 k;                           // holds the number of frames in the audio buffer
char const* hyp;                   // pointer to "hypothesis" (best guess at the decoded result)
char const* decoded_speech;

int main(int argc, char** argv)
{
    ros::init(argc, argv, "speech_recognizer");
    ros::NodeHandle n;
    ros::Publisher chatter_pub = n.advertise<std_msgs::String>("speech_input", 1000);
    ros::Rate loop_rate(10);

    config = cmd_ln_init(NULL, ps_args(), TRUE,                   		// Load the configuration structure - ps_args() passes the default values
		"-hmm", "/usr/share/pocketsphinx/model/en-us/en-us",  				// path to the standard english language model
		"-lm", "/usr/share/pocketsphinx/model/en-us/en-us.lm.bin",                          // custom language model (file must be present)
		"-dict", "/usr/share/pocketsphinx/model/en-us/cmudict-en-us.dict",                  // custom dictionary (file must be present)
		"-logfn", "/dev/null",                                      	// suppress log info from being sent to screen
        "-remove_noise","yes",
		NULL);
	if (config == NULL)
	{
		fprintf(stderr, "Failed to create config object, see log for details\n");
		return -1;
    	}

	// initialize the pocketsphinx decoder
	ps = ps_init(config);
	if (ps == NULL)
	{
		fprintf(stderr, "Failed to create recognizer, see log for details\n");
		return -1;
	}

	// open default microphone at default samplerate
	ad = ad_open_dev(NULL, (int) cmd_ln_float32_r(config, "-samprate"));
	if (ad == NULL)
	{
		fprintf(stderr, "Failed to create the audio input device\n");
		return -1;
	}

	while(ros::ok())
	{
        std_msgs::String msg;
		msg.data = recognize_from_microphone();	    // call the function to capture and decode speech
        ROS_INFO("%s", msg.data.c_str());
        chatter_pub.publish(msg);
        ros::spinOnce();
        loop_rate.sleep();
   	}

	// Cleanup	
	ad_close(ad);
    ps_free(ps);
    cmd_ln_free_r(config);

	return 0;
}

const char * recognize_from_microphone()
{
    ad_start_rec(ad);                                // start recording
    ps_start_utt(ps);                                // mark the start of the utterance
    utt_started = FALSE;                             // clear the utt_started flag

    while(1) {                                       
        k = ad_read(ad, adbuf, 4096);                // capture the number of frames in the audio buffer
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);  // send the audio buffer to the pocketsphinx decoder

        in_speech = ps_get_in_speech(ps);            // test to see if speech is being detected

        if (in_speech && !utt_started) {             // if speech has started and utt_started flag is false                           
            utt_started = TRUE;                      // then set the flag
        }
 
        if (!in_speech && utt_started) {             // if speech has ended and the utt_started flag is true
            ps_end_utt(ps);                          // then mark the end of the utterance
            ad_stop_rec(ad);                         // stop recording
            hyp = ps_get_hyp(ps, NULL );             // query pocketsphinx for "hypothesis" of decoded statement
            return hyp;                              // the function returns the hypothesis
            break;                                   // exit the while loop and return to main
        }
    }
}
