#include <csignal>
#include <glib.h>
#include "Zoom.h"


/**
 *  Callback fired atexit()
 */
void onExit() {
    auto* zoom = &Zoom::getInstance();
    zoom->leave();
    zoom->clean();

    cout << "exiting..." << endl;
}

/**
 * Callback fired when a signal is trapped
 * @param signal type of signal
 */
void onSignal(int signal) {
    onExit();
    _Exit(signal);
}


/**
 * Callback for glib event loop
 * @param data event data
 * @return always TRUE
 */
gboolean onTimeout (gpointer data) {
    return TRUE;
}

/**
 * Run the Zoom Meeting Bot
 * @param argc argument count
 * @param argv argument vector
 * @return SDKError
 */
SDKError run(int argc, char** argv) {
    SDKError err{SDKERR_SUCCESS};

    auto* zoom = &Zoom::getInstance();

    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);

    atexit(onExit);

    std::string botName = "Bot Name";
    std::string clientId = "Your Client ID";
    std::string clientSecret = "Your Client Secret";
    std::string joinUrl = "https://us05web.zoom.us/j/85696810271?pwd=pwd";

    err = zoom->config("-n='" + botName + "' --client-id='" + clientId + "' --client-secret='" + clientSecret + "' --join-url='" + joinUrl + "' RawAudio --separate-participants");
    if (Zoom::hasError(err, "configure"))
        return err;

    // initialize the Zoom SDK

    Zoom::RawAudioRecordingCallback audioCallback = [](AudioRawData* data, uint32_t node_id) {
        std::cout << "Received AudioRawData with node ID: " << node_id << " " << data->GetBufferLen() << "b " << " at " << data->GetSampleRate() << "Hz" << data->GetChannelNum() << "ch" << std::endl;
        // Process the AudioRawData as needed
    };
    err = zoom->init(audioCallback);
    if(Zoom::hasError(err, "initialize"))
        return err;

    // authorize with the Zoom SDK
    err = zoom->auth();
    if (Zoom::hasError(err, "authorize"))
        return err;

    return err;
}

int main(int argc, char **argv) {
    // Run the Meeting Bot
    SDKError err = run(argc, argv);

    if (Zoom::hasError(err)) 
        return err;

    // Use an event loop to receive callbacks
    GMainLoop* eventLoop;
    eventLoop = g_main_loop_new(NULL, FALSE);
    g_timeout_add(100, onTimeout, eventLoop);
    g_main_loop_run(eventLoop);

    return err;
}



