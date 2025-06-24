# UnMovie
A set of command-line tools for backporting assets from The SpongeBob SquarePants Movie to Battle for Bikini Bottom.

## UnSCRP

UnSCRP converts a Script asset to a Group asset and one Timer asset per timed link.

Usage:
```
UnSCRP input_scrp output_name
```

where output_name is the desired prefix for the Group and Timer asset names (e.g. an output_name of "MY_SCRIPT" will generate a Group named "MY_SCRIPT_GROUP" and Timers named "MY_SCRIPT_TIMER_xx").

Notes:
* Each generated asset will be saved as its own file in the "output" folder. You can import these into your level in Industrial Park using Edit > Import Multiple Assets. I recommend deleting these files once you're done with them.
* Links in the Script asset will be copied 1:1 without changing asset IDs, so if any of these links targeted the Script, you may want to update those targets to the Group.
* Any other links (e.g. Run, Reset, Stop) that would have been used for a Script can target the Group instead.
