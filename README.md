# webTestbench
Expandable testbench platform for websites to measure parameters like:
1. system's power by utilizing [phidget](https://www.phidgets.com/docs/Phidget22) voltage sensor
2. system's temperature by using linux lm_sensors
3. user experience by using [y-cruncher](https://www.numberworld.org/y-cruncher/) and pi digit calculation as example
4. memory usage
5. cpu utilization
6. web traffic by using [chrome-har-capturer](https://github.com/cyrus-and/chrome-har-capturer)

## How to install
`make install`

## How to use
Usage: ruby runner.rb [options] <SiteList><br>
    <p>-t, --time [TIME]                Time (sec) to wait for each probe [default:30 min]</p>
    <p>-h, --help                       Show help message</p>

## How to demo
`make demo` will probe for 10 minutes every landing page of the Alexa top 15 list.
