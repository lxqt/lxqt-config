export KSCREEN_BACKEND=Fake
export KSCREEN_BACKEND_ARGS=TEST_DATA=/home/lucas/prog/lxqt/libkscreen/autotests/configs/multipleoutput.json
killall kscreen_backend_launcher
#`find /usr -name 'kscreen_backend_launcher'` 
lxqt-config-monitor

