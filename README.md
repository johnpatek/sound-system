# Sound System

Bluetooth client and RTSP server used to upgrade my home entertainment system.

## Basic Design

The basic flow of both system can be described in a few stages. Many of the design
decisions came down to 2 factors:

1. A lot of the existing system was sufficient for use with modern tech.
2. Some of the components could not be replaced without tearing the house apart.

Note: Some of the hardware details are left out to avoid overcomplicating the logic.

### Before (circa 2006)

This was a very high end analog system for the mid 2000s. The controls were mounted in
the wall and the speakers were built into the ceiling. There were actually 4 inputs and
4 outputs so the system is connected to 4 different rooms, but the diagram below only
describes one room for the sake of simplicity.

```
*************                        ****************                           ***********
* Input and *                        *    Audio     *                           *         *
*  Volume   *========Cat 5e=========>* Distribution *=======Speaker Wire=======>* Speaker *
* Controls  *                        *    System    *                           *         *
*************                        ****************                           ***********
    (1)               (2)                  (3)                   (4)                (5)
```

1. Input is selected and volume is adjusted using buttons on a wall mounted panel.
2. Button signals are transmitted from the controller board over ethernet. The specifics 
of this control protocol are not documented.
3. Audio Distribution System will receive control signals and adjust the input and volume
accordingly. Input storage is read and decoded.
4. Speaker Wire carries decoded audio from Audio Distribution System.
5. 8 ohm Speaker receive and play signal from Speaker Wire.

### After

The only major changes to the hardware occur at (1) and (3). The wall controls were replaced 
with a Rasberry Pi. The Audio Distribution System was replaced with a small computer running 
Ubuntu Server that was equipped with 4 USB sound cards, 1 for each zone. The connection from
the sound card to the speaker wire is not included as it is entirely hardware based and not
relevant to the overall logic. Logically, another difference is now the input starts at (1),
whereas before all input sources were connected directly to (3). As a result, (2) is now also
used for transmitting encoded audio data. One could argue the etherner was wasted before, as 
each zone has its own cable that was only used for sending user control signals.

```
*************                        ******************                          ***********
* A2DP Sink *                        *  RTSP Server   *                          *         *
* and RTSP  *========Cat 5e=========>* and PulseAudio *=======Speaker Wire======>* Speaker *
* Client    *                        *      Sink      *                          *         *
*************                        ******************                          ***********
    (1)               (2)                   (3)                   (4)                (5)
```

1. A2DP sink receives audio from a bluetooth device, which is then encoded in an RTP packet
and prepared for transmission by the RTSP client.
2. The data and control signals between the RTSP Client and RTSP server are transmitted over
ethernet.
3. The RTSP Server is connected to a pipeline with a PulseAudio Sink for each output. The
pipeline is responsible for decoding the audio data from each RTP packet.
4. The speaker wire carries the data from the output associated with the PulseAudio Sink.
5. 8 ohm Speaker receive and play signal from Speaker Wire.
