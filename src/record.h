/**
 *  @file   record.h
 *  @author Weston Nielson <wnielson@github>
 *
 */

#ifndef rfswitch_record_h
#define rfswitch_record_h

#define SAMPLE_RATE           (44100.0)
#define ZERO_PREAMBLE_THRESH  (SAMPLE_RATE/88)

#define PA_SAMPLE_TYPE        paFloat32
#define FRAMES_PER_BUFFER     (32)

// 1/0 threshold
#define SIGNAL_THRESH         (0.02)
#define ZERO_PREAMBLE_THRESH  (SAMPLE_RATE/88)
#define CODE_COUNT            (20)
#define MIN_CODE_LENGTH       (10)
#define MIN_BIT_LENGTH        (12)

int run_record(int argc, char **argv);

#endif
