# glidercoll

This is a program to search random glider collision configurations, written by Simon Ekstrom, and modified to extend its functionalities.

The functionalities of this program is described here: https://conwaylife.com/forums/viewtopic.php?p=157840#p157840

## Usage 

To use this program, clone this repository using:
`git clone https://github.com/carson-cheng/glidercoll`
`cd glidercoll`

Then, run search.py;
`python3 search.py`

Note that this program needs to be run with two arguments:
`-g: number of gliders (only values between 4 and 6 are supported)`
`-c: number of searchers`

Therefore, you need to run the program like this:
`python3 search.py -g 6 -c 6`

Search.py will compile the suitable program, and then run it, writing the results to the `results` directory. Note that search.py will run the program indefinitely, so if you want to terminate the program, run

`killall -9 coll_gen_*`

After some time running the program, you might want to parse the results. You will need to run `parse.py`, which sifts through the `results` directory and reports interesting results to Catagolue (https://catagolue.hatsya.com).

*Note*: Instead of supplying arguments like those in `search.py`, you provide user input in `parse.py`. The interface should be intuitive, with the first user input specifying the number of searchers and the second one asking for the number of gliders for the search.

## Dependencies

To use this program, you need to install these packages:

1. gcc and python3
2. lifelib (`pip3 install python-lifelib`)
3. shinjuku (`git clone https://gitlab.com/parclytaxel/Shinjuku`)

Note that, for the shinjuku package, you need to move the shinjuku folder `shinjuku/shinjuku` to your `site-packages` directory where all of your Python modules are located.

For those who are unable to install the shinjuku package, you must run `parse_noshinjuku.py` instead of `parse.py`, which comments out the lines that depends on this package to run.

## Improvements

Last but not least, there are some improvements that can be made to this program.

1. Call `parse.py` automatically within `search.py` so that the task doesn't need to be done manually
