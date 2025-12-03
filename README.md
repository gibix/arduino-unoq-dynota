# Arduino Uno Q Dynamic Sketch Loader

Load and update Arduino sketches dynamically without stopping your running
system.

## What it does

Update parts of your sketch at runtime while other components keep running. No
need to stop and restart the entire Arduino.

## Credits

Forked from [kartben/zephyr_unoq_demo](https://github.com/kartben/zephyr_unoq_demo)

## How it works

1. Push code to `main` branch
2. GitHub CI compiles the sketch
3. Webhook triggers deployment
4. Code loads into Arduino via WebSocket bridge
5. Arduino hot-swaps the module using LLEXT

## Components

- `sketch_loader/` - Arduino sketch with LLEXT loader
- `bridge_loader/` - Python WebSocket bridge for RPC
- `.github/` - CI workflow for compilation

## TODO

- [ ] add gitlab ci for zephyr build
- [ ] document build of zephyr core
- [ ] verify upload asset with secure components
