rfswitch
========

Utility for controlling RF-based power sockets from the Raspberry Pi.

There are two modes of operation; `switch` and `record`.  The `record`
mode can be used to record and decode the messages sent via the RF remote.
The `switch` mode can be used to turn a socket on or off using the codes
captured from the `record` mode.

Capturing Control Codes
-----------------------

To capture the codes emitted by the remote control, you can use the `record`
functionality provided by `rfswitch`.  In order to use this function, you must
have PortAudio installed and have built `rfswitch` with PortAudio support.

To run `record` mode, simply type::

    $ ./rfswitch r

You will be prompted to select an input device which is connected to the RF
recevier.  After you select a device, press repeatedly (or hold down) the same
key until the entire code has been recorded.  The output will look something
like the folowing::

    Available input devices
      [1] Built-in Microph, input channels: 2
      [2] Built-in Input, input channels: 2
      [4] Soundflower (2ch), input channels: 2
      [5] Soundflower (16ch), input channels: 16
    Select input device: 2
    Listening on device 2 (Built-in Input)
    .....................
    Found code
      code:     0110100010000100
      hi-long:  74
      hi-short: 21
      lo-long:  84
      lo-short: 31
      timings:  476190,1904761,1678004,702947

    Done recording samples

The important lines here are ``code: 0110100010000100`` and
``timings: 476190,1904761,1678004,702947``.  The ``code`` line defines the
actual signal that the reciever on the selected socket will respond to and the
``timings`` line contains the duration of the pulses that make up the signal.

You will then need to create (or update) a configuration file that holds all
the codes.  Every code needs an ``ID`` and an ``on`` and ``off`` code.  The
format of the config file is::

    <id 1>
    <on code 1>,<short-hi>,<long-lo>,<long-hi>,<short-lo>,<delay>
    <off code 1>,<short-hi>,<long-lo>,<long-hi>,<short-lo>,<delay>
    <id 2>
    <on code 2>,<short-hi>,<long-lo>,<long-hi>,<short-lo>,<delay>
    <off code 2>,<short-hi>,<long-lo>,<long-hi>,<short-lo>,<delay>
    ...

The default config file is located at ``~/.rfswitch``.  If the file doesn't
exist, just create it:

    touch ~/.rfswitch
    nano ~/.rfswitch

Now we need to add the codes that were retreived from the ``rfswitch r``
command to the config file.  Each set of ``on`` and ``off`` codes need to have
unique IDs, which should be positive integers.  Using the above example, we can
add the following to ``~/.rfswitch``::

    1
    0110100010000100,476190,1904761,1678004,702947,10000000

We then need to capture the `off` code for the same switch.


Example Signal
---------------


Here's an example of a typical (but short) RF signal that might be used
to control a switch (e.g. turn switch 1 on):

       0         1         0       0        0 (delay)   code-bit
    ├────────┼───────────┼────────┼────────┼─────────┼
 hi ┌──┐     ┌──────┐    ┌──┐     ┌──┐
    │  │     │      │    │  │     │  │
    │  │     │      │    │  │     │  │
    │  │     │      │    │  │     │  │
 0 ─│──│─────│──────│────│──│─────│──│───────────────
    │  │     │      │    │  │     │  │
 lo ┘  └─────┘      └────┘  └─────┘  └───────────────
     2   5      6     3   2    5   2    5      8        length


In this example signal the lengths of the individual signals are:
    
    hi-long   6
    hi-short  2
    lo-long   5
    lo-short  3
    delay     8
