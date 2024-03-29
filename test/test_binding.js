const ObjectWrapDemo = require("../lib/binding.js");
const assert = require("assert");
const fs = require('fs');

assert(ObjectWrapDemo, "The expected module is undefined");

const { fork } = require('child_process');

async function testBasic()
{
    const clientId = "Your Client ID";
    const clientSecret = "Your Client Secret";
    const joinUrl = "https://us05web.zoom.us/j/95167681525?pwd=dEROM1lXbVZjTGM1WTZpTlJYWGV1UT09";

    const getArgs = (botName) => Object.entries({
        "-n": botName,
        "--client-id": clientId,
        "--client-secret": clientSecret,
        "--join-url": joinUrl,
    }).map(([key, value]) => `${key}="${value}"`).join(' ')

    // for (let i = 0; i < 3; i++){
        const i = 0
        const childProcess = fork('test/child.js');
        childProcess.on('message', message => {
            if (message.event === 'audio') {
                // Assuming message.args.buffer is the audio data
                const audioData = message.args.buffer;
                console.log(new Date().getTime(), 'Message from child process:', audioData);
                
                // Write the audio data to a file
                fs.writeFileSync('audio.wav', audioData);

       
            }
        });
        childProcess.send({ command: 'init', args: getArgs(`Bot #${i + 1}`) });
    // }
}

assert.doesNotThrow(testBasic, undefined, "testBasic threw an expection");

console.log("Tests passed- everything looks OK!");