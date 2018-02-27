all: .unzip power run

power:
	make -C tests/power

run:
	ruby runner.rb -t 600 ../siteList.csv #run for 10 min

install: .unzip
	sudo apt-get install python-dev python-pip python-matplotlib python-tk google-chrome-stable nodejs
	sudo -H pip install --upgrade pip
	sudo pip install setuptools psutil psrecord
	sudo npm install -g chrome-har-capturer

.unzip:
	make -C tests/interference
